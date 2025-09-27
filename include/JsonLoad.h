#pragma once
#include "DataDefs.h"
#include <json.hpp>
#include <optional>
#include <vector>

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

private:
    // conversion helpers
    static void to_json(json& j, const SavedEntity& e);
    static void from_json(const json& j, SavedEntity& e);

    static void to_json(json& j, const SavedMap& m);
    static void from_json(const json& j, SavedMap& m);

    static void to_json(json& j, const GameSettings& s);
    static void from_json(const json& j, GameSettings& s);
};