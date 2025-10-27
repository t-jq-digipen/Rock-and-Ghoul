/* ========================================================
 * @file        DataDefs.h
 * @author      Yasmin Tazi         | yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa   | t.yingclarissa@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @date        26 OCT 2025
 *
 * @brief   Data structures for game serialization
 *
 * This header file defines data structures for saving and
 * loading game data through the JSON serialization system.
 * These structures bridge the runtime ECS and persistent
 * storage formats.
 *
 * Declares Editor data to extract editable values from components to
 * be utilized in the editor.
 * For each component, developer MUST provide an interface function to
 * acquire this data in order to use the editor.
 * 
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <variant>
#include "Entity.h"

// First string would be the data type name, second string would hold the value
using SerializedComponent = std::unordered_map<std::string, std::string>;

 /**
  * @struct SavedEntity
  * @brief Serializable entity structure with component data.
  */
struct SavedEntity {                                              
    std::string name;                   
    // First string is type name, second string holds data
    std::unordered_map<std::string, std::string> componentData; 

    /**
     * @brief Default constructor with zero ID and empty name.
     */
    SavedEntity() : name("") {}
};

/**
 * @struct SavedPhysics
 * @brief Legacy physics serialization structure.
 */
struct SavedPhysics {
    float velX, velY;                
    float decay;                  
    bool willCollide;            

    // AABB data (center + dim for Create_AABB)
    float aabbCenterX, aabbCenterY; 
    float aabbDimX, aabbDimY;   

    // Circle data
    float circlePosX, circlePosY;  
    float circleRad;     
};

/**
 * @struct SavedScene
 * @brief Serializable scene structure with all entities.
 */
struct SavedScene {
    std::string sceneName;  
    std::vector<SavedEntity> entities; 

    /**
     * @brief Default constructor with no name.
     */
    SavedScene() : sceneName("") {}
};

/**
 * @struct GameSettings
 * @brief Persistent game configuration settings.
 */
struct GameSettings {
    float volume;      
    int resolutionWidth;     // Display width in pixels
    int resolutionHeight;    // Display height in pixels
    bool fullscreen;       

    /**
     * @brief Default constructor with 1920x1080 fullscreen off.
     */
    GameSettings() : volume(1.0f), resolutionWidth(1920), resolutionHeight(1080), fullscreen(false) {}
};


namespace Editor {
    // Determines the type of editing widget
    enum WidgetType {
        WIDGET_CHECKMARK,
        WIDGET_DROPDOWN,
        WIDGET_INT,
        WIDGET_FLOAT,
        WIDGET_STRING
    };

    // Widget data variants based on widget type
#pragma region WidgetData
    struct Widget_Checkmark {
        bool* ref = nullptr;
        bool check;
    };

    struct Widget_String {
        std::string* ref = nullptr;
        std::string string;
    };

    struct Widget_Dropdown {
        unsigned int* ref = nullptr;
        std::vector<std::string> options;
        std::unordered_map<std::string, unsigned int> enumMap;

        // NEED TO MAKE DEBUG CASES IF FAIL
        void Add_Option(std::string const& name, unsigned int val) {
            options.push_back(name);
            enumMap[name] = val;
        }
    };

    struct Widget_Int {
        int* ref = nullptr;
        bool slider = false;
        int min = 0;
        int max = 0;

        Widget_Int(int* val) : ref{ val } {}
    };

    struct Widget_Float {
        float* ref = nullptr;
        bool slider = false;
        float min = 0.f;
        float max = 0.f;
    };

    using WidgetData = std::variant<Widget_Checkmark, Widget_String, Widget_Dropdown, Widget_Int, Widget_Float>;

#pragma endregion WidgetData




    // Keyed with VALUE NAME
    using DataName = std::string;

    // Keyed with VALUE NAME
    // Data holds 
    class ComponentData {
    private:
        std::string ComponentName;
        std::map<DataName, WidgetData> ComponentInfo;
    public:
        ComponentData(const std::string& name) : ComponentName{ name }, ComponentInfo{} {}

        void Add_Data(const DataName& name, const WidgetData& dataf) {
            ComponentInfo[name] = dataf;
        }

        void Clear_Data() {
            for (auto& [name, data] : ComponentInfo) {
                std::visit([](auto& widget) {
                    // For any widget type that has a `ref` member, set it to nullptr
                    if constexpr (requires { widget.ref; }) {
                        widget.ref = nullptr;
                    }
                    }, data);
            }
        }

        std::map<DataName, WidgetData>& GetDataMap() { return ComponentInfo; }

        const std::string& GetComponentName() const { return ComponentName; }

    };

}