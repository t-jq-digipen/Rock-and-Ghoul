/* ========================================================
 * @file    AudioComponent.cpp
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    4 NOV 2025
 *
 * @brief   Implementation file for entity-based audio component system
 *
 * This file contains the implementation of the AudioComponent member
 * functions and the global Update_AudioComponents system function.
 * It handles audio playback initialization, channel management,
 * spatial audio positioning updates, and playback state tracking.
 * The implementation integrates with the AudioCache for sound asset
 * loading by name, the AudioSystem for FMOD channel control, and
 * the Registry for entity component access. Editor integration is
 * provided through the EditorData() function, which creates UI
 * widgets for sound selection (dropdowns populated from AudioCache),
 * volume control (sliders), auto-play toggles (checkboxes), and
 * spatial audio configuration (distance parameters). The system
 * update function processes all AudioComponent instances each frame,
 * handling auto-play triggers, playback state updates, and spatial
 * position synchronization with entity transforms.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "AudioComponent.h"
#include "Registry.h"
#include "AudioCache.h"
#include "Audio.h"
#include "Component.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

void AudioComponent::PlaySound(size_t index, Entity eID) {
    // chceck exist? index
    if (index >= sounds.size()) {
        std::cerr << "[AudioComponent] Invalid sound index: " << index << std::endl;
        return;
    }

    if (sounds[index].empty()) {
        std::cerr << "[AudioComponent] No sound assigned at index: " << index << std::endl;
        return;
    }

    // Ensure vectors are properly sized
    if (channels.size() <= index) {
        channels.resize(sounds.size(), nullptr);
    }
    if (isPlaying.size() <= index) {
        isPlaying.resize(sounds.size(), false);
    }

    // Stop current sound at this index
    StopSound(index);

    // Get sound from cache
    FMOD::Sound* sound = AudioCache::GetSound(sounds[index]);
    if (!sound) {
        std::cerr << "[AudioComponent] Failed to load sound: " << sounds[index] << std::endl;
        return;
    }

    // Play with spatial audio or normal THIS DOESNT F WORD WORK
    if (spatial && Registry::Get().Has_Component<TransformComponent>(eID)) {
        auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
        channels[index] = AudioSystem::PlaySoundAt(sounds[index], transform.pos.x, transform.pos.y, volume);
    }
    else {
        channels[index] = AudioSystem::PlaySoundGetChannel(sound, false);
        if (channels[index]) {
            channels[index]->setVolume(volume);
        }
    }

    isPlaying[index] = (channels[index] != nullptr);
}

void AudioComponent::StopSound(size_t index) {
    if (index >= channels.size()) return;

    if (channels[index]) {
        channels[index]->stop();
        channels[index] = nullptr;
    }

    if (index < isPlaying.size()) {
        isPlaying[index] = false;
    }
}

size_t AudioComponent::AddSound(const std::string& soundName) {
    sounds.push_back(soundName);
    channels.push_back(nullptr);
    isPlaying.push_back(false);
    return sounds.size() - 1;
}

void AudioComponent::RemoveSound(size_t index) {
    if (index >= sounds.size()) return;

    // Stop the sound first
    StopSound(index);

    // Remove from all vectors
    sounds.erase(sounds.begin() + index);
    channels.erase(channels.begin() + index);
    isPlaying.erase(isPlaying.begin() + index);
}

void AudioComponent::UpdateSpatialPosition(Entity eID) {
    if (!spatial) return;
    if (!Registry::Get().Has_Component<TransformComponent>(eID)) return;

    auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
    FMOD_VECTOR pos = { transform.pos.x, transform.pos.y, 0.0f };
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

    // Update all active channels
    for (size_t i = 0; i < channels.size(); ++i) {
        if (channels[i] && isPlaying[i]) {
            channels[i]->set3DAttributes(&pos, &vel);
            channels[i]->set3DMinMaxDistance(minDistance, maxDistance);
        }
    }
}

void AudioComponent::UpdatePlaybackState() {
    for (size_t i = 0; i < channels.size(); ++i) {
        if (channels[i]) {
            bool playing = false;
            channels[i]->isPlaying(&playing);
            isPlaying[i] = playing;

            if (!playing) {
                channels[i] = nullptr;  // Clean up finished channel
            }
        }
    }
}

void AudioComponent::SetVolume(float vol) {
    volume = vol;

    for (auto& channel : channels) {
        if (channel) {
            channel->setVolume(volume);
        }
    }
}

Editor::ComponentData AudioComponent::EditorData() {
    Editor::ComponentData data("AudioComponent");

    // Display current sounds list
    for (size_t i = 0; i < sounds.size(); ++i) {
        std::string label = "Sound [" + std::to_string(i) + "]";

        Editor::Widget_Dropdown soundDropdown;
        soundDropdown.ref = reinterpret_cast<unsigned int*>(&sounds[i]);
        soundDropdown.Add_Option("None", 0);

        unsigned int idx = 1;
        for (const auto& [name, asset] : AudioCache::GetAllAssets()) {
            std::string displayName = name + " [" + asset.group + "]";
            soundDropdown.Add_Option(displayName, idx++);
        }
        data.Add_Data(label, soundDropdown);
    }

    // Spatial Audio Checkbox
    Editor::Widget_Checkmark spatialWidget;
    spatialWidget.ref = &spatial;
    data.Add_Data("Spatial Audio", spatialWidget);

    // Volume Slider
    Editor::Widget_Float volumeWidget;
    volumeWidget.ref = &volume;
    volumeWidget.slider = true;
    volumeWidget.min = 0.0f;
    volumeWidget.max = 1.0f;
    data.Add_Data("Volume", volumeWidget);

    // Spatial Distance Settings
    if (spatial) {
        Editor::Widget_Float minDistWidget;
        minDistWidget.ref = &minDistance;
        data.Add_Data("Min Distance", minDistWidget);

        Editor::Widget_Float maxDistWidget;
        maxDistWidget.ref = &maxDistance;
        data.Add_Data("Max Distance", maxDistWidget);
    }

    return data;
}

// ============================================================================
// SYSTEM UPDATE FUNCTION
// ============================================================================

/**
 * @brief Updates all AudioComponents in the registry
 * @param dt Delta time (unused, but follows system pattern)
 */
void Update_AudioComponents(float dt) {
    (void)dt;  // Unused param

    Registry& registry = Registry::Get();

    // Update all AudioComponents
    for (auto& [entity, audio] : registry.Get_ComponentMap<AudioComponent>()) {
        // Update playback state for all channels
        audio.UpdatePlaybackState();

        // Update spatial positioning if enabled
        if (audio.spatial) {
            audio.UpdateSpatialPosition(entity);
        }
    }
}