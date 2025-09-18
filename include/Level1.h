#pragma once
#ifndef LEVEL1_H
#define LEVEL1_H

class Level1
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

#endif // LEVEL1_H