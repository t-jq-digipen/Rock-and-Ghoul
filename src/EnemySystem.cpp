#pragma once
#include "EnemySystem.h"
#include "Registry.h"
#include "Component.h"
#include "Remath.h"
#include "BehaviourFCT.h"
#include "LogicSystem.h"
#include "GameScripts.h"
#include "CoreEngine.h"

namespace EnemyAI
{
	inline void Start(Entity eID)
	{
		std::cout << "Enemy script started" << std::endl;
	}

	inline void Update(Entity eID, float dt)
	{
        if (!Registry::Get().Has_Component<Enemy::EnemyAIComponent>(eID)) return;
	}
}

// ============================
// Internal helpers (file-local)
// ============================

static inline void ApplyDesiredVelocity(Entity e, const Vec2& desiredVel, float dt)
{
    auto& reg = Registry::Get();
    if (reg.Has_Component<Physics::PhysicsComponent>(e))
    {
        auto& phys = reg.Get_Component<Physics::PhysicsComponent>(e);
        // Simple proportional force towards desired velocity (no extra math helpers needed)
        const Vec2 v = phys.Get_Vel();
        const Vec2 dv = RG_Subtract(desiredVel, v);
        const Vec2 f = RG_Scale(dv, 200.0f); // tweakable gain
        phys.AddForce(f, 1.f, 0.f);
    }
    else if (reg.Has_Component<TransformComponent>(e))
    {
        auto& tr = reg.Get_Component<TransformComponent>(e);
        tr.pos = RG_Add(tr.pos, RG_Scale(desiredVel, dt));
    }
}

static inline bool InRange(const Vec2& a, const Vec2& b, float r)
{
    return RG_Distance(a, b) <= r;
}

void EnemySystem::Initialize()
{
    RegisterAllGameScripts();
}

void EnemySystem::Shutdown()
{
    // Nothing to clean.
}

