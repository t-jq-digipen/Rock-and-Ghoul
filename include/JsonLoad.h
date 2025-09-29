#pragma once
#include "DataDefs.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

// files to include in json
#include "Component.h" 
#include "Registry.h"    // For Has_Component, Get_Component, etc.
#include "Entity.h"
#include "Physics.h"

using json = nlohmann::json;

class JsonLoad {
public:
    // entity functions
    static std::optional<SavedEntity> LoadEntity(const std::string& filepath);
    static std::optional<std::vector<SavedEntity>> LoadEntities(const std::string& filepath);
    static bool SaveEntity(const std::string& filepath, const SavedEntity& entity);
    static bool SaveEntities(const std::string& filepath, const std::vector<SavedEntity>& entities);

    // map functions
    static std::optional<SavedMap> LoadMap(const std::string& filepath);
    static bool SaveMap(const std::string& filepath, const SavedMap& map);

    // settings functions
    static std::optional<GameSettings> LoadSettings(const std::string& filepath);
    static bool SaveSettings(const std::string& filepath, const GameSettings& settings);

    // create entity from saved data
    static Entity CreateEntityFromSave(const SavedEntity& savedEntity);

private:
    // conversion helpers
    static void to_json(json& j, const SavedEntity& e);
    static void from_json(const json& j, SavedEntity& e);

    static void to_json(json& j, const SavedMap& m);
    static void from_json(const json& j, SavedMap& m);

    static void to_json(json& j, const GameSettings& s);
    static void from_json(const json& j, GameSettings& s);

    static std::string SerializeComponent(Entity eID, ComponentTypeID type);
    static void DeserializeComponent(Entity eID, ComponentTypeID type, const std::string& data);

    // component-specific serialization
    //static std::string Serialize_Transform(const TransformComponent& comp);
    //static std::string Serialize_Sprite(const SpriteComponent& comp);
    //static std::string Serialize_Physics(const PhysicsComponent& comp);
    //static std::string Serialize_Health(const HealthComponent& comp);
    //static std::string Serialize_Tag(const TagComponent& comp);
    //static std::string Serialize_Input(const InputComponent& comp);
    //static std::string Serialize_BossGhoul(const BossGhoulComponent& comp);
    //
    //// component-specific deserialization
    //static TransformComponent Deserialize_Transform(const std::string& data);
    //static SpriteComponent Deserialize_Sprite(const std::string& data);
    //static PhysicsComponent Deserialize_Physics(const std::string& data);
    //static HealthComponent Deserialize_Health(const std::string& data);
    //static TagComponent Deserialize_Tag(const std::string& data);
    //static InputComponent Deserialize_Input(const std::string& data);
    //static BossGhoulComponent Deserialize_BossGhoul(const std::string& data);
};