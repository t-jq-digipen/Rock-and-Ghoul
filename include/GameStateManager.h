/* ========================================================
 * @file	    GameStateManager.h
 * @author	    Manfred Sim | manfred.sim@digipen.edu
 * @date	    29 SEP 2025
 *
 * @brief	Game State Manager
 *
 * The game state manager handles the switching of game states
 * while the program is running and calls the corresponding
 * state functions. This header file contains the declaration
 * of functions and enumerations used by the game state manager.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include "Registry.h"
#include "GameStates.h"

// Enumeration for Game States
enum class GameState
{
    GS_MAIN_MENU = 0,
    GS_LEVEL1,
    GS_RESTART,
    GS_QUIT
};

// Class for Game State Manager
class GSM
{
private:
    // GAME STATE IDENTIFIERS
    // ================================================================================
    // Current Game State
    GameState currentState;

    // Previous Game State
    GameState previousState;

    // Next Game State
    GameState nextState;

public:

#pragma region GSM LOGIC
    // Default constructor
    GSM() {
        currentState = previousState = nextState = GameState::GS_MAIN_MENU;
    }

    // Initialize Game State Manager
    void Init(GameState gameState);

    // Shutdown Game State Manager
    void Close();

    // Update Game State Manager
    void Update();

    // Restart Current Game State
    void Restart();

    // Change to Next Game State
    void ChangeNextState();
#pragma endregion

    // GAME STATE CALLERS
    // ----------------------------------------
#pragma region STATE CALLERS
    void CallLoad();    // Load files from assets
    void CallInit();    // Initialize values
    void CallUpdate();  // Update state
    void CallDraw();    // Draw state
    void CallFree();    // Free any allocated memories or values
    void CallUnload();  // Unload files
    // ----------------------------------------

#pragma endregion STATE CALLERS

    // GAME STATE ACCESSORS
    // ----------------------------------------
#pragma region ACCESSORS

    // GETTERS
    // ----------------------------------------
    // Get Current Game State
    const GameState& GetCurrentState() const;
    // Get Next Game State
    const GameState& GetNextState() const;
    // Get Previous Game State
    const GameState& GetPreviousState() const;
    // ----------------------------------------

    // SETTERS
    // ----------------------------------------
    // Set Current Game State
    void SetCurrentState(GameState state);
    // Set Next Game State
    void SetNextState(GameState state);
    // Set Previous Game State
    void SetPreviousState(GameState state);
    // ----------------------------------------

#pragma endregion ACCESSORS
    // ----------------------------------------
    
};

#endif // GAMESTATEMANAGER_H
