#pragma once
#include <iostream>
#ifndef LEVEL2_H
#define LEVEL2_H

class Level2
{
public:
	// Loads level
	static void Load();

	// Initializes level
	static void Init();

	// Updates level
	static void Update();

	// Draws level
	static void Draw();

	// Frees level
	static void Free();

	// Unloads level
	static void Unload();
};

#endif // LEVEL2_H