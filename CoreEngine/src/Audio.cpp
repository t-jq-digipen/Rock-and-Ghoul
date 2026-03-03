/* ========================================================
 * @file    Audio.cpp
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    5 FEB 2025
 *
 * @brief   Implementation file for AudioSystem and AudioManager classes
 *
 * This file contains the implementation of the AudioSystem and
 * AudioManager classes, which provide audio playback functionality
 * using the FMOD audio library. The AudioSystem class handles
 * low-level FMOD operations including system initialization, sound
 * loading, and playback control. The AudioManager class provides
 * higher-level functionality such as music management, crossfading
 * between tracks, and state-based audio transitions. The file also
 * implements a flexible button sound system that maps button types
 * to specific sound effects. It also supports spatial audio positioning.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "FilePath.h"
#include "Audio.h"
#include "AudioSystem.h"
#include "AudioCache.h"
#include "AudioComponent.h"
#include "CoreEngine.h"
#include "PlayerSystem.h"
#include "Camera.h"
#include <iostream>
#include "SceneManager.h"
#include "GuiSystem.h"
#include "Input.h"
 // Static members
FMOD::System* AudioSystem::mSystem = nullptr;

// button sound storage
std::unordered_map<AudioSystem::ButtonType, FMOD::Sound*> AudioSystem::buttonSounds;

// listener state for spatial audio
static FMOD_VECTOR listenerPos = { 0.0f, 0.0f, 0.0f };
static FMOD_VECTOR listenerVel = { 0.0f, 0.0f, 0.0f };
static FMOD_VECTOR listenerForward = { 0.0f, 0.0f, 1.0f };
static FMOD_VECTOR listenerUp = { 0.0f, 1.0f, 0.0f };

/**
 * @brief Initializes the FMOD audio system.
 *
 * This function creates and initializes the FMOD audio system.
 * It also loads default button sounds for the application.
 *
 * @return bool True if the FMOD system was successfully created and
 *         initialized, false if any errors occurred during initialization.
 */
bool AudioSystem::Init() {
    FMOD_RESULT result;
    // Create the FMOD system object
    result = FMOD::System_Create(&mSystem);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: Failed to create system (" << result << ")" << std::endl;
        return false;
    }

    // DISABLE VOLUME RAMPING..WORKS SOMETIMES?
    FMOD_ADVANCEDSETTINGS advancedSettings;
    memset(&advancedSettings, 0, sizeof(FMOD_ADVANCEDSETTINGS));
    advancedSettings.cbSize = sizeof(FMOD_ADVANCEDSETTINGS);
    advancedSettings.vol0virtualvol = 0.0f;

    result = mSystem->setAdvancedSettings(&advancedSettings);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Warning: Failed to set advanced settings (" << result << ")" << std::endl;
        // Continue anyway - this is not critical
    }

    // Create the FMOD system object
    result = mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: Failed to initialize system (" << result << ")" << std::endl;
        return false;
    }
    // Load the default button click sounds
    LoadDefaultButtonSounds();

    return true;
}

/**
 * @brief Loads default button sound effects.
 *
 * This function registers default sound effects for different button types
 * in the application.
 */
void AudioSystem::LoadDefaultButtonSounds() {
    RegisterButtonSound(ButtonType::PLAY, AudioCache::GetSound("button_play"));
    RegisterButtonSound(ButtonType::QUIT, AudioCache::GetSound("button_quit"));
    RegisterButtonSound(ButtonType::CLICK, AudioCache::GetSound("ui_click"));
}

/**
 * @brief Plays the sound effect associated with a specific button type.
 *
 * This function looks up and plays the sound registered for the given
 * button type.
 *
 * @param type The type of button whose sound effect should be played,
 *             as defined in the ButtonType enum.
 */
