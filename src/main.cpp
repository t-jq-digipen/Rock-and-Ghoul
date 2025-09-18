/*!
@file    	main.cpp
@author  	pghali@digipen.edu
@co-author  parminder.singh@digipen.edu
@date    	17/04/2024

This file now uses the Engine class to manage the entire game.
*/
#include <GL/glew.h>        // First!
#include <GLFW/glfw3.h>     // Then GLFW
#include "glapp.h" 
#include "Engine.h"
#include <iostream>

int main() {
    try {
        // Create engine instance
        Engine gameEngine(2400, 1350, "Rock & Ghoul");

        // Run the game
        gameEngine.Run();

        std::cout << "Game ended successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Engine error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}