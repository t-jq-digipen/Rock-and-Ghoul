/* ========================================================
 * @file		Level1.cpp
 * @author		Manfred Sim | manfred.sim@digipen.edu
 * @date		29 SEP 2025
 *
 * @brief	Level 1
 *
 * This source file contains the implementation of functions
 * used by the Level 1. While in Level 1, the game is in
 * its playing state.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

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
constexpr float LEVEL_WIDTH = 1920.f;
constexpr float LEVEL_HEIGHT = 1080.f;
constexpr float WALL_THICKNESS = 50.f;


// Background Texture
GLuint bgTexture = 0;

// Entities for Level 1
Entity ECSplayer{};
Entity rock;
Entity square;
Entity roundy;
Entity waffle;

// Registry ref
Registry& registry = Registry::Get();

// Loads Level 1
void Level1::Load()
{
	Render& renderer = Render::GetInstance();
	CrashHandler::Safe("Level1", "Load", [&] {
		std::cout << "Level 1 Loaded!!!" << std::endl;

		bgTexture = renderer.LoadTexture("../Assets/background.jpg");
		CrashHandler::LogIfZeroTexture(bgTexture, "../Assets/background.jpg");
		});

	// ===== FIX: Ensure prefabs folder exists =====
	std::cout << "\n=== CHECKING PREFABS FOLDER ===" << std::endl;
	std::cout << "Prefabs folder path: " << FilePath::PREFABS_FOLDER << std::endl;

	if (!std::filesystem::exists(FilePath::PREFABS_FOLDER)) {
		std::cout << "FOLDER DOESNT EXIST, CREATING..." << std::endl;
		try {
			std::filesystem::create_directories(FilePath::PREFABS_FOLDER);
			std::cout << "Created prefabs directory successfully!" << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "ERROR: Failed to create prefabs directory: " << e.what() << std::endl;
		}
	}

	// Check if individual prefab files exist
	bool needsGeneration = false;
	std::vector<std::string> missingPrefabs;

	if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "Player.json")) {
		needsGeneration = true;
		missingPrefabs.push_back("Player.json");
	}
	if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "Waffle.json")) {
		needsGeneration = true;
		missingPrefabs.push_back("Waffle.json");
	}
	if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "Rectangle.json")) {
		needsGeneration = true;
		missingPrefabs.push_back("Rectangle.json");
	}
	if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "Circle.json")) {
		needsGeneration = true;
		missingPrefabs.push_back("Circle.json");
	}

	if (needsGeneration) {
		std::cout << "=== MISSING PREFABS ===" << std::endl;
		for (const auto& prefab : missingPrefabs) {
			std::cout << "  - " << prefab << std::endl;
		}
		std::cout << "GENERATING PREFABS NOW..." << std::endl;
		GenerateInitialPrefabs(false);

		// ===== FIX: Verify prefabs were actually created =====
		std::cout << "\n=== VERIFYING PREFAB GENERATION ===" << std::endl;
		bool allGenerated = true;
		if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "Player.json")) {
			std::cerr << "ERROR: Player.json was NOT generated!" << std::endl;
			allGenerated = false;
		}
		if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "Waffle.json")) {
			std::cerr << "ERROR: Waffle.json was NOT generated!" << std::endl;
			allGenerated = false;
		}
		if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "Rectangle.json")) {
			std::cerr << "ERROR: Rectangle.json was NOT generated!" << std::endl;
			allGenerated = false;
		}
		if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "Circle.json")) {
			std::cerr << "ERROR: Circle.json was NOT generated!" << std::endl;
			allGenerated = false;
		}

		if (allGenerated) {
			std::cout << "SUCCESS: All prefabs generated correctly!" << std::endl;
		}
		else {
			std::cerr << "WARNING: Some prefabs failed to generate. Check error messages above." << std::endl;
		}
	}
	else {
		std::cout << "All prefabs already exist!" << std::endl;
	}
	std::cout << "==============================\n" << std::endl;
}

// Initializes Level 1
void Level1::Init()
{
	Render& renderer = Render::GetInstance();
	CrashHandler::Safe("Level1", "Init", [&] {
		std::cout << "Level 1 Initialized!!!" << std::endl;
		// 1. Initialize the Renderer
		renderer.Init();

		// 2. Create FretBoard after OpenGL is ready
		if (!fretboard) {
			fretboard = new FretBoard(0, 0, 800, 200);
		}
		fretboard->LoadTextures();
		fretboard->StartGame();

		// 3. NOW load textures (OpenGL is ready)
		bgTexture = renderer.LoadTexture("../Assets/background.jpg");

		// 4. Create FretBoard after OpenGL is ready (duplicate - can remove)
		if (!fretboard) {
			fretboard = new FretBoard(0, 0, 800, 200);
		}
		fretboard->LoadTextures();
		fretboard->StartGame();

		// 5. Create entities
		// If there is no pre-existing/saved scene, it will load from prefabs
		// Otherwise, it will load the saved scene
		std::cout << "\n=== LOADING ENTITIES ===" << std::endl;
		bool sceneLoaded = false;
		if (std::filesystem::exists(FilePath::LEVEL1_SCENE)) {
			std::cout << "Found level1.json, attempting to load..." << std::endl;
			sceneLoaded = JsonLoad::LoadScene(FilePath::LEVEL1_SCENE);
			if (sceneLoaded) {
				std::cout << "Scene loaded successfully from level1.json!" << std::endl;
			}
			else {
				std::cout << "Failed to load scene from level1.json, will create from prefabs instead." << std::endl;
			}
		}
		else {
			std::cout << "No level1.json found, will create entities from prefabs." << std::endl;
		}

		if (!sceneLoaded) {
			// ===== FIX: Better error handling when creating from prefabs =====
			std::cout << "Creating fresh entities from prefabs..." << std::endl;

			waffle = Create_Waffle(400.f, 400.f);
			if (waffle == 0) {
				std::cerr << "ERROR: Failed to create Waffle! Check if Waffle.json exists." << std::endl;
			}
			else {
				std::cout << "Created Waffle (Entity " << waffle << ")" << std::endl;
			}

			rock = Create_Player(200.f, 500.f);
			if (rock == 0) {
				std::cerr << "ERROR: Failed to create Player! Check if Player.json exists." << std::endl;
			}
			else {
				std::cout << "Created Player (Entity " << rock << ")" << std::endl;
			}

			square = Create_Shape(600.f, 500.f, SHAPE_RECTANGLE, CS_IJKL);
			if (square == 0) {
				std::cerr << "ERROR: Failed to create Rectangle! Check if Rectangle.json exists." << std::endl;
			}
			else {
				std::cout << "Created Rectangle (Entity " << square << ")" << std::endl;
			}

			roundy = Create_Shape(800.f, 500.f, SHAPE_CIRCLE, CS_ARROWS);
			if (roundy == 0) {
				std::cerr << "ERROR: Failed to create Circle! Check if Circle.json exists." << std::endl;
			}
			else {
				std::cout << "Created Circle (Entity " << roundy << ")" << std::endl;
			}

			std::cout << "Finished creating entities from prefabs." << std::endl;	
			Create_Wall(LEVEL_WIDTH / 2, LEVEL_HEIGHT - WALL_THICKNESS / 2, LEVEL_WIDTH, WALL_THICKNESS);

			// Bottom wall
			Create_Wall(LEVEL_WIDTH / 2, WALL_THICKNESS / 2, LEVEL_WIDTH, WALL_THICKNESS);

			// Left wall
			Create_Wall(WALL_THICKNESS / 2, LEVEL_HEIGHT / 2, WALL_THICKNESS, LEVEL_HEIGHT);

			// Right wall
			Create_Wall(LEVEL_WIDTH - WALL_THICKNESS / 2, LEVEL_HEIGHT / 2, WALL_THICKNESS, LEVEL_HEIGHT);
		}
		std::cout << "==============================\n" << std::endl;
		});
	// Top wall


	//Create_RotatingObject(500.f, 400.f);
	//Create_MovingObject(800.f, 400.f);
}

// Updates Level 1
void Level1::Update()
{
	Render& renderer = Render::GetInstance();

	static bool spawned = false;
	std::vector<Entity> toRemove;

	// Toggle with B key
	if (Input::IsKeyPressed(GLFW_KEY_B))
	{
		if (!spawned)
		{
			int totalEntities = 2500;
			float aspectRatio = (float)GLApp::width / (float)GLApp::height;
			int cols = static_cast<int>(std::sqrt(totalEntities * aspectRatio));
			int rows = static_cast<int>(std::ceil((float)totalEntities / cols));

			float paddingX = 5.0f, paddingY = 5.0f;
			float spacingX = (GLApp::width - 2 * paddingX) / cols;
			float spacingY = (GLApp::height - 2 * paddingY) / rows;
			float spacing = std::min(spacingX, spacingY);
			float size = spacing * 0.8f;

			for (int i = 0; i < totalEntities; ++i) {
				int col = i % cols;
				int row = i / cols;

				float px = paddingX + col * spacing + spacing * 0.5f;
				float py = paddingY + row * spacing + spacing * 0.5f;

				Create_InstancedSquare(px, py, size, { 0.2f, 0.8f, 1.0f }); //u are calling drawrect per entity, not all at once. lagg
			}

			std::cout << "Spawned " << totalEntities << " squares with B key!\n";
			spawned = true;
		}
		else
		{
			// === Clear all ===
			for (auto& [e, tag] : registry.Get_ComponentMap<TagComponent>()) {
				if (tag.tag == T_INSTANCING && EntityManager::get().isAlive(e)) {
					toRemove.push_back(e);
				}
			}
			for (Entity e : toRemove) {
				registry.Remove_Entity(e);
			}
			std::cout << "Cleared all spawned squares!\n";
			spawned = false;
		}
	}

	// === Rotation update for spawned squares ===
	if (spawned) {
		static double lastTime = glfwGetTime();
		double currentTime = glfwGetTime();
		float dt = static_cast<float>(currentTime - lastTime);
		lastTime = currentTime;

		for (auto& [e, t] : registry.Get_ComponentMap<TransformComponent>()) {
			if (registry.Has_Component<TagComponent>(e)) {
				auto& tag = registry.Get_Component<TagComponent>(e);
				if (tag.tag == T_INSTANCING) {
					t.rot += 45.f * dt;
				}
			}
		}
	}


	//std::cout << "Level 1 Updating!!!" << std::endl;
	// Save scene on F5
	if (Input::IsKeyPressed(GLFW_KEY_F5)) {
		if (!std::filesystem::exists(FilePath::SAVES_FOLDER)) {
			std::filesystem::create_directories(FilePath::SAVES_FOLDER);
		}

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

				if (registry.Has_Component<Sprite::SpriteComponent>(e)) {
					auto& s = registry.Get_Component<Sprite::SpriteComponent>(e);
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

		if (Input::IsKeyPressed(GLFW_KEY_P) && !GSM_IsPaused())
		{
			GSM_SetIsPaused(true);
		}
		else if (Input::IsKeyPressed(GLFW_KEY_P) && GSM_IsPaused())
		{
			GSM_SetIsPaused(false);
		}

		if (fretboard) {
			fretboard->PlayerInput();
		}
		});
}

// Draws Level 1
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

// Frees Level 1
void Level1::Free()
{
	std::cout << "Level 1 Freed!!!" << std::endl;
	JsonLoad::ClearAllEntities();
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

// Unloads Level 1
void Level1::Unload()
{
	// Clean up background texture
	Render& renderer = Render::GetInstance();
	renderer.ClearAllTextures();

	std::cout << "Level 1 Unloaded!!!" << std::endl;

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
		std::cout << "Level 1 Unloaded!!!" << std::endl;
		});
}