/* ========================================================
 * @file	MainMenu.cpp
 * @author	Manfred Sim | manfred.sim@digipen.edu
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
#include "GuiSystem.h"
// Initialize Audio Manager
AudioManager* MainMenu::audio = nullptr;
bool showInstancingDemo = false;

GLuint mainMenuBG = 0;
// static GuiSystem guiSystem;
/**
* @brief Sets the AudioManager instance for the MainMenu.
*
* This function assigns an AudioManager pointer to the MainMenu, allowing
* the menu to access audio playback functionality. The AudioManager is used
* to play sound effects during certain instances.
*
* @param audioMgr Pointer to the AudioManager instance to be used by this
*                 MainMenu.
*/
// Set Audio Manager
void MainMenu::SetAudioManager(AudioManager* audioMgr) {
    audio = audioMgr;
    GuiSystem::SetAudioManager(audioMgr);
}

/**
* @brief Plays the button click sound effect for the Play button.
*
* This function triggers the playback of a button click sound specifically
* for the Play button interaction. If no AudioManager has been set, the
* function returns without playing any sound.
*
*/
// Play Audio When Button Clicked
void MainMenu::PlayButtonClick() {
    if (audio) {
        audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
    }
}

// Store Textures
//GLuint titleTexture{ 0 };
//GLuint rockTexture{ 0 };
//GLuint ghoulTexture{ 0 };
//GLuint playTexture{ 0 };
//GLuint quitTexture{ 0 };
GLuint MainMenuTexture{ 0 };
GLuint mainMenuBG1{ 0 };
//Image titleImage;

// Convert GLFW/screen cords (origin: top-left) to render coords (origin: bottom-left)
static inline glm::vec2 ScreenToRender(const glm::vec2& mouseScreen, float windowHeight)
{
    return { mouseScreen.x, windowHeight - mouseScreen.y };
}

// Loads Main Menu
void MainMenu::Load()
{
    //Render& renderer = Render::GetInstance();

    // Preload all Image assets from JSON (they’ll be stored in textureCache)
    //auto menuTextures = renderer.LoadImageAssetsFromJSON("../Assets/imageAssets.json"); //i added an imageAssets.json inside assets folder
    //MainMenuTexture = menuTextures["mainmenu"]; // alias from JSON
    //CrashHandler::LogIfZeroTexture(MainMenuTexture, "../Assets/MainMenu_Rock&Ghoul.PNG");
    //CrashHandler::Safe("MainMenu", "Load", [&] {
    //    std::cout << "Main Menu Loaded!!!" << std::endl;
    Render& renderer = Render::GetInstance();
    renderer.useCamera = false;
    auto FontAssets = renderer.LoadFontAssetsFromJSON("../Assets/fontAssets.json");
    CrashHandler::Safe("MainMenu", "Load", [&] {
        std::cout << "Main Menu Loaded!!!" << std::endl;
        mainMenuBG1 = renderer.LoadTexture("../Assets/MainMenu_Background.JPG");
        mainMenuBG = renderer.LoadTexture("../Assets/MainMenu_Rock&Ghoul.PNG");
        CrashHandler::LogIfZeroTexture(mainMenuBG, "../Assets/MainMenu_Rock&Ghoul.PNG");
        });
    RenderSystem::AddBackgroundTexture(mainMenuBG1);
    RenderSystem::AddBackgroundTexture(mainMenuBG);
        // Load Textures
      /*  titleTexture = renderer.LoadTexture("../Assets/title.png");
        CrashHandler::LogIfZeroTexture(titleTexture, "../Assets/title.png");

        //rockTexture = renderer.LoadTexture("../Assets/Rock_2048px.png");
        rockTexture = menuTextures["rock"];
        CrashHandler::LogIfZeroTexture(rockTexture, "../Assets/title.png");

        //ghoulTexture = renderer.LoadTexture("../Assets/Ghoul_2048px.png");
        ghoulTexture = menuTextures["ghoul"];
        CrashHandler::LogIfZeroTexture(ghoulTexture, "../Assets/title.png");

        //playTexture = renderer.LoadTexture("../Assets/play.png");
        playTexture = menuTextures["play"];
        CrashHandler::LogIfZeroTexture(playTexture, "../Assets/title.png");

        quitTexture = renderer.LoadTexture("../Assets/quit.png");
        CrashHandler::LogIfZeroTexture(quitTexture, "../Assets/title.png");*/

      /*  MainMenuTexture = renderer.LoadTexture("../Assets/MainMenu_Rock&Ghoul.PNG");
        CrashHandler::LogIfZeroTexture(MainMenuTexture, "../Assets/MainMenu_Rock&Ghoul.PNG");*/
    
    //titleImage = Render::GetInstance().LoadImage("../Assets/quit.png"); 
}

