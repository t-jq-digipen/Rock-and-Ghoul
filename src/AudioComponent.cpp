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

// ============================================================================
// AUDIOCOMPONENT MEMBER FUNCTIONS
// ============================================================================

/**
* @brief Play the main sound effect
* @param eID Entity ID (for spatial positioning)
*/
void AudioComponent::PlaySound(Entity eID) {
    if (soundName.empty()) return;  // No sound assigned

    // Stop current sound
    StopSound();

    // Get sound from cache
    FMOD::Sound* sound = AudioCache::GetSound(soundName);
    if (!sound) {
        std::cerr << "[AudioComponent] Failed to load sound: " << soundName << std::endl;
        return;
    }

    // Play with spatial audio or normal
    if (spatial && Registry::Get().Has_Component<TransformComponent>(eID)) {
        auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
        channel = AudioSystem::PlaySoundAt(soundName, transform.pos.x, transform.pos.y, volume);
    }
    else {
        channel = AudioSystem::PlaySoundGetChannel(sound, false);
        if (channel) {
            channel->setVolume(volume);
        }
    }

    isPlaying = (channel != nullptr);
}

/**
 * @brief Play the looping sound
 * @param eID Entity ID (for spatial positioning)
 */
void AudioComponent::PlayLoopingSound(Entity eID) {
    if (loopingSoundName.empty()) return;  // No looping sound assigned

    // Stop current looping sound
    StopLoopingSound();

    // Get sound from cache
    FMOD::Sound* sound = AudioCache::GetSound(loopingSoundName);
    if (!sound) {
        std::cerr << "[AudioComponent] Failed to load looping sound: " << loopingSoundName << std::endl;
        return;
    }

    // Play with spatial audio or normal
    if (spatial && Registry::Get().Has_Component<TransformComponent>(eID)) {
        auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
        loopingChannel = AudioSystem::PlaySoundAt(loopingSoundName, transform.pos.x, transform.pos.y, volume);
    }
    else {
        loopingChannel = AudioSystem::PlaySoundGetChannel(sound, false);
        if (loopingChannel) {
            loopingChannel->setVolume(volume);
        }
    }

    isLoopingPlaying = (loopingChannel != nullptr);
}
/**
 * @brief Stop the main sound
 */
void AudioComponent::StopSound() {
    if (channel) {
        channel->stop();
        channel = nullptr;
    }
    isPlaying = false;
}

/**
 * @brief Stop the looping sound
 */
void AudioComponent::StopLoopingSound() {
    if (loopingChannel) {
        loopingChannel->stop();
        loopingChannel = nullptr;
    }
    isLoopingPlaying = false;
}
/**
 * @brief Update spatial audio position (called per frame)
 * @param eID Entity ID to get position from
 */
void AudioComponent::UpdateSpatialPosition(Entity eID) {
    if (!spatial) return;

    if (!Registry::Get().Has_Component<TransformComponent>(eID)) return;

    auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
    FMOD_VECTOR pos = { transform.pos.x, transform.pos.y, 0.0f };
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

    // Update main sound position
    if (channel && isPlaying) {
        channel->set3DAttributes(&pos, &vel);
        channel->set3DMinMaxDistance(minDistance, maxDistance);
    }

    // Update looping sound position
    if (loopingChannel && isLoopingPlaying) {
        loopingChannel->set3DAttributes(&pos, &vel);
        loopingChannel->set3DMinMaxDistance(minDistance, maxDistance);
    }
}

/**
 * @brief Update playback state (checks if sounds finished)
 */
void AudioComponent::UpdatePlaybackState() {
    // Check main sound
    if (channel) {
        bool playing = false;
        channel->isPlaying(&playing);
        isPlaying = playing;

        if (!playing) {
            channel = nullptr;  // Clean up finished channel
        }
    }

    // Check looping sound
    if (loopingChannel) {
        bool playing = false;
        loopingChannel->isPlaying(&playing);
        isLoopingPlaying = playing;

        if (!playing) {
            loopingChannel = nullptr;  // Clean up finished channel
        }
    }
}

/**
 * @brief Set volume for all channels
 * @param vol Volume level (0.0 - 1.0)
 */
