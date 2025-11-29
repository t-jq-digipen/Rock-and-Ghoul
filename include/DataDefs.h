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
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <variant>
#include <iostream>
#include "Entity.h"
#include "TextSystem.h"

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
 * @enum SceneType
 * @brief Determines if a scene is a menu or game type.
 */
enum SceneType {
    ST_MENU,
    ST_GAME
};

/**
 * @struct SavedScene
 * @brief Serializable scene structure with all entities.
 */
struct SavedScene {
    std::string sceneName;  
    std::vector<SavedEntity> entities; 
    std::vector<std::string> backgroundTextures;
    std::string fontConfigPath;
    std::string bgmAssetName;  // name of BGM asset from AudioCache i.e. "main_menu", "level_1"

    struct SavedButton {
        int ID = 0;
        std::string label;
        Vec2 pos;
        Vec2 size;
        Colour3 color;
        std::string shape;
        std::string renderData;
        std::string action;
        std::string actionName{"{}"};
        json actionParams{"{}"};
        int soundType = 0; // default to PLAY(0)
    };
    std::vector<SavedButton> guiButtons;

    struct SavedTextBox {
        int ID = 0;
        std::string text;
        Vec2 pos;
        float scale;
        Colour3 color;
        bool centered;
    };
    std::vector<SavedTextBox> guiTextboxes;

    struct SavedDialogue {
        int ID = 0;
        SavedTextBox textBox;
        std::string jsonPath;
        std::string sceneID = "0";
        std::vector<std::string> lines;
        int currentLineIndex = 0;
    };
    std::vector<SavedDialogue> guiDialogues;

    struct SavedCamera {
        Vec2 pos;
        float zoom;
        Vec2 worldMin;
        Vec2 worldMax;

        SavedCamera();
    };

    SavedCamera camSettings;

    SceneType sceneType;

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

enum RenderType {
    RT_SHAPE,
    RT_IMAGE,
    RT_SPRITESHEET
};

enum RenderLayer {
    LAYER_BACKGROUND,
    LAYER_GAMEOBJECT,
    LAYER_FOREGROUND,
    LAYER_UI
};

// Type erased render data type
class IRenderData {
public:

    virtual ~IRenderData() = default;

    virtual void Update(float) = 0;

    virtual std::string Serialize() const = 0;

    virtual RenderType GetType() const = 0;

    virtual std::unique_ptr<IRenderData> Clone() const = 0;
};

/**
* @struct ActiveFrames
* @brief Represents a set of frames for a given animation and a duration it should take to play it.
*/
struct ActiveFrames {
    int row = 0;	// Row index
    int col_start = 0;	// First column index
    int col_end = 0;	// Last column index
    float duration = 0.f;	// Duration of time in seconds the animation takes to go from first column to last column

    /**
        * @brief Equality operator for ActiveFrames.
        * @param rhs The ActiveFrames object to compare with.
        * @return True if all fields (row, col_start, col_end, duration) match, false otherwise.
        */
    bool operator==(const ActiveFrames& rhs) const;
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
        bool check = false;
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
            if (enumMap.find(name) != enumMap.end())
            { 
                std::cout << "Dropdown Widget option " << name << "is already registered, please do not duplicate names in editor widgets\n"; 
                return;
            }
            options.push_back(name);
            enumMap[name] = val;
        }

        void Rename_Option(std::string const& name, unsigned int val) {
            bool exists = false;
            std::string toremove = "";
            for (auto& [key,srcval] : enumMap) {
                if (key == name) // Key already exists in map!
                {
                    return;
                }
                if (srcval == val)
                {
                    toremove = key;
                    exists = true;
                }
            }
            if (!exists) { return; }

            enumMap.erase(toremove);
            enumMap[name] = val;

        }
    };

    struct Widget_Int {
        int* ref = nullptr;
        bool slider = false;
        int min = 0;
        int max = 0;

    };

    struct Widget_Float {
        float* ref = nullptr;
        bool slider = false;
        float min = 0.f;
        float max = 0.f;

    };

    /*struct Widget_Group;

    struct Widget_List {
        std::vector<std::string> options;
        std::unordered_map<std::string, Widget_Group> widgetMap;
    };*/


    struct Widget_Animator {
        std::unordered_map<unsigned int, ActiveFrames>* ref = nullptr;

        Widget_Int total_rows;
        Widget_Int total_cols;

        Widget_Dropdown current_state;


        Widget_Animator(std::unordered_map<unsigned int, ActiveFrames>* src, int* totrow, int* totcol, 
                                unsigned int* currentstate) :
            ref{src}, total_rows{totrow}, total_cols{totcol}, 
            current_state{currentstate}
        {
            for (auto& [state, frame] : *src)
            {
                current_state.Add_Option(std::to_string(state), state);
            }
        }

        // returns outcome in message format
        std::string Add_New_AnimState(const std::string& name) {
            if (!ref) { return "Component data problem! reference has no data"; }   // can only add if ref is valid!

            unsigned int key{ 0 };
            if (current_state.enumMap.find(name) != current_state.enumMap.end())
            {
                return "State " + name + " already exists!";
            }
            // find available key position
            while (!(ref->try_emplace(++key, ActiveFrames{}).second));

            current_state.Add_Option(name, key);

            return "State " + name + "successfully added with key " + std::to_string(key) + "!";
        }

        // returns outcome in message format
        std::string Remove_AnimState(const std::string& name) {
            if (!ref) { return "Component data problem! reference has no data"; }   // can only remove if ref is valid!

            auto it = current_state.enumMap.find(name);
            if (it == current_state.enumMap.end()) { return "State does not exist!"; }

            unsigned int keyval = it->second;


            // current state is the state to be removed
            if (*current_state.ref == keyval) { return "Cannot remove currently active state!"; }

            // if if fail to remove, return false
            if (!ref->erase(keyval)) { return "Could not remove this state :o"; }

        }

    };

    using WidgetData = std::variant<Widget_Checkmark, Widget_String, Widget_Dropdown, Widget_Int, Widget_Float, Widget_Animator>;

    /*struct Widget_Group {
        std::string label;
        std::vector<WidgetData> childWidget;
    };*/

#pragma endregion WidgetData




    // Keyed with VALUE NAME
    using DataName = std::string;

    // Keyed with VALUE NAME
    // Data holds 
    class ComponentData {
    private:
        std::string ComponentName;
        std::vector<std::pair<DataName, WidgetData>> ComponentInfo;
        //std::map<DataName, WidgetData> ComponentInfo;
    public:
        ComponentData(const std::string& name) : ComponentName{ name }, ComponentInfo{} {}

        void Add_Data(const DataName& name, const WidgetData& dataf) {
            //ComponentInfo[name] = dataf;
            ComponentInfo.emplace_back(name,dataf);
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

        // std::map<DataName, WidgetData>& GetDataMap() { return ComponentInfo; }
        std::vector<std::pair<DataName, WidgetData>>& GetDataMap() { return ComponentInfo; }

        const std::string& GetComponentName() const { return ComponentName; }

    };

}