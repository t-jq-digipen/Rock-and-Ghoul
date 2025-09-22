#include "Fretboard.h"
#include <cstdlib>  // rand()
#include <ctime>    // random number generator seed

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glapp.h"
#include "MovementController.h"

glm::vec2 arrowPositions[4] = {
    {300.0f, 900.0f}, // LEFT
    {400.0f, 900.0f}, // DOWN
    {500.0f, 900.0f}, // UP
    {600.0f, 900.0f}  // RIGHT
};
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
    // direction of note (1=Up, 2=Right, 3=Down, 4=Left)
    int spawnDir = (rand() % 4) + 1;

    currNote.direction = spawnDir;
    currNote.circle.x = arrowPositions[spawnDir - 1].x;
    currNote.circle.y = arrowPositions[spawnDir - 1].y;
    currNote.circle.rad = 20.0f;
    currNote.isActive = true;

    std::cout << "Spawned note dir = " << currNote.direction
        << " at (" << currNote.circle.x << ", " << currNote.circle.y << ")\n";
}

void FretBoard::PlayerInput() {
    if (!currNote.isActive) return;   // no note to check

    bool keyPressed = false;
    bool correct = false;

    // Check the current expected arrow
    if (currNote.direction == 1) {            // UP
        if (Input::IsKeyPressed(GLFW_KEY_UP)) { 
            correct = true; keyPressed = true; 
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            keyPressed = true;
        }
    }
    else if (currNote.direction == 2) {       // RIGHT
        if (Input::IsKeyPressed(GLFW_KEY_RIGHT)) { 
            correct = true; keyPressed = true; 
        }
        else if (Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_DOWN) ||Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            keyPressed = true;
        }
    }
    else if (currNote.direction == 3) {       // DOWN
        if (Input::IsKeyPressed(GLFW_KEY_DOWN)) { 
            correct = true; keyPressed = true; 
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            keyPressed = true;
        }
    }
    else if (currNote.direction == 4) {       // LEFT
        if (Input::IsKeyPressed(GLFW_KEY_LEFT)) { 
            correct = true; keyPressed = true; 
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_DOWN) ||Input::IsKeyPressed(GLFW_KEY_UP)) {
            keyPressed = true;
        }
    }

    if (!keyPressed) return;   // wait until a key is pressed

    if (correct) {
        std::cout << "Correct key pressed! YAY!\n";
        currNote.isActive = false;
        notesPlayed++;

        if (Continue()) {
            NewNote();              // spawn next
            std::cout << "Next note dir = " << currNote.direction << "\n"; // debug
        }
        else {
            std::cout << "Sequence finished! (5 notes correct)\n";
        }
    }
    else {
        // wrong key ends the game immediately
        std::cout << "Wrong key pressed! GAME OVER!\n";
        currNote.isActive = false;
        notesPlayed = 5;   // force end
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
    waitingForInput = true;
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
        glm::vec2 size(60.0f, 60.0f);
        renderer.DrawTexturedQuad(pos, size, tex);
    }
}

void FretBoard::LoadTextures() {
    arrowUpTex = Render::LoadTexture("../Assets/up.png");
    arrowRightTex = Render::LoadTexture("../Assets/right.png");
    arrowDownTex = Render::LoadTexture("../Assets/down.png");
    arrowLeftTex = Render::LoadTexture("../Assets/left.png");
}