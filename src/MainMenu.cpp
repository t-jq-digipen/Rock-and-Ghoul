#include "Render.h"
#include "MainMenu.h"
#include "GameStateManager.h"
#include "Input.h"
#include "System.h"
#include "glapp.h"
#include "RenderSystem.h"

#include "Registry.h"

#include "GameStates.h"

#include "CoreEngine.h"

RenderSystem renderSystem;
Render renderer;
GLuint titleTexture{ 0 };
GLuint rockTexture{ 0 };
GLuint ghoulTexture{ 0 };
GLuint playTexture{ 0 };
GLuint quitTexture{ 0 };


// Convert GLFW/screen cords (origin: top-left) to render coords (origin: bottom-left)
static inline glm::vec2 ScreenToRender(const glm::vec2& mouseScreen, float windowHeight)
{
    return { mouseScreen.x, windowHeight - mouseScreen.y };
}


void MainMenu::Load()
{
	std::cout << "Main Menu Loaded!!!" << std::endl;
	titleTexture = Render::LoadTexture("../Assets/title.png");
	rockTexture = Render::LoadTexture("../Assets/Rock_2048px.png");
	ghoulTexture = Render::LoadTexture("../Assets/Ghoul_2048px.png");
    playTexture = Render::LoadTexture("../Assets/play.png");
    quitTexture = Render::LoadTexture("../Assets/quit.png");
}

void MainMenu::Init()
{
	std::cout << "Main Menu Initialized!!!" << std::endl;
    renderSystem.renderer.Init();

    // ECS TEST
    

}

void MainMenu::Update()
{
    //std::cout << "Main Menu Updating!!!" << std::endl;

    std::cout << Input::GetMouseX() << ", " << Input::GetMouseY() << std::endl;

    // Clear background, add background color
    glClearColor(0.5, 0, 0.7, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render main menu textures (placeholders rn)

    // Game Title
    if (titleTexture)
    {
        renderSystem.renderer.DrawTexturedQuad({ GLApp::width * 0.5f, GLApp::height * 0.8f }, { 1200.f, 300.f }, titleTexture);
    }

    // ROCK
    if (rockTexture)
    {
        renderSystem.renderer.DrawTexturedQuad({ GLApp::width * 0.3f, GLApp::height * 0.5f }, { 400.f, 400.f }, rockTexture);
    }

    // GHOUL (image is reversed)
    if (ghoulTexture)
    {
        renderSystem.renderer.DrawTexturedQuad({ GLApp::width * 0.7f, GLApp::height * 0.5f }, { -400.f, 400.f }, ghoulTexture);
    }

    // draw play button
    renderSystem.renderer.DrawRect({ GLApp::width * 0.3f, GLApp::height * 0.2f }, 300.f, 150.f, { 0, 1, 0 });
    if (renderSystem.renderer.IsAreaTouched(ScreenToRender({ Input::GetMouseX(), Input::GetMouseY() }, static_cast<float>(GLApp::height)),
        { GLApp::width * 0.3f, GLApp::height * 0.2f }, 300.f, 150.f))
    {
        renderSystem.renderer.DrawRect({ GLApp::width * 0.3f, GLApp::height * 0.2f }, 300.f, 150.f, { 0, 1, 0.5 });
        if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            // nextState = GS_LEVEL1;
            GSM_SetNextState(GameState::GS_LEVEL1);
        }
    }
    

    // draw quit button
    renderSystem.renderer.DrawRect({ GLApp::width * 0.7f, GLApp::height * 0.2f }, 300.f, 150.f, { 1, 0, 0 });
    if (renderSystem.renderer.IsAreaTouched(ScreenToRender({ Input::GetMouseX(), Input::GetMouseY() }, static_cast<float>(GLApp::height)), 
        { GLApp::width * 0.7f, GLApp::height * 0.2f }, 300.f, 150.f))
    {
        renderSystem.renderer.DrawRect({ GLApp::width * 0.7f, GLApp::height * 0.2f }, 300.f, 150.f, { 1, 0, 0.5 });
        if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            GSM_SetNextState(GameState::GS_QUIT);
            // nextState = GS_QUIT;
        }
    }

    // PLAY text for button
    if (playTexture)
    {
        renderSystem.renderer.DrawTexturedQuad({ GLApp::width * 0.3f, GLApp::height * 0.2f }, { 300.f, 150.f }, playTexture);
    }

    // QUIT text for button
    if (quitTexture)
    {
        renderSystem.renderer.DrawTexturedQuad({ GLApp::width * 0.7f, GLApp::height * 0.2f }, { 300.f, 150.f }, quitTexture);
    }
    

    // press space (or click play button) to enter level 1
    if (Input::IsKeyPressed(GLFW_KEY_SPACE))
    {
        GSM_SetNextState(GameState::GS_LEVEL1);
        // nextState = GS_LEVEL1;
    }
}

void MainMenu::Draw()
{
	// std::cout << "Main Menu Drawing!!!" << std::endl;
}

void MainMenu::Free()
{
	std::cout << "Main Menu Freed!!!" << std::endl;
}

void MainMenu::Unload()
{
	std::cout << "Main Menu Unloaded!!!" << std::endl;
    if (titleTexture)
    {
        glDeleteTextures(1, &titleTexture);
        titleTexture = 0;
    }

    if (rockTexture)
    {
        glDeleteTextures(1, &rockTexture);
        rockTexture = 0;
    }

    if (ghoulTexture)
    {
        glDeleteTextures(1, &ghoulTexture);
        ghoulTexture = 0;
    }

    if (playTexture)
    {
        glDeleteTextures(1, &playTexture);
        playTexture = 0;
    }

    if (quitTexture)
    {
        glDeleteTextures(1, &quitTexture);
        quitTexture = 0;
    }
}
