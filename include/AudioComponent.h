/* ========================================================
 * @file    AudioComponent.h
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    4 NOV 2025
 *
 * @brief   Header file for entity-based audio component system
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include "fmod.hpp"
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "Entity.h"
#include "Component.h"  // For Editor::ComponentData

 /**
  * @class AudioComponent
  * @brief Entity component for managing multiple audio sources.
  */
struct AudioComponent { // JQ CHECK HERE
    // ========== Sound Collection ==========
    std::unordered_map<std::string, std::string> sounds; // map for IDs, store audio file names in there in the order of name assigned by whoever, then name of audio from manifest

    // ========== Auto-Play Settings ==========
    std::string autoPlaySound = ""; // empty = no sound attached

    // ========== Playback Settings ==========
    float volume = 1.0f;
    bool spatial = false;
    float minDistance = 1.0f;
    float maxDistance = 100.0f;

    // ========== Active Playback Tracking ==========
    std::unordered_map<std::string, FMOD::Channel*> activeChannels;

    // ========== Core Functions ==========
    bool PlaySound(Entity eID, const std::string& soundID); // informs which sound to play NO MORE TO CHECK
    void StopSound(const std::string& soundID);
    void StopAllSounds();
    bool IsPlaying(const std::string& soundID);
    void UpdatePlaybackStates(Entity eID);
    void SetVolume(float vol);

    // ========== Serialization (follows your project pattern) ==========
    std::string Serialize() const;
    static AudioComponent DeSerialize(Entity eID, const std::string& data);

    // ========== Editor Integration ==========
    Editor::ComponentData EditorData();

private:
    void UpdateChannelSpatialPosition(Entity eID, FMOD::Channel* channel);
};

/**
 * @brief System update function for all AudioComponents
 */
void Update_AudioComponents(float dt);