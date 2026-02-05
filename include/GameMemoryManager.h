/* ========================================================
 * @file        GameMemoryManager.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        6 FEB 2026
 *
 * - Projectiles
 * - AOE
 * - Shield
 * - Particle
 * - Enemy projectiles

 *
 * Copyright © 2026 DigiPen, All rights reserved.
 * ========================================================
 */

#ifndef GAMEMEMORYMANAGER_H
#define GAMEMEMORYMANAGER_H

#include "MemoryPool.h"
#include <iostream>

//random pool parameters
static constexpr size_t PROJECTILE_OBJECTS_PER_PAGE = 100;
static constexpr size_t PROJECTILE_MAX_PAGES = 3; //300

static constexpr size_t AOE_OBJECTS_PER_PAGE = 20;
static constexpr size_t AOE_MAX_PAGES = 2;  // 40

static constexpr size_t SHIELD_OBJECTS_PER_PAGE = 5;
static constexpr size_t SHIELD_MAX_PAGES = 1;  // 5

static constexpr size_t PARTICLE_OBJECTS_PER_PAGE = 500;
static constexpr size_t PARTICLE_MAX_PAGES = 2;  // 1000
static constexpr size_t ENEMY_OBJECTS_PER_PAGE = 50;
static constexpr size_t ENEMY_MAX_PAGES = 3;  // 150


struct ProjectileData {
    float posX, posY;
    float velX, velY;
    float lifetime;
    int damage;
    bool active;

    ProjectileData(): posX(0), posY(0), velX(0), velY(0), lifetime(0), damage(0), active(false) {}
};

struct AOEData {
    float posX, posY;
    float radius;
    float duration;
    int damage;
    bool active;

    AOEData(): posX(0), posY(0), radius(0), duration(0), damage(0), active(false) {}
};

struct ShieldData {
    float posX, posY;
    float radius;
    float duration;
    int absorb;
    bool active;

    ShieldData(): posX(0), posY(0), radius(0), duration(0), absorb(0), active(false) {}
};

struct ParticleData {
    float posX, posY;
    float velX, velY;
    float lifetime;
    float scaleX, scaleY;
    bool active;

    ParticleData(): posX(0), posY(0), velX(0), velY(0), lifetime(0), scaleX(1), scaleY(1), active(false) {}
};

struct EnemyData {
    float posX, posY;
    int health;
    int damage;
    bool active;

    EnemyData(): posX(0), posY(0), health(0), damage(0), active(false) {}
};

/**
 * @brief all game object memory pools
 *
 */
class GameMemoryManager {
private:
  
    DynamicMemoryPool<ProjectileData, PROJECTILE_OBJECTS_PER_PAGE> projectilePool_;
    DynamicMemoryPool<AOEData, AOE_OBJECTS_PER_PAGE> aoePool_;
    DynamicMemoryPool<ShieldData, SHIELD_OBJECTS_PER_PAGE> shieldPool_;
    DynamicMemoryPool<ParticleData, PARTICLE_OBJECTS_PER_PAGE> particlePool_;
    DynamicMemoryPool<EnemyData, ENEMY_OBJECTS_PER_PAGE> enemyPool_;

    GameMemoryManager() : projectilePool_(PROJECTILE_MAX_PAGES) , aoePool_(AOE_MAX_PAGES) , shieldPool_(SHIELD_MAX_PAGES) , particlePool_(PARTICLE_MAX_PAGES), enemyPool_(ENEMY_MAX_PAGES)
    {}

public:
    static GameMemoryManager& Get() {
        static GameMemoryManager instance;
        return instance;
    }

