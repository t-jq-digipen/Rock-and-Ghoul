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

Entity Create_Shape(float const& spawnx, float const& spawny, ShapeType shapeType, ControlScheme scheme) {
     Entity shape = 0;
    if (shapeType == SHAPE_RECTANGLE) {
        return LoadEntity("Rectangle", spawnx, spawny);
    }
    else if (shapeType == SHAPE_CIRCLE) {
        return LoadEntity("Circle", spawnx, spawny);
    }

    if (shape != 0) {
        Registry::Get().Add_Component<InputComponent>(shape, InputComponent{ scheme });
        // Add_Component(shape, InputComponent{ scheme }); // <--- use scheme here
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
        Entity player = Registry::Get().Create_Entity();
        Registry::Get().Set_EntityName(player, "Player");
        Registry::Get().Add_Component<TransformComponent>(player, TransformComponent{ {0, 0}, {100.f, 100.f} });

        AABB aabb = Create_AABB({ 0, 0 }, { 200.f, 200.f });
        Circle circle = { {0, 0}, 200.f };
        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
        Registry::Get().Add_Component<Physics::PhysicsComponent>(player, temp);

        Registry::Get().Add_Component<InputComponent>(player, InputComponent{ CS_WASD });
        Registry::Get().Add_Component<TagComponent>(player, TagComponent{ T_PLAYER });

        Image img = CreateImage("../Assets/Rock_FightingPose_2048px.png");
        Registry::Get().Add_Component<Sprite::SpriteComponent>(player, Sprite::SpriteComponent{ img, {.1f,.1f} });

        Registry::Get().Add_Component<HealthComponent>(player, HealthComponent{ 100.f, 100.f });

        JsonLoad::SaveCurrentScene(FilePath::PREFABS_FOLDER + "Player.json", "Player");
        Registry::Get().Remove_Entity(player);
    }

    // Waffle
    {
        Entity waffle = Registry::Get().Create_Entity();
        Registry::Get().Set_EntityName(waffle, "Waffle");
        Registry::Get().Add_Component<TransformComponent>(waffle, TransformComponent{ {0, 0}, {100.f, 100.f} });

        AABB aabb = Create_AABB({ 0, 0 }, { 200.f, 200.f });
        Circle circle = { {0, 0}, 200.f };
        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
        Registry::Get().Add_Component<Physics::PhysicsComponent>(waffle, temp);

        Registry::Get().Add_Component<InputComponent>(waffle, InputComponent{ CS_IJKL });
        Registry::Get().Add_Component<TagComponent>(waffle, TagComponent{ T_PLAYER });

        Image imgtemp = CreateImage("../Assets/Waffle_Sprites.png");
        Registry::Get().Add_Component<Sprite::SpriteComponent>(waffle, Sprite::SpriteComponent{ imgtemp, {100.f,100.f }, true });

        Animator::AnimatorComponent animtemp(3, 6, imgtemp.widthPixels, imgtemp.heightPixels, { 2,0,1 }, 1.f);
        animtemp.AddState(PS_IDLE, { 3, 0, 1, 1.f });
        animtemp.AddState(PS_WALK, { 3,2,3,0.5f });
        Registry::Get().Add_Component<Animator::AnimatorComponent>(waffle, animtemp);

        Registry::Get().Add_Component<HealthComponent>(waffle, HealthComponent{ 100.f, 100.f });

        JsonLoad::SaveCurrentScene(FilePath::PREFABS_FOLDER + "Waffle.json", "Waffle");
        Registry::Get().Remove_Entity(waffle);
    }

    // Rectangle
    {
        Entity rect = Registry::Get().Create_Entity();
        Registry::Get().Set_EntityName(rect, "Rectangle");
        Registry::Get().Add_Component<TransformComponent>(rect, TransformComponent{ {0, 0}, {100.f, 100.f} });

        AABB aabb = Create_AABB({ 0, 0 }, { 200.f, 200.f });
        Circle circle = { {0, 0}, 200.f };
        Physics::PhysicsComponent phys(0.015f, true, aabb, circle);
        Registry::Get().Add_Component<Physics::PhysicsComponent>(rect, phys);

        Registry::Get().Add_Component<ShapeComponent>(rect, ShapeComponent{ SHAPE_RECTANGLE, {200.f, 200.f}, {1.0f, 0.0f, 0.0f} });
        Registry::Get().Add_Component<InputComponent>(rect, InputComponent{ CS_WASD });
        Registry::Get().Add_Component<TagComponent>(rect, TagComponent{ T_PLAYER });
        Registry::Get().Add_Component<HealthComponent>(rect, HealthComponent{ 100.f, 100.f });

        JsonLoad::SaveCurrentScene(FilePath::PREFABS_FOLDER + "Rectangle.json", "Rectangle");
        Registry::Get().Remove_Entity(rect);
    }

    // Circle
    {
        Entity circle_ent = Registry::Get().Create_Entity();
        Registry::Get().Set_EntityName(circle_ent, "Circle");
        Registry::Get().Add_Component<TransformComponent>(circle_ent, TransformComponent{ {0, 0}, {100.f, 100.f} });

        AABB aabb = Create_AABB({ 0, 0 }, { 200.f, 200.f });
        Circle circle = { {0, 0}, 100.f };
        Physics::PhysicsComponent phys(0.015f, true, aabb, circle);
        Registry::Get().Add_Component<Physics::PhysicsComponent>(circle_ent, phys);

        Registry::Get().Add_Component<ShapeComponent>(circle_ent, ShapeComponent{ SHAPE_CIRCLE, {200.f, 200.f}, {0.0f, 1.0f, 0.0f} });
        Registry::Get().Add_Component<InputComponent>(circle_ent, InputComponent{ CS_ARROWS });
        Registry::Get().Add_Component<TagComponent>(circle_ent, TagComponent{ T_PLAYER });
        Registry::Get().Add_Component<HealthComponent>(circle_ent, HealthComponent{ 100.f, 100.f });

        JsonLoad::SaveCurrentScene(FilePath::PREFABS_FOLDER + "Circle.json", "Circle");
        Registry::Get().Remove_Entity(circle_ent);
    }

    std::cout << "=== ALL PREFABS GENERATED ===" << std::endl;
}
Entity Create_Wall(float x, float y, float width, float height) {
    Entity wall = Registry::Get().Create_Entity();
    Registry::Get().Add_Component<ShapeComponent>(wall, ShapeComponent{ SHAPE_RECTANGLE });
    Registry::Get().Add_Component<TransformComponent>(wall, TransformComponent{ {x, y}, {width, height} });
    AABB aabb = Create_AABB({ x, y }, { width, height });
    Circle circle = { {x, y}, 0.0f }; // zero radius
    Physics::PhysicsComponent phys(0.0f, true, aabb, circle, true);
    Registry::Get().Add_Component<Physics::PhysicsComponent>(wall, phys);

    Registry::Get().Add_Component<TagComponent>(wall, TagComponent{ T_WALL });
    return wall;
}

Entity Create_InstancedSquare(float x, float y, float size, Vec3 color)
{
    Entity instance = Registry::Get().Create_Entity();


    Registry::Get().Add_Component<TransformComponent>(instance, TransformComponent{ {x, y}, {size, size} });
    //for rendersystem
    ShapeComponent s;
    s.shape = SHAPE_RECTANGLE;
    s.dimensions = { size, size };
    s.color = color;
    Registry::Get().Add_Component<ShapeComponent>(instance, s);
    Registry::Get().Add_Component<TagComponent>(instance, TagComponent{ T_INSTANCING });
    return instance;
}
//Entity Create_Square(float x, float y) {
//    Entity square = Create_Entity();
//    Add_Component(square, TransformComponent{ {x, y}, {100.f, 100.f} });
//
//    {
//        AABB aabb = Create_AABB({ x,y }, { 200.f,200.f });
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
//        AABB aabb = Create_AABB({ x,y }, { 200.f,200.f });
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