#pragma once
#include "DataDefs.h"
#include <nlohmann/json.hpp>
#include <optional>
#include "Entity.h"

using json = nlohmann::json;

class JsonLoad {
public:
    // ===== MAIN FUNCTIONS =====
    // Save all current entities to a scene file
    static bool SaveCurrentScene(const std::string& filepath, const std::string& sceneName);
    
    // Clear all entities before loading (no cloning)
    static void ClearAllEntities();

    // Load scene and create all entities
    static bool LoadScene(const std::string& filepath);

    // Save game settings
    static bool SaveSettings(const std::string& filepath, const GameSettings& settings);
    static std::optional<GameSettings> LoadSettings(const std::string& filepath);

private:
    // Convert ECS entity to SavedEntity
    static SavedEntity EntityToSaved(Entity eID);

    // Create ECS entity from SavedEntity
    static Entity SavedToEntity(const SavedEntity& saved);

    // Component loaders
    static void Load_Transform(Entity eID, const std::string& data);
    static void Load_Sprite(Entity eID, const std::string& data);
    static void Load_Physics(Entity eID, const std::string& data);
    static void Load_Shape(Entity eID, const std::string& data);
    static void Load_Health(Entity eID, const std::string& data);
    static void Load_Tag(Entity eID, const std::string& data);
    static void Load_Input(Entity eID, const std::string& data);
    static void Load_Animator(Entity eID, const std::string& data);

    // Component savers
    static std::string Save_Transform(Entity eID);
    static std::string Save_Sprite(Entity eID);
    static std::string Save_Physics(Entity eID);
    static std::string Save_Shape(Entity eID);
    static std::string Save_Health(Entity eID);
    static std::string Save_Tag(Entity eID);
    static std::string Save_Input(Entity eID);
    static std::string Save_Animator(Entity eID);
    // JSON serialization
    static void to_json(json& j, const SavedEntity& e);
    static void from_json(const json& j, SavedEntity& e);
    static void to_json(json& j, const SavedScene& s);
    static void from_json(const json& j, SavedScene& s);
    static void to_json(json& j, const GameSettings& s);
    static void from_json(const json& j, GameSettings& s);
};