    void InitializePools() {
       /* std::cout << "\n========================================\n";
        std::cout << "LOAD TIME - Initializing Dynamic Memory Pools\n";
        std::cout << "========================================\n";


        std::cout << "  - Projectiles: " << PROJECTILE_OBJECTS_PER_PAGE << " per page, " << "max " << PROJECTILE_MAX_PAGES << " pages (" 
        << (PROJECTILE_OBJECTS_PER_PAGE * PROJECTILE_MAX_PAGES) << " total)\n";

        std::cout << "  - AOE Effects: " << AOE_OBJECTS_PER_PAGE << " per page, "<< "max " << AOE_MAX_PAGES << " pages ("
            << (AOE_OBJECTS_PER_PAGE * AOE_MAX_PAGES) << " total)\n";

        std::cout << "  - Shields:     " << SHIELD_OBJECTS_PER_PAGE << " per page, "<< "max " << SHIELD_MAX_PAGES << " pages ("
            << (SHIELD_OBJECTS_PER_PAGE * SHIELD_MAX_PAGES) << " total)\n";

        std::cout << "  - Particles:   " << PARTICLE_OBJECTS_PER_PAGE << " per page, " << "max " << PARTICLE_MAX_PAGES << " pages ("
            << (PARTICLE_OBJECTS_PER_PAGE * PARTICLE_MAX_PAGES) << " total)\n";

        std::cout << "  - Enemies:     " << ENEMY_OBJECTS_PER_PAGE << " per page, "<< "max " << ENEMY_MAX_PAGES << " pages ("
            << (ENEMY_OBJECTS_PER_PAGE * ENEMY_MAX_PAGES) << " total)\n";*/

        // Calculate initial memory
        size_t initialMemory =
            sizeof(ProjectileData) * PROJECTILE_OBJECTS_PER_PAGE +
            sizeof(AOEData) * AOE_OBJECTS_PER_PAGE +
            sizeof(ShieldData) * SHIELD_OBJECTS_PER_PAGE +
            sizeof(ParticleData) * PARTICLE_OBJECTS_PER_PAGE +
            sizeof(EnemyData) * ENEMY_OBJECTS_PER_PAGE;


        //  maximum possible memory
        size_t maxMemory =
            sizeof(ProjectileData) * PROJECTILE_OBJECTS_PER_PAGE * PROJECTILE_MAX_PAGES +
            sizeof(AOEData) * AOE_OBJECTS_PER_PAGE * AOE_MAX_PAGES +
            sizeof(ShieldData) * SHIELD_OBJECTS_PER_PAGE * SHIELD_MAX_PAGES +
            sizeof(ParticleData) * PARTICLE_OBJECTS_PER_PAGE * PARTICLE_MAX_PAGES +
            sizeof(EnemyData) * ENEMY_OBJECTS_PER_PAGE * ENEMY_MAX_PAGES;


        std::cout << "\nDynamic memory pools ready\n";
       
    }

    // Projectile allocation
    ProjectileData* AllocateProjectile() {
        ProjectileData* proj = projectilePool_.Allocate();
        if (proj == nullptr) {
            std::cerr << "[GameMemoryManager] ERROR: Projectile pool exhausted!\n";
        }
        else {
            std::cout << "RUNTIME: Booked projectile memory (address: " << proj << ")\n";
        }
        return proj;
    }

    void FreeProjectile(ProjectileData* proj) {
        if (proj != nullptr) {
            std::cout << "RUNTIME: Freed projectile memory (address: "<< proj << ")\n";
            projectilePool_.Deallocate(proj);
        }
    }

    // AOE allocation
    AOEData* AllocateAOE() {
        AOEData* aoe = aoePool_.Allocate();
        if (aoe == nullptr) {
            std::cerr << "[GameMemoryManager] ERROR: AOE pool exhausted!\n";
        }
        else {
            std::cout << "RUNTIME: Booked AOE memory (address: "<< aoe << ")\n";
        }
        return aoe;
    }

    void FreeAOE(AOEData* aoe) {
        if (aoe != nullptr) {
            std::cout << "RUNTIME: Freed AOE memory (address: " << aoe << ")\n";
            aoePool_.Deallocate(aoe);
        }
    }

    // Shield allocation
    ShieldData* AllocateShield() {
        ShieldData* shield = shieldPool_.Allocate();
        if (shield == nullptr) {
            std::cerr << "[GameMemoryManager] ERROR: Shield pool exhausted!\n";
        }
        else {
            std::cout << "RUNTIME: Booked shield memory (address: "<< shield << ")\n";
        }
        return shield;
    }

    void FreeShield(ShieldData* shield) {
        if (shield != nullptr) {
            std::cout << "RUNTIME: Freed shield memory (address: "<< shield << ")\n";
            shieldPool_.Deallocate(shield);
        }
    }

    // Particle allocation
    ParticleData* AllocateParticle() {
        return particlePool_.Allocate();
    }

    void FreeParticle(ParticleData* particle) {
        particlePool_.Deallocate(particle);
    }

    // Enemy allocation
    EnemyData* AllocateEnemy() {
        return enemyPool_.Allocate();
    }

    void FreeEnemy(EnemyData* enemy) {
        enemyPool_.Deallocate(enemy);
    }

    void PrintStats() const {
        std::cout << "\n=== Game Memory Pool Stats ===\n";
        projectilePool_.PrintStats("Projectiles");
        aoePool_.PrintStats("AOE Effects");
        shieldPool_.PrintStats("Shields    ");
        particlePool_.PrintStats("Particles  ");
        enemyPool_.PrintStats("Enemies    ");
        std::cout << "==============================\n";
    }

};

#endif // GAMEMEMORYMANAGER_H