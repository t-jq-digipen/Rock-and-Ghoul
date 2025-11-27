/* ========================================================
 * @file    AudioSystem.h
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    26 SEP 2025
 *
 * @brief   Header file declaring the AudioManager class
 *
 * This header file contains the declaration of the AudioManager
 * class. The AudioManager provides functionality
 * for loading and playing sounds, managing background music
 * with fade transitions, controlling playback
 * through channels, and handling state-based music changes. It acts
 * as a wrapper around the lower-level AudioSystem while adding
 * game-specific audio management features such as automatic music
 * transitions based on game state.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include "System.h"
#include "Audio.h"

 /**
  * @class AudioManager
  * @brief High-level audio management system for the game.
  *
  * The AudioManager class handles music playback, sound effects,
  * fade-in/fade-out transitions between music tracks, and transitions based on game state changes.
  */
class AudioManager : public ISystem {
public:
    static AudioManager& Get() {
        static AudioManager instance;
        return instance;
    }

    void FadeOutAndStop(float duration); // FIXTHISSHIT DOCUMENTATION

    /**
     * @brief Initializes the FMOD audio system and preloads music tracks.
     *
     * Sets up FMOD, loads music for different game states, and starts
     * playing initial background music.
     *
     * @param dt Delta time in seconds.
     */
    void Initialize() override;

    /**
     * @brief Updates FMOD system and processes audio transitions.
     *
     * Updates the FMOD system, processes fade transitions, and handles
     * music changes based on game state.
     *
     * @param dt Delta time in seconds.
     */
    void Update(float dt) override;

    /**
     * @brief Shuts down the audio system and releases resources.
     *
     * Closes FMOD and releases all loaded sounds and channels.
     */
    void Shutdown() override;

    /**
     * @brief Updates the audio listener position to follow the camera.
     *
     * This function should be called every frame to ensure spatial audio
     * correctly positions sounds relative to the camera.
     */
    void UpdateListenerToCamera();

    /**
     * @brief Updates fade-in/fade-out effects.
     *
     * Handles volume transitions for smooth music fading.
     *
     * @param dt Delta time in seconds since last frame.
     */
    void UpdateFade(float dt);

    /**
     * @brief Starts a fade transition to new background music.
     *
     * Fades out current music, then fades in the specified BGM.
     *
     * @param bgmAssetName The name of the BGM asset in AudioCache.
     * @param duration Duration in seconds for each fade (out and in).
     */
    void TransitionToBGM(const std::string& bgmAssetName, float duration);

    /**
     * @brief Fades in background music from silence.
     *
     * Used when entering play mode or starting a scene.
     *
     * @param bgmAssetName The name of the BGM asset in AudioCache.
     * @param duration Duration in seconds for the fade-in.
     */
    void FadeInBGM(const std::string& bgmAssetName, float duration);

    /**
     * @brief Pauses the currently playing music.
     *
     * Stores the current playback position and pauses the music channel.
     * The music remains loaded in memory and can be resumed from the same position.
     */
    void PauseMusic();

    /**
     * @brief Resumes paused music from where it was stopped.
     *
     * Resumes playback of music that was previously paused, continuing
     * from the exact position where it was paused.
     */
    void ResumeMusic();

    /**
     * @brief Checks if music is currently paused.
     *
     * @return bool True if music is paused, false otherwise.
     */
    bool IsMusicPaused() const { return musicPaused; }

    /**
     * @brief Loads a sound file and returns a pointer to it.
     *
     * Loads an audio file that can loop or play once.
     *
     * @param path The file path to the audio file
     * @param loop If true, the sound will loop continuously when played.
     *             If false (default), the sound will play once and stop.
     * @return FMOD::Sound* Pointer to the loaded sound object, or nullptr if
     *         loading failed.
     */
    FMOD::Sound* Load(const std::string& path, bool loop = false);

