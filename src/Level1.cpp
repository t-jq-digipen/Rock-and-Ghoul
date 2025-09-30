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
#include "EntityCreation.h"
#include "JsonLoad.h"
#include "FilePath.h"


static FretBoard* fretboard = nullptr;


GLuint bgTexture = 0;
Entity ECSplayer{};
Entity rock;
Entity square;
Entity roundy;
Entity waffle;
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

	// 4. Create FretBoard after OpenGL is ready
	if (!fretboard) {
		fretboard = new FretBoard(0, 0, 800, 200);
	}
	fretboard->LoadTextures();
	fretboard->StartGame();

	// JQ COMMENT OUT HAHAHA
	// 5. Create entities
	/*rock = Create_Player(200.f, 500.f);
	square = Create_Square(800.f, 500.f);
	roundy = Create_Circle(500.f, 500.f);*/
	waffle = Create_Waffle(400.f, 400.f);
	rock = Create_Player(200.f, 500.f);
}

void Level1::Update()
{
	//std::cout << "Level 1 Updating!!!" << std::endl;
	// Save scene on F5
	if (Input::IsKeyPressed(GLFW_KEY_F5)) {
		JsonLoad::SaveCurrentScene(FilePath::LEVEL1_SCENE, "Level1");
		std::cout << "Scene saved!" << std::endl;
	}

	// Load scene on F9
	if (Input::IsKeyPressed(GLFW_KEY_F9)) {
		// IMPORTANT: Clear existing entities first
		JsonLoad::ClearAllEntities();

		if (JsonLoad::LoadScene(FilePath::LEVEL1_SCENE)) {
			std::cout << "Scene loaded successfully!" << std::endl;
		}
		else {
			std::cout << "Failed to load scene!" << std::endl;
			// Recreate default entities if load failed
			rock = Create_Player(200.f, 500.f);
			square = Create_Square(800.f, 500.f);
			roundy = Create_Circle(500.f, 500.f);
		}
	}
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


	if (fretboard) {
		fretboard->Draw(renderer);
		fretboard->GetSpellManager().Draw(renderer);
	}

	
}

void Level1::Free()
{
	std::cout << "Level 1 Freed!!!" << std::endl;
	JsonLoad::ClearAllEntities();
	Remove_Entity(rock);
	Remove_Entity(ECSplayer);
	Remove_Entity(waffle);

}

void Level1::Unload()
{
	delete fretboard;
	fretboard = nullptr;
	/*if (bgTexture) {
		glDeleteTextures(1, &bgTexture);
		bgTexture = 0;
	}*/

	Render::GetInstance().UnloadTexture(bgTexture);
	//Render::GetInstance().ClearAllTextures();
	std::cout << "Level 1 Unloaded!!!" << std::endl;
}