void AudioSystem::PlayButtonSound(ButtonType type) {
    // Look up the button sound in the map
    auto it = buttonSounds.find(type);
    // If found and valid, play it
    if (it != buttonSounds.end() && it->second) {
        // IMPORTANT: Button sounds bypass pause check
        // We call playSound directly instead of using PlaySound()
        // This ensures UI sounds work even when game is paused
        if (mSystem) {
            FMOD::Channel* channel = nullptr;
            mSystem->playSound(it->second, nullptr, false, &channel);
        }
    }
    else {
        // hail mary: play PLAY sound if the requested type isn't found
        if (type != ButtonType::PLAY) {
            auto playIt = buttonSounds.find(ButtonType::PLAY);
            if (playIt != buttonSounds.end() && playIt->second && mSystem) {
                FMOD::Channel* channel = nullptr;
                mSystem->playSound(playIt->second, nullptr, false, &channel);
            }
        }
    }
}

/**
 * @brief Registers a button sound effect by loading it from a file path.
 *
 * This function loads a sound file from the specified path and associates
 * it with a button type.
 *
 * @param type The ButtonType to associate with this sound effect.
 * @param soundPath The file path to the sound file
 */
void AudioSystem::RegisterButtonSound(ButtonType type, const std::string& soundPath) {
    FMOD::Sound* sound = LoadSound(soundPath, false);
    if (sound) {
        buttonSounds[type] = sound;
    }
    else {
        std::cerr << "Failed to load button sound: " << soundPath << std::endl;
    }
}

/**
 * @brief Registers a button sound effect using an existing FMOD::Sound pointer.
 *
 * This overload allows registration of a button sound using an already
 * loaded FMOD::Sound object.
 *
 * @param type The ButtonType to associate with this sound effect.
 * @param sound Pointer to an already loaded FMOD::Sound object.
 */
void AudioSystem::RegisterButtonSound(ButtonType type, FMOD::Sound* sound) {
    if (sound) {
        buttonSounds[type] = sound;
    }
}

/**
 * @brief Closes and cleans up the FMOD audio system.
 *
 * This function releases all button sounds, closes the FMOD system,
 * and frees all associated resources.
 */
void AudioSystem::Close() {
    if (!mSystem) return;  // ALREADY closed


    // stop all channels first
    FMOD::ChannelGroup* masterGroup = nullptr;
    FMOD_RESULT result = mSystem->getMasterChannelGroup(&masterGroup);
    if (result == FMOD_OK && masterGroup) {
        masterGroup->stop();
    }

    // update fmod until the process is over
    for (int i = 0; i < 5; ++i) {
        mSystem->update();
    }

    // clear button sounds from cache
    buttonSounds.clear();

    mSystem->close();
    mSystem->release();
    mSystem = nullptr;
}

/**
 * @brief Updates the FMOD audio system.
 *
 * This function must be called once per frame to update FMOD's internal
 * state, including channel management, streaming, and effects processing.
 */
void AudioSystem::Update() {
    if (mSystem) {
        mSystem->update();
    }
}

/**
 * @brief Loads a sound file from disk with specified FMOD mode flags.
 *
 * This function loads an audio file using FMOD with custom playback settings
 * for 2D/3D positioning, looping, and distance attenuation.
 *
 * @param filename The file path to the audio file to load
 * @param mode FMOD mode flags (e.g., FMOD_2D, FMOD_3D, FMOD_LOOP_NORMAL, FMOD_LOOP_OFF).
 *             Multiple flags can be combined using bitwise OR (|).
 * @return FMOD::Sound* Pointer to the loaded sound object, or nullptr
 *         if loading failed (e.g., file not found, unsupported format, system not initialized).
 */
FMOD::Sound* AudioSystem::LoadSound(const std::string& filename, FMOD_MODE mode) {
    if (!mSystem) {
        std::cerr << "[AudioSystem] System not initialized!" << std::endl;
        return nullptr;
    }
    FMOD::Sound* sound = nullptr;
    FMOD_RESULT result = mSystem->createSound(filename.c_str(), mode, nullptr, &sound);
    if (result != FMOD_OK) {
        std::cerr << "[AudioSystem] Failed to load: " << filename << std::endl;
        return nullptr;
    }
    return sound;
}

