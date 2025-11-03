/* ========================================================
 * @file		Level2.h
 * @author		Manfred Sim | manfred.sim@digipen.edu
 * @date		01 NOV 2025
 *
 * @brief	Level 2
 *
 * This header file contains the declaration of functions
 * used by the Level 2. While in Level 2, the game is in
 * its playing state.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include <iostream>
#ifndef LEVEL2_H
#define LEVEL2_H

 // Class for Level 2
class Level2
{
public:
	// Loads Level 2
	static void Load();

	// Initializes Level 2
	static void Init();

	// Updates Level 2
	static void Update();

	// Draws Level 2
	static void Draw();

	// Frees Level 2
	static void Free();

	// Unloads Level 2
	static void Unload();
};

#endif // LEVEL2_H