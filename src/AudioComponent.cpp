/* ========================================================
 * @file    AudioComponent.cpp
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    4 NOV 2025
 *
 * @brief   Implementation file for entity-based audio component system
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "AudioComponent.h"
#include "Registry.h"
#include "AudioCache.h"
#include "Audio.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

// Static helper for one-line calls
bool AudioComponent::Play(Entity eID, int index) {
    if (eID == INVALID_ENTITY) return false;

    Registry& registry = Registry::Get();
    if (registry.Has_Component<AudioComponent>(eID)) {
        auto& audioComp = registry.Get_Component<AudioComponent>(eID);
        return audioComp.PlaySound(eID, index);
    }
    return false;
}

// Instance method
bool AudioComponent::PlaySound(Entity eID, int index) {
    if (index < 0 || index >= static_cast<int>(soundFiles.size())) {
        std::cerr << "[AudioComponent] Invalid sound index: " << index << std::endl;
        return false;
    }

    const std::string& assetName = soundFiles[index];
    StopSound(index);  // Stop if already playing

    FMOD::Sound* sound = AudioCache::GetSound(assetName);
    if (!sound) {
        std::cerr << "[AudioComponent] Failed to load sound: " << assetName << std::endl;
        return false;
    }

    FMOD::Channel* channel = nullptr;

    if (spatial && Registry::Get().Has_Component<TransformComponent>(eID)) {
        auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
        channel = AudioSystem::PlaySoundAt(assetName, transform.pos.x, transform.pos.y, volume);
    }
    else {
        channel = AudioSystem::PlaySoundGetChannel(sound, false);
        if (channel) {
            channel->setVolume(volume);
        }
    }

    if (channel) {
        activeChannels[index] = channel;
        return true;
    }

    return false;
}

void AudioComponent::StopSound(int index) {
    auto it = activeChannels.find(index);
    if (it != activeChannels.end() && it->second) {
        it->second->stop();
        activeChannels.erase(it);
    }
}

bool AudioComponent::IsPlaying(int index) {
    auto it = activeChannels.find(index);
    if (it == activeChannels.end() || !it->second) {
        return false;
    }

    bool playing = false;
    it->second->isPlaying(&playing);
    return playing;
}

void AudioComponent::UpdatePlaybackStates(Entity eID) {
    std::vector<int> toRemove;

    for (auto& [index, channel] : activeChannels) {
        if (!channel) {
            toRemove.push_back(index);
            continue;
        }

        bool playing = false;
        channel->isPlaying(&playing);

        if (!playing) {
            toRemove.push_back(index);
        }
        else if (spatial) {
            UpdateChannelSpatialPosition(eID, channel);
        }
    }

    for (const auto& index : toRemove) {
        activeChannels.erase(index);
    }
}

// ============================================================================
// SERIALIZATION (Simple - only new format)
// ============================================================================

std::string AudioComponent::Serialize() const {
    json j;
    j["soundFiles"] = soundFiles;
    j["autoPlayIndex"] = autoPlayIndex;
    j["volume"] = volume;
    j["spatial"] = spatial;
    j["minDistance"] = minDistance;
    j["maxDistance"] = maxDistance;
    return j.dump();
}

AudioComponent AudioComponent::DeSerialize(Entity eID, const std::string& data) {
    (void)eID;

    if (data.empty()) return AudioComponent();

    try {
        json j = json::parse(data);
        AudioComponent audio;

        // ========== HANDLE BOTH FORMATS ==========

        // NEW FORMAT: soundFiles array
        if (j.contains("soundFiles") && j["soundFiles"].is_array()) {
            audio.soundFiles = j["soundFiles"].get<std::vector<std::string>>();
        }
        // OLD FORMAT: sounds map (convert to array for EntityCreation.cpp compatibility)
        else if (j.contains("sounds") && j["sounds"].is_object()) {
            // Convert map to array
            for (auto it = j["sounds"].begin(); it != j["sounds"].end(); ++it) {
                audio.soundFiles.push_back(it.value().get<std::string>());
            }
        }

        // ========== AUTO-PLAY ==========
        // NEW: autoPlayIndex
        if (j.contains("autoPlayIndex")) {
            audio.autoPlayIndex = j["autoPlayIndex"].get<unsigned int>();
        }
        // OLD: autoPlaySound
        else if (j.contains("autoPlaySound")) {
            std::string autoPlaySound = j["autoPlaySound"].get<std::string>();
            audio.autoPlayIndex = autoPlaySound.empty() ? 0 : 1;
        }

        // ========== COMMON SETTINGS ==========
        audio.volume = j.value("volume", 1.0f);
        audio.spatial = j.value("spatial", false);
        audio.minDistance = j.value("minDistance", 1.0f);
        audio.maxDistance = j.value("maxDistance", 100.0f);

        return audio;
    }
    catch (...) {
        return AudioComponent();
    }
}

// ============================================================================
// PRIVATE HELPER
// ============================================================================

void AudioComponent::UpdateChannelSpatialPosition(Entity eID, FMOD::Channel* channel) {
    if (!channel || !Registry::Get().Has_Component<TransformComponent>(eID)) return;

    auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
    FMOD_VECTOR pos = { transform.pos.x, transform.pos.y, 0.0f };
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

    channel->set3DAttributes(&pos, &vel);
    channel->set3DMinMaxDistance(minDistance, maxDistance);
}

// ============================================================================
// EDITOR INTEGRATION
// ============================================================================

Editor::ComponentData AudioComponent::EditorData() {
    Editor::ComponentData data("Audio");
    // Minimal implementation - just returns empty data
    return data;
}

// ============================================================================
// SYSTEM UPDATE FUNCTION
// ============================================================================

void Update_AudioComponents(float dt) {
    (void)dt; // Unused parameter

    Registry& registry = Registry::Get();

    for (auto& [entity, audio] : registry.Get_ComponentMap<AudioComponent>()) {
        // Handle auto-play
        if (audio.autoPlayIndex > 0) {
            int playIndex = static_cast<int>(audio.autoPlayIndex) - 1;
            if (playIndex >= 0 && playIndex < static_cast<int>(audio.soundFiles.size()) &&
                !audio.IsPlaying(playIndex)) {
                audio.PlaySound(entity, playIndex);
                audio.autoPlayIndex = 0; // Play once
            }
        }

        audio.UpdatePlaybackStates(entity);
    }
}