/**
 * @brief Plays a sound immediately without returning a channel handle.
 *
 * This function plays the specified sound at full volume without
 * channel control.
 *
 * @param sound Pointer to the FMOD::Sound object to play.
 */
void AudioSystem::PlaySound(FMOD::Sound* sound) {
    if (!mSystem || !sound) return;

    // DON'T PLAY AUDIO IN EDIT MODE
    if (SceneManager::Get().IsInEditMode()) {
        return;
    }

    // NOTE: Button sounds bypass this check (called separately via PlayButtonSound)
    // When paused, only UI/button sounds should play - regular SFX are blocked
    if (SceneManager::Get().IsPaused()) {
        // Block regular sound effects when paused
        return;
    }

    FMOD::Channel* channel = nullptr;
    mSystem->playSound(sound, nullptr, false, &channel);
}

/**
 * @brief Stops all currently playing sounds.
 *
 * This function stops all active audio channels by closing and
 * reinitializing the FMOD system.
 *
 * @warning This will stop ALL audio including music and sound effects.
 */
void AudioSystem::StopAll() {
    if (!mSystem) return;

    // OLD CODE (destroys everything):
    // mSystem->close();
    // mSystem->init(32, FMOD_INIT_NORMAL, nullptr);

    // STOPS CHANNELS BUT KEEPS SOUNDS LAODED
    FMOD::ChannelGroup* masterGroup = nullptr;
    FMOD_RESULT result = mSystem->getMasterChannelGroup(&masterGroup);

    if (result == FMOD_OK && masterGroup) {
        masterGroup->stop();
    }
}

/**
 * @brief Plays a sound and returns the channel handle for playback control.
 *
 * This function plays a sound and returns a pointer to the channel.
 *
 * @param sound Pointer to the FMOD::Sound object to play.
 * @param paused If true, the sound is loaded onto a channel but not played
 *               immediately. If false (default), playback begins immediately.
 * @return FMOD::Channel* Pointer to the channel playing the sound, or
 *         nullptr if playback failed or the audio system is not initialized.
 */
FMOD::Channel* AudioSystem::PlaySoundGetChannel(FMOD::Sound* sound, bool paused) {
    if (!mSystem || !sound) return nullptr;

    // DON'T PLAY AUDIO IN EDIT MODE
    if (SceneManager::Get().IsInEditMode()) {
        return nullptr;
    }

    // NOTE: This function is used by BGM (which should play when paused)
    // and by regular SFX (which should NOT play when paused).
    // Callers are responsible for checking pause state before calling this.

    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = mSystem->playSound(sound, nullptr, paused, &channel);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: Failed to play sound with channel\n";
        return nullptr;
    }
    return channel;
}



/**
 * @brief Sets the playback position of a channel.
 *
 * This function seeks to a specific position in the audio stream.
 *
 * @param channel Pointer to the FMOD::Channel whose position should be set.
 * @param ms The position in milliseconds from the start of the sound.
 */
void AudioSystem::SetChannelPosition(FMOD::Channel* channel, unsigned int ms) {
    //FMOD::Channel* musicChannel = AudioSystem::PlaySoundGetChannel(bgm, false);
    if (channel) {
        channel->setPosition(ms, FMOD_TIMEUNIT_MS);
    }
}

/**
 * @brief Gets the current playback position of a channel.
 *
 * This function retrieves the current position of the audio playhead
 * in milliseconds from the start of the sound
 *
 * @param channel Pointer to the FMOD::Channel to query.
 * @return unsigned int The current playback position in milliseconds.
 */
unsigned int AudioSystem::GetChannelPosition(FMOD::Channel* channel) {
    //AudioSystem::SetChannelPaused(musicChannel, true);
    unsigned int pos = 0;
    if (channel) {
        channel->getPosition(&pos, FMOD_TIMEUNIT_MS);
    }
    return pos;
}

