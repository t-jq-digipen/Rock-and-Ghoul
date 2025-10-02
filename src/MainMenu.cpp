/* ========================================================
 * @file	MainMenu.cpp
 * @author	Manfred Sim
 * @date	29 SEP 2025
 *
 * @brief	Main Menu
 *
 * This source file contains the implementation of functions
 * used by the main menu. While in the main menu, the player
 * may enter the game by clicking the PLAY button or quit the
 * application by clicking the QUIT button.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

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
#include "CrashHandler.h"

// Initialize Audio Manager
AudioManager* MainMenu::audio = nullptr;

// Set Audio Manager
void MainMenu::SetAudioManager(AudioManager* audioMgr) {
    audio = audioMgr;
}

// Play Audio When Button Clicked
void MainMenu::PlayButtonClick() {
    if (audio) {
        audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
    }
}

// Store Textures
GLuint titleTexture{ 0 };
GLuint rockTexture{ 0 };
GLuint ghoulTexture{ 0 };
GLuint playTexture{ 0 };
GLuint quitTexture{ 0 };

//Image titleImage;

// Convert GLFW/screen cords (origin: top-left) to render coords (origin: bottom-left)
static inline glm::vec2 ScreenToRender(const glm::vec2& mouseScreen, float windowHeight)
{
    return { mouseScreen.x, windowHeight - mouseScreen.y };
}

// Loads Main Menu
void MainMenu::Load()
{
    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Load", [&] {
        std::cout << "Main Menu Loaded!!!" << std::endl;

        // Load Textures
        titleTexture = renderer.LoadTexture("../Assets/title.png");
        CrashHandler::LogIfZeroTexture(titleTexture, "../Assets/title.png");

        rockTexture = renderer.LoadTexture("../Assets/Rock_2048px.png");
        CrashHandler::LogIfZeroTexture(rockTexture, "../Assets/title.png");

        ghoulTexture = renderer.LoadTexture("../Assets/Ghoul_2048px.png");
        CrashHandler::LogIfZeroTexture(ghoulTexture, "../Assets/title.png");

        playTexture = renderer.LoadTexture("../Assets/play.png");
        CrashHandler::LogIfZeroTexture(playTexture, "../Assets/title.png");

        quitTexture = renderer.LoadTexture("../Assets/quit.png");
        CrashHandler::LogIfZeroTexture(quitTexture, "../Assets/title.png");
    });
    //titleImage = Render::GetInstance().LoadImage("../Assets/quit.png");
}

// Initializes Main Menu
void MainMenu::Init()
{
    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Init", [&] {
        std::cout << "Main Menu Initialized!!!" << std::endl;
        renderer.Init();
    });
}

// Updates Main Menu
void MainMenu::Update()
{
    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Update", [&] {

        // Clear background, add background color
        renderer.ClearBackground({ 0.5f, 0.f, 0.7f, 1.f });

        // PLAY BUTTON
        renderer.DrawRect({ GLApp::width * 0.3f, GLApp::height * 0.2f }, 300.f, 150.f, { 0, 1, 0 });
        if (renderer.IsAreaTouched(ScreenToRender({ Input::GetMouseX(), Input::GetMouseY() }, static_cast<float>(GLApp::height)),
            { GLApp::width * 0.3f, GLApp::height * 0.2f }, 300.f, 150.f))
        {
            if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (audio) 
                {
                    audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
                }
                else
                {
                    CrashHandler::Log("AudioManager is null on PLAY click");
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
                if (audio) 
                {
                    audio->PlayButtonSound(AudioSystem::ButtonType::QUIT);
                }
                else
                {
                    CrashHandler::Log("AudioManager is null on QUIT click");
                }

                // Wait a bit for the sound to play before quitting
                double startTime = glfwGetTime();
                while (glfwGetTime() - startTime < 0.5) // Wait 500ms
                {
                    // Keep updating audio system during the wait
                    AudioSystem::Update();
                    glfwPollEvents();
                }
                GSM_SetNextState(GameState::GS_QUIT);
            }
        }

        // Alternatively press SPACE to enter LEVEL 1
        if (Input::IsKeyPressed(GLFW_KEY_SPACE))
        {
            std::cout << "PLAY CLICKED � switching to Level1\n";
            if (audio) 
            {
                audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
            }
            else
            {
                CrashHandler::Log("AudioManager is null on SPACE press");
            }
            GSM_SetNextState(GameState::GS_LEVEL1);
        }

        // Non-fatal test: press F2 to simulate a runtime error, writes to log file (should NOT quit the program!)
        if (Input::IsKeyPressed(GLFW_KEY_F2))
        {
            int rc = CrashHandler::RunProtected([] {
                throw std::runtime_error("F2 has been pressed! std::exception test successful!");
            });
            CrashHandler::Logf("int rc = %d (expected value: 1)", rc);
            return;
        }

        // Fatal test: press F3 to simulate a crash, program automatically hard-exits
        if (Input::IsKeyPressed(GLFW_KEY_F3))
        {
            throw std::runtime_error("F3 has been pressed! std::terminate handler test successful!");
        }

        // Fatal signal test: press F4 to simulate an abortion (DON'T ALT + F4 THOUGH)
        if (Input::IsKeyPressed(GLFW_KEY_F4))
        {
            CrashHandler::Log("F4 has been pressed! std::abort test successful!");
            std::abort();
        }
    });
}

// Draws Main Menu
void MainMenu::Draw()
{
    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Draw", [&] {
        // Game Title
        if (titleTexture)
        {
            renderer.DrawTexturedQuad({ GLApp::width * 0.5f, GLApp::height * 0.8f }, { 1200.f, 300.f }, titleTexture);
        }
        else
        {
            CrashHandler::Log("Title texture missing; skipping draw!");
        }

        // ROCK
        if (rockTexture)
        {
            renderer.DrawTexturedQuad({ GLApp::width * 0.3f, GLApp::height * 0.5f }, { 400.f, 400.f }, rockTexture);
        }
        else
        {
            CrashHandler::Log("Rock texture missing; skipping draw!");
        }

        // GHOUL (image is reversed)
        if (ghoulTexture)
        {
            renderer.DrawTexturedQuad({ GLApp::width * 0.7f, GLApp::height * 0.5f }, { -400.f, 400.f }, ghoulTexture);
        }
        else
        {
            CrashHandler::Log("Ghoul texture missing; skipping draw!");
        }

        // PLAY text for button
        if (playTexture)
        {
            renderer.DrawTexturedQuad({ GLApp::width * 0.3f, GLApp::height * 0.2f }, { 300.f, 150.f }, playTexture);
        }
        else
        {
            CrashHandler::Log("PLAY button texture missing; skipping draw!");
        }

        // QUIT text for button
        if (quitTexture)
        {
            renderer.DrawTexturedQuad({ GLApp::width * 0.7f, GLApp::height * 0.2f }, { 300.f, 150.f }, quitTexture);
        }
        else
        {
            CrashHandler::Log("QUIT button texture missing; skipping draw!");
        }
    });
}

// Frees Main Menu
void MainMenu::Free()
{
    CrashHandler::Safe("MainMenu", "Free", [&] {
        std::cout << "Main Menu Freed!!!" << std::endl;
    });
}

// Unloads Main Menu
void MainMenu::Unload()
{
    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Unload", [&] {
        std::cout << "Main Menu Unloaded!!!" << std::endl;

        // Unload Textures
        renderer.UnloadTexture(titleTexture);
        renderer.UnloadTexture(rockTexture);
        renderer.UnloadTexture(ghoulTexture);
        renderer.UnloadTexture(playTexture);
        renderer.UnloadTexture(quitTexture);
    });
}