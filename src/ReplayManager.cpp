#include "ReplayManager.h"
#include "Registry.h"
#include "Component.h" // <-- where TransformComponent is
#include <unordered_set>
#include "CombatSystem.h"
#include "BehaviourComponent.h"
#include "MonoScriptComponent.h"
#include <SceneManager.h>
#include <Camera.h>
#include <PlayerSystem.h>
//#include "RenderComponent.h"



ReplayManager& ReplayManager::Get()
{
    static ReplayManager inst;
    return inst;
}

void ReplayManager::Configure(int fps_, int secondsToKeep)
{
    if (fps_ <= 0) fps_ = 60;
    if (secondsToKeep <= 0) secondsToKeep = 10;

    fps = fps_;
    seconds = secondsToKeep;
    capacity = fps * seconds;

    ring.clear();
    ring.resize(capacity);

    writeIdx = 0;
    validCount = 0;
    playCursor = 0;
}

void ReplayManager::StartRecording()
{
    mode = Mode::Recording;
    writeIdx = 0;
    validCount = 0;
    playCursor = 0;

    if ((int)ring.size() != capacity)
    {
        ring.clear();
        ring.resize(capacity);
    }

    for (int i = 0; i < capacity; ++i)
        ring[i].entities.clear();
}

void ReplayManager::StopRecording()
{
    if (mode == Mode::Recording) mode = Mode::Off;
}

void ReplayManager::StartPlayback()
{
    if (validCount <= 0) return;
    mode = Mode::Playback;
    playCursor = 0;
}

void ReplayManager::StopPlayback()
{
    if (mode == Mode::Playback) mode = Mode::Off;

    //SceneManager::Get().ReloadScene();
}

int ReplayManager::OldestIndex() const
{
    return (validCount < capacity) ? 0 : writeIdx;
}

int ReplayManager::IndexAt(int logicalIndex) const
{
    int idx = OldestIndex() + logicalIndex;
    while (idx >= capacity) idx -= capacity;
    return idx;
}

void ReplayManager::CaptureTick()
{
    if (mode != Mode::Recording) return;

    Registry& reg = Registry::Get();
    FrameSnapshot& frame = ring[writeIdx];
    frame.entities.clear();

    // Iterate all entities that have TransformComponent
    auto& tmap = reg.Get_ComponentMap<TransformComponent>();
    frame.entities.reserve(tmap.size());



    for (auto& [e, tc] : tmap)
    {
        //transform component
        EntitySnapshot s{};
        s.id = e;
        s.px = tc.pos.x;
        s.py = tc.pos.y;
        s.sx = tc.scale.x;
        s.sy = tc.scale.y;
        s.rot = tc.rot;

        //physics compoennt
        if (reg.Has_Component<Physics::PhysicsComponent>(e))
        {
            auto& phys = reg.Get_Component<Physics::PhysicsComponent>(e);

            s.hasPhysics = true;

            Vec2 v = phys.Get_Vel();
            s.vx = v.x;
            s.vy = v.y;
        }

        //movement component
        if (reg.Has_Component<Movement::MovementComponent>(e))
        {
            auto& mv = reg.Get_Component<Movement::MovementComponent>(e);

            s.hasMovement = true;
            s.isMoving = mv.GetIsMoving();

            Vec2 dir = mv.GetMovementDirection();
            s.mdx = dir.x;
            s.mdy = dir.y;

            s.inputDir = static_cast<int>(mv.GetInputDirection());

            // infer facing direction
            if (dir.x < 0)
                s.faceRight = false;
            else if (dir.x > 0)
                s.faceRight = true;
        }

        //animations
        if (reg.Has_Component<RenderComponent>(e))
        {
            auto& rc = reg.Get_Component<RenderComponent>(e);
            if (rc.GetRenderType() == RT_SPRITESHEET)
            {
                auto& ss = rc.GetSpriteSheet();
                s.hasAnim = true;
                s.animState = ss.GetCurrentState();
                s.animCol = ss.GetCurrentCol();
                s.animTimer = ss.GetAnimTimer();
                s.animFlipX = ss.Sprite().GetFlipX();
            }

        }

        if (reg.Has_Component<HealthComponent>(e))
        {
            auto& hp = reg.Get_Component<HealthComponent>(e);
            s.hasHealth = true;
            s.currHp = hp.currhp;
            s.maxHp = hp.maxhp;
        }

        if (reg.Has_Component<LifetimeComponent>(e))
        {
            auto& lt = reg.Get_Component<LifetimeComponent>(e);
            s.hasLifetime = true;
            s.age = lt.age;
            s.lifetime = lt.lifetime;
        }

        if (reg.Has_Component<StatusEffectComponent>(e))
        {
            auto& se = reg.Get_Component<StatusEffectComponent>(e);
            s.hasStatus = true;
            s.activeEffect = static_cast<int>(se.activeEffect);
            s.effectTimer = se.effectTimer;
            s.burnTickTimer = se.burnTickTimer;
            s.originalMoveSpeed = se.originalMoveSpeed;
            s.isStunned = se.isStunned;
        }

 


        frame.entities.push_back(s);
    }

    // advance ring
    writeIdx++;
    if (writeIdx >= capacity) writeIdx = 0;
    if (validCount < capacity) validCount++;
   
}