/**
 * @brief Sets the pause state of a channel.
 *
 * This function pauses or resumes playback on a specific channel.
 * When paused, can be resumed from the same point.
 *
 * @param channel Pointer to the FMOD::Channel to pause or unpause.
 * @param paused If true, the channel is paused. If false, playback resumes.
 */
void AudioSystem::SetChannelPaused(FMOD::Channel* channel, bool paused) {
    if (channel) {
        channel->setPaused(paused);
    }
}

// ============================================================================
// AudioManager Implementation
// ============================================================================

/**
 * @brief Initializes the AudioManager and preloads audio assets.
 *
 * This function initializes the underlying AudioSystem, preloads music
 * tracks for different game states (main menu, level 1), and starts
 * playing the main menu music.
 */
void AudioManager::Initialize() {
    // Initialize AudioCache
    if (!AudioCache::Initialize(FilePath::AUDIO_MANIFEST)) {
        std::cerr << "AudioCache failed to initialize!" << std::endl;
    }

    AudioSystem::Init();
    AudioSystem::Configure2DSpatialAudio();
    AudioSystem::SetListenerPosition(0.0f, 0.0f);

    AudioCache::LoadBank("init");

    // Preload music tracks
    mainMenuMusic = AudioCache::GetSound("main_menu");
    level1Music = AudioCache::GetSound("level_1");

    // Initialize fade state
    fadeActive = false;
    fadeTime = 0.0f;
    pendingBGM = "";

    GuiSystem::SetAudioManager(this);
}

/**
 * @brief Updates fade out effect (no fade in).
 *
 * This function handles only fade-out transitions. When a new track is ready,
 * it starts immediately at full volume after the fade-out completes.
 *
 * @param dt Delta time in seconds since last frame.
 */
void AudioManager::UpdateFade(float dt) {
    if (!fadeActive) return;

    fadeTime += dt;
    float t = fadeTime / fadeDuration;
    if (t > 1.0f) t = 1.0f;

    // Fade out using BOTH volume
    if (currentChannel) {
        float newVolume = musicVol * masterVolume * (1.0f - t);
        currentChannel->setVolume(newVolume);
    }

    if (t >= 1.0f) {
        // Fade out complete aka stop old music
        if (currentChannel) {
            currentChannel->stop();
            currentChannel = nullptr;
        }

        // Start new music using StartBGM (which handles paused start properly)
        if (!pendingBGM.empty()) {
            // Save the pending BGM before clearing it
            std::string nextBGM = pendingBGM;
            pendingBGM = "";

            // Use StartBGM which handles the paused-unpause technique
            StartBGM(nextBGM);
        }

        // Reset fade state
        fadeActive = false;
        fadeTime = 0.0f;
    }
}

/**
 * @brief Starts a fade-out transition to new BGM.
 *
 * Fades out current music, then starts the new music instantly at full volume.
 *
 * @param bgmAssetName The name of the BGM asset in AudioCache to play after fade out.
 * @param duration Duration in seconds for the fade-out.
 */
void AudioManager::TransitionToBGM(const std::string& bgmAssetName, float duration) {
    if (bgmAssetName.empty()) return;

    fadeDuration = duration;
    fadeTime = 0.0f;
    fadeActive = true;
    pendingBGM = bgmAssetName;
}

/**
 * @brief Starts playing BGM instantly at full volume.
 *
 * This function is used when entering play mode. The music starts immediately
 * without any fade-in effect.
 *
 * @param bgmAssetName The name of the BGM asset in AudioCache.
 * @param duration Unused parameter (kept for API compatibility).
 */
void AudioManager::StartBGM(const std::string& bgmAssetName) {
    if (bgmAssetName.empty()) return;

    FMOD::Sound* bgm = AudioCache::GetSound(bgmAssetName);
    if (!bgm) return;

    fadeActive = false;
    fadeTime = 0.0f;
    pendingBGM = "";

    currentChannel = PlayGetChannel(bgm, true);
    if (currentChannel) {
        // Apply BOTH base musicVol AND masterVolume
        currentChannel->setVolume(musicVol * masterVolume);
        currentChannel->setPaused(false);
    }
}

