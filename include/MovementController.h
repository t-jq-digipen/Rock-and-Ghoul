#pragma once
#ifndef MOVEMENT_CONTROLLER_H
#define MOVEMENT_CONTROLLER_H

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include "Input.h"

enum class ControlScheme {
    WASD,
    Arrows
};

class MovementController {
public:
    MovementController(ControlScheme scheme, float speed = 200.0f);

    void Update(float& x, float& y, float dt);

private:
    ControlScheme controlScheme;
    float speed;
};

#endif // MOVEMENT_CONTROLLER_H
