
#pragma once
#include "EntityManager.h"
#include "Render.h"
#include "Registry.h"
// i added names to them so the textures would load properly
Entity Create_Player(float x, float y, const std::string& name = "Rock");
Entity Create_Square(float x, float y, const std::string& name = "Square");
Entity Create_Circle(float x, float y, const std::string& name = "Circle");
// Entity Create_Ghoul(float x, float y);