/**
 * @brief Fades out and stops current music.
 *
 * Gradually reduces volume to zero, then stops playback.
 *
 * @param duration Duration in seconds for the fade-out.
 */
void AudioManager::FadeOutAndStop(float duration) {
    if (!currentChannel) return;

    fadeDuration = duration;
    fadeTime = 0.0f;
    fadeActive = true;
    pendingBGM = "";  // just stop, don't play new music
}


/**
 * @brief Pauses the currently playing music.
 *
 * Stores the current playback position and pauses the music channel.
 */
void AudioManager::PauseMusic() {
    if (!currentChannel || musicPaused) return;

    bool isPlaying = false;
    currentChannel->isPlaying(&isPlaying);

    if (isPlaying) {
        // Store current position
        currentChannel->getPosition(&pausedPosition, FMOD_TIMEUNIT_MS);

        // Pause the channel
        currentChannel->setPaused(true);
        musicPaused = true;
    }
}

/**
 * @brief Resumes paused music from where it was stopped.
 */
void AudioManager::ResumeMusic() {
    if (!currentChannel || !musicPaused) return;

    // Resume from paused position
    currentChannel->setPaused(false);
    musicPaused = false;
}

/**
 * @brief Shuts down the AudioManager and releases all audio resources.
 */
void AudioManager::Shutdown() {
    // stop all active channels first
    if (currentChannel) {
        currentChannel->stop();
        currentChannel = nullptr;
    }

    // clear references to sounds in cache
    mainMenuMusic = nullptr;
    level1Music = nullptr;

    // unload ALL sounds from AudioCache & close
    AudioCache::Shutdown();
    AudioSystem::Close();
}

/**
 * @brief Loads a sound file through the AudioManager.
 *
 * This is a convenience wrapper around AudioSystem::LoadSound that
 * maintains consistent interface across the AudioManager API.
 *
 * @param path The file path to the audio file to load.
 * @param loop If true, the sound will loop continuously when played.
 *             If false, the sound will play once and stop.
 * @return FMOD::Sound* Pointer to the loaded sound object, or nullptr if
 *         loading failed.
 */
FMOD::Sound* AudioManager::Load(const std::string& path, bool loop) {
    return AudioSystem::LoadSound(path, loop);
}

/**
 * @brief Plays a sound immediately through the AudioManager.
 *
 * This is a convenience wrapper around AudioSystem::PlaySound for
 * playing simple sound effects without requiring channel control.
 *
 * @param sound Pointer to the FMOD::Sound object to play.
 */
void AudioManager::Play(FMOD::Sound* sound) {
    AudioSystem::PlaySound(sound);
}

/**
 * @brief Plays a sound and returns the channel handle.
 *
 * This is a convenience wrapper around AudioSystem::PlaySoundGetChannel
 * for playing sounds that require precise playback control.
 *
 * @param sound Pointer to the FMOD::Sound object to play.
 * @param paused If true, the sound is loaded but not played immediately.
 * @return FMOD::Channel* Pointer to the channel playing the sound, or nullptr
 *         if playback failed.
 */
FMOD::Channel* AudioManager::PlayGetChannel(FMOD::Sound* sound, bool paused) {
    // Check if game is paused - block regular SFX but allow BGM
    // (BGM calls are made directly from AudioManager's internal functions,
    //  so we can trust those. External calls are likely SFX and should be blocked.)

    // For now, we'll let everything through here since BGM uses this path
    // Individual sound-playing systems should check pause state themselves
    return AudioSystem::PlaySoundGetChannel(sound, paused);
}

/**
 * @brief Sets the playback position of a channel.
 *
 * This function seeks to a specific position in the audio stream.
 *
 * @param channel Pointer to the FMOD::Channel whose position should be set.
 * @param ms The position in milliseconds from the start of the sound.
 */
void AudioManager::SetPosition(FMOD::Channel* channel, unsigned int ms) {
    if (channel) {
        channel->setPosition(ms, FMOD_TIMEUNIT_MS);
    }
}

