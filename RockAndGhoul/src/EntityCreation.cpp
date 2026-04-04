/* ========================================================
 * @file        EntityCreation.cpp
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @co-author   Yasmin Tazi         | yasmin.t@digipen.edu
 * @date        28 NOV 2025
 *
 * @brief
 *
 * This file centralizes entity construction logic to ensure consistency
 * when spawning game objects. Each creation function initializes a new
 * entity, attaches the required components (transform, physics, sprite,
 * input, health, etc.), and sets defaults such as position,
 * scale, collision bounds, and textures.
 *
 * Copyright   2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "reginegine.h"
#include "EntityCreation.h"
#include "FileIO.h"
#include "Registry.h"
#include "Component.h"
#include "Physics.h"
#include "Movement.h"
#include "PlayerSystem.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include "AudioComponent.h"
#include "AudioCache.h"
#include "BehaviourComponent.h"
#include "GameScripts.h"
using json = nlohmann::json;

// Helper function to load from JSON
Entity LoadEntity(const std::string& entityName, float x, float y) {
    std::string prefabPath = FilePath::PREFABS_FOLDER + entityName + ".json";

    // Check if file exists before attempting to load
    if (!std::filesystem::exists(prefabPath)) {
        std::cerr << "ERROR: Prefab file doesn't exist: " << prefabPath << std::endl;
        return 0;
    }

    Entity e = JsonLoad::LoadEntityFromFile(prefabPath, x, y);
    if (e == 0) {
        std::cerr << "ERROR: Failed to load entity from: " << prefabPath << std::endl;
    }
    return e;
}

//Entity Create_Player(float const& x, float const& y) {
//    return LoadEntity("Player", x, y);
//}
Entity Create_Player(float const& x, float const& y) {
    // Load from Rock.json instead of Player.json (Rock is the actual player entity)
    return LoadEntity("Rock", x, y);
}
Entity Create_Waffle(float const& x, float const& y) {
    return LoadEntity("Waffle", x, y);
}
Entity Create_Ghoul(float const& x, float const& y) {
    Entity ghoul = LoadEntity("ghoul", x, y);

    // Add behaviour component with the ManfredEnemy script
    BehaviourComponent bc;
    bc.AddScript("ManfredEnemy");
    Registry::Get().Add_Component<BehaviourComponent>(ghoul, bc);

    return ghoul;
}
Entity Create_Shape(float const& spawnx, float const& spawny, ShapeType shapeType, ControlScheme) {
    //Entity shape = 0;
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
 /**
  * @brief Generates initial prefab JSON files in Assets/prefabs/.
  *
  * Creates the prefabs directory if missing and generates default
  * entity prefabs (Player, Waffle, Rectangle, Circle).
  *
  * @param forceRegenerate If true, regenerates all prefabs even if they exist.
  */
