/* ========================================================
 * @file    FilePath.cpp
 * @author  yasmin.t@digipen.edu
 * @author  t.yingclarissa@digipen.edu
 * @date    26/09/2025
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

const std::string FilePath::SAVES_FOLDER = getExeDir() + "/saves/";
const std::string FilePath::SETTINGS_FILE = FilePath::SAVES_FOLDER + "settings.json";
const std::string FilePath::LEVEL1_SCENE = FilePath::SAVES_FOLDER + "level1.json";
const std::string FilePath::LEVEL2_SCENE = FilePath::SAVES_FOLDER + "level2.json";