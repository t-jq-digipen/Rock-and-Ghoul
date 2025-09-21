#pragma once
#ifndef FRETBOARD_H
#define FRETBOARD_H

#include "Collision.h"
#include <iostream>

// idt i need this for now until i figure out a way to tie it to input stuff 
enum ArrowKeys {
	UP_ARROW = 1,
	RIGHT_ARROW,
	DOWN_ARROW,
	LEFT_ARROW
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

class FretBoard {
private:
	FretFrame fretframe;
	String strings[2];
	Note currNote;
	int notesPlayed; // count of notes
	int segments;
	bool waitingForInput;   // waiting for player response

public:
	FretBoard(float x, float y, float length, float width);
	void NewNote();
	Vec2 CalculateNotePosition(int stringIndex, int segmentIndex);
	void PlayerInput();
	
	void StartGame();
	bool Continue();

	const Note& GetcurrNote() const { return currNote; }
	const String* GetStrings() const { return strings;  }
	const FretFrame& GetFretboard() const { return fretframe; }
	
	bool WaitingForInput() const { return waitingForInput; }
};
#endif