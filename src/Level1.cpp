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

#include "CrashHandler.h"

static FretBoard* fretboard = nullptr;


GLuint bgTexture = 0;
Entity ECSplayer{};
Entity rock;
Entity square;
Entity roundy;
Entity waffle;
void Level1::Load()
{
	Render& renderer = Render::GetInstance();
	CrashHandler::Safe("Level1", "Load", [&] {
		std::cout << "Level 1 Loaded!!!" << std::endl;

		bgTexture = renderer.LoadTexture("../Assets/background.jpg");
		CrashHandler::LogIfZeroTexture(bgTexture, "../Assets/background.jpg");
	});
}

void Level1::Init()
{
	Render& renderer = Render::GetInstance();
	CrashHandler::Safe("Level1", "Init", [&] {
		std::cout << "Level 1 Initialized!!!" << std::endl;
		renderer.Init();

		// 1. Create FretBoard after OpenGL is ready
		if (!fretboard) {
			fretboard = new FretBoard(0, 0, 800, 200);
		}
		fretboard->LoadTextures();
		fretboard->StartGame();

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
	if (!JsonLoad::LoadScene(FilePath::LEVEL1_SCENE)) {
		
		// Create with names (updated for json)
		waffle = Create_Waffle(400.f, 400.f);
		rock = Create_Player(200.f, 500.f);
		square = Create_Shape(600.f, 500.f, SHAPE_RECTANGLE, CS_IJKL);
		roundy = Create_Shape(800.f, 500.f, SHAPE_CIRCLE, CS_ARROWS);
	}
	});
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
		JsonLoad::ClearAllEntities();

		if (JsonLoad::LoadScene(FilePath::LEVEL1_SCENE)) {
			std::cout << "Scene loaded successfully!" << std::endl;

			// Find waffle by checking for its sprite texture
			for (Entity e = 0; e < 10; ++e) {
				if (!EntityManager::get().isAlive(e)) continue;

				if (Has_Component(e, CT_Sprite)) {
					auto& s = Get_SpriteComponent(e);
					if (s.Get_TexturePath().find("Waffle") != std::string::npos) {
						std::cout << "Found waffle at Entity " << e << std::endl;
						waffle = e;  // Update the waffle variable
						break;
					}
				}
			}
		}
	}

	CrashHandler::Safe("Level1", "Update", [&] {
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
	});
}

void Level1::Draw()
{	
	Render& renderer = Render::GetInstance();
	CrashHandler::Safe("Level1", "Draw", [&] {
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
	});
}

void Level1::Free()
{
	std::cout << "Level 1 Freed!!!" << std::endl;
	JsonLoad::ClearAllEntities();
	//Render::GetInstance().ClearAllTextures();
	if (bgTexture) {
		Render::GetInstance().UnloadTexture(bgTexture);
		bgTexture = 0;
	}
	//Remove_Entity(rock);
	//Remove_Entity(ECSplayer);
	//Remove_Entity(waffle);

	CrashHandler::Safe("Level1", "Free", [&] {
		std::cout << "Level 1 Freed!!!" << std::endl;
		Render::GetInstance().ClearAllTextures();
	});
}

void Level1::Unload()
{

	// Clean up background texture
	Render::GetInstance().ClearAllTextures();
	//Render::GetInstance().ClearAllTextures();
	std::cout << "Level 1 Unloaded!!!" << std::endl;
	Render& renderer = Render::GetInstance();
	CrashHandler::Safe("Level1", "Unload", [&] {
		if (fretboard) {
		// FretBoard should clean its own textures before deletion
		delete fretboard;
		fretboard = nullptr;
	}
		/*if (bgTexture) {
			glDeleteTextures(1, &bgTexture);
			bgTexture = 0;
		}*/

		renderer.UnloadTexture(bgTexture);
		//Render::GetInstance().ClearAllTextures();
		std::cout << "Level 1 Unloaded!!!" << std::endl;
	});
}

