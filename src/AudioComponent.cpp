/* ========================================================
 * @file    AudioComponent.cpp
 * @author  [Your Name]
 * @date    04 NOV 2025
 *
 * @brief   Implementation of AudioComponent
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

void AudioComponent::PlaySound(Entity eID) {
    if (soundID < 0) return;  // No sound assigned

    // Stop any currently playing sound
    StopSound();

    AudioAsset* asset = AudioCache::GetAssetByID(soundID);
    if (!asset) {
        // Silent fail - asset not found
        return;
    }

    // Ensure sound is loaded
    FMOD::Sound* sound = AudioCache::GetSound(asset->name);
    if (!sound) {
        // Silent fail - couldn't load
        return;
    }

    // Play spatially or normally
    if (spatial && Registry::Get().Has_Component<TransformComponent>(eID)) {
        auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
        channel = AudioSystem::PlaySoundAt(asset->name, transform.pos.x, transform.pos.y, volume);
    }
    else {
        channel = AudioSystem::PlaySoundGetChannel(sound, false);
        if (channel) {
            channel->setVolume(volume);
        }
    }

    isPlaying = (channel != nullptr);
}

void AudioComponent::PlayLoopingSound(Entity eID) {
    if (loopingSoundID < 0) return;  // No looping sound assigned

    // Stop any currently playing looping sound
    StopLoopingSound();

    AudioAsset* asset = AudioCache::GetAssetByID(loopingSoundID);
    if (!asset) {
        return;
    }

    // Ensure sound is loaded
    FMOD::Sound* sound = AudioCache::GetSound(asset->name);
    if (!sound) {
        return;
    }

    // Play spatially or normally
    if (spatial && Registry::Get().Has_Component<TransformComponent>(eID)) {
        auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
        loopingChannel = AudioSystem::PlaySoundAt(asset->name, transform.pos.x, transform.pos.y, volume);
    }
    else {
        loopingChannel = AudioSystem::PlaySoundGetChannel(sound, false);
        if (loopingChannel) {
            loopingChannel->setVolume(volume);
        }
    }

    isLoopingPlaying = (loopingChannel != nullptr);
}

void AudioComponent::StopSound() {
    if (channel) {
        channel->stop();
        channel = nullptr;
    }
    isPlaying = false;
}

void AudioComponent::StopLoopingSound() {
    if (loopingChannel) {
        loopingChannel->stop();
        loopingChannel = nullptr;
    }
    isLoopingPlaying = false;
}

void AudioComponent::UpdateSpatialPosition(Entity eID) {
    if (!spatial) return;

    if (!Registry::Get().Has_Component<TransformComponent>(eID)) return;

    auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
    FMOD_VECTOR pos = { transform.pos.x, transform.pos.y, 0.0f };
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

    // Update main sound position
    if (channel && isPlaying) {
        channel->set3DAttributes(&pos, &vel);
    }

    // Update looping sound position
    if (loopingChannel && isLoopingPlaying) {
        loopingChannel->set3DAttributes(&pos, &vel);
    }
}

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

Editor::ComponentData AudioComponent::EditorData() {
    Editor::ComponentData data("Audio");

    // ========== Sound ID Dropdown ==========
    Editor::Widget_Dropdown soundDropdown;
    soundDropdown.ref = reinterpret_cast<unsigned int*>(&soundID);
    soundDropdown.Add_Option("None", static_cast<unsigned int>(-1));

    // Populate with all available audio assets
    for (const auto& [name, asset] : AudioCache::GetAllAssets()) {
        std::string displayName = name + " [" + asset.group + "]";
        soundDropdown.Add_Option(displayName, static_cast<unsigned int>(asset.id));
    }
    data.Add_Data("Sound Asset", soundDropdown);

    // ========== Looping Sound ID Dropdown ==========
    Editor::Widget_Dropdown loopingDropdown;
    loopingDropdown.ref = reinterpret_cast<unsigned int*>(&loopingSoundID);
    loopingDropdown.Add_Option("None", static_cast<unsigned int>(-1));

    // Only show looping sounds
    for (const auto& [name, asset] : AudioCache::GetAllAssets()) {
        if (asset.loop) {
            std::string displayName = name + " [" + asset.group + "]";
            loopingDropdown.Add_Option(displayName, static_cast<unsigned int>(asset.id));
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

    // ========== Spatial Distance Settings (only if spatial enabled) ==========
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

void Update_AudioComponents(float dt) {
    (void)dt;  // Unused parameter

    Registry& registry = Registry::Get();

    // Update all AudioComponents
    for (auto& [entity, audio] : registry.Get_ComponentMap<AudioComponent>()) {
        // Update playback state (check if sounds finished)
        audio.UpdatePlaybackState();

        // Update spatial positioning if enabled
        if (audio.spatial) {
            audio.UpdateSpatialPosition(entity);
        }
    }
}