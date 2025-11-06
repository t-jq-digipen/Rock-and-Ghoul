/* ========================================================
 * @file        MonoBindings.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        07 NOV 2025
 *
 * @brief
 * Provides C-compatible exported ECS access functions for use
 * by managed C# scripts through Mono or .NET interoperability.
 *
 * These functions bridge between the native C++ ECS Registry and
 * external C# scripts by exposing basic component operations
 * through extern "C" and __declspec(dllexport).
 *
 * EXPORTED FUNCTIONS
 * - CS_GetPosition(entity, &x, &y)   | Gets Transform position
 * - CS_SetPosition(entity, x, y)     | Updates Transform position
 * - CS_GetRotation(entity)           | Gets Transform rotation
 * - CS_SetRotation(entity, rot)      | Updates Transform rotation
 *
 * Each function automatically checks if the entity has a
 * TransformComponent before performing the operation.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ======================================================== 
 */


#include "Registry.h"
#include "Component.h"
#include <iostream>
#include "Input.h"
extern "C" {
    //https://learn.microsoft.com/en-us/cpp/build/exporting-c-functions-for-use-in-c-or-cpp-language-executables?view=msvc-170

    //    // MyCFuncs.h
    //#ifdef __cplusplus
    //    extern "C" {  // only need to export C interface if
    //        // used by C++ source code
    //#endif
    //
    //        __declspec(dllimport) void MyCFunc();
    //        __declspec(dllimport) void AnotherCFunc();
    //
    //#ifdef __cplusplus
    //    }
    //#endif
    __declspec(dllexport) void CS_GetPosition(unsigned int entity, float* x, float* y)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity)) return;

        auto& transform = reg.Get_Component<TransformComponent>(entity);
        *x = transform.pos.x;
        *y = transform.pos.y;
    }


    __declspec(dllexport) void CS_SetPosition(unsigned int entity, float x, float y)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity)) return;

        auto& transform = reg.Get_Component<TransformComponent>(entity);
        transform.pos.x = x;
        transform.pos.y = y;
    }


    __declspec(dllexport) float CS_GetRotation(unsigned int entity)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity))
            return 0.0f;
        auto& t = reg.Get_Component<TransformComponent>(entity);
        return t.rot;
    }


    __declspec(dllexport) void CS_SetRotation(unsigned int entity, float rot)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity))
            return;
        auto& t = reg.Get_Component<TransformComponent>(entity);
        t.rot = rot;
    }

}
