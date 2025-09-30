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
const std::string FilePath::LEVEL1_SCENE = FilePath::SAVES_FOLDER + "level1.json";
const std::string FilePath::LEVEL2_SCENE = FilePath::SAVES_FOLDER + "level2.json";