#pragma once
#include <iostream>
#include "AudioSystem.h"
#ifndef MAINMENU_H
#define MAINMENU_H

class MainMenu
{
private:
    static AudioManager* audio;
public:
 /**
 * @brief Sets the AudioManager instance for the MainMenu.
 *
 * This function assigns an AudioManager pointer to the MainMenu, allowing
 * the menu to access audio playback functionality. The AudioManager is used
 * to play sound effects during certain instances.
 *
 * @param audioMgr Pointer to the AudioManager instance to be used by this
 *                 MainMenu.
 */
    static void SetAudioManager(AudioManager* audioMgr);

 /**
 * @brief Plays the button click sound effect for the Play button.
 *
 * This function triggers the playback of a button click sound specifically
 * for the Play button interaction. If no AudioManager has been set, the
 * function returns without playing any sound.
 *
 */
    static void PlayButtonClick();
public:
    // Loads main menu
    static void Load();
    // Initializes main menu
    static void Init();
    // Updates main menu
    static void Update();
    // Draws main menu
    static void Draw();
    // Frees main menu
    static void Free();
    // Unloads main menu
    static void Unload();
};

#endif // MAINMENU_H