// Initializes Main Menu
void MainMenu::Init()
{
    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Init", [&] {
        std::cout << "Main Menu Initialized!!!" << std::endl;
        renderer.Init();
        GuiSystem::Get().Initialize();

        //guiSystem.AddButton("PLAY",
        //    { 1600, 600 },
        //    { 200.f, 100.f },
        //    { 20.0f / 255.0f, 182.0f / 255.0f, 191.0f / 255.0f },
        //    [] { GSM_SetNextState(GameState::GS_LEVEL1); },
        //    AudioSystem::ButtonType::PLAY);

        float uiScaleX = GLApp::width / 1920.0f;
        float uiScaleY = GLApp::height / 1080.0f;
        float uiScale = std::min(uiScaleX, uiScaleY);

        GuiSystem::Get().AddButton({
            .label = "PLAY",
            .pos = {1650.0f * uiScaleX, 550.0f * uiScaleY},
            .size = {300.0f * uiScale, 100.0f * uiScale},
            .color = {20.0f / 255.0f, 182.0f / 255.0f, 191.0f / 255.0f},
            .onClick = [] { GSM_SetNextState(GameState::GS_LEVEL1); },
            .soundType = AudioSystem::ButtonType::PLAY,
            .shape = ButtonShape::Rounded
            });

        GuiSystem::Get().AddButton({
            .label = "QUIT",
            .pos = {1650.0f * uiScaleX, 150.0f * uiScaleY},
            .size = {300.0f * uiScale, 100.0f * uiScale},
            .color = {20.0f / 255.0f, 182.0f / 255.0f, 191.0f / 255.0f},
            .onClick = [] {
                // Wait for sound to finish before quitting
                double startTime = glfwGetTime();
                while (glfwGetTime() - startTime < 0.4) {  // 400ms delay
                    AudioSystem::Update();
                    glfwPollEvents();
                }
                GSM_SetNextState(GameState::GS_QUIT);
            },
            .soundType = AudioSystem::ButtonType::QUIT,
            .shape = ButtonShape::Rounded
            });

        GuiSystem::Get().AddTextBox({
            .text = "Press 8 or Click Play",
            .pos = {1650.0f * uiScaleX, 50.0f * uiScaleY},
            .scale = 0.5f * uiScale,
            .color = {0.f, 0.f, 0.f},
            .centered = true
            });

    });
}

