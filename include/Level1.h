#pragma once
#include <iostream>
#ifndef LEVEL1_H
#define LEVEL1_H

class Level1
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

#endif // LEVEL1_H