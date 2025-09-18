#pragma once
#include <iostream>
#ifndef MAINMENU_H
#define MAINMENU_H

class MainMenu
{
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