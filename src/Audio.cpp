/* ========================================================
 * @file    Audio.cpp
 * @author  Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    26 SEP 2025
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
 * to specific sound effects.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "audio.h"
#include "AudioSystem.h"
#include "CoreEngine.h"
#include <iostream>

 // Static members
FMOD::System* AudioSystem::mSystem = nullptr;

// button sound storage
std::unordered_map<AudioSystem::ButtonType, FMOD::Sound*> AudioSystem::buttonSounds;

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
    // Create the FMOD system object
    result = mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: Failed to initialize system (" << result << ")" << std::endl;
        return false;
    }
    // Load the default button click sounds
    LoadDefaultButtonSounds();

    std::cout << "FMOD Audio initialized successfully!" << std::endl;
    return true;
}

/**
 * @brief Loads default button sound effects.
 *
 * This function registers default sound effects for different button types
 * in the application.
 */
void AudioSystem::LoadDefaultButtonSounds() {
    // Register default button sounds based on your ButtonType enum
    RegisterButtonSound(ButtonType::PLAY, "../Assets/music/playclick.mp3");
    RegisterButtonSound(ButtonType::QUIT, "../Assets/music/quitclick.mp3");

    std::cout << "Default button sounds loaded!" << std::endl;
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
        PlaySound(it->second);
        std::cout << "Playing button sound for type: " << static_cast<int>(type) << std::endl;
    }
    else {
        std::cerr << "Warning: Button sound not found for type " << static_cast<int>(type) << std::endl;

        // Fallback: try to play PLAY sound if the requested type isn't found
        if (type != ButtonType::PLAY) {
            auto playIt = buttonSounds.find(ButtonType::PLAY);
            if (playIt != buttonSounds.end() && playIt->second) {
                PlaySound(playIt->second);
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
    //RegisterButtonSound(ButtonType::PLAY, "../Assets/music/playclick.mp3");
    //RegisterButtonSound(ButtonType::QUIT, "../Assets/music/quitclick.mp3");
    FMOD::Sound* sound = LoadSound(soundPath, false);
    if (sound) {
        buttonSounds[type] = sound;
        std::cout << "Registered button sound for type " << static_cast<int>(type)
            << " from: " << soundPath << std::endl;
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

    //FMOD::Sound* clickSound = LoadSound("../Assets/music/click.mp3", false);
    //RegisterButtonSound(ButtonType::PLAY, clickSound);
    //RegisterButtonSound(ButtonType::OPTIONS, clickSound);
    if (sound) {
        buttonSounds[type] = sound;
        std::cout << "Registered button sound for type " << static_cast<int>(type) << std::endl;
    }
}

/**
 * @brief Closes and cleans up the FMOD audio system.
 *
 * This function releases all button sounds, closes the FMOD system,
 * and frees all associated resources.
 */
void AudioSystem::Close() {
    for (auto& pair : buttonSounds) { // cleaning up button sounds
        if (pair.second) {
            pair.second->release(); // free FMOD sound memory
        }
    }
    buttonSounds.clear();

    if (mSystem) { // Shut down and release the FMOD system itself
        mSystem->close();
        mSystem->release();
        mSystem = nullptr;
    }
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
 * @brief Loads a sound file from disk.
 *
 * This function loads an audio file using FMOD and returns a pointer
 * to the loaded sound.
 *
 * @param filename The file path to the audio file to load
 * @param loop If true, the sound will loop continuously when played.
 *             If false (default), the sound will play once and stop.
 * @return FMOD::Sound* Pointer to the loaded sound object, or nullptr
 *         if loading failed (e.g., file not found, unsupported format).
 */
FMOD::Sound* AudioSystem::LoadSound(const std::string& filename, bool loop) {
    if (!mSystem) return nullptr;

    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;

    FMOD_RESULT result = mSystem->createSound(filename.c_str(), mode, nullptr, &sound);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: Failed to load sound (" << filename << ")" << std::endl;
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
    mSystem->close();
    mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
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
    AudioSystem::Init();

    // preload music tracks
    mainMenuMusic = Load("../Assets/music/mainmenu.wav", true);
    level1Music = Load("../Assets/music/level1.wav", true);

    // start main menu music
    if (mainMenuMusic) {
        PlayMusic(mainMenuMusic);
        std::cout << "[AudioManager] Main menu music started\n";
    }

    lastState = GameState::GS_MAIN_MENU;
}

/**
 * @brief Updates the AudioManager each frame.
 *
 * This function updates the FMOD system, processes active crossfades,
 * and handles music transitions based on game state changes.
 *
 * @param dt Delta time in seconds.
 */
void AudioManager::Update(float dt) {
    AudioSystem::Update();

    UpdateCrossfade(dt);

    // STATE BASED MUSIC CHANGES
    GameState currentState = GSM_CurrentState();
    HandleMusicForState(currentState, lastState);
    lastState = currentState;
}

/**
 * @brief Updates the active crossfade effect between music tracks.
 *
 * This function handles the transition between two music tracks
 * by gradually decreasing the volume of the current track while increasing
 * the volume of the next track. When crossfade completes, current
 * channel is stopped and the next channel becomes the current channel.
 *
 * @param dt Delta time in seconds.
 */
void AudioManager::UpdateCrossfade(float dt) {
    if (!crossfadeActive) return;

    crossfadeTime += dt;
    float t = crossfadeTime / crossfadeDuration;
    if (t > 1.0f) t = 1.0f;

    if (currentChannel)
        currentChannel->setVolume(musicVol * (1.0f - t));  // Fade from musicVol to 0
    if (nextChannel)
        nextChannel->setVolume(musicVol * t);              // Fade from 0 to musicVol

    if (t >= 1.0f) {
        if (currentChannel) currentChannel->stop();
        currentChannel = nextChannel;
        nextChannel = nullptr;
        crossfadeActive = false;
        std::cout << "[AudioManager] Crossfade completed\n";
    }
}

/**
 * @brief Handles music transitions based on game state changes.
 *
 * This function detects when the game state changes and triggers
 * appropriate music transitions.
 *
 * @param currentState Current game state.
 * @param previousState The game state from previous frame.
 */
void AudioManager::HandleMusicForState(GameState currentState, GameState previousState) {
    // DETECT TRANSITION TO LEVEL 1!
    if (currentState == GameState::GS_LEVEL1 && previousState != GameState::GS_LEVEL1) {
        std::cout << "[AudioManager] Level 1 started - crossfading music...\n";
        if (level1Music) {
            CrossfadeTo(level1Music, 4.0f);
        }
        else {
            std::cerr << "[AudioManager] Error: Level 1 music not loaded!\n";
        }
    }
}

/**
 * @brief Shuts down the AudioManager and releases all audio resources.
 */
void AudioManager::Shutdown() {
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
 * @brief Initiates a smooth crossfade transition to a new music track.
 *
 * This function starts playing a new music track on a separate channel
 * and begins a gradual volume transition from the current track to the
 * new track over the specified duration.
 *
 * @param newTrack Pointer to the FMOD::Sound object for the new music track.
 * @param duration The duration of the crossfade effect in seconds.
 */
void AudioManager::CrossfadeTo(FMOD::Sound* newTrack, float duration) {
    if (!newTrack) return;

    nextChannel = PlayGetChannel(newTrack, true);
    if (!nextChannel) return;

    nextChannel->setVolume(0.0f);
    nextChannel->setPaused(false);

    crossfadeTime = 0.0f;
    crossfadeDuration = duration;
    crossfadeActive = true;

    std::cout << "[AudioManager] Starting crossfade (" << duration << "s)\n";
}