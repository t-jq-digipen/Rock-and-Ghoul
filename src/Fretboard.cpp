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

    notesPlayed = 0;
    waitingForInput = false;


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

    arrowUpTex = Render::LoadTexture("../Assets/up.png");
    arrowRightTex = Render::LoadTexture("../Assets/right.png");
    arrowDownTex = Render::LoadTexture("../Assets/down.png");
    arrowLeftTex = Render::LoadTexture("../Assets/left.png");

}

Vec2 FretBoard::CalculateNotePosition(int stringIndex, int segmentIndex) {

    String& spawnString = strings[stringIndex];

// i used claude for this... basically to center the circles
    float segmentProgress = (float)(segmentIndex * 2 + 1) / (float)(segments * 2);

    Vec2 stringDirection = {
        spawnString.end.x - spawnString.start.x,  
        spawnString.end.y - spawnString.start.y  
    };

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
            std::cout << "Correct key pressed! YAY!\n";
            break;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            std::cout << "Wrong key pressed! GRRR!\n";
            break;
        }
    case 2:
        if (Input::IsKeyPressed(GLFW_KEY_RIGHT)) {
            std::cout << "Correct key pressed! YAY!\n";
            break;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            std::cout << "Wrong key pressed! GRRR!\n";
            break;
        }
    case 3:
        if (Input::IsKeyPressed(GLFW_KEY_DOWN)) {
            std::cout << "Correct key pressed! YAY!\n";
            break;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            std::cout << "Wrong key pressed! GRRR!\n";
            break;
        }
    case 4:
        if (Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            std::cout << "Correct key pressed! YAY!\n";
            break;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_UP)) {
            std::cout << "Wrong key pressed! GRRR!\n";
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
        std::cout << "Game Over! \n";
    }

}

bool FretBoard::Continue() {
    // sequence of 5 notes played
    return notesPlayed < 5;

}

void FretBoard::StartGame() {

    notesPlayed = 0;
    waitingForInput = false;
    currNote.isActive = false;

    std::cout << "Starting Game\n";
    printf("1 = Up, 2 = Right, 3 = Down, 4 = Left\n");

    NewNote();
}
void FretBoard::Draw(Render& renderer) {
    if (!currNote.isActive) return;

    GLuint tex = 0;
    switch (currNote.direction) {
    case 1: tex = arrowUpTex; break;
    case 2: tex = arrowRightTex; break;
    case 3: tex = arrowDownTex; break;
    case 4: tex = arrowLeftTex; break;
    }

    if (tex != 0) {
        glm::vec2 pos(currNote.circle.x, currNote.circle.y);
        glm::vec2 size(40.0f, 40.0f); // adjust to match PNG scale
        renderer.DrawTexturedQuad(pos, size, tex);
    }
}