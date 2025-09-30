#include "EntityCreation.h"
#include "Render.h"

Entity Create_Player(float x, float y, const std::string& name) {
    Entity player = Create_Entity();

    static std::unordered_map<Entity, std::string>& nameMap = GetEntityNameMap();
    nameMap[player] = name;

    Add_Component(player, TransformComponent{ {x, y}, {100.f, 100.f} });

    {
        AABB aabb = Set_AABB({ x,y }, { 200.f,200.f });
        Circle circle = { {x,y},200.f };
        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
        Add_Component(player, temp);
    }

    //Add_Component(player, AABBComponent{ true, {}, 200.0f, 200.0f });
    Add_Component(player, InputComponent{ CS_WASD });
    Add_Component(player, TagComponent{ T_PLAYER });
    GLuint tex = Render::GetInstance().LoadTexture("../Assets/Rock_FightingPose_2048px.png");
    Add_Component(player, SpriteComponent{ tex, {400.f, 400.f} });
    Add_Component(player, HealthComponent{ 100.f, 100.f });
    return player;
}

Entity Create_Square(float x, float y, const std::string& name) {
    Entity square = Create_Entity();
    Add_Component(square, TransformComponent{ {x, y}, {100.f, 100.f} });

    {
        AABB aabb = Set_AABB({ x,y }, { 200.f,200.f });
        Circle circle = { {x,y},200.f };
        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
        Add_Component(square, temp);
    }

    //Add_Component(player, AABBComponent{ true, {}, 200.0f, 200.0f });
    Add_Component(square, InputComponent{ CS_IJKL });
    Add_Component(square, TagComponent{ T_PLAYER });
    Add_Component(square, SpriteComponent{ 0, {200.f, 200.f}, {1.0f, 0.0f, 0.0f}, SHAPE_RECT });
    Add_Component(square, HealthComponent{ 100.f, 100.f });

    // store the name
    SetEntityName(square, name);

    return square;
}

Entity Create_Circle(float x, float y, const std::string& name) {
    Entity round = Create_Entity();
    Add_Component(round, TransformComponent{ {x, y}, {100.f, 100.f} });
    {
        AABB aabb = Set_AABB({ x,y }, { 200.f,200.f });
        Circle circle = { {x,y},100.f };
        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
        Add_Component(round, temp);  // round refers to the entity
    }

    Add_Component(round, InputComponent{ CS_ARROWS });
    Add_Component(round, TagComponent{ T_PLAYER });
    Add_Component(round, SpriteComponent{ 0, {200.f, 200.f}, {0.0f, 1.0f, 0.0f}, SHAPE_CIRCLE }); // textureID=1 for circle, green color
    Add_Component(round, HealthComponent{ 100.f, 100.f });
    
    // store the name
    SetEntityName(round, name);

    return round;
}
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