/**
 * @brief Gets the current playback position of a channel.
 *
 * This function retrieves the current position of the audio playhead.
 *
 * @param channel Pointer to the FMOD::Channel to query.
 * @return unsigned int The current playback position in milliseconds.
 */
unsigned int AudioManager::GetPosition(FMOD::Channel* channel) {
    unsigned int pos = 0;
    if (channel) {
        channel->getPosition(&pos, FMOD_TIMEUNIT_MS);
    }
    return pos;
}

/**
 * @brief Sets the pause state of a channel.
 *
 * This function pauses or resumes playback on a specific channel.
 *
 * @param channel Pointer to the FMOD::Channel to pause or unpause.
 * @param paused If true, the channel is paused. If false, playback resumes.
 */
void AudioManager::SetPaused(FMOD::Channel* channel, bool paused) {
    if (channel) {
        channel->setPaused(paused);
    }
}

/**
 * @brief Plays a music track on the current music channel.
 *
 * This function starts playing a music track and sets it as the current
 * music channel. The track's volume is set to the configured music volume
 * level.
 *
 * @param track Pointer to the FMOD::Sound object containing the music track.
 *              Must not be nullptr.
 * @param paused If true, the track is loaded but not played immediately.
 *               If false (default), playback begins immediately.
 */
void AudioManager::PlayMusic(FMOD::Sound* track, bool paused) {
    currentChannel = PlayGetChannel(track, paused);
    if (currentChannel && !paused)
        currentChannel->setVolume(musicVol);
}



/**
 * @brief Updates the listener position to follow the game camera.
 *
 * This function retrieves the current camera position and updates the
 * audio listener's position to match it, ensuring spatial audio remains
 * synchronized with the player's view. Called automatically each frame
 * during AudioManager::Update().
 */
void AudioManager::UpdateListenerToCamera() {
    // Get camera position
    Vec2 cameraPos = Camera::GetInstance().GetPosition();

    // Update listener position to camera position
    AudioSystem::SetListenerPosition(cameraPos.x, cameraPos.y);
}

/**
 * @brief Updates the AudioManager each frame.
 *
 * This function updates the FMOD system, processes active fade-outs,
 * and handles music transitions based on game state changes.
 *
 * @param dt Delta time in seconds.
 */

