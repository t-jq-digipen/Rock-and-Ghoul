/* ========================================================
 * @file        FilePath.cpp
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date        04 NOV 2025
 *
 * @brief   Implementation file for FilePath class
 *
 * This file contains the implementation of the FilePath class
 * which provides utilities for managing directories. It dynamically
 * determines the executable directory and constructs paths for
 * save files, settings, and level scenes.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include <windows.h>
#include "FilePath.h"
#include <filesystem>

 /**
  * @brief Retrieves the directory path of current executable.
  *
  * This function uses the Windows API to obtain the full path of the
  * running executable and extracts its parent directory.
  *
  * @return std::string The absolute path to the directory containing
  *         the executable, without a trailing slash.
  */
std::string getExeDir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path().string();
}

#ifdef NO_IMGUI

//this one to work for installer, use the commented out for debug/release
const std::string FilePath::SAVES_FOLDER = getExeDir() + "/../Assets/saves/";
const std::string FilePath::SETTINGS_FILE = getExeDir() + "/../Assets/others/settings.json";
const std::string FilePath::COLLISIONSETTINGS_FILE = getExeDir() + "/../Assets/others/CollisionSettings.json";

const std::string FilePath::LEVEL1_SCENE = FilePath::SAVES_FOLDER + "level1.json";
const std::string FilePath::LEVEL2_SCENE = FilePath::SAVES_FOLDER + "level2.json";

const std::string FilePath::LEVEL1_TEMP_SCENE = FilePath::SAVES_FOLDER + "level1_temp.json";
const std::string FilePath::LEVEL2_TEMP_SCENE = FilePath::SAVES_FOLDER + "level2_temp.json";

const std::string FilePath::SCENES_LIST = getExeDir() + "/../Assets/others/ScenesList.json";

// assets
const std::string FilePath::ASSETS_FOLDER = getExeDir() + "/../Assets/";
const std::string FilePath::PREFABS_FOLDER = FilePath::ASSETS_FOLDER + "prefabs/";

#else

const std::string FilePath::SAVES_FOLDER = getExeDir() + "/../../Assets/saves/";
const std::string FilePath::SETTINGS_FILE = getExeDir() + "/../../Assets/others/settings.json";
const std::string FilePath::COLLISIONSETTINGS_FILE = getExeDir() + "/../../Assets/others/CollisionSettings.json";
const std::string FilePath::LEVEL1_SCENE = FilePath::SAVES_FOLDER + "level1.json";
const std::string FilePath::LEVEL2_SCENE = FilePath::SAVES_FOLDER + "level2.json";
const std::string FilePath::LEVEL1_TEMP_SCENE = FilePath::SAVES_FOLDER + "level1_temp.json";
const std::string FilePath::LEVEL2_TEMP_SCENE = FilePath::SAVES_FOLDER + "level2_temp.json";
const std::string FilePath::SCENES_LIST = getExeDir() + "/../../Assets/others/ScenesList.json";




// save prefabs into assets folder
const std::string FilePath::ASSETS_FOLDER = getExeDir() + "/../../Assets/";
const std::string FilePath::PREFABS_FOLDER = FilePath::ASSETS_FOLDER + "prefabs/";

#endif