/* ========================================================
 * @file        PooledEntityFactory.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        24 JAN 2026
 *
 * @brief
 * Factory system that combines memory pools with JSON prefab loading.
 * This allows data-driven entity creation (from JSON) while using
 * pre-allocated memory pools for performance.
 
 * 1. Load entity configuration from JSON prefab
 * 2. Allocate memory from pool (not heap)
 * 3. Initialize entity with prefab data
 * 4. Track entity in pool for cleanup
 *
 *
 * Copyright © 2026 DigiPen, All rights reserved.
 * ========================================================
 */

#ifndef POOLED_ENTITY_FACTORY_H
#define POOLED_ENTITY_FACTORY_H

#include "GameMemoryManager.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>  // ADD THIS LINE

using json = nlohmann::json;  // ADD THIS LINE

// ========================================================
// Entity Types - maps to different pools
// ========================================================
enum class EntityType {
    PROJECTILE,
    AOE,
    SHIELD,
    PARTICLE,
    ENEMY,
    UNKNOWN
};

// ========================================================
// Prefab Configuration - loaded from JSON
// ========================================================
struct PrefabConfig {
    // Transform data
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float rotation = 0.0f;

    // Physics/Movement data
    float speed = 0.0f;
    float lifetime = 0.0f;

    // Combat data
    int damage = 0;
    float health = 0.0f;
    float radius = 0.0f;

    // Visual data
    std::string texturePath;
    std::string spriteSheetPath;

    // Audio data
    std::string soundEffect;

    // Type-specific flags
    bool hasCollision = false;
    bool isAnimated = false;

    // Layer/Render settings
    int renderLayer = 0;

    // Constructor
    PrefabConfig() = default;
};

// ========================================================
// Pooled Entity - wraps pool data with ECS entity
// ========================================================
struct PooledEntity {
    unsigned int ecsEntity;     // ECS entity ID
    EntityType type;            // What pool it came from
    void* poolData;             // Pointer to pool memory
    bool active;

    // Default constructor
    PooledEntity()
        : ecsEntity(0), type(EntityType::UNKNOWN),
        poolData(nullptr), active(false) {
    }

    // Constructor with data
    PooledEntity(unsigned int e, EntityType t, void* data)
        : ecsEntity(e), type(t), poolData(data), active(true) {
    }
};

// ========================================================
// PooledEntityFactory - Creates entities from prefabs using pools
// ========================================================
class PooledEntityFactory {
public:
    // Singleton access
    static PooledEntityFactory& Get() {
        static PooledEntityFactory instance;
        return instance;
    }

    // Initialize factory (call after GameMemoryManager initialized)
    void Initialize();

    // Load prefab configuration from JSON file
    bool LoadPrefabConfig(const std::string& prefabPath, PrefabConfig& outConfig);

    // Create entity from prefab using memory pool
   /* PooledEntity CreateFromPrefab(
        const std::string& prefabPath,
        EntityType type,
        float spawnX,
        float spawnY
    );*/

    // Specialized creation functions with prefab support
    PooledEntity CreateProjectile(const std::string& prefabPath, float x, float y, float dirX, float dirY);
    PooledEntity CreateAOE(const std::string& prefabPath, float x, float y);
    PooledEntity CreateShield(const std::string& prefabPath, float x, float y);
    /*PooledEntity CreateParticle(const std::string& prefabPath, float x, float y, float velX, float velY);
    PooledEntity CreateEnemy(const std::string& prefabPath, float x, float y);*/

    // Destroy pooled entity (returns memory to pool)
    void DestroyEntity(PooledEntity& entity);

    // Get pooled data for an entity
    template<typename T>
    T* GetPoolData(const PooledEntity& entity);

    // Cache prefab configs for performance
    void CachePrefab(const std::string& prefabPath);
    void ClearPrefabCache();

    // Stats
    size_t GetActiveEntityCount(EntityType type) const;
    void PrintStats() const;

private:
    PooledEntityFactory() = default;
    ~PooledEntityFactory() = default;

    // Prevent copying
    PooledEntityFactory(const PooledEntityFactory&) = delete;
    PooledEntityFactory& operator=(const PooledEntityFactory&) = delete;