void AudioManager::Update(float dt) {
    // Update listener to follow camera every frame
    UpdateListenerToCamera();

    // Update FMOD system
    AudioSystem::Update();

    SceneManager& sm = SceneManager::Get();

    // ============================================================
    // HANDLE EDIT/PLAY MODE TRANSITIONS
    // ============================================================
    static bool wasInEditMode = true;
    static std::string lastBGM = "";

    // ENTERING PLAY MODE (edit → play)
    if (!sm.IsInEditMode() && wasInEditMode) {
        fadeActive = false;
        fadeTime = 0.0f;
        pendingBGM = "";

        std::string bgmAssetName = sm.GetCurrentSceneBGM();

        if (!bgmAssetName.empty()) {
            StartBGM(bgmAssetName);
        }

        lastBGM = bgmAssetName;
        lastScene = sm.GetCurrentSceneName();

        wasInEditMode = sm.IsInEditMode();
        return;
    }

    // ENTERING EDIT MODE (play to edit)
    if (sm.IsInEditMode() && !wasInEditMode) {
        if (currentChannel) {
            currentChannel->stop();
            currentChannel = nullptr;
        }

        fadeActive = false;
        fadeTime = 0.0f;
        pendingBGM = "";

        lastScene = "";
        lastBGM = "";

        wasInEditMode = sm.IsInEditMode();
    }
    else {
        wasInEditMode = sm.IsInEditMode();
    }

    // ============================================================
    // HANDLE SCENE TRANSITIONS (ONLY IN PLAY MODE)
    // ============================================================
    if (!sm.IsInEditMode()) {
        std::string currentBGM = sm.GetCurrentSceneBGM();
        std::string currentScene = sm.GetCurrentSceneName();

        float transitionProgress = sm.GetTransitionProgress();

        static bool wasTransitioning = false;
        bool isTransitioning = (transitionProgress > 0.0f && transitionProgress < 1.0f);

        // If we're in a transition, fade out BGM volume
        if (isTransitioning) {
            if (currentChannel) {
                float fadeOutVolume = musicVol * (1.0f - transitionProgress);
                currentChannel->setVolume(fadeOutVolume);
            }
            wasTransitioning = true;
        }

        // Transition completed - DON'T start music immediately, wait for fade to finish
        if (wasTransitioning && !isTransitioning) {
            // Stop old music
            if (currentChannel) {
                currentChannel->stop();
                currentChannel = nullptr;
            }

            // Queue the new BGM to start after audio fade completes
            // If new scene has BGM, transition to it
            if (!currentBGM.empty() && currentBGM != lastBGM) {
                // Set a very short fade (0.5s) so music starts quickly after screen fade
                TransitionToBGM(currentBGM, 0.5f);
            }
            // If new scene has no BGM, just clear state
            else if (currentBGM.empty()) {
                fadeActive = false;
                fadeTime = 0.0f;
                pendingBGM = "";
            }

            lastBGM = currentBGM;
            lastScene = currentScene;
            wasTransitioning = false;
        }

        // Scene changed without transition (e.g., cutscene auto-transition)
        if (!isTransitioning && !wasTransitioning && currentScene != lastScene) {
            // Check if BGM changed
            if (currentBGM != lastBGM) {
                // If current scene has no BGM, fade out
                if (currentBGM.empty() && currentChannel) {
                    FadeOutAndStop(1.0f);
                }
                // If we have BGM and it's different, ALWAYS fade between them
                else if (!currentBGM.empty()) {
                    // Fade from old to new music
                    if (currentChannel && !lastBGM.empty()) {
                        TransitionToBGM(currentBGM, 1.0f);  // 1 second fade
                    }
                    // No music playing, just start new BGM
                    else {
                        // Clear fade state
                        fadeActive = false;
                        fadeTime = 0.0f;
                        pendingBGM = "";

                        StartBGM(currentBGM);
                    }
                }

                lastBGM = currentBGM;
            }

            lastScene = currentScene;
        }
    }

    // *** Process fades LAST, after all scene logic ***
    UpdateFade(dt);
}

// ============================================================================
// Spatial Audio Implementation
// ============================================================================

/**
 * @brief Configures the FMOD system for 2D spatial audio.
 *
 * This function sets up the FMOD audio system to handle spatial audio
 * in a 2D environment. It configures 3D audio settings appropriately
 * for 2D games and initializes the listener's
 * position, velocity, and orientation to default values. This allows
 * sounds to have distance-based volume attenuation while maintaining
 * a 2D coordinate system.
 */
void AudioSystem::Configure2DSpatialAudio() {
    if (!mSystem) return;

    // Set 3D settings for 2D spatial audio
    // Parameters: doppler scale (0.0 for 2D), distance factor, rolloff scale
    FMOD_RESULT result = mSystem->set3DSettings(0.0f, 0.05f, 1.0f);
    if (result != FMOD_OK) {
        std::cerr << "[AudioSystem] Failed to set 3D settings!" << std::endl;
        return;
    }

    // init listener state and set
    listenerPos = { 0.0f, 0.0f, 0.0f };
    listenerVel = { 0.0f, 0.0f, 0.0f };
    listenerForward = { 0.0f, 0.0f, 1.0f };
    listenerUp = { 0.0f, 1.0f, 0.0f };

    result = mSystem->set3DListenerAttributes(
        0,                  // Listener index (0 for single listener)
        &listenerPos,
        &listenerVel,
        &listenerForward,
        &listenerUp
    );

    if (result != FMOD_OK) {
        std::cerr << "[AudioSystem] Failed to set listener attributes!" << std::endl;
        return;
    }
}

