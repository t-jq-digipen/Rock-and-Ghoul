// AudioSystem.h
#pragma once
#include "System.h"
#include "Audio.h"
#include "GameStateManager.h"

class AudioManager : public ISystem {
public:
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;

    // helper: load a sound (music or SFX)
    FMOD::Sound* Load(const std::string& path, bool loop = false);

    // helper: play a sound
    void Play(FMOD::Sound* sound);

    // play sound and get channel pointer
    FMOD::Channel* PlayGetChannel(FMOD::Sound* sound, bool paused = false);

    // set playback position in milliseconds
    void SetPosition(FMOD::Channel* channel, unsigned int ms);

    // get playback position in milliseconds
    unsigned int GetPosition(FMOD::Channel* channel);

    // set paused state
    void SetPaused(FMOD::Channel* channel, bool paused);

    // play and store the current music channel
    void PlayMusic(FMOD::Sound* track, bool paused = false);

    // crossfade to a new track over time
    void CrossfadeTo(FMOD::Sound* newTrack, float duration);

    // ===== UPDATED: Unified Button Sound System =====
    void PlayButtonSound(AudioSystem::ButtonType type) {
        AudioSystem::PlayButtonSound(type);
    }
    // ===== END UPDATED =====

private:
    float musicVol = 0.1f;
    FMOD::Channel* currentChannel = nullptr;
    FMOD::Channel* nextChannel = nullptr;
    float crossfadeTime = 0.0f;
    float crossfadeDuration = 0.0f;
    bool crossfadeActive = false;

    // Music tracks
    FMOD::Sound* mainMenuMusic = nullptr;
    FMOD::Sound* level1Music = nullptr;

    // State tracking
    GameState lastState = GameState::GS_MAIN_MENU;

    // Helper methods
    void UpdateCrossfade(float dt);
    void HandleMusicForState(GameState currentState, GameState previousState);
};