    // Prefab cache for performance
    std::unordered_map<std::string, PrefabConfig> prefabCache;

    // Track active entities by type
    std::unordered_map<EntityType, size_t> activeEntityCounts;

    bool initialized = false;

    // Helper to parse JSON (simplified version - adapt to your JSON library)
    bool ParsePrefabJSON(const std::string& content, PrefabConfig& config);

    // Apply prefab config to pool data
    void ApplyPrefabToProjectile(ProjectileData* proj, const PrefabConfig& config,
        float x, float y, float dirX, float dirY);
    void ApplyPrefabToAOE(AOEData* aoe, const PrefabConfig& config, float x, float y);
    void ApplyPrefabToShield(ShieldData* shield, const PrefabConfig& config, float x, float y);
    /*void ApplyPrefabToParticle(ParticleData* particle, const PrefabConfig& config,
        float x, float y, float velX, float velY);
    void ApplyPrefabToEnemy(EnemyData* enemy, const PrefabConfig& config, float x, float y);*/
};


inline void PooledEntityFactory::Initialize() {
    if (initialized) {
        std::cout << "[PooledEntityFactory] Already initialized!\n";
        return;
    }

    std::cout << "[PooledEntityFactory] Initializing factory...\n";

    // Initialize entity counts
    activeEntityCounts[EntityType::PROJECTILE] = 0;
    activeEntityCounts[EntityType::AOE] = 0;
    activeEntityCounts[EntityType::SHIELD] = 0;
    activeEntityCounts[EntityType::PARTICLE] = 0;
    activeEntityCounts[EntityType::ENEMY] = 0;

    initialized = true;
    std::cout << "[PooledEntityFactory] Factory ready!\n";
}

