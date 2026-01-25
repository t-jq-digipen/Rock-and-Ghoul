/* ========================================================
 * @file        GameMemoryManager.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        24 JAN 2026
 *
 * @brief
 * Central memory manager for Rock & Ghoul game objects.
 * Provides pre-allocated pools for all major game entity types:
 * - Projectiles (soundwave spells)
 * - AOE effects
 * - Shield instances
 * - Particle effects
 * - Enemy entities
 *
 * Usage:
 *   GameMemoryManager::Get().InitializePools();  // At game start
 *   Projectile* proj = GameMemoryManager::Get().AllocateProjectile();
 *   GameMemoryManager::Get().FreeProjectile(proj);
 *
 * Copyright © 2026 DigiPen, All rights reserved.
 * ========================================================
 */

#ifndef GAME_MEMORY_MANAGER_H
#define GAME_MEMORY_MANAGER_H

#include "MemoryPool.h"
#include <iostream>

 // ========================================================
 // Forward declarations for game types
 // ========================================================

 // Projectile data (soundwave spell)
struct ProjectileData {
    float posX, posY;
    float velX, velY;
    float lifetime;
    int damage;
    bool active;

    ProjectileData()
        : posX(0), posY(0), velX(0), velY(0),
        lifetime(0), damage(0), active(false) {
    }
};

// AOE effect data
struct AOEData {
    float centerX, centerY;
    float radius;
    float duration;
    int damage;
    bool active;

    AOEData()
        : centerX(0), centerY(0), radius(0),
        duration(0), damage(0), active(false) {
    }
};

// Shield data
struct ShieldData {
    float posX, posY;
    float health;
    float duration;
    bool active;

    ShieldData()
        : posX(0), posY(0), health(0),
        duration(0), active(false) {
    }
};

// Particle effect data
struct ParticleData {
    float posX, posY;
    float velX, velY;
    float lifetime;
    float size;
    uint8_t r, g, b, a;  // Color
    bool active;

    ParticleData()
        : posX(0), posY(0), velX(0), velY(0),
        lifetime(0), size(1.0f),
        r(255), g(255), b(255), a(255),
        active(false) {
    }
};

// Enemy ghost data
struct EnemyData {
    float posX, posY;
    float health;
    int type;  // Different enemy types
    bool active;

    EnemyData()
        : posX(0), posY(0), health(100),
        type(0), active(false) {
    }
};

// ========================================================
// GameMemoryManager - Singleton pool manager
// ========================================================
class GameMemoryManager {
public:
    // Singleton access
    static GameMemoryManager& Get() {
        static GameMemoryManager instance;
        return instance;
    }

    // Pool sizes (tune these based on your game needs)
    static constexpr size_t MAX_PROJECTILES = 100;
    static constexpr size_t MAX_AOE_EFFECTS = 20;
    static constexpr size_t MAX_SHIELDS = 5;
    static constexpr size_t MAX_PARTICLES = 500;
    static constexpr size_t MAX_ENEMIES = 50;

    // Initialize all pools (call at game start)
    void InitializePools();

    // Projectile pool
    ProjectileData* AllocateProjectile();
    void FreeProjectile(ProjectileData* proj);

    // AOE pool
    AOEData* AllocateAOE();
    void FreeAOE(AOEData* aoe);

    // Shield pool
    ShieldData* AllocateShield();
    void FreeShield(ShieldData* shield);

    // Particle pool
    ParticleData* AllocateParticle();
    void FreeParticle(ParticleData* particle);

    // Enemy pool
    EnemyData* AllocateEnemy();
    void FreeEnemy(EnemyData* enemy);

    // Stats and debugging
    void PrintStats() const;
    void ResetPools();

private:
    GameMemoryManager() = default;
    ~GameMemoryManager() = default;

    // Prevent copying
    GameMemoryManager(const GameMemoryManager&) = delete;
    GameMemoryManager& operator=(const GameMemoryManager&) = delete;

    // Memory pools
    MemoryPool<ProjectileData, MAX_PROJECTILES> projectilePool;
    MemoryPool<AOEData, MAX_AOE_EFFECTS> aoePool;
    MemoryPool<ShieldData, MAX_SHIELDS> shieldPool;
    MemoryPool<ParticleData, MAX_PARTICLES> particlePool;
    MemoryPool<EnemyData, MAX_ENEMIES> enemyPool;

    bool initialized = false;
};

// ========================================================
// Implementation
// ========================================================

