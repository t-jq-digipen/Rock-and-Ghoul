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

AudioManager* MainMenu::audio = nullptr;

void MainMenu::SetAudioManager(AudioManager* audioMgr) {
    audio = audioMgr;
}

void MainMenu::PlayButtonClick() {
    if (audio) {
        audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
    }
}

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
    titleTexture = Render::GetInstance().LoadTexture("../Assets/title.png");
    rockTexture = Render::GetInstance().LoadTexture("../Assets/Rock_2048px.png");
    ghoulTexture = Render::GetInstance().LoadTexture("../Assets/Ghoul_2048px.png");
    playTexture = Render::GetInstance().LoadTexture("../Assets/play.png");
    quitTexture = Render::GetInstance().LoadTexture("../Assets/quit.png");
}

void MainMenu::Init()
{
    std::cout << "Main Menu Initialized!!!" << std::endl;
    Render::GetInstance().Init();
}

void MainMenu::Update()
{
    Render& renderer = Render::GetInstance();

    // Clear background, add background color
    renderer.ClearBackground({ 0.5f, 0.f, 0.7f, 1.f });

    // Render main menu textures
    // Game Title
    if (titleTexture)
    {
        renderer.DrawTexturedQuad({ GLApp::width * 0.5f, GLApp::height * 0.8f }, { 1200.f, 300.f }, titleTexture);
    }

    // ROCK
    if (rockTexture)
    {
        renderer.DrawTexturedQuad({ GLApp::width * 0.3f, GLApp::height * 0.5f }, { 400.f, 400.f }, rockTexture);
    }

    // GHOUL (image is reversed)
    if (ghoulTexture)
    {
        renderer.DrawTexturedQuad({ GLApp::width * 0.7f, GLApp::height * 0.5f }, { -400.f, 400.f }, ghoulTexture);
    }

    // PLAY BUTTON
    renderer.DrawRect({ GLApp::width * 0.3f, GLApp::height * 0.2f }, 300.f, 150.f, { 0, 1, 0 });
    if (renderer.IsAreaTouched(ScreenToRender({ Input::GetMouseX(), Input::GetMouseY() }, static_cast<float>(GLApp::height)),
        { GLApp::width * 0.3f, GLApp::height * 0.2f }, 300.f, 150.f))
    {
        if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            if (audio) {
                audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
            }
            GSM_SetNextState(GameState::GS_LEVEL1);
        }
    }

    // QUIT BUTTON
    renderer.DrawRect({ GLApp::width * 0.7f, GLApp::height * 0.2f }, 300.f, 150.f, { 1, 0, 0 });
    if (renderer.IsAreaTouched(ScreenToRender({ Input::GetMouseX(), Input::GetMouseY() }, static_cast<float>(GLApp::height)),
        { GLApp::width * 0.7f, GLApp::height * 0.2f }, 300.f, 150.f))
    {
        if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            if (audio) {
                audio->PlayButtonSound(AudioSystem::ButtonType::QUIT);
            }

            // Wait a bit for the sound to play before quitting
            double startTime = glfwGetTime();
            while (glfwGetTime() - startTime < 0.5) { // Wait 500ms
                // Keep updating audio system during the wait
                AudioSystem::Update();
                glfwPollEvents();
            }

            GSM_SetNextState(GameState::GS_QUIT);
        }
    }

    // PLAY text for button
    if (playTexture)
    {
        renderer.DrawTexturedQuad({ GLApp::width * 0.3f, GLApp::height * 0.2f }, { 300.f, 150.f }, playTexture);
    }

    // QUIT text for button
    if (quitTexture)
    {
        renderer.DrawTexturedQuad({ GLApp::width * 0.7f, GLApp::height * 0.2f }, { 300.f, 150.f }, quitTexture);
    }

    // press space (or click play button) to enter level 1
    if (Input::IsKeyPressed(GLFW_KEY_SPACE))
    {
        std::cout << "PLAY CLICKED — switching to Level1\n";
        if (audio) {
            audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
        }
        GSM_SetNextState(GameState::GS_LEVEL1);
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
    Render::GetInstance().UnloadTexture(titleTexture);
    Render::GetInstance().UnloadTexture(rockTexture);
    Render::GetInstance().UnloadTexture(ghoulTexture);
    Render::GetInstance().UnloadTexture(playTexture);
    Render::GetInstance().UnloadTexture(quitTexture);
}