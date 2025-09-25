#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Level1.h"
#include "Input.h"
#include "Fretboard.h"
#include "Render.h"
#include "Spells.h"
#include "glapp.h"
#include "CoreEngine.h"
#include "RenderSystem.h"
#include "BossGhoul.h"
#include "EntityCreation.h"

static SpellManager spellManager;
static FretBoard* fretboard = nullptr;
extern BossGhoulSystem* bossGhoulSystem;

GLuint bgTexture = 0;
Entity ECSplayer{};


void Level1::Load()
{
	std::cout << "Level 1 Loaded!!!" << std::endl;
}

void Level1::Init()
{
	std::cout << "Level 1 Initialized!!!" << std::endl;
	Render& renderer = Render::GetInstance();
	renderer.Init();


	// 3. NOW load textures (OpenGL is ready)
	bgTexture = renderer.LoadTexture("../Assets/background.jpg");
	spellManager.LoadTextures();

	// 4. Create FretBoard after OpenGL is ready
	if (!fretboard) {
		fretboard = new FretBoard(0, 0, 800, 200, spellManager);
	}
	fretboard->LoadTextures();
	fretboard->StartGame();

	// 5. Create entities
	Entity rock = Create_Player(200.f, 500.f);
	Entity ghoul = Create_Ghoul(500.f, 500.f);


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
	if (fretboard) {
		fretboard->PlayerInput();
	}

}

void Level1::Draw()
{	
	Render& renderer = Render::GetInstance();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	if (bgTexture) {
		renderer.DrawTexturedQuad(
			{ GLApp::width * 0.5f, GLApp::height * 0.5f },   // center of screen
			{ (float)GLApp::width, (float)GLApp::height },   // full screen size
			bgTexture
		);
	}

	if (bossGhoulSystem) {
		bossGhoulSystem->Draw(renderer);
	}

	if (fretboard) {
		fretboard->Draw(renderer);
	}
	spellManager.Draw(renderer);

	
}

void Level1::Free()
{
	std::cout << "Level 1 Freed!!!" << std::endl;

	Remove_Entity(ECSplayer);
}

void Level1::Unload()
{
	delete fretboard;
	fretboard = nullptr;
	if (bgTexture) {
		glDeleteTextures(1, &bgTexture);
		bgTexture = 0;
	}
	std::cout << "Level 1 Unloaded!!!" << std::endl;
}

