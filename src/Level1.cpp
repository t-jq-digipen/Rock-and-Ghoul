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
#include "GuiSystem.h"

#include "CrashHandler.h"
#include <PlayerSystem.h>
#include <Camera.h>
#include "AudioComponent.h"


#include "LogicSystem.h"
#include "GameScripts.h"
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
Entity ghoul;
Entity testEntity; // for fixed dt, pink square
// Registry ref
Registry& registry = Registry::Get();
//Camera mainCamera;


// Loads Level 1
void Level1::Load()
{
	Render& renderer = Render::GetInstance();
	Camera& cam = Camera::GetInstance();
	
	renderer.useCamera = true;
	//is there a way to get the bgsize, kinda iffy erhh
	Vec2 bgSize = { 2200.0f, 1080.0f };
	cam.SetBounds({ -250.0f, 0.0f }, bgSize);

	auto FontAssets = renderer.LoadFontAssetsFromJSON("../Assets/fontAssets.json");
	CrashHandler::Safe("Level1", "Load", [&] {
		std::cout << "Level 1 Loaded!!!" << std::endl;

		bgTexture = renderer.LoadTexture("../Assets/background.jpg");
		CrashHandler::LogIfZeroTexture(bgTexture, "../Assets/background.jpg");
		});
	RenderSystem::ClearBackgroundTextures();
	RenderSystem::AddBackgroundTexture(bgTexture);


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
	if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "ghoul.json")) {
		needsGeneration = true;
		missingPrefabs.push_back("ghoul.json");
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
		if (!std::filesystem::exists(FilePath::PREFABS_FOLDER + "ghoul.json")) {
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
		bgTexture = renderer.LoadTexture("../Assets/background.jpg");
		if (!bgTexture)
			std::cerr << "[Level1] Failed to reload background texture!" << std::endl;
		else {
			RenderSystem::ClearBackgroundTextures();
			RenderSystem::AddBackgroundTexture(bgTexture);
		}
		// 2. Create FretBoard after OpenGL is ready
		if (!fretboard) {
			fretboard = new FretBoard(0, 0, 800, 200);
		}
		fretboard->LoadTextures();
		fretboard->StartGame();

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
		
		// Button to go to Level 2
		GuiSystem::Get().AddButton({
			.label = "LEVEL 2",
			.pos = {1650.0f, 150.0f},
			.size = {300.0f, 150.0f},
			.color = {20.0f / 255.0f, 182.0f / 255.0f, 191.0f / 255.0f},
			.onClick = [] { GSM_SetNextState(GameState::GS_LEVEL2); },
			.soundType = AudioSystem::ButtonType::PLAY,
			.shape = ButtonShape::Rounded
		});

		// Initial dialogue box when opening Level 1 state
		TextBoxConfig initDialogue;
		initDialogue.text = "";
		//initDialogue.basePos = { 450, 150 };
		initDialogue.basePos = { 960, 150 }; //adjusted the x cus calculateTextSize change
		initDialogue.baseScale = 0.9f;
		initDialogue.pos = initDialogue.basePos;
		initDialogue.scale = initDialogue.baseScale;
		initDialogue.color = { 0,0,0 };
		initDialogue.centered = true;

		GuiSystem::Get().AddDialogueFromJson("../Assets/prefabs/Dialogue.json", initDialogue);

		// 6. Create entities
		// If there is no pre-existing/saved scene, it will load from prefabs
		// Otherwise, it will load the saved scene
		std::cout << "\n=== LOADING ENTITIES ===" << std::endl;
		bool sceneLoaded = false;
		if (std::filesystem::exists(FilePath::LEVEL1_SCENE)) {
			std::cout << "Found level1.json, attempting to load..." << std::endl;
			sceneLoaded = JsonLoad::LoadScene(FilePath::LEVEL1_SCENE);
			if (sceneLoaded) {
				waffle = registry.Get_EntityID("Waffle");
				rock = registry.Get_EntityID("Player");
				square = registry.Get_EntityID("Rectangle");
				roundy = registry.Get_EntityID("Circle");
				ghoul = registry.Get_EntityID("ghoul");

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
			// ===== Better error handling when creating from prefabs =====
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
			ghoul = Create_Ghoul(1000.f, 500.f);
			if (ghoul == 0) {
				std::cerr << "ERROR: Failed to create Ghoul! Check if ghoul.json exists." << std::endl;
			}
			else {
				std::cout << "Created Ghoul (Entity " << ghoul << ")" << std::endl;
			}
			std::cout << "Finished creating entities from prefabs." << std::endl;

			// fixed dt test entity
			std::cout << "\n[FIXED DT TEST] Creating test entity with constant velocity\n";

			// create test entity
			testEntity = registry.Create_Entity();
			registry.Set_EntityName(testEntity, "FixedDT_Test");

			// add Transform component in the centre
			TransformComponent testTransform;
			testTransform.pos = { 400.0f, 400.0f };  // same as waffle position
			testTransform.scale = { 50.0f, 50.0f };
			testTransform.rot = 0.0f;
			registry.Add_Component(testEntity, testTransform);

			// add Shape component (magenta square so you can see it)
			ShapeComponent testShape;
			testShape.shape = SHAPE_RECTANGLE;
			testShape.dimensions = { 50.0f, 50.0f };  // shape dimensions
			testShape.color = { 1.0f, 0.0f, 1.0f };  // bright pink
			registry.Add_Component(testEntity, testShape);

			// physics component with CONSTANT velocity
			Physics::PhysicsComponent testPhysics;
			testPhysics.Set_Vel({ 50.0f, 0.0f });  // 50 pixels/second to the right
			registry.Add_Component(testEntity, testPhysics);

			// Tag to identify
			TagComponent testTag;
			testTag.tag = T_TEST;
			registry.Add_Component(testEntity, testTag);

			std::cout << "[FIXED DT TEST] Test entity created at (400, 400) moving at 50px/s\n";
			std::cout << "[FIXED DT TEST] Screen width: " << LEVEL_WIDTH << "px\n";
			std::cout << "[FIXED DT TEST] Expected time to cross screen: ~9.6 seconds\n";
			std::cout << "[FIXED DT TEST] Press L to simulate lag - square should NOT slow down! Press L again to stop lag.\n";
			std::cout << "[FIXED DT TEST] Look for PINK SQUARE moving left to right\n";
			std::cout << "========================================\n\n";

			Create_Wall(LEVEL_WIDTH / 2, LEVEL_HEIGHT - WALL_THICKNESS / 2, LEVEL_WIDTH, WALL_THICKNESS);

			// Bottom wall
			Create_Wall(LEVEL_WIDTH / 2, WALL_THICKNESS / 2, LEVEL_WIDTH, WALL_THICKNESS);

			// Left wall
			Create_Wall(WALL_THICKNESS / 2, LEVEL_HEIGHT / 2, WALL_THICKNESS, LEVEL_HEIGHT);

			// Right wall
			Create_Wall(LEVEL_WIDTH - WALL_THICKNESS / 2, LEVEL_HEIGHT / 2, WALL_THICKNESS, LEVEL_HEIGHT);
			Create_ScriptedEntity("Mover", 400.f, 300.f);
			Create_Arrow(1400.f, 400.f);
		}
		std::cout << "==============================\n" << std::endl;
		});
	// Top wall
	Camera& cam = Camera::GetInstance();
	cam.SetMode(Camera::Mode::GAME);

	cam.SetPosition({ GLApp::width / 2.f, GLApp::height / 2.f });
	cam.SetZoom(1.15f);
	cam.UpdateMatrices();
	Render::GetInstance().UpdateProjection();

	// ========== COMMENTED OUT: Prevents console spam during testing ==========
	// Entity mover = Create_ScriptedEntity("Mover", 400.f, 300.f);
	// Create_Arrow(1400.f, 400.f);
	// ========== END COMMENT ==========
//Create_RotatingObject(500.f, 400.f);
//Create_MovingObject(800.f, 400.f);
}

