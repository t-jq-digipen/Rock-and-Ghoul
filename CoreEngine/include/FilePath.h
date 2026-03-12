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

    // Editor-only: sets REPO_ROOT without loading any game.
    // Lets the editor prompt the user to choose a game on startup.
    static void InitRepoRoot();

    // Returns true if a game folder has been selected (SetGameRoot was called).
    static bool IsGameLoaded() { return !GAME_ROOT.empty(); }

    // Converts any asset path from a JSON file to a valid path on this machine.
    // Handles three cases:
    //   1. Old relative prefix  "../Assets/..."      (legacy format)
    //   2. Absolute path from a different machine    (e.g. "E:/School/.../Assets/foo.png")
    //   3. Already-correct absolute path             (pass-through)
    static std::string Resolve(const std::string& raw) {
        if (raw.empty()) return raw;

        // Case 1: legacy "../Assets/" prefix
        const std::string oldPrefix = "../Assets/";
        if (raw.size() >= oldPrefix.size() &&
            raw.compare(0, oldPrefix.size(), oldPrefix) == 0)
        {
            return ASSETS_FOLDER + raw.substr(oldPrefix.size());
        }

        // Case 2: absolute path from another machine — rebase to current ASSETS_FOLDER.
        // Detect by looking for "/Assets/" or "\Assets\" inside the string.
        if (!ASSETS_FOLDER.empty()) {
            // Already on this machine — pass through
            if (raw.compare(0, ASSETS_FOLDER.size(), ASSETS_FOLDER) == 0)
                return raw;

            // Find the "/Assets/" boundary (handle both slash styles)
            for (const char* marker : { "/Assets/", "\\Assets\\" , "/Assets\\", "\\Assets/" }) {
                auto pos = raw.find(marker);
                if (pos != std::string::npos) {
                    // Take everything after "/Assets/"
                    std::string rel = raw.substr(pos + 8); // len("/Assets/") == 8
                    // Normalise backslashes to forward slashes
                    for (char& c : rel) if (c == '\\') c = '/';
                    return ASSETS_FOLDER + rel;
                }
            }
        }

        return raw;
    }

    static std::string REPO_ROOT;      // absolute path to the repo root (set by SetGameRoot)
    static std::string ASSETS_FOLDER;
    static std::string SAVES_FOLDER;
    static std::string PREFABS_FOLDER;
    static std::string SETTINGS_FILE;
    static std::string COLLISIONSETTINGS_FILE;
    static std::string CONTROLS_FILE;
    static std::string AUDIO_MANIFEST;
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
