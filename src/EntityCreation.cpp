/* ========================================================
 * @file        EntityCreation.cpp
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @co-author   Yasmin Tazi         | yasmin.t@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * This file centralizes entity construction logic to ensure consistency
 * when spawning game objects. Each creation function initializes a new
 * entity, attaches the required components (transform, physics, sprite,
 * input, health, etc.), and sets defaults such as position,
 * scale, collision bounds, and textures.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "EntityCreation.h"
#include "JsonLoad.h"
#include "FileIO.h"
#include "FilePath.h"
#include "Registry.h"
#include "Component.h"
#include "Physics.h"
#include "PlayerSystem.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

// Helper function to load from JSON
Entity LoadEntity(const std::string& entityName, float x, float y) {
    std::string prefabPath = FilePath::PREFABS_FOLDER + entityName + ".json";
    return JsonLoad::LoadEntityFromFile(prefabPath, x, y);
}

Entity Create_Player(float const& x, float const& y) {
    return LoadEntity("Player", x, y);
}

Entity Create_Waffle(float const& x, float const& y) {
    return LoadEntity("Waffle", x, y);
}

Entity Create_Shape(float const& spawnx, float const& spawny, ShapeType shapeType, ControlScheme controls) {
    if (shapeType == SHAPE_RECTANGLE) {
        return LoadEntity("Rectangle", spawnx, spawny);
    }
    else if (shapeType == SHAPE_CIRCLE) {
        return LoadEntity("Circle", spawnx, spawny);
    }
    return 0;
}

/**
 * @brief Generates initial prefab JSON files in Assets/prefabs/.
 *
 * Creates the prefabs directory if missing and generates default
 * entity prefabs (Player, Waffle, Rectangle, Circle).
 * 
 */
void GenerateInitialPrefabs() {
    std::cout << "=== GENERATING PREFABS ===" << std::endl;

    if (!std::filesystem::exists(FilePath::PREFABS_FOLDER)) {
        std::filesystem::create_directories(FilePath::PREFABS_FOLDER);
    }

    // Player
    {
        Entity player = Create_Entity();
        SetEntityName(player, "Player");
        Add_Component(player, TransformComponent{ {0, 0}, {100.f, 100.f} });

        AABB aabb = Set_AABB({ 0, 0 }, { 200.f, 200.f });
        Circle circle = { {0, 0}, 200.f };
        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
        Add_Component(player, temp);

        Add_Component(player, InputComponent{ CS_WASD });
        Add_Component(player, TagComponent{ T_PLAYER });

        Image img = CreateImage("../Assets/Rock_FightingPose_2048px.png");
        Add_Component(player, Sprite::SpriteComponent{ img, {.1f,.1f} });

        Add_Component(player, HealthComponent{ 100.f, 100.f });

        JsonLoad::SaveCurrentScene(FilePath::PREFABS_FOLDER + "Player.json", "Player");
        Remove_Entity(player);
    }

    // Waffle
    {
        Entity waffle = Create_Entity();
        SetEntityName(waffle, "Waffle");
        Add_Component(waffle, TransformComponent{ {0, 0}, {100.f, 100.f} });

        AABB aabb = Set_AABB({ 0, 0 }, { 200.f, 200.f });
        Circle circle = { {0, 0}, 200.f };
        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
        Add_Component(waffle, temp);

        Add_Component(waffle, InputComponent{ CS_IJKL });
        Add_Component(waffle, TagComponent{ T_PLAYER });

        Image imgtemp = CreateImage("../Assets/Waffle_Sprites.png");
        Add_Component(waffle, Sprite::SpriteComponent{ imgtemp, {100.f,100.f }, true });

        Animator::AnimatorComponent animtemp(3, 6, imgtemp.widthPixels, imgtemp.heightPixels, { 2,0,1 }, 1.f);
        animtemp.AddState(PS_IDLE, { 3, 0, 1, 1.f });
        animtemp.AddState(PS_WALK, { 3,2,3,0.5f });
        Add_Component(waffle, animtemp);

        Add_Component(waffle, HealthComponent{ 100.f, 100.f });

        JsonLoad::SaveCurrentScene(FilePath::PREFABS_FOLDER + "Waffle.json", "Waffle");
        Remove_Entity(waffle);
    }

    // Rectangle
    {
        Entity rect = Create_Entity();
        SetEntityName(rect, "Rectangle");
        Add_Component(rect, TransformComponent{ {0, 0}, {100.f, 100.f} });

        AABB aabb = Set_AABB({ 0, 0 }, { 200.f, 200.f });
        Circle circle = { {0, 0}, 200.f };
        Physics::PhysicsComponent phys(0.015f, true, aabb, circle);
        Add_Component(rect, phys);

        Add_Component(rect, ShapeComponent{ SHAPE_RECTANGLE, {200.f, 200.f}, {1.0f, 0.0f, 0.0f} });
        Add_Component(rect, InputComponent{ CS_WASD });
        Add_Component(rect, TagComponent{ T_PLAYER });
        Add_Component(rect, HealthComponent{ 100.f, 100.f });

        JsonLoad::SaveCurrentScene(FilePath::PREFABS_FOLDER + "Rectangle.json", "Rectangle");
        Remove_Entity(rect);
    }

    // Circle
    {
        Entity circle_ent = Create_Entity();
        SetEntityName(circle_ent, "Circle");
        Add_Component(circle_ent, TransformComponent{ {0, 0}, {100.f, 100.f} });

        AABB aabb = Set_AABB({ 0, 0 }, { 200.f, 200.f });
        Circle circle = { {0, 0}, 100.f };
        Physics::PhysicsComponent phys(0.015f, true, aabb, circle);
        Add_Component(circle_ent, phys);

        Add_Component(circle_ent, ShapeComponent{ SHAPE_CIRCLE, {200.f, 200.f}, {0.0f, 1.0f, 0.0f} });
        Add_Component(circle_ent, InputComponent{ CS_ARROWS });
        Add_Component(circle_ent, TagComponent{ T_PLAYER });
        Add_Component(circle_ent, HealthComponent{ 100.f, 100.f });

        JsonLoad::SaveCurrentScene(FilePath::PREFABS_FOLDER + "Circle.json", "Circle");
        Remove_Entity(circle_ent);
    }

    std::cout << "=== ALL PREFABS GENERATED ===" << std::endl;
}
Entity Create_Wall(float x, float y, float width, float height) {
    Entity wall = Create_Entity();
    Add_Component(wall, ShapeComponent{ SHAPE_RECTANGLE });
    Add_Component(wall, TransformComponent{ {x, y}, {width, height} });
    AABB aabb = Set_AABB({ x, y }, { width, height });
    Circle circle = { {x, y}, 0.0f }; // zero radius
    Physics::PhysicsComponent phys(0.0f, true, aabb, circle, true);
    Add_Component(wall, phys);

    Add_Component(wall, TagComponent{ T_WALL });
    return wall;
}

