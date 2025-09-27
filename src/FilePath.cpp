#include <windows.h>
#include "FilePath.h"
#include <filesystem>

std::string getExeDir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path().string();
}

const std::string FilePath::SAVES_FOLDER = getExeDir() + "/saves/";
const std::string FilePath::SETTINGS_FILE = FilePath::SAVES_FOLDER + "settings.json";
const std::string FilePath::ENTITIES_FILE = FilePath::SAVES_FOLDER + "entities.json";
const std::string FilePath::MAP_FILE = FilePath::SAVES_FOLDER + "map.json";