/*void ReplayManager::ApplyTick()
{
    if (mode != Mode::Playback) return;

    if (playCursor >= validCount)
    {
        mode = Mode::Off;
        return;
    }

    const FrameSnapshot& frame = ring[IndexAt(playCursor)];
    Registry& reg = Registry::Get();

    for (const EntitySnapshot& s : frame.entities)
    {
        // Entity might have been destroyed; skip if Transform missing
        if (!reg.Has_Component<TransformComponent>(s.id))
            continue;

        auto& tc = reg.Get_Component<TransformComponent>(s.id);
        tc.pos.x = s.px;
        tc.pos.y = s.py;
        tc.scale.x = s.sx;
        tc.scale.y = s.sy;
        tc.rot = s.rot;

        //physics component
        if (s.hasPhysics && reg.Has_Component<Physics::PhysicsComponent>(s.id))
        {
            auto& phys = reg.Get_Component<Physics::PhysicsComponent>(s.id);
            phys.Set_Vel({ s.vx, s.vy });
        }

        //movement component
        if (s.hasMovement && reg.Has_Component<Movement::MovementComponent>(s.id))
        {
            auto& mv = reg.Get_Component<Movement::MovementComponent>(s.id);

            mv.SetIsMoving(s.isMoving);
            mv.SetMovementDirection({ s.mdx, s.mdy });
            mv.SetInputDirection(static_cast<Movement::InputDirection>(s.inputDir));
        }

        //animations
        if (s.hasAnim && reg.Has_Component<RenderComponent>(s.id))
        {
            auto& rc = reg.Get_Component<RenderComponent>(s.id);
            if (rc.GetRenderType() == RT_SPRITESHEET)
            {
                auto& ss = rc.GetSpriteSheet();
                if ((unsigned int)ss.GetCurrentState() != s.animState)
                    ss.SetState(s.animState);
                ss.SetCurrentCol(s.animCol);
                ss.SetAnimTimer(s.animTimer);
                ss.Sprite().SetFlipX(s.animFlipX);
                ss.UpdateUV();
            }
        }

        //camera 
        if (Registry::Get().Has_Component<TagComponent>(s.id))
        {
            auto& tag = Registry::Get().Get_Component<TagComponent>(s.id);
            if (tag.tag == T_PLAYER)
            {
                Camera::GetInstance().FollowTarget({ s.px, s.py }, 1.f / 60.f);
                break;
            }
        }


        if (s.hasHealth && reg.Has_Component<HealthComponent>(s.id))
        {
            auto& hp = reg.Get_Component<HealthComponent>(s.id);
            hp.currhp = s.currHp;
            hp.maxhp = s.maxHp;
        }

        if (s.hasLifetime && reg.Has_Component<LifetimeComponent>(s.id))
        {
            auto& lt = reg.Get_Component<LifetimeComponent>(s.id);
            lt.age = s.age;
            lt.lifetime = s.lifetime;
        }

        if (s.hasStatus && reg.Has_Component<StatusEffectComponent>(s.id))
        {
            auto& se = reg.Get_Component<StatusEffectComponent>(s.id);
            se.activeEffect = static_cast<Element>(s.activeEffect);
            se.effectTimer = s.effectTimer;
            se.burnTickTimer = s.burnTickTimer;
            se.originalMoveSpeed = s.originalMoveSpeed;
            se.isStunned = s.isStunned;
        }

        // Update player health UI after restoring state
        /*Entity player = PlayerSystem::FindPlayer();
        if (player != INVALID_ENTITY && PlayerSystem::Instance())
            PlayerSystem::Instance()->UpdateHealthUI(player);
      
    }

    playCursor++;

   
}*/

