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
  * of important files and directories used.
  */
class FilePath {
public:
    /**
     * @brief The absolute path to the saves directory.
     *
     * This constant holds the path to the folder where save files are stored.
     */
    static const std::string SAVES_FOLDER;

    /**
     * @brief The absolute path to the settings configuration file.
     *
     * This constant holds the full path to the settings.json file.
     */
    static const std::string SETTINGS_FILE;
    static const std::string COLLISIONSETTINGS_FILE;
    static const std::string CONTROLS_FILE;

    /**
     * @brief The absolute path to the Level 1 scene data file.
     *
     * This constant holds the full path to the level1.json file.
     */
    static const std::string LEVEL1_SCENE;

    /**
     * @brief The absolute path to the Level 2 scene data file.
     *
     * This constant holds the full path to the level2.json file.
     */
    static const std::string LEVEL2_SCENE;
    static const std::string LEVEL1_TEMP_SCENE;
    static const std::string LEVEL2_TEMP_SCENE;
    static const std::string ASSETS_FOLDER;
    static const std::string PREFABS_FOLDER;
    static const std::string SCENES_LIST;
    static const std::string DIALOGUE;

};