inline void GameMemoryManager::InitializePools() {
    if (initialized) {
        std::cout << "[GameMemoryManager] Pools already initialized!\n";
        return;
    }

    std::cout << "[GameMemoryManager] Initializing memory pools...\n";
    std::cout << "  - Projectiles: " << MAX_PROJECTILES << " slots\n";
    std::cout << "  - AOE Effects: " << MAX_AOE_EFFECTS << " slots\n";
    std::cout << "  - Shields: " << MAX_SHIELDS << " slots\n";
    std::cout << "  - Particles: " << MAX_PARTICLES << " slots\n";
    std::cout << "  - Enemies: " << MAX_ENEMIES << " slots\n";

    size_t totalMemory =
        (sizeof(ProjectileData) * MAX_PROJECTILES) +
        (sizeof(AOEData) * MAX_AOE_EFFECTS) +
        (sizeof(ShieldData) * MAX_SHIELDS) +
        (sizeof(ParticleData) * MAX_PARTICLES) +
        (sizeof(EnemyData) * MAX_ENEMIES);

    std::cout << "  Total pre-allocated memory: " << totalMemory << " bytes ("
        << (totalMemory / 1024.0f) << " KB)\n";

    initialized = true;
}

inline ProjectileData* GameMemoryManager::AllocateProjectile() {
    ProjectileData* proj = projectilePool.Construct();
    if (proj == nullptr) {
        std::cerr << "[GameMemoryManager] ERROR: Projectile pool exhausted!\n";
    }
    else {
        std::cout << "RUNTIME: Booked projectile memory from pre-allocated pool (address: "
            << proj << ")\n";
    }
    return proj;
}

inline void GameMemoryManager::FreeProjectile(ProjectileData* proj) {
    if (proj != nullptr) {
        std::cout << "RUNTIME: Freed projectile memory back to pool (address: "
            << proj << ")\n";
        projectilePool.Destroy(proj);
    }
}

inline AOEData* GameMemoryManager::AllocateAOE() {
    AOEData* aoe = aoePool.Construct();
    if (aoe == nullptr) {
        std::cerr << "[GameMemoryManager] ERROR: AOE pool exhausted!\n";
    }
    else {
        std::cout << "RUNTIME: Booked AOE memory from pre-allocated pool (address: "
            << aoe << ")\n";
    }
    return aoe;
}

inline void GameMemoryManager::FreeAOE(AOEData* aoe) {
    if (aoe != nullptr) {
        std::cout << "RUNTIME: Freed AOE memory back to pool (address: "
            << aoe << ")\n";
        aoePool.Destroy(aoe);
    }
}

inline ShieldData* GameMemoryManager::AllocateShield() {
    ShieldData* shield = shieldPool.Construct();
    if (shield == nullptr) {
        std::cerr << "[GameMemoryManager] ERROR: Shield pool exhausted!\n";
    }
    return shield;
}

inline void GameMemoryManager::FreeShield(ShieldData* shield) {
    if (shield != nullptr) {
        shieldPool.Destroy(shield);
    }
}

inline ParticleData* GameMemoryManager::AllocateParticle() {
    ParticleData* particle = particlePool.Construct();
    if (particle == nullptr) {
        std::cerr << "[GameMemoryManager] ERROR: Particle pool exhausted!\n";
    }
    return particle;
}

inline void GameMemoryManager::FreeParticle(ParticleData* particle) {
    if (particle != nullptr) {
        particlePool.Destroy(particle);
    }
}

inline EnemyData* GameMemoryManager::AllocateEnemy() {
    EnemyData* enemy = enemyPool.Construct();
    if (enemy == nullptr) {
        std::cerr << "[GameMemoryManager] ERROR: Enemy pool exhausted!\n";
    }
    return enemy;
}

inline void GameMemoryManager::FreeEnemy(EnemyData* enemy) {
    if (enemy != nullptr) {
        enemyPool.Destroy(enemy);
    }
}

inline void GameMemoryManager::PrintStats() const {
    std::cout << "\n=== Game Memory Pool Stats ===\n";
    std::cout << "Projectiles: " << projectilePool.GetUsedCount() << "/" << MAX_PROJECTILES << " used\n";
    std::cout << "AOE Effects: " << aoePool.GetUsedCount() << "/" << MAX_AOE_EFFECTS << " used\n";
    std::cout << "Shields:     " << shieldPool.GetUsedCount() << "/" << MAX_SHIELDS << " used\n";
    std::cout << "Particles:   " << particlePool.GetUsedCount() << "/" << MAX_PARTICLES << " used\n";
    std::cout << "Enemies:     " << enemyPool.GetUsedCount() << "/" << MAX_ENEMIES << " used\n";
    std::cout << "==============================\n\n";
}

inline void GameMemoryManager::ResetPools() {
    std::cout << "[GameMemoryManager] WARNING: Resetting all pools (potential memory leak if objects not destroyed!)\n";
    // Note: This doesn't call destructors - ensure all objects are properly freed first
    initialized = false;
    InitializePools();
}

#endif // GAME_MEMORY_MANAGER_H