#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Level1.h"
#include "Input.h"
#include "Fretboard.h"
#include "Render.h"
#include "Spells.h"

#include "CoreEngine.h"

static Render renderer;
static SpellManager spellManager;
static FretBoard fretboard(0, 0, 800, 200, spellManager);


Entity ECSplayer{};


void Level1::Load()
{
	std::cout << "Level 1 Loaded!!!" << std::endl;
}

void Level1::Init()
{
	std::cout << "Level 1 Initialized!!!" << std::endl;
	renderer.Init();
	fretboard.LoadTextures();
	fretboard.StartGame();
	spellManager.LoadTextures();

	ECSplayer = Create_Entity();
	Add_Component(ECSplayer, TransformComponent{ {0.f, 0.f}, {50.f, 50.f} });
	Add_Component(ECSplayer, PhysicsComponent{});
	Add_Component(ECSplayer, TagComponent{ T_PLAYER });
}

void Level1::Update()
{
	//std::cout << "Level 1 Updating!!!" << std::endl;

	// to test game state changes
	if (Input::IsKeyPressed(GLFW_KEY_M))
	{
		GSM_SetNextState(GameState::GS_MAIN_MENU);
		//nextState = GameState::GS_MAIN_MENU;     // go to main menu state
	}
	else if (Input::IsKeyPressed(GLFW_KEY_R)) 
	{
		GSM_SetNextState(GameState::GS_RESTART);
		//nextState = GameState::GS_RESTART;       // restart state
		std::cout << "Level 1 Restarted!!!" << std::endl;
	}
	fretboard.PlayerInput();

}

void Level1::Draw()
{
	//std::cout << "Level 1 Drawing!!!" << std::endl;
	fretboard.Draw(renderer);
	spellManager.Draw(renderer);
}

void Level1::Free()
{
	std::cout << "Level 1 Freed!!!" << std::endl;

	Remove_Entity(ECSplayer);
}

void Level1::Unload()
{
	std::cout << "Level 1 Unloaded!!!" << std::endl;
}

