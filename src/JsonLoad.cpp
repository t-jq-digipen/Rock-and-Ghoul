#include "JsonLoad.h"
#include "FileIO.h"
#include <iostream>

/*
** PART 1: CONVERSION HELPERS
** A: to_json -- convert structs to json format so they can be saved to disk
** B: from_json -- convert json back to structs so we can use the data
** (there are 3 types: SavedEntity, SavedMap, GameSettings)
*/

// 1A. ENTITY TO JSON
void JsonLoad::to_json(json& j, const SavedEntity& e) {
    // take each field from the struct and put it in json format
    j = json{
        {"id", e.id},
        {"name", e.name},
        {"type", e.type},
        {"x", e.x},
        {"y", e.y},
        {"health", e.health}
    };
}

// 1B. JSON TO ENTITY
void JsonLoad::from_json(const json& j, SavedEntity& e) {
    // grab each field from json and put it back in the struct
    j.at("id").get_to(e.id);
    j.at("name").get_to(e.name);
    j.at("type").get_to(e.type);
    j.at("x").get_to(e.x);
    j.at("y").get_to(e.y);
    j.at("health").get_to(e.health);
}

// 1A. MAP TO JSON
void JsonLoad::to_json(json& j, const SavedMap& m) {
    // convert basic map info to json
    j = json{
        {"name", m.name},
        {"width", m.width},
        {"height", m.height},
        {"tiles", m.tiles},
        {"entities", json::array()}  // start with empty entities array
    };

    // loop through all entities in the map and add them to json array
    for (const auto& entity : m.entities) {
        json entityJson;
        to_json(entityJson, entity);  // convert each entity to json
        j["entities"].push_back(entityJson);  // add to array
    }
}

// 1B. JSON TO MAP
void JsonLoad::from_json(const json& j, SavedMap& m) {
    // get basic map info from json
    j.at("name").get_to(m.name);
    j.at("width").get_to(m.width);
    j.at("height").get_to(m.height);
    j.at("tiles").get_to(m.tiles);

    // clear and rebuild entities list from json array
    m.entities.clear();
    for (const auto& entityJson : j.at("entities")) {
        SavedEntity entity;
        from_json(entityJson, entity); // convert each json to entity
        m.entities.push_back(entity);  // add to list
    }
}

// 1A. SETTINGS TO JSON
void JsonLoad::to_json(json& j, const GameSettings& s) {
    // put settings data in json format
    j = json{
        {"volume", s.volume},
        {"resolutionWidth", s.resolutionWidth},
        {"resolutionHeight", s.resolutionHeight},
        {"fullscreen", s.fullscreen}
    };
}

// 1B. JSON TO SETTINGS
void JsonLoad::from_json(const json& j, GameSettings& s) {
    // get settings from json back into struct
    j.at("volume").get_to(s.volume);
    j.at("resolutionWidth").get_to(s.resolutionWidth);
    j.at("resolutionHeight").get_to(s.resolutionHeight);
    j.at("fullscreen").get_to(s.fullscreen);
}

/*
** PART 2: LOAD/SAVE FUNCTIONS
** Load functions: read file, get the data needed from json aka parse json, convert to struct, RETURN STRUCT
** Save functions: convert struct to json, write to file on disk, return success/fail (fail if like no space lol)
*/

/*
** LOADING EXPLAINED
** it's like there may or may not be entities/settings wtv to load
** so thats why use optional and if there arent any to even load return nullopt
** and at the same time if stuff does load then the stuff in try block executes
** but if we cant do the things in try (corrupted/bad) then catch happens huzzah
*/

// LOAD ***ONE*** ENTITY FROM FILE
std::optional<SavedEntity> JsonLoad::LoadEntity(const std::string& filepath) {
    std::string content = FileIO::ReadFile(filepath); // read file as STRING
    if (content.empty()) return std::nullopt;

    try {
        json j = json::parse(content); // turn string into json object
        SavedEntity entity;
        from_json(j, entity); // convert json to struct
        return entity;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading entity: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// LOAD ***MULTIPLE*** ENTITIES FROM FILE
std::optional<std::vector<SavedEntity>> JsonLoad::LoadEntities(const std::string& filepath) {
    std::string content = FileIO::ReadFile(filepath);
    if (content.empty()) return std::nullopt;

    try {
        json j = json::parse(content); // parse the json array
        std::vector<SavedEntity> entities;

        // loop through json array and convert each to entity back to struct
        for (const auto& entityJson : j) {
            SavedEntity entity;
            from_json(entityJson, entity);
            entities.push_back(entity);
        }

        return entities;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading entities: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// SAVE ONE ENTITY TO FILE
bool JsonLoad::SaveEntity(const std::string& filepath, const SavedEntity& entity) {
    try {
        json j;
        to_json(j, entity);  // convert struct to json
        return FileIO::WriteFile(filepath, j.dump(4));  // write to disk (4 = pretty format)
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving entity: " << e.what() << std::endl;
        return false;
    }
}

// SAVE MULTIPLE ENTITIES TO FILE
bool JsonLoad::SaveEntities(const std::string& filepath, const std::vector<SavedEntity>& entities) {
    try {
        json j = json::array();  // create json array

        // loop through entities and add each to json array
        for (const auto& entity : entities) {
            json entityJson;
            to_json(entityJson, entity);  // convert to json
            j.push_back(entityJson);  // add to array
        }

        return FileIO::WriteFile(filepath, j.dump(4));  // write to disk
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving entities: " << e.what() << std::endl;
        return false;
    }
}

// LOAD MAP FROM FILE
std::optional<SavedMap> JsonLoad::LoadMap(const std::string& filepath) {
    std::string content = FileIO::ReadFile(filepath);  // read file
    if (content.empty()) return std::nullopt;

    try {
        json j = json::parse(content);  // parse json
        SavedMap map;
        from_json(j, map);  // convert to map struct
        return map;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading map: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// SAVE MAP TO FILE
bool JsonLoad::SaveMap(const std::string& filepath, const SavedMap& map) {
    try {
        json j;
        to_json(j, map);  // convert map to json
        return FileIO::WriteFile(filepath, j.dump(4));  // write to disk
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving map: " << e.what() << std::endl;
        return false;
    }
}

// LOAD GAME SETTINGS FROM FILE (volume, resolution, etc)
std::optional<GameSettings> JsonLoad::LoadSettings(const std::string& filepath) {
    std::string content = FileIO::ReadFile(filepath);  // read file
    if (content.empty()) return std::nullopt;

    try {
        json j = json::parse(content);  // parse json
        GameSettings settings;
        from_json(j, settings);  // convert to settings struct
        return settings;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading settings: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// SAVE GAME SETTINGS TO FILE (volume, resolution, etc)
bool JsonLoad::SaveSettings(const std::string& filepath, const GameSettings& settings) {
    try {
        json j;
        to_json(j, settings);  // convert settings to json
        return FileIO::WriteFile(filepath, j.dump(4));  // write to disk
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving settings: " << e.what() << std::endl;
        return false;
    }
}