// Updates Main Menu
//void MainMenu::Update()
//{
//    Render& renderer = Render::GetInstance();
//    CrashHandler::Safe("MainMenu", "Update", [&] {
//
//        // Clear background, add background color
//        renderer.ClearBackground({ 0.5f, 0.f, 0.7f, 1.f });
//      //if (MainMenuTexture)
//       // {
//            //renderer.DrawTexturedQuad({ GLApp::width * 0.5f, GLApp::height * 0.5f }, { 1920.f, 1080.f}, MainMenuTexture);
//        //    if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
//        //    {
//        //        if (audio) 
//        //        {
//        //            audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
//        //        }
//        //        else
//        //        {
//        //            CrashHandler::Log("AudioManager is null on PLAY click");
//        //        }
//
//
//
//        //        // test to hear audio on LEFT SPEAKER
//        //        //std::cout << "[Test] Playing PLAY click on LEFT" << std::endl;
//        //        //AudioSystem::PlaySoundAt("button_play", -300.0f, 0.0f, 1.0f);
//        //        //GSM_SetNextState(GameState::GS_LEVEL1);
//        //    }
//        //}
//        //else
//        //{
//        //    CrashHandler::Log("MainMenuTexture texture missing; skipping draw!");
//        //    if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
//        //    {
//        //        if (audio) 
//        //        {
//        //            audio->PlayButtonSound(AudioSystem::ButtonType::QUIT);
//        //        }
//        //        else
//        //        {
//        //            CrashHandler::Log("AudioManager is null on QUIT click");
//        //        }
//        //        // test to hear audio on the RIGHT SPEAKER
//        //        //std::cout << "[Test] Playing QUIT click on RIGHT" << std::endl;
//        //        //AudioSystem::PlaySoundAt("button_quit", 300.0f, 0.0f, 1.0f);
//
//        //        // Wait a bit for the sound to play before quitting
//        //        double startTime = glfwGetTime();
//        //        while (glfwGetTime() - startTime < 0.5) // Wait 500ms
//        //        {
//        //            // Keep updating audio system during the wait
//        //            AudioSystem::Update();
//        //            glfwPollEvents();
//        //        }
//        //        //GSM_SetNextState(GameState::GS_QUIT);
//        //    }
//        //}
//        //guiSystem.Update(0.0f);
//        // 
//            if (MainMenuTexture)
//            {
//                renderer.DrawTexturedQuad({ GLApp::width * 0.5f, GLApp::height * 0.5f }, { 1920.f, 1080.f }, MainMenuTexture);
//            }
//            else
//            {
//            CrashHandler::Log("MainMenuTexture texture missing; skipping draw!");
//            }
//
//            guiSystem.Update(0.0f);
//        // Alternatively press SPACE to enter LEVEL 1
//        if (Input::IsKeyPressed(GLFW_KEY_8))
//        {
//            std::cout << "PLAY CLICKED switching to Level1\n";
//            if (audio) 
//            {
//                audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
//            }
//            else
//            {
//                CrashHandler::Log("AudioManager is null on SPACE press");
//            }
//            // test to hear centred audio
//            //std::cout << "[Test] Playing SPACE at CENTER" << std::endl;
//            //AudioSystem::PlaySoundAt("button_play", 0.0f, 0.0f, 1.0f);
//            GSM_SetNextState(GameState::GS_LEVEL1);
//        }
//
//        // Non-fatal test: press F2 to simulate a runtime error, writes to log file (should NOT quit the program!)
//        if (Input::IsKeyPressed(GLFW_KEY_F2))
//        {
//            int rc = CrashHandler::RunProtected([] {
//                throw std::runtime_error("F2 has been pressed! std::exception test successful!");
//            });
//            CrashHandler::Logf("int rc = %d (expected value: 1)", rc);
//            return;
//        }
//
//        // Fatal test: press F3 to simulate a crash, program automatically hard-exits
//        if (Input::IsKeyPressed(GLFW_KEY_F3))
//        {
//            throw std::runtime_error("F3 has been pressed! std::terminate handler test successful!");
//        }
//
//        // Fatal signal test: press F4 to simulate an abortion (DON'T ALT + F4 THOUGH)
//        if (Input::IsKeyPressed(GLFW_KEY_F4))
//        {
//            CrashHandler::Log("F4 has been pressed! std::abort test successful!");
//            std::abort();
//        }
//    });
//
//
//
//    // Render main menu textures (placeholders rn)
//    
//    //just testing stuff for rendering (can delete)
//    //renderer.DrawImage(titleImage, 400.f, 400.f);
//    //renderer.DrawImageRotated(titleImage, 400.f, 400.f, 1.57f);
//    //renderer.DrawImageSub(titleImage, 400.f, 400.f, 100.f, 400.f, 0.f, 0.f, 256.f, 256.f);
//    //renderer.DrawText("Let's play", 400.f, 700.f, 0.75f, { 0.f,0.f,1.f });
//    //renderer.DrawText("Game Over", 1400.f, 700.f, 0.75f, { 1.f,1.f,1.f });
////
////    if (Input::IsKeyPressed(GLFW_KEY_B)) {
////        //std::cout << showInstancingDemo << std::endl;
////        showInstancingDemo = !showInstancingDemo;
////    }
////    if (showInstancingDemo) {
////        std::vector<InstanceData> instances;
////
////        int totalInstances = 2500;
////        
////        // Calculate grid dimensions that fit the screen
////        float aspectRatio = (float)GLApp::width / (float)GLApp::height;
////        int cols = static_cast<int>(std::sqrt(totalInstances * aspectRatio));
////        int rows = static_cast<int>(std::ceil((float)totalInstances / cols));
////
////        // Calculate space to fit screen
////        float paddingX = 5.0f;
////        float paddingY = 5.0f;
////        float spacingX = (GLApp::width - 2 * paddingX) / cols;
////        float spacingY = (GLApp::height - 2 * paddingY) / rows;
////
////        // Use the smaller spacing to ensure everything fits
////        float spacing = std::min(spacingX, spacingY);
////
////        // Calculate sprite size 
////        float spriteSize = spacing * 0.8f;
////
////        for (int i = 0; i < totalInstances; i++) {
////            InstanceData data;
////
////            // Grid layout centered on screen
////            int col = i % cols;
////            int row = i / cols;
////
////            data.position = glm::vec2(
////                paddingX + col * spacing + spacing * 0.5f,  // Center
////                paddingY + row * spacing + spacing * 0.5f
////            );
////
////            // Sprite size
////            data.scale = glm::vec2(spriteSize, spriteSize);
////
////            // Rotation over time (animation)
////            data.rotation = static_cast<float>(glfwGetTime() + i * 0.1f);
////
////            // Full texture
////            data.uvRect = glm::vec4(0, 0, 1, 1);
////
////            instances.push_back(data);
////        }
////        double currentTime = glfwGetTime();
////        frames++;
////        if (currentTime - lastTime >= 1.0) { // update once per second
////            fps = double(frames) / (currentTime - lastTime);
////            frames = 0;
////            lastTime = currentTime;
////        }
////        renderer.DrawImageInstanced(titleImage, instances);
////        std::ostringstream oss;
////        oss << "Rock & Ghoul - Instancing: " << totalInstances << " | FPS: " << int(fps);
////        glfwSetWindowTitle(GLApp::ptr_window, oss.str().c_str());
////    }
////
//}
void MainMenu::Update()
{
    Render& renderer = Render::GetInstance();
    renderer.UpdateProjection();
    CrashHandler::Safe("MainMenu", "Update", [&] {

        // Clear background
        //renderer.ClearBackground({ 0.5f, 0.f, 0.7f, 1.f });

    /*    if (MainMenuTexture)
        {
            RenderSystem::SetBackgroundTexture(MainMenuTexture);
            
            renderer.DrawTexturedQuad({ GLApp::width * 0.5f, GLApp::height * 0.5f }, { 1920.f, 1080.f}, MainMenuTexture);
        }
        else
        {
            CrashHandler::Log("MainMenuTexture texture missing; skipping draw!");
        }*/


        if (Input::IsKeyPressed(GLFW_KEY_8))
        {
            std::cout << "PLAY CLICKED switching to Level1\n";
            if (audio)
                audio->PlayButtonSound(AudioSystem::ButtonType::PLAY);
            else
                CrashHandler::Log("AudioManager is null on SPACE press");

            GSM_SetNextState(GameState::GS_LEVEL1);
        }

        if (Input::IsKeyPressed(GLFW_KEY_F2))
        {
            int rc = CrashHandler::RunProtected([] {
                throw std::runtime_error("F2 has been pressed! std::exception test successful!");
                });
            CrashHandler::Logf("int rc = %d (expected value: 1)", rc);
            return;
        }

        if (Input::IsKeyPressed(GLFW_KEY_F3))
            throw std::runtime_error("F3 has been pressed! std::terminate handler test successful!");

        if (Input::IsKeyPressed(GLFW_KEY_F4))
        {
            CrashHandler::Log("F4 has been pressed! std::abort test successful!");
            std::abort();
        }
    });



    // Render main menu textures (placeholders rn)
    
    //just testing stuff for rendering (can delete)
    //renderer.DrawImage(titleImage, 400.f, 400.f);
    //renderer.DrawImageRotated(titleImage, 400.f, 400.f, 1.57f);
    //renderer.DrawImageSub(titleImage, 400.f, 400.f, 100.f, 400.f, 0.f, 0.f, 256.f, 256.f);
    
    //Two fonts different font type testing (using permanent marker and antonio-bold)
    //renderer.DrawTextBox("Let's play and try to make this text box work idk it should wrap into the new line and look nice", 400.f, 700.f, 500.f, 0.75f, { 1.f,1.f,1.f }, "Marker");
    //renderer.DrawText("Game Over", 1400.f, 700.f, 0.75f, { 1.f,1.f,1.f }, "Bold");
//
//    if (Input::IsKeyPressed(GLFW_KEY_B)) {
//        //std::cout << showInstancingDemo << std::endl;
//        showInstancingDemo = !showInstancingDemo;
//    }
//    if (showInstancingDemo) {
//        std::vector<InstanceData> instances;
//
//        int totalInstances = 2500;
//        
//        // Calculate grid dimensions that fit the screen
//        float aspectRatio = (float)GLApp::width / (float)GLApp::height;
//        int cols = static_cast<int>(std::sqrt(totalInstances * aspectRatio));
//        int rows = static_cast<int>(std::ceil((float)totalInstances / cols));
//
//        // Calculate space to fit screen
//        float paddingX = 5.0f;
//        float paddingY = 5.0f;
//        float spacingX = (GLApp::width - 2 * paddingX) / cols;
//        float spacingY = (GLApp::height - 2 * paddingY) / rows;
//
//        // Use the smaller spacing to ensure everything fits
//        float spacing = std::min(spacingX, spacingY);
//
//        // Calculate sprite size 
//        float spriteSize = spacing * 0.8f;
//
//        for (int i = 0; i < totalInstances; i++) {
//            InstanceData data;
//
//            // Grid layout centered on screen
//            int col = i % cols;
//            int row = i / cols;
//
//            data.position = glm::vec2(
//                paddingX + col * spacing + spacing * 0.5f,  // Center
//                paddingY + row * spacing + spacing * 0.5f
//            );
//
//            // Sprite size
//            data.scale = glm::vec2(spriteSize, spriteSize);
//
//            // Rotation over time (animation)
//            data.rotation = static_cast<float>(glfwGetTime() + i * 0.1f);
//
//            // Full texture
//            data.uvRect = glm::vec4(0, 0, 1, 1);
//
//            instances.push_back(data);
//        }
//        double currentTime = glfwGetTime();
//        frames++;
//        if (currentTime - lastTime >= 1.0) { // update once per second
//            fps = double(frames) / (currentTime - lastTime);
//            frames = 0;
//            lastTime = currentTime;
//        }
//        renderer.DrawImageInstanced(titleImage, instances);
//        std::ostringstream oss;
//        oss << "Rock & Ghoul - Instancing: " << totalInstances << " | FPS: " << int(fps);
//        glfwSetWindowTitle(GLApp::ptr_window, oss.str().c_str());
//    }
//
}
// Draws Main Menu
void MainMenu::Draw()
{
    //GuiSystem::Get().Update(0.0f);
//    Render& renderer = Render::GetInstance();
//    CrashHandler::Safe("MainMenu", "Draw", [&] {
//        //// Game Title
//        //if (titleTexture)
//        //{
//        //    renderer.DrawTexturedQuad({ GLApp::width * 0.5f, GLApp::height * 0.8f }, { 1200.f, 300.f }, titleTexture);
//        //}
//        //else
//        //{
//        //    CrashHandler::Log("Title texture missing; skipping draw!");
//        //}
//
//        //// ROCK
//        //if (rockTexture)
//        //{
//        //    renderer.DrawTexturedQuad({ GLApp::width * 0.3f, GLApp::height * 0.5f }, { 400.f, 400.f }, rockTexture);
//        //}
//        //else
//        //{
//        //    CrashHandler::Log("Rock texture missing; skipping draw!");
//        //}
//
//        //// GHOUL (image is reversed)
//        //if (ghoulTexture)
//        //{
//        //    renderer.DrawTexturedQuad({ GLApp::width * 0.7f, GLApp::height * 0.5f }, { -400.f, 400.f }, ghoulTexture);
//        //}
//        //else
//        //{
//        //    CrashHandler::Log("Ghoul texture missing; skipping draw!");
//        //}
//
//        //// PLAY text for button
//        //if (playTexture)
//        //{
//        //    renderer.DrawTexturedQuad({ GLApp::width * 0.3f, GLApp::height * 0.2f }, { 300.f, 150.f }, playTexture);
//        //}
//        //else
//        //{
//        //    CrashHandler::Log("PLAY button texture missing; skipping draw!");
//        //}
//
//        // QUIT text for button
//        if (MainMenuTexture)
//        {
//            renderer.DrawTexturedQuad({ GLApp::width * 0.5f, GLApp::height * 0.5f }, { 1920.f, 1080.f}, MainMenuTexture);
//        }
//        else
//        {
//            CrashHandler::Log("MainMenuTexture texture missing; skipping draw!");
//        }
//    });
}