void GenerateInitialPrefabs(bool forceRegenerate) {
    std::cout << "\n=== CHECKING PREFABS ===" << std::endl;

    // Create directory if it doesn't exist
    if (!std::filesystem::exists(FilePath::PREFABS_FOLDER)) {
        try {
            std::filesystem::create_directories(FilePath::PREFABS_FOLDER);
            std::cout << "Created prefabs directory: " << FilePath::PREFABS_FOLDER << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to create prefabs directory: " << e.what() << std::endl;
            return;
        }
    }

    // Helper lambda to check if we should generate a prefab
    auto shouldGenerate = [&](const std::string& filename) -> bool {
        std::string filepath = FilePath::PREFABS_FOLDER + filename;
        if (!forceRegenerate && std::filesystem::exists(filepath)) {
            std::cout << filename << " already exists" << std::endl;
            return false;
        }
        return true;
        };

    int generatedCount = 0;
    int skippedCount = 0;

    // ========== Player ==========
    if (shouldGenerate("Rock.json")) {
        std::cout << "Generating Rock.json..." << std::endl;

        try {
            Entity player = Registry::Get().Create_Entity();
            Registry::Get().Set_EntityName(player, "Player");
            Registry::Get().Add_Component<TransformComponent>(player, TransformComponent{ {0, 0}, {100.f, 100.f} });

            AABB aabb = Create_AABB({ 0, 0 }, { 200.f, 200.f });
            Registry::Get().Add_Component<Physics::PhysicsComponent>(player, Physics::PhysicsComponent(0.015f, 500.f, false));

            ColliderComponent collider(aabb, true);
            collider.SetLayer("Player");
            //collider.SetMask({"Enemy", "Projectile", "Wall"});
            Registry::Get().Add_Component<ColliderComponent>(player, collider);

            Registry::Get().Add_Component<TagComponent>(player, TagComponent{ T_PLAYER });
            

            Image img = CreateImage((FilePath::ASSETS_FOLDER + "Rock_FightingPose_2048px.png"));
            //Registry::Get().Add_Component<Sprite::SpriteComponent>(player, Sprite::SpriteComponent{ img, {.1f,.1f} });

            Registry::Get().Add_Component<HealthComponent>(player, HealthComponent{ 100.f, 100.f });

            Registry::Get().Add_Component<Movement::MovementComponent>(player, Movement::MovementComponent{ 500.f, Movement::MOVE_NONE, {0.f, 0.f}, Movement::MOVE_POSITION, false });

            //Registry::Get().Add_Component<RenderLayerComponent>(player, RenderLayerComponent{ LAYER_BACKGROUND });
            AudioComponent playerAudio;
            // Configure spatial properties
            playerAudio.spatial = true;
            playerAudio.volume = 0.8f;
            playerAudio.minDistance = 10.0f;
            playerAudio.maxDistance = 100.0f;

            // Add sounds using the vector-based system
            playerAudio.AddSound("player_footstep_wood");  // Index 0
            playerAudio.AddSound("player_dodge");          // Index 1
            playerAudio.AddSound("player_spawnSound");     // Index 2
            playerAudio.AddSound("guitar_strum_heavy");    // Index 3

            Registry::Get().Add_Component<AudioComponent>(player, playerAudio);

            Registry::Get().Remove_Entity(player);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR generating Player: " << e.what() << std::endl;
        }
    }
    else {
        skippedCount++;
    }

    // ========== Waffle ==========
    if (shouldGenerate("Waffle.json")) {
        std::cout << "Generating Waffle.json..." << std::endl;

        try {
            Entity waffle = Registry::Get().Create_Entity();
            Registry::Get().Set_EntityName(waffle, "Waffle");
            Registry::Get().Add_Component<TransformComponent>(waffle, TransformComponent{ {0, 0}, {100.f, 100.f} });

            AABB aabb = Create_AABB({ 0, 0 }, { 200.f, 200.f });
            Registry::Get().Add_Component<Physics::PhysicsComponent>(waffle, Physics::PhysicsComponent(0.015f, 500.f, false));

            ColliderComponent waffleCollider(aabb, true);
            waffleCollider.SetLayer("Player");
            //waffleCollider.SetMask({"Enemy", "Projectile", "Wall"});
            Registry::Get().Add_Component<ColliderComponent>(waffle, waffleCollider);

            Registry::Get().Add_Component<TagComponent>(waffle, TagComponent{ T_OBJECT });

            Image imgtemp = CreateImage((FilePath::ASSETS_FOLDER + "Waffle_Sprites.png"));
           // Registry::Get().Add_Component<Sprite::SpriteComponent>(waffle, Sprite::SpriteComponent{ imgtemp, {100.f,100.f }, true });
            Registry::Get().Add_Component<Movement::MovementComponent>(waffle, Movement::MovementComponent{ 500.f, Movement::MOVE_NONE, {0.f, 0.f}, Movement::MOVE_POSITION, false });
            // CRITICAL: Ensure imageWidth and imageHeight are set correctly!
            Animator::AnimatorComponent animtemp(3, 6, { 2,0,1 }, 1.f);
            animtemp.AddState(PS_IDLE, { 3, 0, 1, 1.f });
            animtemp.AddState(PS_WALK, { 3,2,3,0.5f });

            animtemp.UpdateUV(imgtemp.widthPixels, imgtemp.heightPixels);

            Registry::Get().Add_Component<Animator::AnimatorComponent>(waffle, animtemp);

            Registry::Get().Add_Component<HealthComponent>(waffle, HealthComponent{ 100.f, 100.f });

            if (JsonLoad::SaveEntity(FilePath::PREFABS_FOLDER + "Waffle.json", waffle)) {
                std::cout << "Waffle.json saved successfully" << std::endl;
                generatedCount++;
            }
            else {
                std::cerr << "Failed to save Waffle.json" << std::endl;
            }

            Registry::Get().Remove_Entity(waffle);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR generating Waffle: " << e.what() << std::endl;
        }
    }
    else {
        skippedCount++;
    }

    // ========== Rectangle ==========
    if (shouldGenerate("Rectangle.json")) {
        std::cout << "Generating Rectangle.json..." << std::endl;

        try {
            Entity rect = Registry::Get().Create_Entity();
            Registry::Get().Set_EntityName(rect, "Rectangle");
            Registry::Get().Add_Component<TransformComponent>(rect, TransformComponent{ {0, 0}, {100.f, 100.f} });

            AABB aabb = Create_AABB({ 0, 0 }, { 200.f, 200.f });
            Registry::Get().Add_Component<Physics::PhysicsComponent>(rect, Physics::PhysicsComponent(0.015f, 500.f, false));

            ColliderComponent rectCollider(aabb, true);
            rectCollider.SetLayer("Player");
            //rectCollider.SetMask({"Enemy", "Projectile", "Wall"});
            Registry::Get().Add_Component<ColliderComponent>(rect, rectCollider);

            // Registry::Get().Add_Component<ShapeComponent>(rect, ShapeComponent{ SHAPE_RECTANGLE, {200.f, 200.f}, {1.0f, 0.0f, 0.0f} });
            Registry::Get().Add_Component<TagComponent>(rect, TagComponent{ T_OBJECT });
            Registry::Get().Add_Component<HealthComponent>(rect, HealthComponent{ 100.f, 100.f });

            if (JsonLoad::SaveEntity(FilePath::PREFABS_FOLDER + "Rectangle.json", rect)) {
                std::cout << "Rectangle.json saved successfully" << std::endl;
                generatedCount++;
            }
            else {
                std::cerr << "Failed to save Rectangle.json" << std::endl;
            }

            Registry::Get().Remove_Entity(rect);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR generating Rectangle: " << e.what() << std::endl;
        }
    }
    else {
        skippedCount++;
    }

    // ========== Circle ==========
    if (shouldGenerate("Circle.json")) {
        std::cout << "Generating Circle.json..." << std::endl;

        try {
            Entity circle_ent = Registry::Get().Create_Entity();
            Registry::Get().Set_EntityName(circle_ent, "Circle");
            Registry::Get().Add_Component<TransformComponent>(circle_ent, TransformComponent{ {0, 0}, {100.f, 100.f} });

            AABB aabb = Create_AABB({ 0, 0 }, { 200.f, 200.f });
            Registry::Get().Add_Component<Physics::PhysicsComponent>(circle_ent, Physics::PhysicsComponent(0.015f, 500.f, false));

            ColliderComponent circleCollider(aabb, true);
            circleCollider.SetLayer("Player");
            //circleCollider.SetMask({"Enemy", "Projectile", "Wall"});
            Registry::Get().Add_Component<ColliderComponent>(circle_ent, circleCollider);

            //Registry::Get().Add_Component<ShapeComponent>(circle_ent, ShapeComponent{ SHAPE_CIRCLE, {200.f, 200.f}, {0.0f, 1.0f, 0.0f} });
            Registry::Get().Add_Component<TagComponent>(circle_ent, TagComponent{ T_OBJECT });
            Registry::Get().Add_Component<HealthComponent>(circle_ent, HealthComponent{ 100.f, 100.f });

            if (JsonLoad::SaveEntity(FilePath::PREFABS_FOLDER + "Circle.json", circle_ent)) {
                std::cout << "Circle.json saved successfully" << std::endl;
                generatedCount++;
            }
            else {
                std::cerr << "Failed to save Circle.json" << std::endl;
            }

            Registry::Get().Remove_Entity(circle_ent);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR generating Circle: " << e.what() << std::endl;
        }
    }
    else {
        skippedCount++;
    }

    std::cout << "\n=== PREFAB GENERATION COMPLETE ===" << std::endl;
    std::cout << "Generated: " << generatedCount << " new prefabs" << std::endl;
    std::cout << "Skipped: " << skippedCount << " existing prefabs" << std::endl;
    std::cout << "==============================\n" << std::endl;
}

