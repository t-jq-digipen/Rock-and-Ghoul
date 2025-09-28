// AudioSystem.h
#pragma once
#include "System.h"
#include "Audio.h"   // use your FMOD wrapper
#include <iostream> //debug

class AudioManager : public ISystem {
public:
    void Initialize() override {
        AudioSystem::Init();
    }

    void Update(float dt) override {
        AudioSystem::Update();

        // Crossfade logic
        if (crossfadeActive) {
            std::cout << "CROSSFADE ACTIVE - Time: " << crossfadeTime
                << "/" << crossfadeDuration << std::endl;

            crossfadeTime += dt;
            float t = crossfadeTime / crossfadeDuration;
            if (t > 1.0f) t = 1.0f;

            if (currentChannel)
                currentChannel->setVolume(1.0f - t);
            if (nextChannel)
                nextChannel->setVolume(t);

            if (t >= 1.0f) {
                if (currentChannel) currentChannel->stop();
                currentChannel = nextChannel;
                nextChannel = nullptr;
                crossfadeActive = false;
                std::cout << "CROSSFADE COMPLETED\n";
            }
        }
    }

    void Shutdown() override {
        AudioSystem::Close();
    }

    // helper: load a sound (music or SFX)
    FMOD::Sound* Load(const std::string& path, bool loop = false) {
        return AudioSystem::LoadSound(path, loop);
    }

    // ADDED: Unload a sound to prevent memory leaks
    void Unload(FMOD::Sound* sound) {
        if (sound) {
            sound->release();
        }
    }

    // helper: play a sound
    void Play(FMOD::Sound* sound) {
        AudioSystem::PlaySound(sound);
    }

    // NEW: play sound and get channel pointer
    FMOD::Channel* PlayGetChannel(FMOD::Sound* sound, bool paused = false) {
        return AudioSystem::PlaySoundGetChannel(sound, paused);
    }

    // NEW: set playback position in milliseconds
    void SetPosition(FMOD::Channel* channel, unsigned int ms) {
        if (channel) {
            channel->setPosition(ms, FMOD_TIMEUNIT_MS);
        }
    }

    // NEW: get playback position in milliseconds
    unsigned int GetPosition(FMOD::Channel* channel) {
        unsigned int pos = 0;
        if (channel) {
            channel->getPosition(&pos, FMOD_TIMEUNIT_MS);
        }
        return pos;
    }

    // NEW: set paused state
    void SetPaused(FMOD::Channel* channel, bool paused) {
        if (channel) {
            channel->setPaused(paused);
        }
    }

    // NEW: play and store the current music channel
    void PlayMusic(FMOD::Sound* track, bool paused = false) {
        // Stop previous music if playing
        if (currentChannel) {
            currentChannel->stop();
        }
        currentChannel = PlayGetChannel(track, paused);
        if (currentChannel && !paused)
            currentChannel->setVolume(1.0f);
    }

    // NEW: crossfade to a new track over time
    void CrossfadeTo(FMOD::Sound* newTrack, float duration) {
        if (!newTrack) return;

        // Stop any existing crossfade
        if (crossfadeActive && nextChannel) {
            nextChannel->stop();
        }

        nextChannel = PlayGetChannel(newTrack, true); // start paused
        if (!nextChannel) return;

        nextChannel->setVolume(0.0f);
        nextChannel->setPaused(false);

        crossfadeTime = 0.0f;
        crossfadeDuration = duration;
        crossfadeActive = true;
    }

private:
    FMOD::Channel* currentChannel = nullptr;
    FMOD::Channel* nextChannel = nullptr;
    float crossfadeTime = 0.0f;
    float crossfadeDuration = 0.0f;
    bool crossfadeActive = false;
};