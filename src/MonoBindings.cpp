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
#include "RenderComponent.h"
#include "JsonLoad.h"
#include "FilePath.h"
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


// RENDER APIS
// --------------------------------------------------------------------------------
#pragma region TRANSFORM APIS

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


    __declspec(dllexport) void CS_GetScale(unsigned int entity, float* scaleX, float* scaleY)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity)) return;

        auto& t = reg.Get_Component<TransformComponent>(entity);
        *scaleX = t.scale.x;
        *scaleY = t.scale.y;
    }


    __declspec(dllexport) void CS_SetScale(unsigned int entity, float x, float y)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(entity)) return;

        auto& transform = reg.Get_Component<TransformComponent>(entity);
        transform.scale.x = x;
        transform.scale.y = y;
    }
#pragma endregion TRANSFORM APIS
// --------------------------------------------------------------------------------

    
// RENDER APIS
// --------------------------------------------------------------------------------
#pragma region RENDER STUFF

    __declspec(dllexport) void CS_SetAnimState(unsigned int entity, unsigned int state)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<RenderComponent>(entity)) return;

        RenderComponent& render = reg.Get_Component<RenderComponent>(entity);

        if (render.GetRenderType() != RT_SPRITESHEET) return;

        if (render.GetSpriteSheet().GetCurrentState() == static_cast<int>(state)) return;

        render.GetSpriteSheet().SetState(state);
    }

    // if rendercomp is not of type sprite or spritesheet it will just return false
    __declspec(dllexport) bool CS_GetRenderFlipX(unsigned int entity)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<RenderComponent>(entity)) return false;

        RenderComponent& render = reg.Get_Component<RenderComponent>(entity);

        RenderType renderType = render.GetRenderType();
        if (renderType != RT_SPRITESHEET && renderType != RT_IMAGE) return false;

        Sprite::SpriteComponent& sprite = (renderType == RT_SPRITESHEET) ? render.GetSpriteSheet().Sprite() : render.GetSprite();

        return sprite.GetFlipX();
    }


    __declspec(dllexport) void CS_SetRenderFlipX(unsigned int entity, bool flip)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<RenderComponent>(entity)) return;

        RenderComponent& render = reg.Get_Component<RenderComponent>(entity);

        RenderType renderType = render.GetRenderType();
        if (renderType != RT_SPRITESHEET && renderType != RT_IMAGE) return;

        Sprite::SpriteComponent& sprite = (renderType == RT_SPRITESHEET) ? render.GetSpriteSheet().Sprite() : render.GetSprite();

        sprite.SetFlipX(flip);
    }

    // if rendercomp is not of type sprite or spritesheet it will just return false
    __declspec(dllexport) bool CS_GetRenderFlipY(unsigned int entity)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<RenderComponent>(entity)) return false;

        RenderComponent& render = reg.Get_Component<RenderComponent>(entity);

        RenderType renderType = render.GetRenderType();
        if (renderType != RT_SPRITESHEET && renderType != RT_IMAGE) return false;

        Sprite::SpriteComponent& sprite = (renderType == RT_SPRITESHEET) ? render.GetSpriteSheet().Sprite() : render.GetSprite();

        return sprite.GetFlipY();
    }


    __declspec(dllexport) void CS_SetRenderFlipY(unsigned int entity, bool flip)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<RenderComponent>(entity)) return;

        RenderComponent& render = reg.Get_Component<RenderComponent>(entity);

        RenderType renderType = render.GetRenderType();
        if (renderType != RT_SPRITESHEET && renderType != RT_IMAGE) return;

        Sprite::SpriteComponent& sprite = (renderType == RT_SPRITESHEET) ? render.GetSpriteSheet().Sprite() : render.GetSprite();

        sprite.SetFlipY(flip);
    }
#pragma endregion RENDER APIS
// --------------------------------------------------------------------------------



// PHYSICS/COLLISION APIS
// --------------------------------------------------------------------------------
#pragma region PHYSICS/COLLISION APIS

    __declspec(dllexport) void CS_GetColliderSize(unsigned int entity, float* width, float* height)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<ColliderComponent>(entity)) return;

        ColliderComponent& collider = reg.Get_Component<ColliderComponent>(entity);

        *width = collider.aabbhitbox.dim.x;
        *height = collider.aabbhitbox.dim.y;
    }


    __declspec(dllexport) void CS_SetColliderSize(unsigned int entity, float width, float height)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<ColliderComponent>(entity)) return;

        ColliderComponent& collider = reg.Get_Component<ColliderComponent>(entity);

        collider.aabbhitbox.dim.x = width;
        collider.aabbhitbox.dim.y = height;
    }
#pragma endregion PHYSICS/COLLISION APIS
// --------------------------------------------------------------------------------

    // filename to be in "xxx.json" format, will spawn from prefabs folder
    __declspec(dllexport) unsigned int CS_SpawnPrefab(const std::string& filename, float spawnX, float spawnY) 
    {
        return JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + filename, spawnX, spawnY);
    }
}