/**
 * @brief Sets the 2D position of the audio listener.
 *
 * This function updates the position of the audio listener in 2D space,
 * which is used to calculate spatial audio effects such as distance-based
 * volume attenuation. The Z coordinate is always set to 0 for 2D audio.
 *
 * @param x The X coordinate of the listener position.
 * @param y The Y coordinate of the listener position.
 */
void AudioSystem::SetListenerPosition(float x, float y) {
    if (!mSystem) return;

    // Update listener position (Z is always 0 for 2D)
    listenerPos.x = x;
    listenerPos.y = y;
    listenerPos.z = 0.0f;

    // Velocity is zero for static camera/player
    listenerVel.x = 0.0f;
    listenerVel.y = 0.0f;
    listenerVel.z = 0.0f;

    // Apply updated listener attributes to FMOD
    FMOD_RESULT result = mSystem->set3DListenerAttributes(
        0,
        &listenerPos,
        &listenerVel,
        &listenerForward,
        &listenerUp
    );

    if (result != FMOD_OK) {
        std::cerr << "[AudioSystem] Failed to update listener position!" << std::endl;
    }
}

/**
 * @brief Plays a sound at a specific 2D position with spatial audio effects.
 *
 * This function plays an audio asset at the specified 2D coordinates with
 * spatial audio positioning. Only if the asset is marked as spatial in the audio
 * cache, the sound's volume will be affected by its distance from the listener.
 *
 * @param assetName The name of the audio asset in the AudioCache.
 * @param x The X coordinate where the sound should be positioned.
 * @param y The Y coordinate where the sound should be positioned.
 * @param volume The base volume level for the sound (0.0 to 1.0).
 * @return FMOD::Channel* Pointer to the channel playing the sound, or nullptr
 *         if playback failed or the asset was not found.
 */
FMOD::Channel* AudioSystem::PlaySoundAt(const std::string& assetName, float x, float y, float volume) {
    if (!mSystem) return nullptr;

    // DON'T PLAY AUDIO IN EDIT MODE
    if (SceneManager::Get().IsInEditMode()) {
        return nullptr;
    }

    // DON'T PLAY SFX WHEN PAUSED (spatial sounds are typically SFX)
    if (SceneManager::Get().IsPaused()) {
        return nullptr;
    }

    // check if spatial is true and find distance
    AudioAsset* asset = AudioCache::GetAsset(assetName);
    if (!asset) {
        std::cerr << "[AudioSystem] Asset not found: " << assetName << std::endl;
        return nullptr;
    }

    // Get  loaded sound
    FMOD::Sound* sound = AudioCache::GetSound(assetName);
    if (!sound) {
        std::cerr << "[AudioSystem] Failed to get sound: " << assetName << std::endl;
        return nullptr;
    }

    // play normally without spatial sound if no need
    if (!asset->spatial) {
        return PlaySoundGetChannel(sound, false);
    }

    // For spatial sounds, set up 3D positioning
    // Play the sound from pasued
    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = mSystem->playSound(sound, nullptr, true, &channel);

    if (result != FMOD_OK || !channel) {
        std::cerr << "[AudioSystem] Failed to play sound: " << assetName << std::endl;
        return nullptr;
    }

    // Set 3D position
    FMOD_VECTOR soundPos = { x, y, 0.0f };  // Z is 0 for 2D
    FMOD_VECTOR soundVel = { 0.0f, 0.0f, 0.0f };

    result = channel->set3DAttributes(&soundPos, &soundVel);
    if (result != FMOD_OK) {
        std::cerr << "[AudioSystem] Failed to set 3D attributes for: " << assetName << std::endl;
    }

    result = channel->set3DMinMaxDistance(asset->minDistance, asset->maxDistance);
    if (result != FMOD_OK) {
        std::cerr << "[AudioSystem] Failed to set 3D distances for: " << assetName << std::endl;
    }

    // Set volume
    channel->setVolume(volume);

    // Unpause to start playback
    channel->setPaused(false);

    return channel;
}