#pragma once
#ifndef MOVEMENT_CONTROLLER_H
#define MOVEMENT_CONTROLLER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Input.h"

enum class ControlScheme {
    WASD,
    IJKL
};

class MovementController {
public:
    MovementController(ControlScheme scheme, float speed = 200.0f);

    void ComputeInput(float dt);
    float GetDX() const { return dx; }
    float GetDY() const { return dy; }
    void Reset() { dx = dy = 0.0f; }

private:
    ControlScheme controlScheme;
    float speed;

    float dx;
    float dy;
};

#endif // MOVEMENT_CONTROLLER_H