Entity Create_Wall(float x, float y, float width, float height) {
    Entity wall = Registry::Get().Create_Entity();
    //Registry::Get().Add_Component<ShapeComponent>(wall, ShapeComponent{ SHAPE_RECTANGLE });
    Registry::Get().Add_Component<TransformComponent>(wall, TransformComponent{ {x, y}, {width, height} });
    AABB aabb = Create_AABB({ x, y }, { width, height });
    Registry::Get().Add_Component<Physics::PhysicsComponent>(wall, Physics::PhysicsComponent(0.0f, 500.f, true));
    Registry::Get().Add_Component<ColliderComponent>(wall, ColliderComponent(aabb, true));

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
    Registry::Get().Add_Component<RenderComponent>(instance);
    RenderComponent& renderComp = Registry::Get().Get_Component<RenderComponent>(instance);
    renderComp.SetRenderType(RT_SHAPE);
    renderComp.GetShape() = s;
    //Registry::Get().Add_Component<ShapeComponent>(instance, s);
    Registry::Get().Add_Component<TagComponent>(instance, TagComponent{ T_INSTANCING });
    return instance;
}

/*Entity Create_BatchedSprite(float x, float y, float size, const std::string& texturePath)
{
    Entity instance = Registry::Get().Create_Entity();

    // Transform
    Registry::Get().Add_Component<TransformComponent>(
        instance, TransformComponent{ {x, y}, {size, size} });

    // Sprite (static(false) or animated(true))
    Sprite::SpriteComponent sprite(texturePath, { size, size }, true);
    Registry::Get().Add_Component<Sprite::SpriteComponent>(instance, sprite);

    // --- Animator ---
    Image img = sprite.Get_Image();

    // using original Waffle prefab setup
    //Animator::AnimatorComponent anim(
    //    3, 6,  // total rows, total columns
    //    img.widthPixels, img.heightPixels,
    //    { 2, 0, 1 }, // starting frame
    //    1.0f         // duration
    //);

    Animator::AnimatorComponent anim(
        3, 6,           // total rows, total columns
        { 2, 0, 1, 1.0f }, // ActiveFrames: row=2, col_start=0, col_end=1, duration=1.0f
        1.0f            // animation duration (or you can omit this since it's in ActiveFrames)
    );
    anim.UpdateUV(img.widthPixels, img.heightPixels);
    // Add states
    anim.AddState(PS_IDLE, { 3, 0, 1, 1.0f });
    anim.AddState(PS_WALK, { 3, 2, 3, 0.5f });

    // Randomize start time a bit to avoid maybe?
    /*float randomOffset = static_cast<float>(rand() % 100) / 100.0f;
    anim.UpdateFrame(randomOffset);*/

    /*Registry::Get().Add_Component<Animator::AnimatorComponent>(instance, anim);

    //Tag for filtering
    Registry::Get().Add_Component<TagComponent>(
        instance, TagComponent{ T_BATCH }); //T_BATCH added in C_TagID enum

    return instance;
}*/

