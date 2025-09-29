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

    // here we go again
    // After StopAll(), add these:
    static FMOD::Channel* PlaySoundGetChannel(FMOD::Sound* sound, bool paused = false);
    static void SetChannelPosition(FMOD::Channel* channel, unsigned int ms);
    static unsigned int GetChannelPosition(FMOD::Channel* channel);
    static void SetChannelPaused(FMOD::Channel* channel, bool paused);

private:
    static FMOD::System* mSystem;
};

#endif // AUDIO_H
