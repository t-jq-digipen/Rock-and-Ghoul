#pragma once
#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

class GSM
{
public:
    // Initialize Game State Manager
    static void Init(int gameState);

    // Shutdown Game State Manager
    static void Close();

    // Update Game State Manager
    static void Update();
};

// ----------------------------------------------------------------------------
// Enumeration for game state types
// ----------------------------------------------------------------------------

enum GameStates
{
	GS_MAIN_MENU = 0,
	GS_LEVEL1,
    GS_RESTART,
	GS_QUIT
};

// ----------------------------------------------------------------------------
// Function pointer
// ----------------------------------------------------------------------------
typedef void(*FP)(void);

// ----------------------------------------------------------------------------
// Game states
// ----------------------------------------------------------------------------
extern int currentState, previousState, nextState;

// ----------------------------------------------------------------------------
// Function pointers for respective game state
// ----------------------------------------------------------------------------
extern FP fpLoad, fpInitialize, fpUpdate, fpDraw, fpFree, fpUnload;

#endif // GAMESTATEMANAGER_H