#pragma once
#ifndef AUDIO_H
#define AUDIO_H

#include <string>
#include "fmod.hpp"

class AudioSystem {
public:
    // Initialize FMOD
    static bool Init();

    // Shutdown FMOD
    static void Close();

    // Update FMOD (must be called every frame)
    static void Update();

    // Load and play sound
    static FMOD::Sound* LoadSound(const std::string& filename, bool loop = false);
    static void PlaySound(FMOD::Sound* sound);

    // Stop all sounds
    static void StopAll();

private:
    static FMOD::System* mSystem;
};

#endif // AUDIO_H