// Updates Level 1
void Level1::Update()
{
	//Camera& mainCamera = Camera::GetInstance();

	// Check names correct
	// NEEDTOREPLACE with messaging system
	//if (registry.Get_EntityName(waffle) != "Waffle")	
	//{ waffle = registry.Get_EntityID("Waffle"); }
	//if (registry.Get_EntityName(rock) != "Player")		
	//{ rock = registry.Get_EntityID("Player"); }
	//if (registry.Get_EntityName(square) != "Rectangle")	
	//{ square = registry.Get_EntityID("Rectangle"); }
	//if (registry.Get_EntityName(roundy) != "Circle")	
	//{ roundy = registry.Get_EntityID("Circle"); }
	/*if (registry.Get_EntityName(ghoul) != "ghoul")
	if (registry.Get_EntityName(waffle) != "Waffle")
	{
		waffle = registry.Get_EntityID("Waffle");
	}
	if (registry.Get_EntityName(rock) != "Player")
	{
		rock = registry.Get_EntityID("Player");
	}
	if (registry.Get_EntityName(square) != "Rectangle")
	{
		square = registry.Get_EntityID("Rectangle");
	}
	if (registry.Get_EntityName(roundy) != "Circle")
	{
		roundy = registry.Get_EntityID("Circle");
	}
	if (registry.Get_EntityName(ghoul) != "ghoul")
	{
		ghoul = registry.Get_EntityID("ghoul");
	}*/
	//test camera on an entity

	static double lastTime = glfwGetTime();
	double currentTime = glfwGetTime();
	float dt = static_cast<float>(currentTime - lastTime);
	lastTime = currentTime;


	static bool testEntityStarted = false;
	for (auto& [entID, tag] : registry.Get_ComponentMap<TagComponent>()) {
		if (tag.tag == T_TEST && registry.Has_Component<TransformComponent>(entID)) {
			auto& transform = registry.Get_Component<TransformComponent>(entID);

			// reset when it comes back to start position
			if (transform.pos.x > LEVEL_WIDTH) {
				transform.pos = { 400.0f, 400.0f };
				std::cout << "[FIXED DT] Test entity reset to start position\n";
			}
			break;
		}
	}




	
	// make test square for fixed dt come back to original waffle position when it leaves screen
	for (auto& [entID, tag] : registry.Get_ComponentMap<TagComponent>()) {
		if (tag.tag == T_TEST && registry.Has_Component<TransformComponent>(entID)) {
			auto& transform = registry.Get_Component<TransformComponent>(entID);

			// If square goes past right edge of screen relative to camera, reset to start
			if (transform.pos.x > LEVEL_WIDTH) {
				transform.pos = { 400.0f, 400.0f };  // Reset to starting position
			}
		}
	}

	//========== stress test demo start =========================================
	
	//switch animation state
	if (Input::IsKeyPressed(GLFW_KEY_U))  // U walk animation
	{
		for (auto& [entID, tag] : registry.Get_ComponentMap<TagComponent>())
		{
			if ((tag.tag == T_BATCH ) &&
				registry.Has_Component<Animator::AnimatorComponent>(entID))
			{
				auto& anim = registry.Get_Component<Animator::AnimatorComponent>(entID);
				if (anim.GetCurrentState() == PS_WALK) anim.SetState(PS_IDLE);
				else anim.SetState(PS_WALK);
			}
		}

		std::cout << "Switched animation from IDLE to WALK" << std::endl;
	}
	
	static bool spawnedBatch = false;
	std::vector<Entity> toRemove;

	// === Toggle Batched Sprites with N key (for rubrics demo) ===
	if (Input::IsKeyPressed(GLFW_KEY_N))
	{
		if (!spawnedBatch)
		{
			int totalSprites = 1000;
			/*float aspectRatio = (float)GLApp::width / (float)GLApp::height;
			int cols = static_cast<int>(std::sqrt(totalSprites * aspectRatio));
			int rows = static_cast<int>(std::ceil((float)totalSprites / cols));

			float paddingX = 5.0f, paddingY = 5.0f;
			float spacingX = (GLApp::width - 2 * paddingX) / cols;
			float spacingY = (GLApp::height - 2 * paddingY) / rows;
			float spacing = std::min(spacingX, spacingY);
			//float size = spacing * 0.8f;
			//float normalizedScale = size / 100.0f;*/
			float spriteSize = 50.f;
			float spriteHalfSize = spriteSize * 0.5f;
			//random seed
			srand(static_cast<unsigned>(time(nullptr)));
			for (int i = 0; i < totalSprites; ++i)
			{
				/*int col = i % cols;
				int row = i / cols;

				float px = paddingX + col * spacing + spacing * 0.5f;
				float py = paddingY + row * spacing + spacing * 0.5f;*/

				//spawn within screen
				float px = spriteHalfSize + static_cast<float>(rand() % GLApp::width - static_cast<int>(spriteHalfSize));
				float py = spriteHalfSize + static_cast<float>(rand() % GLApp::height - static_cast<int>(spriteHalfSize));

				Create_BatchedSprite(px, py, spriteSize, "../Assets/Waffle_Sprites.png"); //siz e figure that out later
			}

			std::cout << "Spawned " << totalSprites << " batched sprites with N key!\n";
			spawnedBatch = true;
		}
		else
		{
			// Clear all T_BATCH entities
			for (auto& [e, tag] : registry.Get_ComponentMap<TagComponent>())
			{
				if (tag.tag == T_BATCH && EntityManager::get().isAlive(e)) {
					toRemove.push_back(e);
				}
			}
			for (Entity e : toRemove) {
				registry.Remove_Entity(e);
			}

			std::cout << "Cleared all batched sprites!\n";
			spawnedBatch = false;
		}
	}
	//======================= stress test demon end ===============================

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
			for (auto& [e, tag] : registry.Get_ComponentMap<TagComponent>()) {
				if (tag.tag == T_INSTANCING && EntityManager::get().isAlive(e)) {
					toRemoveInst.push_back(e);
				}
			}
			for (Entity e : toRemoveInst) {
				registry.Remove_Entity(e);
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
		// to test game state changes (do not trigger if GUI Editor is open)
		if (Input::IsKeyPressed(GLFW_KEY_M) && !GuiSystem::Get().GetEditMode())
		{
			GSM_SetNextState(GameState::GS_MAIN_MENU);
			//nextState = GameState::GS_MAIN_MENU;     // go to main menu state
		}
		// --- Restart level with R key (disabled while ImGui editor is active) ---
		//if (Input::IsKeyPressed(GLFW_KEY_R))
		else if (Input::IsKeyPressed(GLFW_KEY_R) && !GuiSystem::Get().GetEditMode())
		{
			// don't trigger restart while editing or paused
			if (!ImGuiSystem::guiEnabled && !GSM_IsPaused() && !GuiSystem::Get().GetEditMode())
			{
				GSM_SetNextState(GameState::GS_RESTART);
				std::cout << "Level 1 Restarted!!!" << std::endl;
			}
			else
			{
				std::cout << "[DEBUG] Ignored R press (editor or paused mode active)\n";
			}
		}


		// if (Input::IsKeyPressed(GLFW_KEY_P) && !GSM_IsPaused() && !guiSystem.GetEditMode())
		if (Input::IsKeyPressed(GLFW_KEY_P) && !GSM_IsPaused() && !GuiSystem::Get().GetEditMode())
		{
			GSM_SetIsPaused(true);
		}
		else if (Input::IsKeyPressed(GLFW_KEY_P) && GSM_IsPaused() && !GuiSystem::Get().GetEditMode())
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
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);
		//if (bgTexture) {
			//renderer.useCamera = false; //uncomment then bg text will cover whole screen but look abit funny
		//	renderer.DrawTexturedQuad(
		//		{ GLApp::width * 0.5f, GLApp::height * 0.5f },   // center of screen
		//		{ (float)GLApp::width, (float)GLApp::height },   // full screen size
		//		bgTexture
		//	);
		//renderer.useCamera = true; //uncomment then bg text will cover whole screen but look abit funny
		//}


		if (fretboard) {
			renderer.useCamera = false;
			fretboard->Draw(renderer);
			fretboard->GetSpellManager().Draw(renderer);
			renderer.useCamera = true;
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
		GuiSystem::Get().Shutdown();
	});
}

// Unloads Level 1
void Level1::Unload()
{
	// Clean up background texture
	Render& renderer = Render::GetInstance();
	//renderer.ClearAllTextures();

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

		renderer.UnloadAllFonts();
		renderer.UnloadTexture(bgTexture);
		std::cout << "Level 1 Unloaded!!!" << std::endl;
		});
}