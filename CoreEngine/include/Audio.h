/* ========================================================
 * @file    Audio.h
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    26 SEP 2025
 *
 * @brief   Header file declaring the AudioSystem class
 *
 * This header file contains the declaration of the AudioSystem
 * class. The class initializes the audio system, loading
 * and playing sounds, managing audio channels for playback
 * control, and handling button sound effects. It also declares
 * functions for spatial audio positioning.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#ifndef AUDIO_H
#define AUDIO_H
#include <string>
#include <unordered_map>
#include "fmod.hpp"

 /**
  * @class AudioSystem
  * @brief Low-level audio system interface using FMOD.
  *
  * The AudioSystem class provides static methods for loading, playing, and controlling audio.
  * The class includes a flexible button sound system that maps button types to specific sound effects.
  */
class AudioSystem {
public:
    static void SetMasterVolume(float vol) { masterVolume = vol; }
    static float GetMasterVolume() { return masterVolume; }
    /**
     * @brief Initializes the FMOD audio system.
     *
     * This function creates and initializes the FMOD audio system and loads default
     * button sounds for the application.
     *
     * @return bool True if FMOD was successfully initialized, false if
     *         an error occurred during system creation or initialization.
     */
    static bool Init();

    /**
     * @brief Closes and cleans up the FMOD audio system.
     *
     * This function releases all button sounds, closes the FMOD system,
     * and frees all associated resources.
     */
    static void Close();

    /**
     * @brief Updates the FMOD audio system.
     *
     * This function must be called once per frame to update FMOD's internal
     * state, including channel management, streaming, and effects processing.
     */
    static void Update();

    /**
     * @brief Loads a sound file from disk with specified FMOD mode flags.
     *
     * This function loads an audio file using FMOD with custom playback settings
     * for 2D/3D positioning, looping, and distance attenuation.
     *
     * @param filename The file path to the audio file to load
     * @param mode FMOD mode flags (e.g., FMOD_2D, FMOD_3D, FMOD_LOOP_NORMAL, FMOD_LOOP_OFF).
     *             Multiple flags can be combined using bitwise OR (|).
     * @return FMOD::Sound* Pointer to the loaded sound object, or nullptr if loading failed.
     */
    static FMOD::Sound* LoadSound(const std::string& filename, FMOD_MODE mode);

    /**
     * @brief Plays a sound immediately without returning a channel handle.
     *
     * This function plays the specified sound at full volume without
     * channel control.
     *
     * @param sound Pointer to the FMOD::Sound object to play.
     */
    static void PlaySound(FMOD::Sound* sound);

    /**
     * @brief Stops all currently playing sounds.
     *
     * This function stops all active audio channels by closing and
     * reinitializing the FMOD system.
     *
     * @warning This will stop ALL audio including music and sound effects.
     */
    static void StopAll();

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
    static FMOD::Channel* PlaySoundGetChannel(FMOD::Sound* sound, bool paused = false);

    /**
     * @brief Sets the playback position of a channel.
     *
     * This function seeks to a specific position in the audio stream.
     *
     * @param channel Pointer to the FMOD::Channel whose position should be set.
     * @param ms The position in milliseconds from the start of the sound.
     */
    static void SetChannelPosition(FMOD::Channel* channel, unsigned int ms);

    /**
     * @brief Gets the current playback position of a channel.
     *
     * This function retrieves the current position of the audio playhead
     * in milliseconds from the start of the sound
     *
     * @param channel Pointer to the FMOD::Channel to query.
     * @return unsigned int The current playback position in milliseconds.
     */
    static unsigned int GetChannelPosition(FMOD::Channel* channel);

    /**
     * @brief Sets the pause state of a channel.
     *
     * This function pauses or resumes playback on a specific channel.
     * When paused, can be resumed from the same point.
     *
     * @param channel Pointer to the FMOD::Channel to pause or unpause.
     * @param paused If true, the channel is paused. If false, playback resumes.
     */
    static void SetChannelPaused(FMOD::Channel* channel, bool paused);

    /**
     * @enum ButtonType
     * @brief Enumeration of button types for UI sound effects.
     *
     * This enum defines different button types in the application, each
     * of which can have a unique sound effect associated with it.
     */
    enum class ButtonType {
        PLAY,    ///< Play button sound effect
        QUIT,    ///< Quit button sound effect
        CLICK,   ///< Any UI clicks
        COUNT    ///< For ImGui
    };

    /**
     * @brief Plays the sound effect associated with a specific button type.
     *
     * This function looks up and plays the sound registered for the given
     * button type.
     *
     * @param type The type of button whose sound effect should be played,
     *             as defined in the ButtonType enum.
     */
    static void PlayButtonSound(ButtonType type);

    /**
     * @brief Registers a button sound effect by loading it from a file path.
     *
     * This function loads a sound file from the specified path and associates
     * it with a button type.
     *
     * @param type The ButtonType to associate with this sound effect.
     * @param soundPath The file path to the sound file
     */
    static void RegisterButtonSound(ButtonType type, const std::string& soundPath);

    /**
     * @brief Registers a button sound effect using an existing FMOD::Sound pointer.
     *
     * This overload allows registration of a button sound using an already
     * loaded FMOD::Sound object.
     *
     * @param type The ButtonType to associate with this sound effect.
     * @param sound Pointer to an already loaded FMOD::Sound object.
     */
    static void RegisterButtonSound(ButtonType type, FMOD::Sound* sound);

    // spatial audio funcs
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
    static void SetListenerPosition(float x, float y);
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
    static FMOD::Channel* PlaySoundAt(const std::string& assetName, float x, float y, float volume = 1.0f);
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
    static void Configure2DSpatialAudio();

private:
    static FMOD::System* mSystem;
    static float masterVolume;
    static std::unordered_map<ButtonType, FMOD::Sound*> buttonSounds;

    /**
     * @brief Loads default button sound effects.
     *
     * This function registers default sound effects for different button types
     * in the application.
     */
    static void LoadDefaultButtonSounds();
};

#endif // AUDIO_H