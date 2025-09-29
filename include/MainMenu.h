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
    static void SetAudioManager(AudioManager* audioMgr);
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