Entity Create_BatchedSprite(float x, float y, float size, const std::string& texturePath)
{
    Entity instance = Registry::Get().Create_Entity();

    // Transform
    Registry::Get().Add_Component<TransformComponent>(
        instance, TransformComponent{ {x, y}, {size, size} });

    // Sprite (animated true)
    Sprite::SpriteComponent sprite(texturePath, { size, size });
    

    Sprite::SpriteSheet spritesheet;
    spritesheet.Sprite() = sprite;

    spritesheet.SetTotalRows(3);
    spritesheet.SetTotalCols(6);

    // Load texture safely through Render texture cache (ensures reuse)
    // Image img = sprite.Get_Image();

    // Correct constructor: include width and height explicitly
    Animator::AnimatorComponent anim(
        3, 6,                      // rows, columns
        //img.widthPixels, img.heightPixels, // actual texture size
        { 2, 0, 1 },               // starting frame (row=2, col=0..1)
        1.0f                       // duration
    );

    // Add proper states like the prefab does
    spritesheet.AddState(PS_IDLE, { 3, 0, 1, 1.0f });
    spritesheet.AddState(PS_WALK, { 3, 2, 3, 0.5f });

    // Update initial UVs
    spritesheet.UpdateUV();

    Registry::Get().Add_Component<RenderComponent>(instance);
    RenderComponent& renderComp = Registry::Get().Get_Component<RenderComponent>(instance);

    renderComp.SetRenderLayer(LAYER_FOREGROUND);

    renderComp.SetSpriteSheet(spritesheet);

    // Tag for batching
    Registry::Get().Add_Component<TagComponent>(instance, TagComponent{ T_BATCH });

    Registry::Get().Add_Component<RenderLayerComponent>(instance, RenderLayerComponent{ LAYER_FOREGROUND });

    return instance;
}


/**
 * @brief Creates a generic entity driven by a C# Mono script.
 * @param scriptName Name of the C# class inside GameScripts.dll (e.g. "PlayerScript").
 * @param x,y        World position to spawn at.
 */
//Entity Create_ScriptedEntity(const std::string& scriptName, float x, float y)
//{
//    Registry& reg = Registry::Get();
//
//    // Create entity and basic info
//    Entity cir = reg.Create_Entity();
//    reg.Set_EntityName(cir, "ScriptedCircle");
//
//    // Add Transform
//    reg.Add_Component<TransformComponent>(cir, TransformComponent{ {x, y}, {100.f, 100.f} });
//
//    // Add Shape + Input (optional)
//    //reg.Add_Component<ShapeComponent>(cir, ShapeComponent{ SHAPE_CIRCLE, {200.f, 200.f}, {0.0f, 1.0f, 0.0f} });
//    reg.Add_Component<InputComponent>(cir, InputComponent{ CS_ARROWS });
//
//    // --- Add the Mono script binding ---
//    MonoScriptComponent msc;
//    msc.className = scriptName; // must match C# class name in GameScripts.dll
//    reg.Add_Component<MonoScriptComponent>(cir, msc);
//
//    std::cout << "[EntityCreation] Created scripted entity with Mono class: "
//        << scriptName << " (Entity " << cir << ")\n";
//
//    return cir;
//}
//
//Entity Create_Arrow(float const& x, float const& y) {
//
//    Entity arr = LoadEntity("arrow", x, y);
//    MonoScriptComponent script;
//    script.className = "Rotator"; // must match your C# class
//    Registry::Get().Add_Component<MonoScriptComponent>(arr, script);
//    return arr;
//}