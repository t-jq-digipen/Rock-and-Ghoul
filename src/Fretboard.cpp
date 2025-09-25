#include "Fretboard.h"
#include <cstdlib>  // rand()
#include <ctime>    // random number generator seed

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glapp.h"
#include "MovementController.h"
#include "Spells.h"
glm::vec2 arrowPositions[4] = {
    {300.0f, 900.0f}, // LEFT
    {400.0f, 900.0f}, // DOWN
    {500.0f, 900.0f}, // UP
    {600.0f, 900.0f}  // RIGHT
};

const Spell FretBoard::spellList[4] = {
    {"BARRIER",              {4, 1, 2}, 3},  // < ^ >
    {"DIRECTIONAL SHOCKWAVE", {2, 3, 2, 2}, 4},  // > v > > 
    {"AOE BLAST",            {4, 3, 2, 1, 3}, 5},  // < v > ^ v
    {"HEAL",                 {3, 4, 1}, 3}   // v < ^
};

FretBoard::FretBoard(float x, float y, float length, float width, SpellManager& sm) : spellManager(sm) {
    fretframe.x = x;
    fretframe.y = y;
    fretframe.length = length;
    fretframe.width = width;

    segments = 3;
    // segment spacing
    float spacing = width / 3.0f;
    currSpellState = SELECTION;
    currNoteIndex = 0;
    currNote.isActive = false;

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
void FretBoard::SpellMenu() {
    std::cout << "~~~~ CHOOSE YOUR SPELL ~~~~\n";
    std::cout << "Input spell number: \n";
    for (int i{ 0 }; i < 4; i++) {
        std::cout << (i + 1) << ". " << spellList[i].name << "\n";
    }
    currSpellState = SELECTION;
}

void FretBoard::HandleSpellSelection() {
    if (Input::IsKeyPressed(GLFW_KEY_1)) {
        StartSpell(0);
    }
    else if (Input::IsKeyPressed(GLFW_KEY_2)) {
        StartSpell(1);
    }
    else if (Input::IsKeyPressed(GLFW_KEY_3)) {
        StartSpell(2);
    }
    else if (Input::IsKeyPressed(GLFW_KEY_4)) {
        StartSpell(3);
    }
}

void FretBoard::StartSpell(int spellIndex) {
    currSpell = spellList[spellIndex];
    currNoteIndex = 0;
    currSpellState = CASTING;

    std::cout << "\nSPELL NAME: " << currSpell.name << "\n";
    NextNote();
}

void FretBoard::NextNote() {
    if (currNoteIndex >= currSpell.length) {
        std::cout << "SPELL COMPLETE! " << currSpell.name << " WAS CAST!!! YIPPEEEE \n\n";
        if (std::strcmp(currSpell.name, "BARRIER") == 0) {
            spellManager.GiveSpell(spellManager.barrierTex);
        }
        else if (std::strcmp(currSpell.name, "DIRECTIONAL SHOCKWAVE") == 0) {
            spellManager.GiveSpell(spellManager.shockwaveTex);
        }
        else if (std::strcmp(currSpell.name, "AOE BLAST") == 0) {
            spellManager.GiveSpell(spellManager.aoeTex);
        }
        else if (std::strcmp(currSpell.name, "HEAL") == 0) {
            spellManager.GiveSpell(spellManager.healTex);
        }
        SpellMenu();
        return;
    }

    int requiredDirection = currSpell.sequence[currNoteIndex];
    currNote.direction = requiredDirection;

    int posIndex;
    if (requiredDirection == 4) {
        posIndex = 0; // LEFT
    }
    else if (requiredDirection == 3) {
        posIndex = 1; // DOWN 
    }
    else if (requiredDirection == 1) {
        posIndex = 2; // UP
    }
    else if (requiredDirection == 2) {
        posIndex = 3; // RIGHT
    }

    currNote.circle.x = arrowPositions[posIndex].x;
    currNote.circle.y = arrowPositions[posIndex].y;
    currNote.circle.rad = 20.0f;
    currNote.isActive = true;
    std::cout << "\nARROW KEY: ";

    switch (requiredDirection) {
    case 1: std::cout << " ^ "; break;
    case 2: std::cout << " > "; break;
    case 3: std::cout << " v "; break;
    case 4: std::cout << " < "; break;
    }
    std::cout << "\n";
}

void FretBoard::HandleSpellCasting() {
    if (!currNote.isActive) {
        return;
    }
    bool keyPressed{ false };
    bool correct{ false };

    if (currNote.direction == 1) {
        if (Input::IsKeyPressed(GLFW_KEY_UP)) {
            keyPressed = true;
            correct = true;
        }

        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            keyPressed = true;
        }
    }

    else if (currNote.direction == 2) {
        if (Input::IsKeyPressed(GLFW_KEY_RIGHT)) {
            keyPressed = true;
            correct = true;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            keyPressed = true;
        }
    }

    else if (currNote.direction == 3) {
        if (Input::IsKeyPressed(GLFW_KEY_DOWN)) {
            keyPressed = true;
            correct = true;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            keyPressed = true;
        }
    }

    else if (currNote.direction == 4) {
        if (Input::IsKeyPressed(GLFW_KEY_LEFT)) {
            keyPressed = true;
            correct = true;
        }
        else if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_UP)) {
            keyPressed = true;
        }
    }

    if (!keyPressed) {
        return;
    }

    if (correct) {
        std::cout << "NICE! ";
        currNote.isActive = false;
        currNoteIndex++;

        NextNote();
    }
    else {
        std::cout << "WRONG :( ";
        currNoteIndex = 0;
        currNote.isActive = false;

        NextNote();
    }
}
void FretBoard::PlayerInput() {
    if (currSpellState == SELECTION) {
        HandleSpellSelection();
    }
    else if (currSpellState == CASTING) {
        HandleSpellCasting();
    }
}
void FretBoard::StartGame() {
    currNote.isActive = false;
    SpellMenu();
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
        Vec2 pos{ currNote.circle.x, currNote.circle.y };
        Vec2 size{ 60.0f, 60.0f };
        renderer.DrawTexturedQuad(pos, size, tex);
    }
}

void FretBoard::LoadTextures() {
    arrowUpTex = Render::LoadTexture("../Assets/up.png");
    arrowRightTex = Render::LoadTexture("../Assets/right.png");
    arrowDownTex = Render::LoadTexture("../Assets/down.png");
    arrowLeftTex = Render::LoadTexture("../Assets/left.png");
}
