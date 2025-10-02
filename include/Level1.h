/* ========================================================
 * @file	Level1.h
 * @author	Manfred Sim
 * @date	29 SEP 2025
 *
 * @brief	Level 1
 *
 * This header file contains the declaration of functions
 * used by the Level 1. While in Level 1, the game is in
 * its playing state.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include <iostream>
#ifndef LEVEL1_H
#define LEVEL1_H

 // Class for Level 1
class Level1
{
public:
	// Loads Level 1
	static void Load();

	// Initializes Level 1
	static void Init();

	// Updates Level 1
	static void Update();

	// Draws Level 1
	static void Draw();

	// Frees Level 1
	static void Free();

	// Unloads Level 1
	static void Unload();
};

#endif // LEVEL1_H