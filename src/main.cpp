/*!
@file    	main.cpp
@author  	pghali@digipen.edu
@co-author  parminder.singh@digipen.edu
@date    	17/04/2024

This file uses functionality defined in type GLApp to initialize an OpenGL
context and implement a game loop.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
// Extension loader library's header must be included before GLFW's header!!!
#include "../include/glapp.h"
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "fmod.hpp"
#include "Input.h"
#include "Player.h"
#include "Audio.h"
#include "Collision.h"
/*                                                   type declarations
----------------------------------------------------------------------------- */

/*                                                      function declarations
----------------------------------------------------------------------------- */
static void draw();
static void update();
static void init();

/*                                                   objects with file scope
----------------------------------------------------------------------------- */

/*                                                      function definitions
----------------------------------------------------------------------------- */
/*  _________________________________________________________________________ */
/*! main

@param none

@return int

Indicates how the program existed. Normal exit is signaled by a return value of
0. Abnormal termination is signaled by a non-zero return value.
Note that the C++ compiler will insert a return 0 statement if one is missing.
*/

int main() {
    // Test FMOD first
    if (!AudioSystem::Init()) {
        std::cout << "FMOD init failed!" << std::endl;
    }

    // Start with a 16:9 aspect ratio
    if (!GLApp::init(2400, 1350, "Rock & Ghoul")) {
        std::cout << "Unable to create OpenGL context" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    FMOD::Sound* bgm = AudioSystem::LoadSound("../Assets/music/bgm.mp3", true);
    AudioSystem::PlaySound(bgm);

    std::vector<Player> players;
    players.emplace_back(200.0f, 200.0f, ControlScheme::WASD);   // Player 1
    players.emplace_back(400.0f, 200.0f, ControlScheme::Arrows); // Player 2

    double lastTime = glfwGetTime();

    // Game loop
    while (!glfwWindowShouldClose(GLApp::ptr_window)) {
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        update();
        
        // update and draw player
        // 1. Compute intended movement
        for (auto& p : players) {
            p.ComputeInput(dt);
        }

        // 2. Collision check
        if (CollisionIntersection_RectRect_Static(players[0].GetFutureAABB(), players[1].GetFutureAABB())) { //where the player will be next frame if they move.
            std::cout << "Collision detected! Players stop." << std::endl;
            players[0].CancelMove();
            players[1].CancelMove();
        }

        // 3. Apply movement + draw
        for (auto& p : players) {
            p.ApplyMove();
            p.Draw();
        }

        // show frame
        glfwSwapBuffers(GLApp::ptr_window);
    }
    AudioSystem::Close();
    GLApp::cleanup();
    return 0;
}

/*  _________________________________________________________________________ */
/*! draw

@param none

@return none

Call application to draw and then swap front and back frame buffers ...
Uses GLHelper::GLFWWindow* to get handle to OpenGL context.
*/
static void draw() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black background
    glClear(GL_COLOR_BUFFER_BIT);


    glfwSwapBuffers(GLApp::ptr_window);
}

/*  _________________________________________________________________________ */
/*! update

@param none

@return none

Make sure callbacks are invoked when state changes in input devices occur.
Ensure time per frame and FPS are recorded.
Let application update state changes (such as animation).
*/
static void update() {
  // process events if any associated with input devices
  glfwPollEvents();

  // main loop computes fps and other time related stuff once for all apps ...
  GLApp::update_time(1.0);

  // animate scene
  GLApp::update();
  Input::Update();
  AudioSystem::Update();
}
