#pragma once
#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include "Registry.h"
#include "GameStates.h"

enum class GameState
{
    GS_MAIN_MENU = 0,
    GS_LEVEL1,
    GS_RESTART,
    GS_QUIT
};

class GSM
{
private:
    // Gamestate identifiers
    GameState currentState;
    GameState previousState;
    GameState nextState;

public:

#pragma region GSM LOGIC
    // ctor
    GSM() {
        currentState = previousState = nextState = GameState::GS_MAIN_MENU;
    }

    // Life cycle
    void Init(GameState gameState); // Initialize
    void Close();                   // Shutdown Game State Manager

    // Update Game State Manager
    void Update();

    // Restart
    void Restart();

    // Change to next state
    void ChangeNextState();
#pragma endregion

#pragma region STATE CALLERS
    void CallLoad();    // Load files from assets
    void CallInit();    // Initialize values
    void CallUpdate();  // Update state
    void CallDraw();    // Draw state
    void CallFree();    // Free any allocated memories or values
    void CallUnload();  // Unload files

#pragma endregion STATE CALLERS


#pragma region ACCESSORS
    const GameState& GetCurrentState() const;

    void SetCurrentState(GameState state);

    const GameState& GetNextState() const;

    void SetNextState(GameState state);

    const GameState& GetPreviousState() const;

    void SetPreviousState(GameState state);

#pragma endregion ACCESSORS
    
};






#endif // GAMESTATEMANAGER_H
