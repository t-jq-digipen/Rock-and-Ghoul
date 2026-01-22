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
// audio test will delete asap
#include "Input.h"
#include <GLFW/glfw3.h>
using json = nlohmann::json;

// ============================================================================
// AUDIOCOMPONENT MEMBER FUNCTIONS
// ============================================================================

bool AudioComponent::PlaySound(Entity eID, const std::string& soundID) {
    auto it = sounds.find(soundID);
    if (it == sounds.end()) {
        std::cerr << "[AudioComponent] Sound ID '" << soundID << "' not found!" << std::endl;
        return false;
    }

    const std::string& assetName = it->second;
    StopSound(soundID);

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
        activeChannels[soundID] = channel;
        std::cout << "[AudioComponent] Playing sound: " << soundID << " (" << assetName << ")" << std::endl;
        return true;
    }

    return false;
}

void AudioComponent::StopSound(const std::string& soundID) {
    auto it = activeChannels.find(soundID);
    if (it != activeChannels.end() && it->second) {
        it->second->stop();
        activeChannels.erase(it);
        std::cout << "[AudioComponent] Stopped sound: " << soundID << std::endl;
    }
}

void AudioComponent::StopAllSounds() {
    for (auto& [soundID, channel] : activeChannels) {
        if (channel) {
            channel->stop();
        }
    }
    activeChannels.clear();
    std::cout << "[AudioComponent] Stopped all sounds" << std::endl;
}

bool AudioComponent::IsPlaying(const std::string& soundID) {
    auto it = activeChannels.find(soundID);
    if (it == activeChannels.end() || !it->second) {
        return false;
    }

    bool playing = false;
    it->second->isPlaying(&playing);
    return playing;
}

void AudioComponent::UpdatePlaybackStates(Entity eID) {
    std::vector<std::string> toRemove;

    for (auto& [soundID, channel] : activeChannels) {
        if (!channel) {
            toRemove.push_back(soundID);
            continue;
        }

        bool playing = false;
        channel->isPlaying(&playing);

        if (!playing) {
            toRemove.push_back(soundID);
        }
        else if (spatial) {
            UpdateChannelSpatialPosition(eID, channel);
        }
    }

    for (const auto& soundID : toRemove) {
        activeChannels.erase(soundID);
    }
}

void AudioComponent::UpdateChannelSpatialPosition(Entity eID, FMOD::Channel* channel) {
    if (!channel || !Registry::Get().Has_Component<TransformComponent>(eID)) {
        return;
    }

    auto& transform = Registry::Get().Get_Component<TransformComponent>(eID);
    FMOD_VECTOR pos = { transform.pos.x, transform.pos.y, 0.0f };
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

    channel->set3DAttributes(&pos, &vel);
    channel->set3DMinMaxDistance(minDistance, maxDistance);
}

void AudioComponent::SetVolume(float vol) {
    volume = vol;

    for (auto& [soundID, channel] : activeChannels) {
        if (channel) {
            channel->setVolume(volume);
        }
    }

    std::cout << "[AudioComponent] Volume set to: " << (volume * 100.0f) << "%" << std::endl;
}

// ============================================================================
// SERIALIZATION
// ============================================================================

std::string AudioComponent::Serialize() const {
    json j;

    j["sounds"] = sounds;
    j["autoPlaySound"] = autoPlaySound;
    j["volume"] = volume;
    j["spatial"] = spatial;
    j["minDistance"] = minDistance;
    j["maxDistance"] = maxDistance;

    return j.dump();
}

AudioComponent AudioComponent::DeSerialize(Entity eID, const std::string& data) {
    (void)eID;

    if (data.empty()) {
        return AudioComponent();
    }

    try {
        json j = json::parse(data);
        AudioComponent audio;

        if (j.contains("sounds") && j["sounds"].is_object()) {
            for (auto it = j["sounds"].begin(); it != j["sounds"].end(); ++it) {
                audio.sounds[it.key()] = it.value().get<std::string>();
            }
        }

        audio.autoPlaySound = j.value("autoPlaySound", std::string{});
        audio.volume = j.value("volume", 1.0f);
        audio.spatial = j.value("spatial", false);
        audio.minDistance = j.value("minDistance", 1.0f);
        audio.maxDistance = j.value("maxDistance", 100.0f);

        // activeChannels start empty

        std::cout << "[AudioComponent] Loaded " << audio.sounds.size() << " sounds" << std::endl;

        return audio;
    }
    catch (const std::exception& e) {
        std::cerr << "[AudioComponent] Failed to deserialize: " << e.what() << std::endl;
        return AudioComponent();
    }
}

// ============================================================================
// EDITOR INTEGRATION
// ============================================================================

Editor::ComponentData AudioComponent::EditorData() {
    Editor::ComponentData data("Audio");

    // Auto-Play Sound Dropdown
    if (!sounds.empty()) {
        Editor::Widget_Dropdown autoPlayDropdown;
        autoPlayDropdown.ref = reinterpret_cast<unsigned int*>(&autoPlaySound);
        autoPlayDropdown.Add_Option("None", 0);

        unsigned int index = 1;
        for (const auto& [soundID, assetName] : sounds) {
            autoPlayDropdown.Add_Option(soundID + " [" + assetName + "]", index++);
        }
        data.Add_Data("Auto Play Sound", autoPlayDropdown);
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
    (void)dt;

    // JQ CHECK HERE
// ========== AUDIO TEST (DELETE WHEN DONE) ==========
    static bool testSetup = false;
    static AudioComponent audio;

    if (!testSetup) {
        audio.sounds["test1"] = "button_play";
        audio.sounds["test2"] = "button_quit";
        audio.sounds["test3"] = "level_1";

        testSetup = true;
        std::cout << "[AUDIO TEST] Setup complete!\n";
        std::cout << "[AUDIO TEST] Press 1,2,3 to play sounds, 4 to stop all\n";
    }

    if (Input::IsKeyPressed(GLFW_KEY_1)) {
        audio.PlaySound(0, "test1");
    }
    if (Input::IsKeyPressed(GLFW_KEY_2)) {
        audio.PlaySound(0, "test2");
    }
    if (Input::IsKeyPressed(GLFW_KEY_3)) {
        audio.PlaySound(0, "test3");
    }
    if (Input::IsKeyPressed(GLFW_KEY_4)) {
        audio.StopAllSounds();
    }
    // ========== END AUDIO TEST ==========

    Registry& registry = Registry::Get();

    for (auto& [entity, audio] : registry.Get_ComponentMap<AudioComponent>()) {
        if (!audio.autoPlaySound.empty() && !audio.IsPlaying(audio.autoPlaySound)) {
            audio.PlaySound(entity, audio.autoPlaySound); // store many sounds to one entity :3
            audio.autoPlaySound = ""; // audio that happens when entities spawn NO MORE TO CHECK
        }

        audio.UpdatePlaybackStates(entity);
    }
}