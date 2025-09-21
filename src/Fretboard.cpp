#include "Fretboard.h"
#include <cstdlib>  // rand()
#include <ctime>    // random number generator seed

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glapp.h"
#include "MovementController.h"

FretBoard::FretBoard(float x, float y, float length, float width) {
    fretframe.x = x;
    fretframe.y = y;
    fretframe.length = length;
    fretframe.width = width;

    segments = 3;
    // segment spacing
    float spacing = width / 3.0f;


    // top string
    strings[0].start.x = x;                    // left edge of fretboard
    strings[0].start.y = y + segments;         // 1/3 down from the top
    strings[0].end.x = x + length;             // right edge of fretboard  
    strings[0].end.y = y + segments;           // same height as start (horizontal)

    // bottom string
    strings[1].start.x = x;                        // left edge of fretboard
    strings[1].start.y = y + 2 * segments;         // 2/3 down from the top
    strings[1].end.x = x + length;                 // right edge of fretboard
    strings[1].end.y = y + 2 * segments;           // same height as start (horizontal)

    currNote.isActive = false;

    srand((unsigned int)time(nullptr));

}

Vec2 FretBoard::CalculateNotePosition(int stringIndex, int segmentIndex) {
    // Get the specific string we're working with
    // This gives us the start and end points that define the string's path
    String& spawnString = strings[stringIndex];

    // Here's the mathematical heart of positioning notes within segments
    // We want notes in the CENTER of each segment, not at the boundaries
    // The formula (segmentIndex * 2 + 1) / (segments * 2) achieves this elegantly
    // For 3 segments: segment 0 gets 1/6, segment 1 gets 3/6, segment 2 gets 5/6
    // This places notes at 16.7%, 50%, and 83.3% along the string - perfect centers!
    float segmentProgress = (float)(segmentIndex * 2 + 1) / (float)(segments * 2);

    // Calculate the direction vector that describes the string's orientation
    // For horizontal strings, this will be purely in the X direction
    Vec2 stringDirection = {
        spawnString.end.x - spawnString.start.x,  // Horizontal distance
        spawnString.end.y - spawnString.start.y   // Vertical distance (should be 0)
    };

    // Use linear interpolation to find the exact position along the string
    // Start at the string's beginning, then move the calculated percentage toward the end
    Vec2 notePos = {
        spawnString.start.x + stringDirection.x * segmentProgress,
        spawnString.start.y + stringDirection.y * segmentProgress
    };

    return notePos;
}

// spawning note
void FretBoard::NewNote() {
    // will be either string 1 or 2
    int spawnString = rand() % 2;

    // which segment the note will spawn in
    int spawnSegment = rand() % segments;

    // direction of note (1, 2, 3, 4)
    int spawnDir = (rand() % 4) + 1; 

    Vec2 notePos = CalculateNotePosition(spawnString, spawnSegment);

    currNote.circle.x = notePos.x;
    currNote.circle.y = notePos.y;
    currNote.circle.rad = 20.0f;
    currNote.direction = spawnDir;
    currNote.stringIndex = spawnString;
    currNote.segmentIndex = spawnSegment;
    currNote.isActive = true;
}

void FretBoard::PlayerInput() {
    switch (currNote.direction) {
    case 1:
        if (Input::IsKeyPressed(GLFW_KEY_UP)) {
            printf("Correct key pressed! YAY!\n");
            break;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            printf("Wrong key pressed! GRRR!\n");
            break;
        }
    case 2:
        if (Input::IsKeyPressed(GLFW_KEY_RIGHT)) {
            printf("Correct key pressed! YAY!\n");
            break;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            printf("Wrong key pressed! GRRR!\n");
            break;
        }
    case 3:
        if (Input::IsKeyPressed(GLFW_KEY_DOWN)) {
            printf("Correct key pressed! YAY!\n");
            break;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            printf("Wrong key pressed! GRRR!\n");
            break;
        }
    case 4:
        if (Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            printf("Correct key pressed! YAY!\n");
            break;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_UP)) {
            printf("Wrong key pressed! GRRR!\n");
            break;
        }
    }

    currNote.isActive = false;        // remove the note from the screen
    waitingForInput = false;          // allow new notes to spawn
    notesPlayed++;                    // increase count of notes

    if (Continue()) {
        NewNote();  // create the next note in the sequence
    }
    else {    
        printf("Game Over! \n");
    }

}

bool FretBoard::Continue() {
    // For now, we'll play exactly 5 notes then stop
    // This provides a manageable game session that's neither too short nor too long
    return notesPlayed < 5;

    // You could enhance this with more sophisticated end conditions:
    // - End after a certain number of mistakes
    // - End when reaching a target score
    // - End based on time elapsed
    // - Continue indefinitely until player chooses to quit
}

void FretBoard::StartGame() {
    // Reset all tracking variables to ensure we start with a clean slate
    // This is important if someone wants to play multiple games in succession
    notesPlayed = 0;
    waitingForInput = false;
    currNote.isActive = false;

    // Provide clear instructions to help the player understand the game
    printf("Starting Fretboard Game!\n");
    printf("Press arrow keys to match the note directions:\n");
    printf("1 = Up, 2 = Right, 3 = Down, 4 = Left\n");
    printf("Good luck!\n\n");

    // Create the very first note to get the game started
    // This begins the cycle of challenge and response that drives your game
    NewNote();
}
