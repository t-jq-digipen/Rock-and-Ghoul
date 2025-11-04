#include "Registry.h"
#include "Component.h"
#include <iostream>
#include "Input.h"
extern "C" {

    // ----------------------------------------------------
    // ECS_GetPosition(uint entity, float* x, float* y)
    // ----------------------------------------------------
    __declspec(dllexport) void CS_GetPosition(unsigned int entity, float* x, float* y)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity)) return;

        auto& transform = reg.Get_Component<TransformComponent>(entity);
        *x = transform.pos.x;
        *y = transform.pos.y;
    }

    // ----------------------------------------------------
    // ECS_SetPosition(uint entity, float x, float y)
    // ----------------------------------------------------
    __declspec(dllexport) void CS_SetPosition(unsigned int entity, float x, float y)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity)) return;

        auto& transform = reg.Get_Component<TransformComponent>(entity);
        transform.pos.x = x;
        transform.pos.y = y;
    }

    // === Get Rotation ===
    __declspec(dllexport) float CS_GetRotation(unsigned int entity)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity))
            return 0.0f;
        auto& t = reg.Get_Component<TransformComponent>(entity);
        return t.rot;
    }

    // === Set Rotation ===
    __declspec(dllexport) void CS_SetRotation(unsigned int entity, float rot)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity))
            return;
        auto& t = reg.Get_Component<TransformComponent>(entity);
        t.rot = rot;
    }
    // ----------------------------------------------------
    // Input check (optional bridge)
    // ----------------------------------------------------
    __declspec(dllexport) bool CS_IsKeyPressed(int key)
    {
        return Input::IsKeyPressed(key);
    }

}
