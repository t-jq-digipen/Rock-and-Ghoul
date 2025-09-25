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
	int direction;  // up = 1
				    // right = 2
					// down = 3
					// left = 4
	int stringIndex;     // which string (0 or 1)
	int segmentIndex;    // which segment (0, 1, or 2)
	bool isActive;
};
struct Spell {
	const char* name;
	int sequence[5];
	int length;		// how many arrows (because theyre diff)
};

class FretBoard {
private:
	FretFrame fretframe;
	String strings[2];
	int segments;

	Note currNote;
	int currNoteIndex;

	SpellState currSpellState;
	Spell currSpell;
	static const Spell spellList[4];
	//bool waitingForInput;   // waiting for player response


	GLuint arrowUpTex;
	GLuint arrowRightTex;
	GLuint arrowDownTex;
	GLuint arrowLeftTex;
	SpellManager& spellManager;

public:

	FretBoard(float x, float y, float length, float width, SpellManager& sm);

	/* SPELL FUNCTIONS */
	void SpellMenu();
	void HandleSpellSelection();
	void HandleSpellCasting();
	void StartSpell(int spellIndex);
	void NextNote();
	Vec2 CalculateNotePosition(int stringIndex, int segmentIndex);
	void PlayerInput();
	/* */

	void StartGame();

	const Note& GetcurrNote() const { return currNote; }
	const String* GetStrings() const { return strings; }
	const FretFrame& GetFretboard() const { return fretframe; }
	const char* GetCurrentSpellName() const { return currSpell.name; }
	int GetSpellLength() const { return currSpell.length; }

	void Draw(Render& renderer);
	void LoadTextures();
};
#endif