#include "GameStateManager.h"
#include "MainMenu.h"
#include "Level1.h"
#include "Level2.h"

// Game states
int currentState = 0, previousState = 0, nextState = 0;

// Function pointers
FP fpLoad = nullptr, fpInitialize = nullptr, fpUpdate = nullptr, fpDraw = nullptr, fpFree = nullptr, fpUnload = nullptr;

void GSM::Init(int gameState)
{
	currentState = previousState = nextState = gameState;
}

void GSM::Close()
{

}

void GSM::Update()
{
	switch (currentState)
	{
	case GS_MAIN_MENU:
		fpLoad = MainMenu::Load;
		fpInitialize = MainMenu::Init;
		fpUpdate = MainMenu::Update;
		fpDraw = MainMenu::Draw;
		fpFree = MainMenu::Free;
		fpUnload = MainMenu::Unload;
		break;
	case GS_LEVEL1:
		fpLoad = Level1::Load;
		fpInitialize = Level1::Init;
		fpUpdate = Level1::Update;
		fpDraw = Level1::Draw;
		fpFree = Level1::Free;
		fpUnload = Level1::Unload;
		break;
	case GS_LEVEL2:
		fpLoad = Level2::Load;
		fpInitialize = Level2::Init;
		fpUpdate = Level2::Update;
		fpDraw = Level2::Draw;
		fpFree = Level2::Free;
		fpUnload = Level2::Unload;
		break;
	default:
		break;
	}
}