/* ========================================================
 * @file		GameStateManager.cpp
 * @author		Manfred Sim | manfred.sim@digipen.edu
 * @date		29 SEP 2025
 *
 * @brief	Game State Manager implementation
 *
 * Creates function pointers that point to the current game
 * state's respective functions and defines the state callers
 * and accessors in the GSM class for handling the switching
 * of game states.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "GameStateManager.h"
#include "MainMenu.h"
#include "Level1.h"
#include "Level2.h"
#include "CoreEngine.h"

// Function Pointer
typedef void(*FP)(void);

// Function Pointers for gamestates
FP fpLoad = nullptr;
FP fpInitialize = nullptr;
FP fpUpdate = nullptr;
FP fpDraw = nullptr;
FP fpFree = nullptr;
FP fpUnload = nullptr;

// Initialize Game State Manager
void GSM::Init(GameState gameState)
{
	currentState = nextState = previousState = gameState;
}

// Shutdown Game State Manager
void GSM::Close()
{
	// free and unload?
}

// Update Game State Manager
void GSM::Update()
{
	std::cout << "GSM Update state to ";
	// Set function pointers to current game state's functionalities
	switch (currentState)
	{
	case GameState::GS_MAIN_MENU:
		std::cout << "Main Menu\n";
			fpLoad			= MainMenu::Load;
			fpInitialize	= MainMenu::Init;
			fpUpdate		= MainMenu::Update;
			fpDraw			= MainMenu::Draw;
			fpFree			= MainMenu::Free;
			fpUnload		= MainMenu::Unload;
			break;
			// end case GS_MAIN_MENU

	case GameState::GS_LEVEL1:
		std::cout << "Level 1\n";
			fpLoad			= Level1::Load;
			fpInitialize	= Level1::Init;
			fpUpdate		= Level1::Update;
			fpDraw			= Level1::Draw;
			fpFree			= Level1::Free;
			fpUnload		= Level1::Unload;
			break;
			// end case GS_LEVEL1

	case GameState::GS_LEVEL2:
		std::cout << "Level 2\n";
			fpLoad			= Level2::Load;
			fpInitialize	= Level2::Init;
			fpUpdate		= Level2::Update;
			fpDraw			= Level2::Draw;
			fpFree			= Level2::Free;
			fpUnload		= Level2::Unload;
			break;
			// end case GS_LEVEL2

		default:
			break;
	}
}

// Restart Current Game State
void GSM::Restart()
{
	nextState = previousState;
	currentState = previousState;
}

// Change to Next Game State
void GSM::ChangeNextState()
{
	previousState = currentState;
	currentState = nextState;
}

// STATE CALLERS
// ----------------------------------------
void GSM::CallLoad()	{ if (fpLoad)		{ fpLoad();			} }	// Load files from assets
void GSM::CallInit()	{ if (fpInitialize) { fpInitialize();	} }	// Initialize values
void GSM::CallUpdate()	{ if (fpUpdate)		{ fpUpdate();		} }	// Update state
void GSM::CallDraw()	{ if (fpDraw)		{ fpDraw();			} }	// Draw state
void GSM::CallFree()	{ if (fpFree)		{ fpFree();			} }	// Free any allocated memories or values
void GSM::CallUnload()	{ if (fpUnload)		{ fpUnload();		} }	// Unload files
// ----------------------------------------

// GETTERS
// ----------------------------------------
const GameState& GSM::GetCurrentState() const	{ return currentState; }	// Get Current Game State
const GameState& GSM::GetNextState() const		{ return nextState; }		// Get Next Game State
const GameState& GSM::GetPreviousState() const	{ return previousState; }	// Get Previous Game State
// ----------------------------------------

// SETTERS
// ----------------------------------------
void GSM::SetCurrentState(GameState state)  { currentState	= state; }		// Set Current Game State
void GSM::SetNextState(GameState state)		{ nextState		= state; }		// Set Next Game State
void GSM::SetPreviousState(GameState state) { previousState = state; }		// Set Previous Game State
// ----------------------------------------