void AudioComponent::SetVolume(float vol) {
    volume = vol;

    if (channel) {
        channel->setVolume(volume);
    }

    if (loopingChannel) {
        loopingChannel->setVolume(volume);
    }
}

// ============================================================================
// EDITOR INTEGRATION
// ============================================================================

/**
 * @brief Get editor-compatible data for this component.
 *
 * This function creates and configures editor widgets for the AudioComponent,
 * including dropdown menus for sound selection, checkboxes for auto-play
 * settings, spatial audio toggles, volume sliders, and distance parameters
 * for 3D spatial positioning. The widgets are populated with available
 * audio assets from the AudioCache.
 *
 * @return Editor::ComponentData Object containing all configured editor widgets
 *         for this component.
 */
Editor::ComponentData AudioComponent::EditorData() {
    Editor::ComponentData data("Audio");

    // ========== Sound Name Dropdown ==========
    Editor::Widget_Dropdown soundDropdown;
    // Store selected name as a string index (editor handles this)
    soundDropdown.ref = reinterpret_cast<unsigned int*>(&soundName);
    soundDropdown.Add_Option("None", 0);

    // Populate with all available audio assets
    unsigned int index = 1;
    for (const auto& [name, asset] : AudioCache::GetAllAssets()) {
        std::string displayName = name + " [" + asset.group + "]";
        soundDropdown.Add_Option(displayName, index++);
        // Store mapping from index to name (you may need to handle this in editor code)
    }
    data.Add_Data("Sound Asset", soundDropdown);

    // ========== Looping Sound Name Dropdown ==========
    Editor::Widget_Dropdown loopingDropdown;
    loopingDropdown.ref = reinterpret_cast<unsigned int*>(&loopingSoundName);
    loopingDropdown.Add_Option("None", 0);

    // Only show looping sounds
    index = 1;
    for (const auto& [name, asset] : AudioCache::GetAllAssets()) {
        if (asset.loop) {
            std::string displayName = name + " [" + asset.group + "]";
            loopingDropdown.Add_Option(displayName, index++);
        }
    }
    data.Add_Data("Looping Sound", loopingDropdown);

    // ========== Auto-Play Checkboxes ==========
    Editor::Widget_Checkmark autoPlayWidget;
    autoPlayWidget.ref = &autoPlay;
    data.Add_Data("Auto Play", autoPlayWidget);

    Editor::Widget_Checkmark autoPlayLoopingWidget;
    autoPlayLoopingWidget.ref = &autoPlayLooping;
    data.Add_Data("Auto Play Looping", autoPlayLoopingWidget);

    // ========== Spatial Audio Checkbox ==========
    Editor::Widget_Checkmark spatialWidget;
    spatialWidget.ref = &spatial;
    data.Add_Data("Spatial Audio", spatialWidget);

    // ========== Volume Slider ==========
    Editor::Widget_Float volumeWidget;
    volumeWidget.ref = &volume;
    volumeWidget.slider = true;
    volumeWidget.min = 0.0f;
    volumeWidget.max = 1.0f;
    data.Add_Data("Volume", volumeWidget);

    // ========== Spatial Distance Settings ==========
    if (spatial) {
        Editor::Widget_Float minDistWidget;
        minDistWidget.ref = &minDistance;
        minDistWidget.slider = false;
        minDistWidget.min = 0.1f;
        minDistWidget.max = 1000.0f;
        data.Add_Data("Min Distance", minDistWidget);

        Editor::Widget_Float maxDistWidget;
        maxDistWidget.ref = &maxDistance;
        maxDistWidget.slider = false;
        maxDistWidget.min = 1.0f;
        maxDistWidget.max = 10000.0f;
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
        // Auto-play sounds on first frame if requested
        if (audio.autoPlay && !audio.isPlaying && !audio.soundName.empty()) {
            audio.PlaySound(entity);
            audio.autoPlay = false;  // Only play once
        }

        if (audio.autoPlayLooping && !audio.isLoopingPlaying && !audio.loopingSoundName.empty()) {
            audio.PlayLoopingSound(entity);
            audio.autoPlayLooping = false;  // Only play once
        }

        // Check if sounds finished playing, update playback state
        audio.UpdatePlaybackState();

        // Update spatial positioning
        if (audio.spatial) {
            audio.UpdateSpatialPosition(entity);
        }
    }
}