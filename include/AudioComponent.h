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
#include <vector>
#include <unordered_map>
#include "Entity.h"

struct AudioComponent {
    // ========== Sound Storage ==========
    std::vector<std::string> soundFiles;  // Simple array of audio files

    // ========== Auto-Play Settings ==========
    unsigned int autoPlayIndex = 0;  // 0 = none, 1 = first sound, 2 = second sound

    // ========== Playback Settings ==========
    float volume = 1.0f;
    bool spatial = false;
    float minDistance = 1.0f;
    float maxDistance = 100.0f;

    // ========== Active Playback Tracking ==========
    std::unordered_map<int, FMOD::Channel*> activeChannels;  // Track by index

    // ========== Core Functions ==========
    // Static helper for one-line calls (what your PM wants!)
    static bool Play(Entity eID, int index);

    // Instance method
    bool PlaySound(Entity eID, int index);
    void StopSound(int index);
    void StopAllSounds();
    bool IsPlaying(int index);
    void UpdatePlaybackStates(Entity eID);
    void SetVolume(float vol);

    // ========== Editor Integration ==========
    Editor::ComponentData EditorData();

    // ========== Serialization ==========
    std::string Serialize() const;
    static AudioComponent DeSerialize(Entity eID, const std::string& data);

private:
    void UpdateChannelSpatialPosition(Entity eID, FMOD::Channel* channel);
};