    /**
     * @brief Plays a sound without returning channel control.
     *
     * Plays a sound at full volume for simple sound effects.
     *
     * @param sound Pointer to the FMOD::Sound object to play.
     */
    void Play(FMOD::Sound* sound);

    /**
     * @brief Plays a sound and returns its channel for playback control.
     *
     * Plays a sound and returns the channel pointer for controlling volume,
     * position, and pause state.
     *
     * @param sound Pointer to the FMOD::Sound object to play. Must not be nullptr.
     * @param paused If true, the sound is loaded onto a channel but not played
     *               immediately. If false (default), playback begins immediately.
     * @return FMOD::Channel* Pointer to the channel playing the sound, or nullptr
     *         if playback failed.
     */
    FMOD::Channel* PlayGetChannel(FMOD::Sound* sound, bool paused = false);

    /**
     * @brief Sets the playback position of a channel in milliseconds.
     *
     * Seeks to a specific position in the audio stream. Useful for resuming
     * music from a saved position.
     *
     * @param channel Pointer to the FMOD::Channel whose position should be set.
     * @param ms The position in milliseconds from the start of the sound.
     */
    void SetPosition(FMOD::Channel* channel, unsigned int ms);

    /**
     * @brief Gets the current playback position of a channel.
     *
     * Retrieves the playhead position in milliseconds.
     *
     * @param channel Pointer to the FMOD::Channel to query.
     * @return unsigned int The current playback position in milliseconds.
     */
    unsigned int GetPosition(FMOD::Channel* channel);

    /**
     * @brief Pauses or resumes playback on a channel.
     *
     * Pauses or resumes a channel while maintaining playback position.
     *
     * @param channel Pointer to the FMOD::Channel to pause or unpause.
     * @param paused If true, the channel is paused. If false, playback resumes.
     */
    void SetPaused(FMOD::Channel* channel, bool paused);

    /**
     * @brief Plays a music track on the current music channel.
     *
     * Starts playing background music at the configured volume level. Replaces
     * any currently playing music.
     *
     * @param track Pointer to the FMOD::Sound object containing the music track.
     * @param paused If true, the track is loaded but not played immediately.
     *               If false (default), playback begins immediately.
     */
    void PlayMusic(FMOD::Sound* track, bool paused = false);

    /**
     * @brief Plays the sound effect for a specific button type.
     *
     * Convenience wrapper that plays button sounds through AudioSystem. Looks up
     * and plays the registered sound for the given button type.
     *
     * @param type The type of button whose sound effect should be played,
     *             as defined in the AudioSystem::ButtonType enum.
     */
    void PlayButtonSound(AudioSystem::ButtonType type) {
        AudioSystem::PlayButtonSound(type);
    }

    /**
     * @brief Gets the current music channel.
     *
     * @return FMOD::Channel* Pointer to the current music channel, or nullptr if no music is playing.
     */
    FMOD::Channel* GetCurrentMusicChannel() const { return currentChannel; }

private:
    // Volume control
    float musicVol = 0.1f;

    // Channel management
    FMOD::Channel* currentChannel = nullptr;

    // ? REMOVED: nextChannel (no longer needed with fade system)

    // Fade system variables (REPLACED crossfade)
    bool fadeActive = false;       ///< Whether a fade is currently active
    bool fadingOut = false;        ///< true = fading out, false = fading in
    float fadeTime = 0.0f;         ///< Current time in fade transition
    float fadeDuration = 2.0f;     ///< Duration of fade effect in seconds
    std::string pendingBGM = "";   ///< BGM to play after fade out completes

    // ? REMOVED: crossfadeTime, crossfadeDuration, crossfadeActive

    // Pause state
    bool musicPaused = false;
    unsigned int pausedPosition = 0;  // Store position when paused

    // Preloaded music tracks
    FMOD::Sound* mainMenuMusic = nullptr;
    FMOD::Sound* level1Music = nullptr;

    // State tracking
    std::string lastScene = "";

    // ? REMOVED: UpdateCrossfade(float dt)
    // ? REMOVED: HandleMusicForScene(...)
};