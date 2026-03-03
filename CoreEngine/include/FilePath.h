/* ========================================================
 * @file        FilePath.h
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date        04 NOV 2025
 *
 * @brief   Header file declaring the FilePath utility class
 *
 * This header file contains the declaration of the FilePath class.
 * These paths include the saves folder location, settings file,
 * and level scene files.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>

 /**
  * @class FilePath
  * @brief Utility class providing static file path constants.
  *
  * The FilePath class contains static string constants that define the locations
  * of important files and directories used by the active game.
  *
  * Call FilePath::SetGameRoot("YourGameFolder") once at the very start of
  * main() before anything else accesses these paths.
  * Example:
  *     FilePath::SetGameRoot("RockAndGhoul");   // in RockAndGhoul/main.cpp
  *     FilePath::SetGameRoot("MySecondGame");   // in MySecondGame/main.cpp
  */
class FilePath {
public:

    // Call this FIRST in main() before anything else
    // Pass the name of the game folder (e.g. "RockAndGhoul", "MySecondGame").
    static void SetGameRoot(const std::string& gameFolderName);

    // Converts "../Assets/rocknghoul" paths (in old JSON files) to the
    // correct path using ASSETS_FOLDER. Pass-through for any other path.
    // Use this wherever a path arrives from a JSON file.
    static std::string Resolve(const std::string& path) {
        const std::string oldPrefix = "../Assets/";
        if (path.size() >= oldPrefix.size() &&
            path.compare(0, oldPrefix.size(), oldPrefix) == 0)
        {
            return ASSETS_FOLDER + path.substr(oldPrefix.size());
        }
        return path;
    }

    static std::string REPO_ROOT;      // absolute path to the repo root (set by SetGameRoot)
    static std::string ASSETS_FOLDER;
    static std::string SAVES_FOLDER;
    static std::string PREFABS_FOLDER;
    static std::string SETTINGS_FILE;
    static std::string COLLISIONSETTINGS_FILE;
    static std::string CONTROLS_FILE;
    static std::string MAIN_MENU_SCENE;
    static std::string LEVEL1_SCENE;
    static std::string LEVEL2_SCENE;
    static std::string LEVEL1_TEMP_SCENE;
    static std::string LEVEL2_TEMP_SCENE;
    static std::string SCENES_LIST;
    static std::string DIALOGUE;

private:
    static std::string GAME_ROOT; // set by SetGameRoot, used to build all paths above
};
