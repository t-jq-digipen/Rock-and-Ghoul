#include "Level1.h"
#include "GameStateManager.h"
#include "Input.h"

void Level1::Load()
{
	std::cout << "Level 1 Loaded!!!" << std::endl;
}

void Level1::Init()
{
	std::cout << "Level 1 Initialized!!!" << std::endl;
}

void Level1::Update()
{
	//std::cout << "Level 1 Updating!!!" << std::endl;

	// to test game state changes
	if (Input::IsKeyPressed(GLFW_KEY_M))
	{
		nextState = GS_MAIN_MENU;     // go to main menu state
	}
	else if (Input::IsKeyPressed(GLFW_KEY_R)) 
	{
		nextState = GS_RESTART;       // restart state
		std::cout << "Level 1 Restarted!!!" << std::endl;
	}
}

void Level1::Draw()
{
	//std::cout << "Level 1 Drawing!!!" << std::endl;
}

void Level1::Free()
{
	std::cout << "Level 1 Freed!!!" << std::endl;
}

void Level1::Unload()
{
	std::cout << "Level 1 Unloaded!!!" << std::endl;
}