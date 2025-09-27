
#include "EntityCreation.h"
#include "Render.h"

Entity Create_Player(float x, float y) {
    Entity player = Create_Entity();
    Add_Component(player, TransformComponent{ {x, y}, {100.f, 100.f} });

    {
        AABB aabb = Set_AABB({ x,y }, { 200.f,200.f });
        Circle circle = { {x,y},200.f };
        Physics::PhysicsComp temp(0.015f, true, aabb, circle);
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