Entity Create_InstancedSquare(float x, float y, float size, Vec3 color)
{
    Entity instance = Create_Entity();


    Add_Component(instance, TransformComponent{ {x, y}, {size, size} });
    //for rendersystem
    ShapeComponent s;
    s.shape = SHAPE_RECTANGLE;
    s.dimensions = { size, size };
    s.color = color;
    Add_Component(instance, s);
    Add_Component(instance, TagComponent{ T_INSTANCING });
    return instance;
}
//Entity Create_Square(float x, float y) {
//    Entity square = Create_Entity();
//    Add_Component(square, TransformComponent{ {x, y}, {100.f, 100.f} });
//
//    {
//        AABB aabb = Set_AABB({ x,y }, { 200.f,200.f });
//        Circle circle = { {x,y},200.f };
//        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
//        Add_Component(square, temp);
//    }
//
//    //Add_Component(player, AABBComponent{ true, {}, 200.0f, 200.0f });
//    Add_Component(square, InputComponent{ CS_IJKL });
//    Add_Component(square, TagComponent{ T_PLAYER });
//    Add_Component(square, SpriteComponent{ 0, {200.f, 200.f}, {1.0f, 0.0f, 0.0f} });
//    Add_Component(square, HealthComponent{ 100.f, 100.f });
//    return square;
//}

//Entity Create_Circle(float x, float y) {
//    Entity round = Create_Entity();
//    Add_Component(round, TransformComponent{ {x, y}, {100.f, 100.f} });
//    {
//        AABB aabb = Set_AABB({ x,y }, { 200.f,200.f });
//        Circle circle = { {x,y},100.f };
//        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
//        Add_Component(round, temp);  // round refers to the entity
//    }
//
//    Add_Component(round, InputComponent{ CS_ARROWS });
//    Add_Component(round, TagComponent{ T_PLAYER });
//    Add_Component(round, SpriteComponent{ 1, {200.f, 200.f}, {0.0f, 1.0f, 0.0f} }); // textureID=1 for circle, green color
//    Add_Component(round, HealthComponent{ 100.f, 100.f });
//    return round;
//}
// 
//Entity Create_Ghoul(float x, float y) {oe
//    Entity ghoul = Create_Entity();
//
//    Add_Component(ghoul, TransformComponent{ {x, y}, {100.f, 100.f} });
//    Add_Component(ghoul, BossGhoulComponent{
//        100.f,   // max hp
//        100.f,   // current hp
//        0.f,     // ??? maybe speed or timer
//        100.f,   // damage or cooldown
//        0,       // phase index?
//        GHOUL_ATTACKING // starting state
//        });
//    Add_Component(ghoul, TagComponent{ T_ENEMY });
//    Add_Component(ghoul, AABBComponent{ true, {}, 200.0f, 200.0f });
//    GLuint tex = Render::GetInstance().LoadTexture("../Assets/Ghoul_2048px.png");
//    Add_Component(ghoul, SpriteComponent{ tex, {400.f, 400.f} });
//    Add_Component(ghoul, PhysicsComponent{ {0.f, 0.f} });
//    Add_Component(ghoul, HealthComponent{ 100.f, 100.f });
//    bossGhoulSystem->AddEntity(ghoul);
//
//    return ghoul;
//}