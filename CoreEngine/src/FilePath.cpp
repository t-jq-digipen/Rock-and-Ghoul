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
 * Call FilePath::SetGameRoot("YourGameFolder") once at the very
 * start of main() to initialise all paths for the active game.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include <windows.h>
#include "FilePath.h"
#include <filesystem>

// ── Static member definitions (start empty, filled by SetGameRoot) ──────────
std::string FilePath::GAME_ROOT;
std::string FilePath::REPO_ROOT;
std::string FilePath::ASSETS_FOLDER;
std::string FilePath::SAVES_FOLDER;
std::string FilePath::PREFABS_FOLDER;
std::string FilePath::SETTINGS_FILE;
std::string FilePath::COLLISIONSETTINGS_FILE;
std::string FilePath::CONTROLS_FILE;
std::string FilePath::MAIN_MENU_SCENE;
std::string FilePath::LEVEL1_SCENE;
std::string FilePath::LEVEL2_SCENE;
std::string FilePath::LEVEL1_TEMP_SCENE;
std::string FilePath::LEVEL2_TEMP_SCENE;
std::string FilePath::SCENES_LIST;
std::string FilePath::DIALOGUE;

/**
 * @brief Retrieves the directory path of the current executable.
 */
static std::string getExeDir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path().string();
}

/**
 * @brief Sets the active game root and rebuilds all file paths.
 *
 * Call this once at the very start of main() before anything else
 * accesses FilePath members.
 *
 * @param gameFolderName  The game's top-level folder name,
 * 
 */
void FilePath::SetGameRoot(const std::string& gameFolderName)
{
    namespace fs = std::filesystem;
#ifdef NO_IMGUI
    // Installer / shipping build: exe sits one level below repo root
    REPO_ROOT  = fs::weakly_canonical(fs::path(getExeDir()) / "..").generic_string() + "/";
    GAME_ROOT  = REPO_ROOT + gameFolderName + "/";
#else
    // Debug / Release build: exe sits two levels below repo root
    REPO_ROOT  = fs::weakly_canonical(fs::path(getExeDir()) / ".." / "..").generic_string() + "/";
    GAME_ROOT  = REPO_ROOT + gameFolderName + "/";
#endif

    ASSETS_FOLDER          = GAME_ROOT + "Assets/";
    SAVES_FOLDER           = ASSETS_FOLDER + "saves/";
    PREFABS_FOLDER         = ASSETS_FOLDER + "prefabs/";
    SETTINGS_FILE          = ASSETS_FOLDER + "saves/settings.json";
    COLLISIONSETTINGS_FILE = ASSETS_FOLDER + "others/CollisionSettings.json";
    CONTROLS_FILE          = ASSETS_FOLDER + "saves/controls.json";
    MAIN_MENU_SCENE        = SAVES_FOLDER  + "MainMenu.json";
    LEVEL1_SCENE           = SAVES_FOLDER  + "level1.json";
    LEVEL2_SCENE           = SAVES_FOLDER  + "level2.json";
    LEVEL1_TEMP_SCENE      = SAVES_FOLDER  + "level1_temp.json";
    LEVEL2_TEMP_SCENE      = SAVES_FOLDER  + "level2_temp.json";
    SCENES_LIST            = ASSETS_FOLDER + "others/ScenesList.json";
    DIALOGUE               = ASSETS_FOLDER + "Dialogue.json";
}
