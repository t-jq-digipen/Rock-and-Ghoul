#include "GameStateManager.h"
#include "MainMenu.h"
#include "Level1.h"

#include "CoreEngine.h"

// GameStateManager.cpp - ADD THESE AT THE TOP:
static GameState currentState = GameState::GS_MAIN_MENU;
static GameState nextState = GameState::GS_MAIN_MENU;
static GameState previousState = GameState::GS_MAIN_MENU;

// Include your AudioManager and FMOD
#include "AudioSystem.h"

// Function pointer
typedef void(*FP)(void);

// Function Pointers for gamestates
FP fpLoad = nullptr;
FP fpInitialize = nullptr;
FP fpUpdate = nullptr;
FP fpDraw = nullptr;
FP fpFree = nullptr;
FP fpUnload = nullptr;

// FIX: Use pointer to match MainMenu's declaration
extern AudioManager* audio;  // CHANGED FROM audioManager

// FIX: Use consistent naming with MainMenu
extern FMOD::Sound* menuMusic;  // CHANGED FROM trackMainMenu
extern FMOD::Sound* gameMusic;  // CHANGED FROM trackLevel1

// To track if we started music already on Init
static bool musicStarted = false;

void GSM::Init(GameState gameState)
{
	currentState = nextState = previousState = gameState;

	// Always start menu music if we're in main menu, regardless of musicStarted flag
	if (currentState == GameState::GS_MAIN_MENU)
	{
		if (audio && menuMusic) {
			audio->PlayMusic(menuMusic, false);
			musicStarted = true;
			std::cout << "Started main menu music\n";
		}
	}
}

void GSM::Close()
{
	// free and unload?
}

void GSM::Update()
{
    std::cout << "GSM Update - Current: " << static_cast<int>(currentState)
        << ", Next: " << static_cast<int>(nextState) << std::endl;

    // Check if state is changing
    if (nextState != currentState)
    {
        std::cout << "STATE CHANGE DETECTED: " << static_cast<int>(currentState)
            << " -> " << static_cast<int>(nextState) << std::endl;

        // If we are switching to LEVEL1 from another state, crossfade music
        if (nextState == GameState::GS_LEVEL1)
        {
            std::cout << "Crossfading music to Level 1 track\n";
            std::cout << "Audio: " << audio << ", Game Music: " << gameMusic << std::endl;
            if (audio && gameMusic) {
                audio->CrossfadeTo(gameMusic, 3.0f);  // 3 second crossfade
                std::cout << "CROSSFADE INITIATED to game music\n";
            }
            else {
                std::cout << "CROSSFADE FAILED - audio or gameMusic is null\n";
                if (!audio) std::cout << "Audio manager is null!\n";
                if (!gameMusic) std::cout << "Game music is null!\n";
            }
        }
        // If we are switching back to MAIN_MENU, crossfade to menu music
        else if (nextState == GameState::GS_MAIN_MENU)
        {
            std::cout << "Crossfading music to Main Menu track\n";
            std::cout << "Audio: " << audio << ", Menu Music: " << menuMusic << std::endl;
            if (audio && menuMusic) {
                audio->CrossfadeTo(menuMusic, 3.0f);
                std::cout << "CROSSFADE INITIATED to menu music\n";
            }
            else {
                std::cout << "CROSSFADE FAILED - audio or menuMusic is null\n";
                if (!audio) std::cout << "Audio manager is null!\n";
                if (!menuMusic) std::cout << "Menu music is null!\n";
            }
        }

        // UPDATE THE CURRENT STATE
        previousState = currentState;
        currentState = nextState;
        std::cout << "State updated - Current: " << static_cast<int>(currentState)
            << ", Previous: " << static_cast<int>(previousState) << std::endl;
    }

    switch (currentState)
    {
    case GameState::GS_MAIN_MENU:
        std::cout << "Main Menu\n";
        fpLoad = MainMenu::Load;
        fpInitialize = MainMenu::Init;
        fpUpdate = MainMenu::Update;
        fpDraw = MainMenu::Draw;
        fpFree = MainMenu::Free;
        fpUnload = MainMenu::Unload;
        break;

    case GameState::GS_LEVEL1:
        std::cout << "Level 1\n";
        fpLoad = Level1::Load;
        fpInitialize = Level1::Init;
        fpUpdate = Level1::Update;
        fpDraw = Level1::Draw;
        fpFree = Level1::Free;
        fpUnload = Level1::Unload;
        break;

    default:
        break;
    }
}

void GSM::Restart()
{
	nextState = previousState;
	currentState = previousState;
}

void GSM::ChangeNextState() {
	currentState = nextState;
}

// STATE CALLERS
// ----------------------------------------
void GSM::CallLoad() { if (fpLoad) { fpLoad(); } }
void GSM::CallInit() { if (fpInitialize) { fpInitialize(); } }
void GSM::CallUpdate() { if (fpUpdate) { fpUpdate(); } }
void GSM::CallDraw() { if (fpDraw) { fpDraw(); } }
void GSM::CallFree() { if (fpFree) { fpFree(); } }
void GSM::CallUnload() { if (fpUnload) { fpUnload(); } }
// ----------------------------------------

// GETTERS
// ----------------------------------------
const GameState& GSM::GetCurrentState() const { return currentState; }
const GameState& GSM::GetNextState() const { return nextState; }
const GameState& GSM::GetPreviousState() const { return previousState; }
// ----------------------------------------

// SETTERS
// ----------------------------------------
void GSM::SetCurrentState(GameState state) { currentState = state; }
void GSM::SetNextState(GameState state) { nextState = state; }
void GSM::SetPreviousState(GameState state) { previousState = state; }
// ----------------------------------------