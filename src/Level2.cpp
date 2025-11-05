/* ========================================================
 * @file		Level2.cpp
 * @author		Manfred Sim | manfred.sim@digipen.edu
 * @date		01 NOV 2025
 *
 * @brief	Level 2
 *
 * This source file contains the implementation of functions
 * used by the Level 2. While in Level 2, the game is in
 * its playing state.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Level2.h"
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
#include "GuiSystem.h"

#include "CrashHandler.h"
#include <PlayerSystem.h>
#include <Camera.h>

constexpr float LEVEL_WIDTH = 1920.f;
constexpr float LEVEL_HEIGHT = 1080.f;
constexpr float WALL_THICKNESS = 50.f;

// Background Texture
GLuint bgTexture2 = 0;

// Entities for Level 2
Entity ECSplayer2{};
Entity rock2;
// Entity ghoul;

// Registry ref
Registry& registry2 = Registry::Get();

void Level2::Load()
{
	Render& renderer = Render::GetInstance();

	renderer.useCamera = true;
	auto FontAssets = renderer.LoadFontAssetsFromJSON("../Assets/fontAssets.json");
	CrashHandler::Safe("Level2", "Load", [&] {
		std::cout << "Level 2 Loaded!!!" << std::endl;

		bgTexture2 = renderer.LoadTexture("../Assets/MainMenu_Background.jpg");
		CrashHandler::LogIfZeroTexture(bgTexture2, "../Assets/MainMenu_Background.jpg");
		});
	RenderSystem::ClearBackgroundTextures();
	RenderSystem::AddBackgroundTexture(bgTexture2);

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
		if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "Enemy.json")) {
			std::cerr << "ERROR: Enemy.json was NOT generated!" << std::endl;
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

void Level2::Init()
{
	Render& renderer = Render::GetInstance();

	float uiScaleX = GLApp::width / 1920.0f;
	float uiScaleY = GLApp::height / 1080.0f;
	float uiScale = std::min(uiScaleX, uiScaleY);

	CrashHandler::Safe("Level2", "Init", [&] {
		std::cout << "Level 2 Initialized!!!" << std::endl;
		// 1. Initialize the Renderer
		renderer.Init();

		// 2. Create FretBoard after OpenGL is ready
		//if (!fretboard) {
		//	fretboard = new FretBoard(0, 0, 800, 200);
		//}
		//fretboard->LoadTextures();
		//fretboard->StartGame();

		// 3. NOW load textures (OpenGL is ready)
		//bgTexture = renderer.LoadTexture("../Assets/background.jpg");

		// 4. Add Gui Editor
		GuiSystem::Get().Initialize();
		//GuiSystem::Get().AddTextBox({
		//	.text = "Insert Dialogue HERE",
		//	.pos = {1650, 50},
		//	.scale = 0.5f,
		//	.color = {0.f, 0.f, 0.f},
		//	.centered = true });

		// Button to go to Level 1
		GuiSystem::Get().AddButton({
			.label = "LEVEL 1",
			.pos = {1650.0f, 150.0f},
			.size = {300.0f, 150.0f},
			.color = {20.0f / 255.0f, 182.0f / 255.0f, 191.0f / 255.0f},
			.onClick = [] { GSM_SetNextState(GameState::GS_LEVEL1); },
			.soundType = AudioSystem::ButtonType::QUIT,
			.shape = ButtonShape::Rounded
			});

		GuiSystem::Get().AddTextBox({
			.text = "ENEMY AI is being tested here",
			.pos = {960.0f * uiScaleX, 150.0f * uiScaleY},
			.scale = 0.5f * uiScale,
			.color = {0.f, 0.f, 0.f},
			.centered = true
		});

		// 5. Create FretBoard after OpenGL is ready (duplicate - can remove)
		//if (!fretboard) {
		//	fretboard = new FretBoard(0, 0, 800, 200);
		//}
		//fretboard->LoadTextures();
		//fretboard->StartGame();

		// 6. Create entities
		// If there is no pre-existing/saved scene, it will load from prefabs
		// Otherwise, it will load the saved scene
		std::cout << "\n=== LOADING ENTITIES ===" << std::endl;
		bool sceneLoaded = false;
		if (std::filesystem::exists(FilePath::LEVEL2_SCENE)) {
			std::cout << "Found level2.json, attempting to load..." << std::endl;
			sceneLoaded = JsonLoad::LoadScene(FilePath::LEVEL2_SCENE);
			if (sceneLoaded) {
				rock2 = registry2.Get_EntityID("Player");
				//ghoul = registry2.Get_EntityID("Enemy");

				std::cout << "Scene loaded successfully from level2.json!" << std::endl;
			}
			else {
				std::cout << "Failed to load scene from level2.json, will create from prefabs instead." << std::endl;
			}
		}
		else {
			std::cout << "No level2.json found, will create entities from prefabs." << std::endl;
		}

		if (!sceneLoaded) {
			// ===== FIX: Better error handling when creating from prefabs =====
			std::cout << "Creating fresh entities from prefabs..." << std::endl;

			rock2 = Create_Player(200.f, 500.f);
			if (rock2 == 0) {
				std::cerr << "ERROR: Failed to create Player! Check if Player.json exists." << std::endl;
			}
			else {
				std::cout << "Created Player (Entity " << rock2 << ")" << std::endl;
			}

			//ghoul = Create_Enemy(1000.f, 500.f);
			//if (ghoul == 0) {
			//	std::cerr << "ERROR: Failed to create Enemy! Check if Enemy.json exists." << std::endl;
			//}
			//else {
			//	std::cout << "Created Enemy (Entity " << ghoul << ")" << std::endl;
			//}

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
}

void Level2::Update()
{

	static double lastTime = glfwGetTime();
	double currentTime = glfwGetTime();
	float dt = static_cast<float>(currentTime - lastTime);
	lastTime = currentTime;


	//switch animation state
	if (Input::IsKeyPressed(GLFW_KEY_U))  // U walk animation
	{
		for (auto& [entID, tag] : registry2.Get_ComponentMap<TagComponent>())
		{
			if (tag.tag == T_BATCH &&
				registry2.Has_Component<Animator::AnimatorComponent>(entID))
			{
				auto& anim = registry2.Get_Component<Animator::AnimatorComponent>(entID);
				anim.SetState(PS_WALK);
			}
		}

		std::cout << "Switched animation from IDLE to WALK" << std::endl;
	}

	static bool spawnedBatch = false;

	static bool spawned = false;
	std::vector<Entity> toRemove;

	// === Toggle Batched Sprites with N key (for rubrics demo) ===
	if (Input::IsKeyPressed(GLFW_KEY_N))
	{
		if (!spawnedBatch)
		{
			int totalSprites = 1000;
			float aspectRatio = (float)GLApp::width / (float)GLApp::height;
			int cols = static_cast<int>(std::sqrt(totalSprites * aspectRatio));
			int rows = static_cast<int>(std::ceil((float)totalSprites / cols));

			float paddingX = 5.0f, paddingY = 5.0f;
			float spacingX = (GLApp::width - 2 * paddingX) / cols;
			float spacingY = (GLApp::height - 2 * paddingY) / rows;
			float spacing = std::min(spacingX, spacingY);
			//float size = spacing * 0.8f;
			//float normalizedScale = size / 100.0f;

			for (int i = 0; i < totalSprites; ++i)
			{
				int col = i % cols;
				int row = i / cols;

				float px = paddingX + col * spacing + spacing * 0.5f;
				float py = paddingY + row * spacing + spacing * 0.5f;

				Create_BatchedSprite(px, py, 50.f, "../Assets/Waffle_Sprites.png"); //siz e figure that out later
			}

			std::cout << "Spawned " << totalSprites << " batched sprites with N key!\n";
			spawnedBatch = true;
		}
		else
		{
			// Clear all T_BATCH entities
			for (auto& [e, tag] : registry2.Get_ComponentMap<TagComponent>())
			{
				if (tag.tag == T_BATCH && EntityManager::get().isAlive(e))
					toRemove.push_back(e);
			}
			for (Entity e : toRemove)
				registry2.Remove_Entity(e);

			std::cout << "Cleared all batched sprites!\n";
			spawnedBatch = false;
		}
	}

	static bool spawnedInst = false;
	std::vector<Entity> toRemoveInst;
	// Toggle with B key
	if (Input::IsKeyPressed(GLFW_KEY_B))
	{
		if (!spawnedInst)
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

			std::cout << "spawnedInst " << totalEntities << " squares with B key!\n";
			spawnedInst = true;
		}
		else
		{
			// === Clear all ===
			for (auto& [e, tag] : registry2.Get_ComponentMap<TagComponent>()) {
				if (tag.tag == T_INSTANCING && EntityManager::get().isAlive(e)) {
					toRemoveInst.push_back(e);
				}
			}
			for (Entity e : toRemoveInst) {
				registry2.Remove_Entity(e);
			}
			std::cout << "Cleared all spawnedInst squares!\n";
			spawnedInst = false;
		}
	}

	// === Rotation update for spawnedInst squares ===
	if (spawnedInst) {
		/*static double lastTime = glfwGetTime();
		double currentTime = glfwGetTime();
		float dt = static_cast<float>(currentTime - lastTime);
		lastTime = currentTime;*/

		for (auto& [e, t] : registry2.Get_ComponentMap<TransformComponent>()) {
			if (registry2.Has_Component<TagComponent>(e)) {
				auto& tag = registry2.Get_Component<TagComponent>(e);
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

		JsonLoad::SaveCurrentScene(FilePath::LEVEL2_SCENE, "Level2");
		std::cout << "Scene saved!" << std::endl;
	}

	// Load scene on F9
	if (Input::IsKeyPressed(GLFW_KEY_F9)) {
		JsonLoad::ClearAllEntities();

		if (JsonLoad::LoadScene(FilePath::LEVEL2_SCENE)) {
			std::cout << "Scene loaded successfully!" << std::endl;

			// Find waffle by checking for its sprite texture
			//for (Entity e = 0; e < 10; ++e) {
			//	if (!EntityManager::get().isAlive(e)) continue;

			//	if (registry.Has_Component<Sprite::SpriteComponent>(e)) {
			//		auto& s = registry.Get_Component<Sprite::SpriteComponent>(e);
			//		if (s.Get_TexturePath().find("Waffle") != std::string::npos) {
			//			std::cout << "Found waffle at Entity " << e << std::endl;
			//			waffle = e;  // Update the waffle variable
			//			break;
			//		}
			//	}
			//}
		}
	}

	CrashHandler::Safe("Level2", "Update", [&] {
		// to test game state changes (do not trigger if GUI Editor is open)
		if (Input::IsKeyPressed(GLFW_KEY_8) && !GuiSystem::Get().GetEditMode())
		{
			GSM_SetNextState(GameState::GS_MAIN_MENU);
			//nextState = GameState::GS_MAIN_MENU;     // go to main menu state
		}
		else if (Input::IsKeyPressed(GLFW_KEY_R) && !GuiSystem::Get().GetEditMode())
		{
			GSM_SetNextState(GameState::GS_RESTART);
			//nextState = GameState::GS_RESTART;       // restart state
			std::cout << "Level 2 Restarted!!!" << std::endl;
		}

		if (Input::IsKeyPressed(GLFW_KEY_P) && !GSM_IsPaused() && !GuiSystem::Get().GetEditMode())
		{
			GSM_SetIsPaused(true);
		}
		else if (Input::IsKeyPressed(GLFW_KEY_P) && GSM_IsPaused() && !GuiSystem::Get().GetEditMode())
		{
			GSM_SetIsPaused(false);
		}

		//if (fretboard) {
		//	fretboard->PlayerInput();
		//}

		});
}

void Level2::Draw()
{
	
	CrashHandler::Safe("Level2", "Draw", [&] {
		});
}

void Level2::Free()
{
	JsonLoad::ClearAllEntities();
	if (bgTexture2) {
		Render::GetInstance().UnloadTexture(bgTexture2);
		bgTexture2 = 0;
	}
	//Remove_Entity(rock);
	//Remove_Entity(ECSplayer);
	//Remove_Entity(waffle);

	CrashHandler::Safe("Level2", "Free", [&] {
		std::cout << "Level 2 Freed!!!" << std::endl;
		Render::GetInstance().ClearAllTextures();
		GuiSystem::Get().Shutdown();
	});
}

void Level2::Unload()
{
	// Clean up background texture
	Render& r = Render::GetInstance();
	//renderer.ClearAllTextures();

	std::cout << "Level 2 Unloaded!!!" << std::endl;

	CrashHandler::Safe("Level2", "Unload", [&] {
		r.UnloadAllFonts();
		r.UnloadTexture(bgTexture2);
		std::cout << "Level 2 Unloaded!!!" << std::endl;
	});
}