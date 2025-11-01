/* ========================================================
 * @file	    MovementSystem.h
 * @author	    Manfred Sim | manfred.sim@digipen.edu
 * @date	    30 OCT 2025
 *
 * @brief	Movement system for ECS
 *
 * This file declares the Movement system for the ECS.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "System.h"
#include "Render.h"
#include "Input.h"
#include "Movement.h"

class MovementSystem : public ISystem {
public:
    MovementSystem() {}
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;
private:
    // Normalize velocity
    static inline bool Normalize(Vec2& vel)
    {
        const float len2 = vel.x * vel.x + vel.y * vel.y;
        if (len2 <= 0.f) return false;
        const float invLen = 1.f / std::sqrt(len2);
        vel.x *= invLen;
        vel.y *= invLen;
        return true;
    }

    // Define velocity by movement direction
    static inline Vec2 DirToVec(Movement::InputDirection dir)
    {
        switch (dir)
        {
            case Movement::MOVE_UP: return { 0.f, 1.f };
            case Movement::MOVE_DOWN: return { 0.f, -1.f };
            case Movement::MOVE_LEFT: return { -1.f, 0.f };
            case Movement::MOVE_RIGHT: return { 1.f, 0.f };
            default: return { 0.f, 0.f };
        }
    }

    //inline Movement::MovementDirection VelToDir(const Vec2& v) {
    //    if (std::fabs(v.y) >= std::fabs(v.x)) {
    //        if (v.y > 0.f)  return Movement::MOVE_UP;
    //        if (v.y < 0.f)  return Movement::MOVE_DOWN;
    //    }
    //    else {
    //        if (v.x > 0.f)  return Movement::MOVE_RIGHT;
    //        if (v.x < 0.f)  return Movement::MOVE_LEFT;
    //    }
    //    return Movement::MOVE_NONE;
    //}

    //// Set horizontal and vertical movement based on respective control scheme and user input
    //inline Vec2 ReadDirFromScheme(ControlScheme scheme) {
    //    int hor = 0, vert = 0;
    //    switch (scheme) {
    //    case CS_WASD:
    //        if (Input::IsKeyHeld(GLFW_KEY_W)) vert += 1;
    //        if (Input::IsKeyHeld(GLFW_KEY_S)) vert -= 1;
    //        if (Input::IsKeyHeld(GLFW_KEY_A)) hor -= 1;
    //        if (Input::IsKeyHeld(GLFW_KEY_D)) hor += 1;
    //        break;
    //    case CS_IJKL:
    //        if (Input::IsKeyHeld(GLFW_KEY_I)) vert += 1;
    //        if (Input::IsKeyHeld(GLFW_KEY_K)) vert -= 1;
    //        if (Input::IsKeyHeld(GLFW_KEY_J)) hor -= 1;
    //        if (Input::IsKeyHeld(GLFW_KEY_L)) hor += 1;
    //        break;
    //    case CS_ARROWS:
    //        if (Input::IsKeyHeld(GLFW_KEY_UP))    vert += 1;
    //        if (Input::IsKeyHeld(GLFW_KEY_DOWN))  vert -= 1;
    //        if (Input::IsKeyHeld(GLFW_KEY_LEFT))  hor -= 1;
    //        if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) hor += 1;
    //        break;
    //    default: break;
    //    }
    //    return { static_cast<float>(hor), static_cast<float>(vert) };
    //}
};