// Frees Main Menu
void MainMenu::Free()
{
    CrashHandler::Safe("MainMenu", "Free", [&] {
        std::cout << "Main Menu Freed!!!" << std::endl;
        GuiSystem::Get().Shutdown();
    });
}

// Unloads Main Menu
void MainMenu::Unload()
{
    //Render& renderer = Render::GetInstance();
    //CrashHandler::Safe("MainMenu", "Unload", [&] {
    //    std::cout << "Main Menu Unloaded!!!" << std::endl;

    //    // Unload Textures
    //    //renderer.UnloadTexture(titleTexture);
    //    //renderer.UnloadTexture(rockTexture);
    //    //renderer.UnloadTexture(ghoulTexture);
    //    //renderer.UnloadTexture(playTexture);
    //    //renderer.UnloadTexture(quitTexture);
    //    renderer.UnloadTexture(MainMenuTexture);
    //});

    Render& renderer = Render::GetInstance();
    CrashHandler::Safe("MainMenu", "Unload", [&] {
        std::cout << "Main Menu Unloaded!!!" << std::endl;
        renderer.UnloadTexture(mainMenuBG);
        renderer.UnloadTexture(mainMenuBG1);
        mainMenuBG = 0;
        mainMenuBG1 = 0;
        renderer.UnloadAllFonts();
        });
}