#pragma once
#ifndef AUDIO_H
#define AUDIO_H

#include <string>
#include <unordered_map>
#include "fmod.hpp"

class AudioSystem {
public:
    // Initialize FMOD
    static bool Init();

    // Close FMOD
    static void Close();

    // Update FMOD (must be called every frame)
    static void Update();

    // Load and play sound
    static FMOD::Sound* LoadSound(const std::string& filename, bool loop = false);
    static void PlaySound(FMOD::Sound* sound);

    // Stop all sounds
    static void StopAll();

    // Channel control functions
    static FMOD::Channel* PlaySoundGetChannel(FMOD::Sound* sound, bool paused = false);
    static void SetChannelPosition(FMOD::Channel* channel, unsigned int ms);
    static unsigned int GetChannelPosition(FMOD::Channel* channel);
    static void SetChannelPaused(FMOD::Channel* channel, bool paused);

    // ===== ADDED: Flexible Button SFX System =====
    enum class ButtonType {
        PLAY,
        QUIT,
    };

    // Main button sound function - handles all button types
    static void PlayButtonSound(ButtonType type);

    // Register custom button sounds at runtime
    static void RegisterButtonSound(ButtonType type, const std::string& soundPath);
    static void RegisterButtonSound(ButtonType type, FMOD::Sound* sound);

    // ===== END ADDED =====

private:
    static FMOD::System* mSystem;

    // ===== ADDED: Button sound storage =====
    static std::unordered_map<ButtonType, FMOD::Sound*> buttonSounds;
    static void LoadDefaultButtonSounds();
    // ===== END ADDED =====
};

#endif // AUDIO_H