void EnemySystem::Update(float dt)
{
    // check if game paused
    if (GSM_IsGamePaused()) {
        return;  // Don't update player movement or footsteps when paused
    }

    auto& reg = Registry::Get();

    // Iterate all entities that have EnemyAIComponent
    for (auto& [e, ai] : reg.Get_ComponentMap<Enemy::EnemyAIComponent>())
    {
        // Require a Transform to operate
        if (!reg.Has_Component<TransformComponent>(e)) continue;
        auto& tr = reg.Get_Component<TransformComponent>(e);

        // Initialize start position once (if (0,0) and you want to store spawn)
        if (ai.startPos.x == 0.f && ai.startPos.y == 0.f)
            ai.startPos = tr.pos;

        // Timers
        ai.stateTimer += dt;
        if (ai.shootTimer > 0.f) ai.shootTimer -= dt;
        if (ai.reloadTimer > 0.f) ai.reloadTimer -= dt;

        // Finish reload when timer elapses
        if (ai.isReloading && ai.reloadTimer <= 0.f)
        {
            ai.isReloading = false;
            ai.currentAmmo = (ai.magazineSize > 0) ? ai.magazineSize : 1;
        }

        // Resolve/refresh target
        Entity target = ai.target;
        float bestDist = std::numeric_limits<float>::max();
        if (target == 0)
            target = FindNearestPlayer(tr.pos, bestDist);

        Vec2 tgtPos = tr.pos;
        if (target && reg.Has_Component<TransformComponent>(target))
            tgtPos = reg.Get_Component<TransformComponent>(target).pos;

        const Vec2 toTgt = RG_Subtract(tgtPos, tr.pos);
        const float dist = RG_Length(toTgt);
        const Vec2 dir = (dist > 0.f ? RG_Normalize(toTgt) : Vec2{ 0.f,0.f });

        // One-frame outputs (if your downstream systems consume these)
        ai.wantsMelee = false;
        ai.wantsShoot = false;
        ai.shootDir = { 0.f, 0.f };

        // FSM
        switch (ai.state)
        {
        case Enemy::ENEMY_IDLE:
        {
            ApplyDesiredVelocity(e, { 0.f,0.f }, dt);

            if (!ai.canMove) break;

            if (ai.stateTimer >= ai.idleDuration)
            {
                PickNewRoamTarget(ai);
                ChangeState(ai, Enemy::ENEMY_ROAMING);
            }
            else if (DetectTarget(ai, tr.pos, tgtPos))
            {
                ChangeState(ai, Enemy::ENEMY_DETECTING);
            }
        } break;

        case Enemy::ENEMY_ROAMING:
        {
            if (!ai.canMove) { ApplyDesiredVelocity(e, { 0.f,0.f }, dt); break; }

            const Vec2 to = RG_Subtract(ai.roamTarget, tr.pos);
            const float d = RG_Length(to);

            if (d > 0.3f)
            {
                const Vec2 vel = RG_Scale(RG_Normalize(to), ai.moveSpeed);
                ApplyDesiredVelocity(e, vel, dt);
            }
            else
            {
                ApplyDesiredVelocity(e, { 0.f,0.f }, dt);
                if (ai.stateTimer >= ai.roamWaitDuration)
                    ChangeState(ai, Enemy::ENEMY_IDLE);
            }

            if (DetectTarget(ai, tr.pos, tgtPos))
                ChangeState(ai, Enemy::ENEMY_DETECTING);
        } break;

        case Enemy::ENEMY_DETECTING:
        {
            // short delay to simulate reaction
            if (ai.stateTimer >= 0.3f)
                ChangeState(ai, Enemy::ENEMY_CHASING);
        } break;

        case Enemy::ENEMY_CHASING:
        {
            if (!ai.canMove) { ApplyDesiredVelocity(e, { 0.f,0.f }, dt); break; }

            if (dist > ai.loseInterestDistance)
            {
                ChangeState(ai, Enemy::ENEMY_ROAMING);
                break;
            }

            ApplyDesiredVelocity(e, RG_Scale(dir, ai.chaseSpeed), dt);

            if (ai.attackMode == Enemy::MELEE_ONLY)
            {
                if (dist <= ai.meleeRange)
                    ChangeState(ai, Enemy::ENEMY_ATTACKING);
            }
            else // PROJECTILE_ONLY
            {
                if (dist <= ai.engageRange)
                    ChangeState(ai, Enemy::ENEMY_ATTACKING);
            }
        } break;

        case Enemy::ENEMY_ATTACKING:
        {
            ApplyDesiredVelocity(e, { 0.f,0.f }, dt);

            if (!ai.canMove) break;

            if (ai.attackMode == Enemy::MELEE_ONLY)
            {
                // use shootTimer as a local cooldown ticker
                if (ai.canHit)
                {
                    ai.wantsMelee = true;            // pulse
                    ai.canHit = false;
                    ai.shootTimer = ai.meleeCooldown;
                }
                if (!ai.canHit && ai.shootTimer <= 0.f)
                {
                    ai.canHit = true;
                    ChangeState(ai, Enemy::ENEMY_COOLDOWN);
                }
            }
            else // PROJECTILE_ONLY
            {
                if (dist <= ai.shootRange)
                {
                    if (!ai.isReloading && ai.shootTimer <= 0.f && ai.currentAmmo > 0)
                    {
                        ai.wantsShoot = true;        // pulse
                        ai.shootDir = dir;
                        ai.currentAmmo -= 1;
                        ai.shootTimer = ai.shootCooldown;
                    }
                    if (ai.currentAmmo <= 0 && !ai.isReloading)
                    {
                        ai.isReloading = true;
                        ai.reloadTimer = ai.reloadTime;
                    }
                }

                // brief window, then back to chase
                if (ai.stateTimer >= ai.shootCooldown)
                    ChangeState(ai, Enemy::ENEMY_COOLDOWN);
            }
        } break;

        case Enemy::ENEMY_COOLDOWN:
        {
            if (ai.stateTimer >= 0.5f)
                ChangeState(ai, Enemy::ENEMY_CHASING);
        } break;

        case Enemy::ENEMY_STAGGERED:
        {
            ApplyDesiredVelocity(e, { 0.f,0.f }, dt);
            const float dur = 0.5f;
            if (ai.stateTimer >= dur)
            {
                if (DetectTarget(ai, tr.pos, tgtPos))
                    ChangeState(ai, Enemy::ENEMY_CHASING);
                else
                    ChangeState(ai, Enemy::ENEMY_ROAMING);

                ai.canMove = true;
                ai.canHit = true;
            }
        } break;

        default: break;
        }

        // Safety: ensure ammo positive if magazine is defined
        if (ai.currentAmmo <= 0 && !ai.isReloading)
            ai.currentAmmo = (ai.magazineSize > 0) ? ai.magazineSize : 1;
    }
}

// ============================
// Helper implementations
// ============================

void EnemySystem::ChangeState(Enemy::EnemyAIComponent& ai, Enemy::EnemyState next)
{
    ai.state = next;
    ai.stateTimer = 0.f;
    ai.wantsMelee = false;
    ai.wantsShoot = false;
    ai.shootDir = { 0.f, 0.f };
}

void EnemySystem::PickNewRoamTarget(Enemy::EnemyAIComponent& ai)
{
    // Simple circular roam around startPos using stateTimer
    const float t = ai.stateTimer;
    const float cs = cos(t);
    const float sn = sin(t);
    ai.roamTarget = RG_Add(ai.startPos, Vec2{ cs * ai.roamRadius, sn * ai.roamRadius });
}

Entity EnemySystem::FindNearestPlayer(const Vec2& enemyPos, float& outDist) const
{
    auto& reg = Registry::Get();
    Entity best = 0;
    outDist = std::numeric_limits<float>::max();

    for (auto& [e, tag] : reg.Get_ComponentMap<TagComponent>())
    {
        if (tag.tag != T_PLAYER) continue;
        if (!reg.Has_Component<TransformComponent>(e)) continue;

        const Vec2 p = reg.Get_Component<TransformComponent>(e).pos;
        const float d = RG_Distance(enemyPos, p);
        if (d < outDist) { outDist = d; best = e; }
    }
    return best;
}

bool EnemySystem::DetectTarget(const Enemy::EnemyAIComponent& ai, const Vec2& enemyPos, const Vec2& targetPos) const
{
    return InRange(enemyPos, targetPos, ai.detectionRadius);
}