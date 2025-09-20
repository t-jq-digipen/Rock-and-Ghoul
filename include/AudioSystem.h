// AudioSystem.h
#pragma once
#include "System.h"
#include "Audio.h"   // use your FMOD wrapper

class AudioManager : public ISystem {
public:
    void Initialize() override { AudioSystem::Init(); }
    void Update(float) override { AudioSystem::Update(); }
    void Shutdown() override { AudioSystem::Close(); }

    // helper: load a sound (music or SFX)
    FMOD::Sound* Load(const std::string& path, bool loop = false) {
        return AudioSystem::LoadSound(path, loop);
    }

    // helper: play a sound
    void Play(FMOD::Sound* sound) {
        AudioSystem::PlaySound(sound);
    }
};
