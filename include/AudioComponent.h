/* ========================================================
 * @file    AudioComponent.h
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    4 NOV 2025
 *
 * @brief   Audio component for entity-based audio playback
 *
 * This component allows entities to have associated audio
 * that can be serialized, edited, and automatically updated.
 * Supports both 2D and spatial 3D audio positioning.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <fmod.hpp>
#include <string>
#include "Entity.h"
#include "DataDefs.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @struct AudioComponent
 * @brief Component for attaching audio to entities with serialization support.
 */
struct AudioComponent {
    // Audio asset references (by ID from AudioCache)
    int soundID = -1;           // Single sound effect ID (-1 = none)
    int loopingSoundID = -1;    // Looping sound ID for ambient audio (-1 = none)

    // Playback state
    FMOD::Channel* channel = nullptr;
    FMOD::Channel* loopingChannel = nullptr;
    bool isPlaying = false;
    bool isLoopingPlaying = false;

    // Playback settings
    bool autoPlay = false;      // Play sound on entity spawn
    bool autoPlayLooping = false; // Play looping sound on spawn
    bool spatial = false;       // Use 3D spatial positioning

    // Volume control
    float volume = 1.0f;        // Master volume (0.0 - 1.0)

    // Spatial audio settings (only used if spatial = true)
    float minDistance = 1.0f;   // Distance at which sound is at full volume
    float maxDistance = 100.0f; // Distance at which sound is silent

    /**
     * @brief Default constructor
     */
    AudioComponent() = default;

    /**
     * @brief Constructor with sound ID
     * @param id Audio asset ID from AudioCache
     * @param autoplay Whether to play immediately on spawn
     * @param vol Volume level (0.0 - 1.0)
     */
    AudioComponent(int id, bool autoplay = false, float vol = 1.0f)
        : soundID(id), autoPlay(autoplay), volume(vol) {}

    /**
     * @brief Play the main sound effect
     * @param eID Entity ID (for spatial positioning)
     */
    void PlaySound(Entity eID);

    /**
     * @brief Play the looping sound
     * @param eID Entity ID (for spatial positioning)
     */
    void PlayLoopingSound(Entity eID);

    /**
     * @brief Stop the main sound
     */
    void StopSound();

    /**
     * @brief Stop the looping sound
     */
    void StopLoopingSound();

    /**
     * @brief Update spatial audio position (called per frame)
     * @param eID Entity ID to get position from
     */
    void UpdateSpatialPosition(Entity eID);

    /**
     * @brief Update playback state (checks if sounds finished)
     */
    void UpdatePlaybackState();

    /**
     * @brief Set volume for all channels
     * @param vol Volume level (0.0 - 1.0)
     */
    void SetVolume(float vol);

    // ============================================================================
    // REQUIRED ECS SERIALIZATION METHODS
    // ============================================================================

    /**
     * @brief Serialize component to JSON string
     * @return JSON string representation of component
     */
    std::string Serialize() const {
        json j = {
            {"soundID", soundID},
            {"loopingSoundID", loopingSoundID},
            {"autoPlay", autoPlay},
            {"autoPlayLooping", autoPlayLooping},
            {"spatial", spatial},
            {"volume", volume},
            {"minDistance", minDistance},
            {"maxDistance", maxDistance}
        };
        return j.dump();
    }

    /**
     * @brief Deserialize component from JSON string
     * @param eID Entity ID (unused but required by pattern)
     * @param data JSON string containing component data
     * @return Deserialized AudioComponent instance
     */
    static AudioComponent DeSerialize(Entity eID, const std::string& data) {
        (void)eID;  // Unused parameter

        if (data.empty()) {
            return AudioComponent();
        }

        json j = json::parse(data);

        AudioComponent audio;
        audio.soundID = j.value("soundID", -1);
        audio.loopingSoundID = j.value("loopingSoundID", -1);
        audio.autoPlay = j.value("autoPlay", false);
        audio.autoPlayLooping = j.value("autoPlayLooping", false);
        audio.spatial = j.value("spatial", false);
        audio.volume = j.value("volume", 1.0f);
        audio.minDistance = j.value("minDistance", 1.0f);
        audio.maxDistance = j.value("maxDistance", 100.0f);

        return audio;
    }

    /**
     * @brief Get editor-compatible data for this component
     * @return ComponentData with editor widgets
     */
    Editor::ComponentData EditorData();
};

// ============================================================================
// SYSTEM UPDATE FUNCTION (Called by AudioManager)
// ============================================================================

/**
 * @brief Updates all AudioComponents in the registry
 * @param dt Delta time (unused, but follows system pattern)
 */
void Update_AudioComponents(float dt);