#include "GameStateManager.h"
#include "MainMenu.h"
#include "Level1.h"

#include "CoreEngine.h"


// Function pointer
typedef void(*FP)(void);

// Function Pointers for gamestates
FP fpLoad = nullptr;
FP fpInitialize = nullptr;
FP fpUpdate = nullptr;
FP fpDraw = nullptr;
FP fpFree = nullptr;
FP fpUnload = nullptr;

void GSM::Init(GameState gameState)
{
	currentState = nextState = previousState = gameState;
}

void GSM::Close()
{
	// free and unload?
}

void GSM::Update()
{
	std::cout << "GSM Update state to ";
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

		default:

			break;
	}
}

void GSM::Restart()
{
	nextState = previousState;
	currentState = previousState;
}

void GSM::ChangeNextState()
{
	previousState = currentState;
	currentState = nextState;
}

// STATE CALLERS
// ----------------------------------------
void GSM::CallLoad()	{ if (fpLoad)		{ fpLoad();			} }
void GSM::CallInit()	{ if (fpInitialize) { fpInitialize();	} }
void GSM::CallUpdate()	{ if (fpUpdate)		{ fpUpdate();		} }
void GSM::CallDraw()	{ if (fpDraw)		{ fpDraw();			} }
void GSM::CallFree()	{ if (fpFree)		{ fpFree();			} }
void GSM::CallUnload()	{ if (fpUnload)		{ fpUnload();		} }
// ----------------------------------------

// GETTERS
// ----------------------------------------
const GameState& GSM::GetCurrentState() const	{ return currentState; }
const GameState& GSM::GetNextState() const		{ return nextState; }
const GameState& GSM::GetPreviousState() const	{ return previousState; }
// ----------------------------------------

// SETTERS
// ----------------------------------------
void GSM::SetCurrentState(GameState state)  { currentState	= state; }
void GSM::SetNextState(GameState state)		{ nextState		= state; }
void GSM::SetPreviousState(GameState state) { previousState = state; }
// ----------------------------------------
