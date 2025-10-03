
#pragma once
#include "EntityManager.h"
#include "Render.h"
#include "Registry.h"
//Entity Create_Player(float x, float y);
//Entity Create_Square(float x, float y);
//Entity Create_Circle(float x, float y);
// Entity Create_Ghoul(float x, float y);
#include "Component.h"
Entity Create_Player(float const& spawnx, float const& spawny);
Entity Create_Waffle(float const& spawnx, float const& spawny);
Entity Create_Shape(float const& spawnx, float const& spawny, ShapeType shapeType, ControlScheme controls);
void GenerateInitialPrefabs();