void ReplayManager::ApplyTick()
{
    if (mode != Mode::Playback) return;

    if (playCursor >= validCount)
    {
        mode = Mode::Off;
        return;
    }

    const FrameSnapshot& frame = ring[IndexAt(playCursor)];
    Registry& reg = Registry::Get();

    bool foundPlayer = false;
    float playerX = 0.f;
    float playerY = 0.f;

    for (const EntitySnapshot& s : frame.entities)
    {
        if (!reg.Has_Component<TransformComponent>(s.id))
            continue;

        auto& tc = reg.Get_Component<TransformComponent>(s.id);
        tc.pos.x = s.px;
        tc.pos.y = s.py;
        tc.scale.x = s.sx;
        tc.scale.y = s.sy;
        tc.rot = s.rot;

        if (s.hasPhysics && reg.Has_Component<Physics::PhysicsComponent>(s.id))
        {
            auto& phys = reg.Get_Component<Physics::PhysicsComponent>(s.id);
            phys.Set_Vel({ s.vx, s.vy });
        }

        if (s.hasMovement && reg.Has_Component<Movement::MovementComponent>(s.id))
        {
            auto& mv = reg.Get_Component<Movement::MovementComponent>(s.id);
            mv.SetIsMoving(s.isMoving);
            mv.SetMovementDirection({ s.mdx, s.mdy });
            mv.SetInputDirection(static_cast<Movement::InputDirection>(s.inputDir));
        }

        if (s.hasAnim && reg.Has_Component<RenderComponent>(s.id))
        {
            auto& rc = reg.Get_Component<RenderComponent>(s.id);
            if (rc.GetRenderType() == RT_SPRITESHEET)
            {
                auto& ss = rc.GetSpriteSheet();
                if ((unsigned int)ss.GetCurrentState() != s.animState)
                    ss.SetState(s.animState);
                ss.SetCurrentCol(s.animCol);
                ss.SetAnimTimer(s.animTimer);
                ss.Sprite().SetFlipX(s.animFlipX);
                ss.UpdateUV();
            }
        }

        if (s.hasHealth && reg.Has_Component<HealthComponent>(s.id))
        {
            auto& hp = reg.Get_Component<HealthComponent>(s.id);
            hp.currhp = s.currHp;
            hp.maxhp = s.maxHp;

            //to remove just try debug
            /*if (reg.Has_Component<TagComponent>(s.id))
            {
                auto& tag = reg.Get_Component<TagComponent>(s.id);
                if (tag.tag == T_PLAYER)
                {
                    std::cout << "[Replay] Player HP restored: "
                        << hp.currhp << " / " << hp.maxhp << "\n";
                }
            }*/
        }

        if (s.hasLifetime && reg.Has_Component<LifetimeComponent>(s.id))
        {
            auto& lt = reg.Get_Component<LifetimeComponent>(s.id);
            lt.age = s.age;
            lt.lifetime = s.lifetime;
        }

        if (s.hasStatus && reg.Has_Component<StatusEffectComponent>(s.id))
        {
            auto& se = reg.Get_Component<StatusEffectComponent>(s.id);
            se.activeEffect = static_cast<Element>(s.activeEffect);
            se.effectTimer = s.effectTimer;
            se.burnTickTimer = s.burnTickTimer;
            se.originalMoveSpeed = s.originalMoveSpeed;
            se.isStunned = s.isStunned;
        }

        if (reg.Has_Component<TagComponent>(s.id))
        {
            auto& tag = reg.Get_Component<TagComponent>(s.id);
            if (tag.tag == T_PLAYER)
            {
                foundPlayer = true;
                playerX = s.px;
                playerY = s.py;
            }
        }

      
    }

    if (foundPlayer)
    {
        Camera::GetInstance().FollowTarget({ playerX, playerY }, 1.f / 60.f);
    }

    playCursor++;

    //make sure health is replayed correctly visually
    Entity player = PlayerSystem::FindPlayer();
    if (player != INVALID_ENTITY)
    {
        PlayerSystem::RefreshPlayerHealthUI(player);
    }
}
