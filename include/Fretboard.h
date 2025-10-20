/* ========================================================
* @file		Fretboard.h
* @author	Yasmin Tazi | yasmin.t@digipen.edu
* @date		03 OCT 2025
*
* @brief	Spell casting system for fretboard using arrow key inputs.
*
* Implements a mechanic where players input arrow key
* sequences to cast spells. Notes appear on a visual fretboard and players
* must press the correct directional keys to complete spell patterns.
*
* Copyright © 2025 DigiPen, All rights reserved.
* ========================================================
*/

#pragma once
#ifndef FRETBOARD_H
#define FRETBOARD_H

#include "Collision.h"
#include <iostream>
#include "Render.h"
#include "Spells.h"

// idt i need this for now until i figure out a way to tie it to input stuff 
enum ArrowKeys {
	UP_ARROW = 1,
	RIGHT_ARROW,
	DOWN_ARROW,
	LEFT_ARROW
};

enum SpellState {
	SELECTION,
	CASTING
};


struct FretFrame {
	float x, y;
	float length, width;
};

struct String {	
	Vec2 start, end;
};

struct Note {
	Circle circle;
	int direction =0;  // up = 1
				    // right = 2
					// down = 3
					// left = 4
	int stringIndex=0;     // which string (0 or 1)
	int segmentIndex=0;    // which segment (0, 1, or 2)
	bool isActive;
};
struct Spell {
	const char* name;
	int sequence[5];
	int length;		// how many arrows (because theyre diff)
};

/**
 * @brief Manages spell casting with visual fretboard (on console for now)
 *
 * Handles spell selection, arrow note spawning, input validation,
 * and progression through spell casting sequences.
 */
class FretBoard {
private:
	FretFrame fretframe;
	String strings[2];
	int segments;

	Note currNote;
	int currNoteIndex;

	SpellState currSpellState;
	static const Spell spellList[4];
	//bool waitingForInput;   // waiting for player response


	GLuint arrowUpTex = 0;
	GLuint arrowRightTex = 0;
	GLuint arrowDownTex = 0 ;
	GLuint arrowLeftTex = 0;
	SpellManager spellManager; //fretboard owns spells.h
	Spell currSpell{};
public:
/**
 * @brief Constructs a fretboard with specified dimensions.
 *
 * @param x The x-coordinate of the fretboard.
 * @param y The y-coordinate of the fretboard.
 * @param length The horizontal length of the fretboard.
 * @param width The vertical width of the fretboard.
 */
	FretBoard(float x, float y, float length, float width);

/**
 * @brief Gets reference to the spell manager.
 *
 * @return SpellManager& Reference to the internal spell manager.
 */
	SpellManager& GetSpellManager() { return spellManager; }

/**
* @brief Displays spell selection menu to console.
*/
	void SpellMenu();

/**
* @brief Handles player input during spell selection state.
*
* Checks for number keys 1-4 to select spells.
*/
	void HandleSpellSelection();

/**
 * @brief Processes arrow key input during spell casting.
 *
 * Validates player input against expected note direction.
 * Progresses or resets spell sequence based on correctness.
 */
	void HandleSpellCasting();

/**
 * @brief Initiates casting of the specified spell.
 *
 * @param spellIndex Index into the spellList array.
 */
	void StartSpell(int spellIndex);

/**
 * @brief Spawns the next note in the current spell sequence.
 *
 * Completes spell and returns to selection if sequence finished.
 */
	void NextNote();

/**
 * @brief Calculates screen position for a note on the fretboard.
 *
 * @param stringIndex Which string the note is on (0 or 1).
 * @param segmentIndex Which segment of the string (0, 1, or 2).
 * @return Vec2 The calculated screen position.
 */
	Vec2 CalculateNotePosition(int stringIndex, int segmentIndex);

/**
 * @brief Routes player input to appropriate handler based on state.
 */
	void PlayerInput();

/**
 * @brief Initializes the fretboard and displays spell menu.
 */
	void StartGame();

	const Note& GetcurrNote() const { return currNote; }
	const String* GetStrings() const { return strings; }
	const FretFrame& GetFretboard() const { return fretframe; }
	const char* GetCurrentSpellName() const { return currSpell.name; }
	int GetSpellLength() const { return currSpell.length; }

/**
 * @brief Renders the current active note arrow on screen (for future use).
 *
 * @param renderer Reference to the rendering system.
 */
	void Draw(Render& renderer);
	void LoadTextures();
	~FretBoard();
};
#endif