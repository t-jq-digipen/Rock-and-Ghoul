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
static Render renderer;
static SpellManager spellManager;
static FretBoard fretboard(0, 0, 800, 200, spellManager);
static RenderSystem renderSystem(renderer);
extern BossGhoulSystem* bossGhoulSystem;


GLuint bgTexture = 0;
Entity ECSplayer{};


void Level1::Load()
{
	std::cout << "Level 1 Loaded!!!" << std::endl;

	bgTexture = Render::LoadTexture("../Assets/background.jpg");
	spellManager.LoadTextures();

	// Then fretboard arrows
	fretboard.LoadTextures();

	// Then start game (after both are ready)
	fretboard.StartGame();
}

void Level1::Init()
{
	std::cout << "Level 1 Initialized!!!" << std::endl;
	renderer.Init();

	spellManager.LoadTextures();
	fretboard.LoadTextures();
	fretboard.StartGame();

	Entity rock = Create_Entity();
	Add_Component(rock, TransformComponent{ {200.f, 500.f}, {100.f, 100.f} });
	Add_Component(rock, PhysicsComponent{});
	Add_Component(rock, InputComponent{ CS_WASD });
	Add_Component(rock, TagComponent{ T_PLAYER });

	// give it a sprite
	GLuint rockTex = renderer.LoadTexture("../Assets/Rock_FightingPose_2048px.png");
	Add_Component(rock, SpriteComponent{ rockTex, {400.f, 400.f} });

	Entity ghoul = Create_Entity();
	Add_Component(ghoul, TransformComponent{ {500.f, 500.f}, {100.f, 100.f} });
	Add_Component(ghoul, BossGhoulComponent{ 100.f, 100.f, 0.f, 100.0f, 0, GHOUL_ATTACKING });
	Add_Component(ghoul, TagComponent{ T_ENEMY });

	// give it a sprite
	GLuint ghoulTex = renderer.LoadTexture("../Assets/Ghoul_2048px.png");
	Add_Component(ghoul, SpriteComponent{ ghoulTex, {400.f, 400.f} });
	bossGhoulSystem->AddEntity(ghoul);
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	if (bgTexture) {
		renderer.DrawTexturedQuad(
			{ GLApp::width * 0.5f, GLApp::height * 0.5f },   // center of screen
			{ (float)GLApp::width, (float)GLApp::height },   // full screen size
			bgTexture
		);
	}
	renderSystem.Update(0.0f);
	bossGhoulSystem->Draw(renderer);
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

