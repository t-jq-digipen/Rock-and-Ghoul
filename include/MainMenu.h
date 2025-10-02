/* ========================================================
 * @file	MainMenu.h
 * @author	Manfred Sim
 * @date	29 SEP 2025
 *
 * @brief	Main Menu
 *
 * This header file contains the declaration of functions 
 * used by the main menu. While in the main menu, the player 
 * may enter the game by clicking the PLAY button or quit the 
 * application by clicking the QUIT button.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include <iostream>
#include "AudioSystem.h"
#ifndef MAINMENU_H
#define MAINMENU_H

// Class for Main Menu
class MainMenu
{
private:
    // Audio Manager
    static AudioManager* audio;

public:
    // Set Audio Manager
    static void SetAudioManager(AudioManager* audioMgr);

    // Play Audio When Button Clicked
    static void PlayButtonClick();
public:
    // Loads Main Menu
    static void Load();

    // Initializes Main Menu
    static void Init();

    // Updates Main Menu
    static void Update();

    // Draws Main Menu
    static void Draw();

    // Frees Main Menu
    static void Free();

    // Unloads Main Menu
    static void Unload();
};

#endif // MAINMENU_H