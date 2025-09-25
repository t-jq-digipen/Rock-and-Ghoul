#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Level1.h"
#include "Input.h"
#include "glapp.h"
#include "CoreEngine.h"
#include "Singleton.h"
#include "EntityCreation.h"

Entity ECSplayer{};


void Level1::Load()
{
	std::cout << "Level 1 Loaded!!!" << std::endl;
	GameContext& ctx = GameContext::Get();
	ctx.bgTexture = Render::LoadTexture("../Assets/background.jpg");
	ctx.spellManager.LoadTextures();

	if (ctx.currentFretboard) {
		ctx.currentFretboard->LoadTextures();
		ctx.currentFretboard->StartGame();
	}
}

void Level1::Init()
{
	std::cout << "Level 1 Initialized!!!" << std::endl;
	GameContext& ctx = GameContext::Get();

	ctx.renderer.Init();
	ctx.spellManager.LoadTextures();

	if (ctx.currentFretboard) {
		ctx.currentFretboard->LoadTextures();
		ctx.currentFretboard->StartGame();
	}


	Entity rock = Create_Player(200.f, 500.f);

	Entity ghoul = Create_Ghoul(500.f, 500.f);
}

void Level1::Update()
{
	//std::cout << "Level 1 Updating!!!" << std::endl;
	GameContext& ctx = GameContext::Get();
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
	if (ctx.currentFretboard) {
		ctx.currentFretboard->PlayerInput();
	}

}

void Level1::Draw()
{	
	GameContext& ctx = GameContext::Get();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	if (ctx.bgTexture) {
		ctx.renderer.DrawTexturedQuad(
			{ GLApp::width * 0.5f, GLApp::height * 0.5f },
			{ (float)GLApp::width, (float)GLApp::height },
			ctx.bgTexture
		);
	}
	if (ctx.renderSystem) {
		ctx.renderSystem->Update(0.0f);
	}
	ctx.bossGhoulSystem.Draw(ctx.renderer);
	//std::cout << "Level 1 Drawing!!!" << std::endl;
	if (ctx.currentFretboard) {
		ctx.currentFretboard->Draw(ctx.renderer);
	}
	ctx.spellManager.Draw(ctx.renderer);

	
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

