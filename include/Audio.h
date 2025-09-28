#pragma once
#ifndef AUDIO_H
#define AUDIO_H

#include <string>
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

    // NEW: play sound and return channel
    static FMOD::Channel* PlaySoundGetChannel(FMOD::Sound* sound, bool paused = false);

    // NEW: set channel playback position
    static void SetChannelPosition(FMOD::Channel* channel, unsigned int ms);

    // NEW: get channel playback position
    static unsigned int GetChannelPosition(FMOD::Channel* channel);

    // NEW: pause/unpause a channel
    static void SetChannelPaused(FMOD::Channel* channel, bool paused);

private:
    static FMOD::System* mSystem;
};

#endif // AUDIO_H
