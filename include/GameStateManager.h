#pragma once
#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include "Registry.h"

enum class GameState
{
    GS_MAIN_MENU = 0,
    GS_LEVEL1,
    GS_RESTART,
    GS_QUIT
};

class GSM
{
public:
    // Function pointer
    using FP = void(*)(void);

private:
    // Gamestate identifiers
    GameState currentState;
    GameState previousState;
    GameState nextState;

    // Function Pointers for gamestates
    FP fpLoad       = nullptr; 
    FP fpInitialize = nullptr; 
    FP fpUpdate     = nullptr; 
    FP fpDraw       = nullptr;
    FP fpFree       = nullptr;
    FP fpUnload     = nullptr;

    // Registry reference to CoreEngine's registry
    Registry& registry;

public:
    // ctor/initialize GSM
    GSM(GameState gameState, Registry& reg);

#pragma region GSM LOGIC

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

    const GameState& GetNextState() const;

    void SetNextState(GameState state);

    const GameState& GetPreviousState() const;

    void SetPreviousState(GameState state);

#pragma endregion ACCESSORS

    // ECS things
    Registry& GetRegistry();
};


class IGameState {
protected:
    GSM& gsm;
private:
    IGameState(GSM& gsmref) : gsm(gsmref) {}
    ~IGameState() = default;
};



#endif // GAMESTATEMANAGER_H
