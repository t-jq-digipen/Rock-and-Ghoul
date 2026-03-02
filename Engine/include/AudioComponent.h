/* ========================================================
 * @file    AudioComponent.h
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    4 NOV 2025
 *
 * @brief   Header file for entity-based audio component system
 *
 * This file defines the AudioComponent structure, which provides
 * audio playback capabilities for entities in the ECS architecture.
 * The component supports both one-shot sound effects and looping
 * ambient audio, with optional audio spatial positioning based
 * on entity transform data. It includes full serialization support
 * for scene saving/loading and editor integration for visual
 * configuration of audio properties. The component automatically
 * manages FMOD channel lifecycles, spatial audio updates, and
 * playback state tracking. Auto-play functionality allows sounds
 * to trigger on entity spawn, and volume/distance parameters
 * provide fine-grained control over audio behavior.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <fmod.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "Entity.h"
#include "AudioCache.h"
#include "DataDefs.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct AudioComponent {
    // Vector of sound names (indices map to specific sounds)
    std::vector<std::string> sounds;

    // Active channels for each sound index
    std::vector<FMOD::Channel*> channels;

    // Playback state tracking
    std::vector<bool> isPlaying;

    // Playback settings
    bool spatial = false;
    float volume = 1.0f;

    // Spatial audio settings
    float minDistance = 1.0f;
    float maxDistance = 100.0f;

    /**
     * @brief Default constructor
     */
    AudioComponent() = default;

    /**
     * @brief Play sound at specific index
     * @param index Index in sounds vector
     * @param eID Entity ID (for spatial positioning)
     */
    void PlaySound(size_t index, Entity eID);

    /**
     * @brief Stop sound at specific index
     * @param index Index in sounds vector
     */
    void StopSound(size_t index);

    /**
     * @brief Add a new sound to the component
     * @param soundName Name of sound from AudioCache
     * @return Index where sound was added
     */
    size_t AddSound(const std::string& soundName);

    /**
     * @brief Remove sound at index
     * @param index Index to remove
     */
    void RemoveSound(size_t index);

    /**
     * @brief Update spatial audio position for all active sounds
     * @param eID Entity ID to get position from
     */
    void UpdateSpatialPosition(Entity eID);

    /**
     * @brief Update playback state for all channels
     */
    void UpdatePlaybackState();

    /**
     * @brief Set volume for all channels
     * @param vol Volume level (0.0 - 1.0)
     */
    void SetVolume(float vol);

    // Serialization
    std::string Serialize() const {
        json j = {
            {"sounds", sounds},
            {"spatial", spatial},
            {"volume", volume},
            {"minDistance", minDistance},
            {"maxDistance", maxDistance}
        };
        return j.dump();
    }

    static AudioComponent DeSerialize(Entity eID, const std::string& data) {
        (void)eID;

        if (data.empty()) {
            return AudioComponent();
        }

        json j = json::parse(data);
        AudioComponent audio;

        audio.sounds = j.value("sounds", std::vector<std::string>{});
        audio.spatial = j.value("spatial", false);
        audio.volume = j.value("volume", 1.0f);
        audio.minDistance = j.value("minDistance", 1.0f);
        audio.maxDistance = j.value("maxDistance", 100.0f);

        // Initialize channels and playback state vectors
        audio.channels.resize(audio.sounds.size(), nullptr);
        audio.isPlaying.resize(audio.sounds.size(), false);

        return audio;
    }

    Editor::ComponentData EditorData();
};

/**
 * @brief Updates all AudioComponents in the registry
 * @param dt Delta time (unused, but follows system pattern)
 */
void Update_AudioComponents(float dt);