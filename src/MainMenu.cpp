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

AudioManager* MainMenu::audio = nullptr;
bool showInstancingDemo = false;

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

Image titleImage;

// Convert GLFW/screen cords (origin: top-left) to render coords (origin: bottom-left)
static inline glm::vec2 ScreenToRender(const glm::vec2& mouseScreen, float windowHeight)
{
    return { mouseScreen.x, windowHeight - mouseScreen.y };
}

void MainMenu::Load()
{
    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Load", [&] {
        std::cout << "Main Menu Loaded!!!" << std::endl;

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

    titleImage = CreateImage("../Assets/quit.png");
    //titleImage = Render::GetInstance().LoadImage("../Assets/quit.png");

}

void MainMenu::Init()
{
    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Init", [&] {
        std::cout << "Main Menu Initialized!!!" << std::endl;
        renderer.Init();
    });
}

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

        // press space (or click play button) to enter level 1
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

        // Non-fatal test: press F1 to simulate a runtime error, writes to log file (should NOT quit the program!)
        if (Input::IsKeyPressed(GLFW_KEY_F1))
        {
            int rc = CrashHandler::RunProtected([] {
                throw std::runtime_error("F1 has been pressed! std::exception test successful!");
            });
            CrashHandler::Logf("int rc = %d (expected value: 1)", rc);
            return;
        }

        // Fatal test: press F2 to simulate a crash, program automatically hard-exits
        if (Input::IsKeyPressed(GLFW_KEY_F2))
        {
            throw std::runtime_error("F2 has been pressed! std::terminate handler successful!");
        }

        // Fatal signal test: press F4 to simulate an abortion (DON'T ALT + F4 THOUGH)
        if (Input::IsKeyPressed(GLFW_KEY_F4))
        {
            CrashHandler::Log("F4 has been pressed! std::abort successful!");
            std::abort();
        }
    });



    // Render main menu textures (placeholders rn)
    
    //just testing stuff for rendering (can delete)
    //renderer.DrawImage(titleImage, 400.f, 400.f);
    //renderer.DrawImageRotated(titleImage, 400.f, 400.f, 1.57f);
    //renderer.DrawImageSub(titleImage, 400.f, 400.f, 100.f, 400.f, 0.f, 0.f, 256.f, 256.f);
    renderer.DrawText("Let's play", 400.f, 700.f, 0.75f, { 0.f,0.f,1.f });
    renderer.DrawText("Game Over", 1400.f, 700.f, 0.75f, { 1.f,1.f,1.f });

    if (Input::IsKeyPressed(GLFW_KEY_B)) {
        //std::cout << showInstancingDemo << std::endl;
        showInstancingDemo = !showInstancingDemo;
    }
    if (showInstancingDemo) {
        std::vector<InstanceData> instances;

        int totalInstances = 2500;

        // Calculate grid dimensions that fit the screen
        float aspectRatio = (float)GLApp::width / (float)GLApp::height;
        int cols = static_cast<int>(std::sqrt(totalInstances * aspectRatio));
        int rows = static_cast<int>(std::ceil((float)totalInstances / cols));

        // Calculate space to fit screen
        float paddingX = 5.0f;
        float paddingY = 5.0f;
        float spacingX = (GLApp::width - 2 * paddingX) / cols;
        float spacingY = (GLApp::height - 2 * paddingY) / rows;

        // Use the smaller spacing to ensure everything fits
        float spacing = std::min(spacingX, spacingY);

        // Calculate sprite size 
        float spriteSize = spacing * 0.8f;

        for (int i = 0; i < totalInstances; i++) {
            InstanceData data;

            // Grid layout centered on screen
            int col = i % cols;
            int row = i / cols;

            data.position = glm::vec2(
                paddingX + col * spacing + spacing * 0.5f,  // Center
                paddingY + row * spacing + spacing * 0.5f
            );

            // Sprite size
            data.scale = glm::vec2(spriteSize, spriteSize);

            // Rotation over time (animation)
            data.rotation = static_cast<float>(glfwGetTime() + i * 0.1f);

            // Full texture
            data.uvRect = glm::vec4(0, 0, 1, 1);

            instances.push_back(data);
        }

        renderer.DrawImageInstanced(titleImage, instances);
    }

}

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

void MainMenu::Free()
{
    CrashHandler::Safe("MainMenu", "Free", [&] {
        std::cout << "Main Menu Freed!!!" << std::endl;
    });
}

void MainMenu::Unload()
{
    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Unload", [&] {
        std::cout << "Main Menu Unloaded!!!" << std::endl;
        renderer.UnloadTexture(titleTexture);
        renderer.UnloadTexture(rockTexture);
        renderer.UnloadTexture(ghoulTexture);
        renderer.UnloadTexture(playTexture);
        renderer.UnloadTexture(quitTexture);
    });
}