inline bool PooledEntityFactory::LoadPrefabConfig(const std::string& prefabPath, PrefabConfig& outConfig) {
    // Check cache first
    auto it = prefabCache.find(prefabPath);
    if (it != prefabCache.end()) {
        outConfig = it->second;
        return true;
    }

    // Load from file
    std::ifstream file(prefabPath);
    if (!file.is_open()) {
        std::cerr << "[PooledEntityFactory] Failed to open prefab: " << prefabPath << "\n";
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    if (!ParsePrefabJSON(content, outConfig)) {
        std::cerr << "[PooledEntityFactory] Failed to parse prefab: " << prefabPath << "\n";
        return false;
    }

    // Cache for future use
    prefabCache[prefabPath] = outConfig;

    return true;
}

inline bool PooledEntityFactory::ParsePrefabJSON(const std::string& content, PrefabConfig& config) {
    try {
        json j = json::parse(content);

        // Your JSON has "components" with nested JSON strings
        if (j.contains("components")) {
            auto components = j["components"];

            // Parse Physics component (double-nested JSON string)
            if (components.contains("Physics")) {
                std::string physicsStr = components["Physics"].get<std::string>();
                json physicsJson = json::parse(physicsStr);

                // Calculate speed from velocity
                float velX = physicsJson.value("velX", 0.0f);
                float velY = physicsJson.value("velY", 0.0f);
                config.speed = std::sqrt(velX * velX + velY * velY);

                std::cout << "[Parser] Parsed Physics: velX=" << velX << ", velY=" << velY
                    << ", speed=" << config.speed << "\n";
            }

            // Parse Lifetime component
            if (components.contains("Lifetime")) {
                std::string lifetimeStr = components["Lifetime"].get<std::string>();
                json lifetimeJson = json::parse(lifetimeStr);
                config.lifetime = lifetimeJson.value("lifetime", 2.0f);

                std::cout << "[Parser] Parsed Lifetime: " << config.lifetime << "s\n";
            }

            // Parse Transform for scale
            if (components.contains("Transform")) {
                std::string transformStr = components["Transform"].get<std::string>();
                json transformJson = json::parse(transformStr);
                config.scaleX = transformJson.value("scaleX", 1.0f);
                config.scaleY = transformJson.value("scaleY", 1.0f);

                std::cout << "[Parser] Parsed Transform: scale(" << config.scaleX
                    << ", " << config.scaleY << ")\n";
            }

            // Parse Render component for texture path
            if (components.contains("Render")) {
                std::string renderStr = components["Render"].get<std::string>();
                json renderJson = json::parse(renderStr);

                // The renderData is TRIPLE nested! 
                if (renderJson.contains("renderData")) {
                    std::string renderDataStr = renderJson["renderData"].get<std::string>();
                    json renderDataJson = json::parse(renderDataStr);

                    if (renderDataJson.contains("sprite")) {
                        std::string spriteStr = renderDataJson["sprite"].get<std::string>();
                        json spriteJson = json::parse(spriteStr);
                        config.texturePath = spriteJson.value("texturePath", "");

                        std::cout << "[Parser] Parsed Sprite: " << config.texturePath << "\n";
                    }
                }
            }

            // Set default damage (not in your JSON, so hardcode for now)
            config.damage = 30;
            std::cout << "[Parser] Using default damage: " << config.damage << "\n";
        }

        std::cout << "[Parser] Successfully parsed JSON!\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[Parser] JSON parse error: " << e.what() << "\n";
        return false;
    }
}

inline PooledEntity PooledEntityFactory::CreateProjectile(
    const std::string& prefabPath,
    float x, float y,
    float dirX, float dirY)
{
    // Load prefab configuration
    PrefabConfig config;
    if (!LoadPrefabConfig(prefabPath, config)) {
        std::cerr << "[PooledEntityFactory] Failed to load projectile prefab\n";
        return PooledEntity();  // Returns default (inactive) entity
    }

    // Allocate from pool
    ProjectileData* proj = GameMemoryManager::Get().AllocateProjectile();
    if (proj == nullptr) {
        std::cerr << "[PooledEntityFactory] Projectile pool exhausted!\n";
        return PooledEntity();  // Returns default (inactive) entity
    }

    // Apply prefab data
    ApplyPrefabToProjectile(proj, config, x, y, dirX, dirY);

    // Track entity
    activeEntityCounts[EntityType::PROJECTILE]++;

    // Return wrapped entity (no ECS entity ID yet)
    PooledEntity entity(0, EntityType::PROJECTILE, proj);  // Use 0 for ECS ID

    std::cout << "[PooledEntityFactory] Created projectile from " << prefabPath
        << " at (" << x << ", " << y << ")\n";

    return entity;
}

inline void PooledEntityFactory::ApplyPrefabToProjectile(
    ProjectileData* proj,
    const PrefabConfig& config,
    float x, float y,
    float dirX, float dirY)
{
    proj->posX = x;
    proj->posY = y;
    proj->velX = dirX * config.speed;
    proj->velY = dirY * config.speed;
    proj->lifetime = config.lifetime;
    proj->damage = config.damage;
    proj->active = true;
}

inline PooledEntity PooledEntityFactory::CreateAOE(
    const std::string& prefabPath,
    float x, float y)
{
    PrefabConfig config;
    if (!LoadPrefabConfig(prefabPath, config)) {
        return PooledEntity();  // Fixed
    }

    AOEData* aoe = GameMemoryManager::Get().AllocateAOE();
    if (aoe == nullptr) {
        std::cerr << "[PooledEntityFactory] AOE pool exhausted!\n";
        return PooledEntity();  // Fixed
    }

    ApplyPrefabToAOE(aoe, config, x, y);
    activeEntityCounts[EntityType::AOE]++;

    std::cout << "[PooledEntityFactory] Created AOE from " << prefabPath << "\n";
    return PooledEntity(0, EntityType::AOE, aoe);  // Fixed
}

inline void PooledEntityFactory::ApplyPrefabToAOE(
    AOEData* aoe,
    const PrefabConfig& config,
    float x, float y)
{
    aoe->posX = x;
    aoe->posY = y;
    aoe->radius = config.radius > 0 ? config.radius : 150.0f;
    aoe->duration = config.lifetime > 0 ? config.lifetime : 1.5f;
    aoe->damage = config.damage;
    aoe->active = true;
}

inline PooledEntity PooledEntityFactory::CreateShield(
    const std::string& prefabPath,
    float x, float y)
{
    PrefabConfig config;
    if (!LoadPrefabConfig(prefabPath, config)) {
        return PooledEntity();  // Fixed
    }

    ShieldData* shield = GameMemoryManager::Get().AllocateShield();
    if (shield == nullptr) {
        std::cerr << "[PooledEntityFactory] Shield pool exhausted!\n";
        return PooledEntity();  // Fixed
    }

    ApplyPrefabToShield(shield, config, x, y);
    activeEntityCounts[EntityType::SHIELD]++;

    std::cout << "[PooledEntityFactory] Created shield from " << prefabPath << "\n";
    return PooledEntity(0, EntityType::SHIELD, shield);  // Fixed
}

inline void PooledEntityFactory::ApplyPrefabToShield(
    ShieldData* shield,
    const PrefabConfig& config,
    float x, float y)
{
    shield->posX = x;
    shield->posY = y;
    // FIXED: Removed shield->health line (ShieldData doesn't have health)
    // If you want shield health, add it to ShieldData struct first
    shield->duration = config.lifetime > 0 ? config.lifetime : 5.0f;
    shield->active = true;
}

inline void PooledEntityFactory::DestroyEntity(PooledEntity& entity) {
    if (!entity.active || entity.poolData == nullptr) {
        return;
    }

    // Return to appropriate pool
    switch (entity.type) {
    case EntityType::PROJECTILE:
        GameMemoryManager::Get().FreeProjectile(static_cast<ProjectileData*>(entity.poolData));
        activeEntityCounts[EntityType::PROJECTILE]--;
        break;
    case EntityType::AOE:
        GameMemoryManager::Get().FreeAOE(static_cast<AOEData*>(entity.poolData));
        activeEntityCounts[EntityType::AOE]--;
        break;
    case EntityType::SHIELD:
        GameMemoryManager::Get().FreeShield(static_cast<ShieldData*>(entity.poolData));
        activeEntityCounts[EntityType::SHIELD]--;
        break;
    case EntityType::PARTICLE:
        GameMemoryManager::Get().FreeParticle(static_cast<ParticleData*>(entity.poolData));
        activeEntityCounts[EntityType::PARTICLE]--;
        break;
    case EntityType::ENEMY:
        GameMemoryManager::Get().FreeEnemy(static_cast<EnemyData*>(entity.poolData));
        activeEntityCounts[EntityType::ENEMY]--;
        break;
    default:
        break;
    }

    entity.active = false;
    entity.poolData = nullptr;
}

template<typename T>
inline T* PooledEntityFactory::GetPoolData(const PooledEntity& entity) {
    if (!entity.active || entity.poolData == nullptr) {
        return nullptr;
    }
    return static_cast<T*>(entity.poolData);
}

inline void PooledEntityFactory::CachePrefab(const std::string& prefabPath) {
    PrefabConfig config;
    LoadPrefabConfig(prefabPath, config);
}

inline void PooledEntityFactory::ClearPrefabCache() {
    prefabCache.clear();
    std::cout << "[PooledEntityFactory] Prefab cache cleared\n";
}

inline size_t PooledEntityFactory::GetActiveEntityCount(EntityType type) const {
    auto it = activeEntityCounts.find(type);
    return (it != activeEntityCounts.end()) ? it->second : 0;
}

inline void PooledEntityFactory::PrintStats() const {
    std::cout << "\n=== PooledEntityFactory Stats ===\n";
    std::cout << "Active Projectiles: " << GetActiveEntityCount(EntityType::PROJECTILE) << "\n";
    std::cout << "Active AOEs:        " << GetActiveEntityCount(EntityType::AOE) << "\n";
    std::cout << "Active Shields:     " << GetActiveEntityCount(EntityType::SHIELD) << "\n";
    std::cout << "Active Particles:   " << GetActiveEntityCount(EntityType::PARTICLE) << "\n";
    std::cout << "Active Enemies:     " << GetActiveEntityCount(EntityType::ENEMY) << "\n";
    std::cout << "Cached Prefabs:     " << prefabCache.size() << "\n";
    std::cout << "==================================\n\n";
}

#endif // POOLED_ENTITY_FACTORY_H