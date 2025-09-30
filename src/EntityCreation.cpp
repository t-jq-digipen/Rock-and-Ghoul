#include "EntityCreation.h"
#include "Render.h"
#include "Animator.h"

Entity Create_Player(float const& x, float const& y) {
    Entity player = Create_Entity();

    //static std::unordered_map<Entity, std::string>& nameMap = GetEntityNameMap();
    //nameMap[player] = name;

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

    Image temp = CreateImage("../Assets/Rock_FightingPose_2048px.png");
    Add_Component(player, Sprite::SpriteComponent{ temp, {.1f,.1f } });

    Add_Component(player, HealthComponent{ 100.f, 100.f });
    return player;
}

Entity Create_Waffle(float const& x, float const& y) {
    Entity player = Create_Entity();
    Add_Component(player, TransformComponent{ {x, y}, {100.f, 100.f} });

    {
        AABB aabb = Set_AABB({ x,y }, { 200.f,200.f });
        Circle circle = { {x,y},200.f };
        Physics::PhysicsComponent temp(0.015f, true, aabb, circle);
        Add_Component(player, temp);
    }

    //Add_Component(player, AABBComponent{ true, {}, 200.0f, 200.0f });
    Add_Component(player, InputComponent{ CS_IJKL });
    Add_Component(player, TagComponent{ T_PLAYER });

    {
        Image temp = CreateImage("../Assets/Waffle_Sprites.png");
        Add_Component(player, Sprite::SpriteComponent{ temp, {10.f,10.f }, true });
    }
    {
        
        Animator::AnimatorComponent temp(3, 6, {0,0,0}, 0.f);
        Add_Component(player, temp);
    }
    Add_Component(player, HealthComponent{ 100.f, 100.f });
    return player;
}


Entity Create_Shape(float const& spawnx, float const& spawny, ShapeType shapeType, ControlScheme controls) {
    Entity shape = Create_Entity();
    Add_Component(shape, TransformComponent{ {spawnx, spawny}, {100.f, 100.f} });

    // Set dimensions and color based on shape type
    Vec2 dimensions;
    Vec3 color;
    AABB aabb;
    Circle circle;

    if (shapeType == SHAPE_RECTANGLE) {
        dimensions = { 200.f, 200.f };
        color = { 1.0f, 0.0f, 0.0f };  // Red
        aabb = Set_AABB({ spawnx, spawny }, dimensions);
        circle = { {spawnx, spawny}, 200.f };
    }
    else if (shapeType == SHAPE_CIRCLE) {
        dimensions = { 200.f, 200.f };
        color = { 0.0f, 1.0f, 0.0f };  // Green
        aabb = Set_AABB({ spawnx, spawny }, dimensions);
        circle = { {spawnx, spawny}, 100.f };  // Smaller radius for circle
    }

    // Add physics component
    Physics::PhysicsComponent phys(0.015f, true, aabb, circle);
    Add_Component(shape, phys);

    // Add shape component
    Add_Component(shape, ShapeComponent{ shapeType, dimensions, color });
    Add_Component(shape, InputComponent{ controls });
    Add_Component(shape, TagComponent{ T_PLAYER });
    Add_Component(shape, HealthComponent{ 100.f, 100.f });

    return shape;
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