/* ========================================================
 * @file        ImguiSystem.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan       | t.junquan@digipen.edu
 * @co-author   Manfred Sim        | manfred.sim@digipen.edu
 * @date        06 FEB 2026
 *
 * 
 * Implements the ImGuiSystem, which provides the in-game editor
 * and debugging interface for the engine.
 *
 * - Engine Info: Displays framerate and system timings.
 * - Collision: Shows collider information, spatial grid details, collision debugging options and collision layer matrix.
 * - Scene: Handles scene loading, saving, backgrounds, and scene configuration
 * - Entity Editor + Prefabs: Allows editing, adding, duplicating, deleting, and saving entities and prefabs.
 * - Assets: Provides a browser for textures and audio files with drag-and-drop import support.
 * - Game View: Renders the current scene with play/stop simulation controls.
 * - Editor Camera: Provides a free-camera overview for navigating the level during editing.
 * - UI Editor: Allows editing, adding, duplicating, deleting, and saving UI entities and groups.
 *
 * Copyright © 2026 DigiPen, All rights reserved.
 * ========================================================
 */

#ifndef NO_IMGUI
#define IMGUI_DEFINE_MATH_OPERATORS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>          // for glm::value_ptr
#include <glm/gtc/matrix_transform.hpp>  // for translate/scale/rotate


#include "imgui_internal.h"
#include "ImGuiSystem.h"
#include "Registry.h"
#include "glapp.h"
#include "CoreEngine.h"
#include "EntityCreation.h"
#include "Input.h"
#include "JsonLoad.h"
#include "FilePath.h"
#include "Audio.h"
#include "AudioCache.h"
#include "RenderSystem.h" 
#include "Camera.h"
#include "LogicSystem.h"
#include "GuiSystem.h"
#include "BehaviourComponent.h"
#include "DataDefs.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_set> 
#include <string>
#include <iostream>
#include "SpatialGrid.h"
#include "CollisionManager.h"
#include "Sprite.h"
#include "Physics.h"
#include "MonoSystem.h"
#include "MonoScriptComponent.h"
#include "SceneManager.h"
#include "ImGuizmo.h"
#include "UndoSystem.h"

 // ========================================================
 // File-Local Static Variables (Editor State)
 // ========================================================

static bool isPlacingPrefab = false; //the mouse prefab spawner
static bool isDraggingEntity = false; //drag handles
static bool showReplacePopup = false; //replace same image

static std::string pendingPrefabPath; //prefab mouse thing
static std::string pendingImportPath; //holds new image replace
static std::string pendingDestPath; //store dest path of existing img

static bool g_showAudioErrorPopup = false;
static std::string g_audioErrorMessage;

// runtime thumbnail cache
static std::vector<Image> g_loadedThumbnails;
static std::vector<std::string> g_imageNames;
//auto& sm = SceneManager::Get();
//g_loadedThumbnails[0] = Image(texture for "player.png")
//g_imageNames[0] = "player.png"

static FMOD::Channel* previewChannel = nullptr; //audio preview channel and tracking
static const std::string AUDIO_MANIFEST_PATH = "../Assets/audio_manifest.json";
// drag drop audio import
static bool         g_showAudioImportPopup = false;
static std::string  g_pendingAudioSourcePath;   // full path from drag drop
static int          g_pendingAudioCategory = 0; // 0=Music, 1=SFX, 2=UI

static bool         g_showDeleteImagePopup = false;
static std::string  g_pendingDeleteImageName;

// Video tab
static std::vector<std::string> g_videoNames;
static bool         g_showDeleteVideoPopup = false;
static std::string  g_pendingDeleteVideoName;

// ========================================================
// Static Member Definitions
// ========================================================

bool   ImGuiSystem::showColliders = false; //rendering system aabb boxes
bool   ImGuiSystem::showVelocity = false;
bool   ImGuiSystem::showSpatialGrid = false;
bool   ImGuiSystem::dragEditMode = false; //drag handle
bool   ImGuiSystem::guiEnabled = true; //f1 
Entity ImGuiSystem::selectedEntity = INVALID_ENTITY;

Vec2   ImGuiSystem::camSpawnPos = { 1920.f / 2.0f, 1080.f / 2.0f };
float  ImGuiSystem::camSpawnZoom = 1.f;

Vec2   ImGuiSystem::sceneMin = { 0.f,0.f };
Vec2   ImGuiSystem::sceneMax = { GLApp::VIRTUAL_W, GLApp::VIRTUAL_H }; 
Vec2   ImGuiSystem::sceneSize = { GLApp::VIRTUAL_W, GLApp::VIRTUAL_H };

static ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
static ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;
// ========================================================
// Anonymous Namespace
// ========================================================
namespace
{

    static Entity g_currentInspectorEntity = INVALID_ENTITY;

    //just a safety check
    void EnsureSelectionValid()
    {
        if (ImGuiSystem::selectedEntity != INVALID_ENTITY &&
            !EntityManager::get().isAlive(ImGuiSystem::selectedEntity))
        {
            ImGuiSystem::selectedEntity = INVALID_ENTITY;
        }
    }

    // refrash button
    void RefreshImageAssets()
    {
        g_loadedThumbnails.clear();
        g_imageNames.clear();

        const std::string assetFolder = "../Assets/";
        //loops through all files and folders in the asset directory subdirectories
        for (auto& entry : std::filesystem::recursive_directory_iterator(assetFolder))
        {
            if (!entry.is_regular_file())
                continue; //skips folder 
            //extension string
            std::string ext = entry.path().extension().string();
            // Use a lambda to ensure the result of ::tolower is cast to char
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
            {
                try {
                    Image img = CreateImage(entry.path().string());
                    if (img.IsValid()) {
                        //hooray add to list
                        g_loadedThumbnails.push_back(img);
                        g_imageNames.push_back(entry.path().filename().string());
                    }
                }
                catch (...) {
                    std::cerr << "Failed to load " << entry.path() << "\n";
                }
            }
        }

        std::cout << "[AssetBrowser] Refreshed image list.\n";
    }

    static std::unordered_map<std::string, AudioAsset> RefreshAudioAssets()
    {
        // reload AudioCache fully
        AudioCache::Shutdown();
        AudioCache::Initialize(AUDIO_MANIFEST_PATH);

        // return fresh copy for drawing
        return AudioCache::GetAllAssets();
    }
    // -------------------------------
    // Audio manifest helpers
    // -------------------------------
    static bool LoadAudioManifest(nlohmann::json& manifest)
    {
        std::ifstream in(AUDIO_MANIFEST_PATH);
        if (!in.is_open())
        {
            std::cerr << "[AudioEditor] Failed to open manifest: "<< AUDIO_MANIFEST_PATH << "\n";
            return false;
        }
        try
        {
            in >> manifest;
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    static bool SaveAudioManifest(const nlohmann::json& manifest)
    {
        std::ofstream out(AUDIO_MANIFEST_PATH, std::ios::trunc);
        if (!out.is_open())
        {
            std::cerr << "[AudioEditor] Failed to write manifest: " << AUDIO_MANIFEST_PATH << "\n";
            return false;
        }
        try
        {
            //.dump() returns the originally stored string value
            out << manifest.dump(4);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    // change to underscore
    static std::string MakeAudioKeyFromFilename(const std::string& filename)
    {
        std::string key = std::filesystem::path(filename).stem().string();
        for (char& c : key)
        {
            if (c == ' ' || c == '-' || c == '.')
                c = '_';
        }
        return key;
    }

    // delete audio remove file, manifest entry, banks, unload from cache
    static bool DeleteAudioAsset(const std::string& assetName)
    {
        using json = nlohmann::json;
        json manifest;
        if (!LoadAudioManifest(manifest))
            return false;

        std::string filePath;

        // remove from categories
        // manifest["music"]["bgm1"], manifest["sfx"]["hit"], etc.
        // loop through each cat and see if exist
        const char* categories[] = { "music", "sfx", "ui"};
        for (const char* cat : categories)
        {
            if (manifest.contains(cat) &&
                manifest[cat].is_object() &&
                manifest[cat].contains(assetName))
            {
                // store the file path, delete the actual sound file later
                filePath = manifest[cat][assetName].value("path", "");
                manifest[cat].erase(assetName);
                std::cout << "[AudioEditor] Removed '" << assetName << "' from category '" << cat << "'\n";
                break;
            }
        }

        // remove from banks
        // "banks": { "level1": ["hit", "jump", "bgm1"] }
        if (manifest.contains("banks") && manifest["banks"].is_object())
        {
            for (auto& [bankName, list] : manifest["banks"].items())
            {
                if (!list.is_array())
                    continue;

                auto& arr = list;

                //erase() shifts elements need manually do 
                for (size_t i = 0; i < arr.size();)
                {
                    if (arr[i].is_string() && arr[i].get<std::string>() == assetName)
                        //remove the reference to the deleted asset
                        //long long coz 64-bit i unsigned
                        arr.erase(arr.begin() + static_cast<long long>(i));
                    else
                        ++i;
                }
            }
        }
        //save updated into json
        if (!SaveAudioManifest(manifest))
            return false;

        // delete audio file (if exists)
        if (!filePath.empty())
        {
            try
            {
                // check if file exists before trying to remove it
                if (std::filesystem::exists(filePath))
                {
                    std::filesystem::remove(filePath);
                    std::cout << "[AudioEditor] Deleted file: " << filePath << "\n";
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "[AudioEditor] Failed to delete file '"
                    << filePath << "': " << e.what() << "\n";
            }
        }

        // unload + refresh cache
        AudioCache::UnloadAsset(assetName);
        AudioCache::Initialize(AUDIO_MANIFEST_PATH);

        return true;
    }
    static void StopAudioPreview()
    {
        if (previewChannel)
        {
            previewChannel->stop();
            previewChannel = nullptr;
        }
    }

}


// add in audio_manifest.json
bool AddOrUpdateAudioInManifest(const std::string& key, const std::string& category, const std::string& relPath, const std::string& group,  bool loop = false, bool spatial = false)
{
    using json = nlohmann::json;

    json manifest;

    // read existing manifest (if it exists)
    {
        std::ifstream in(AUDIO_MANIFEST_PATH);
        if (in.is_open())
        {
            try
            {
                in >> manifest;
            }
            catch (const std::exception& e)
            {
                std::cerr << "[ImGui Audio Import] Failed to parse manifest: " << e.what() << "\n";
                return false;
            }
        }
        else
        {
            // new manifest if file doesn't exist yet
            manifest = json::object();
        }
    }

    // ensure the category object exists
    if (!manifest.contains(category) || !manifest[category].is_object())
        manifest[category] = json::object();

    json entry;
    entry["path"] = relPath;
    entry["loop"] = loop;
    entry["group"] = group;
    if (spatial)
        entry["spatial"] = true;

    manifest[category][key] = entry;

    std::ofstream out(AUDIO_MANIFEST_PATH);
    if (!out.is_open())
    {
        std::cerr << "[ImGui Audio Import] Failed to open manifest for writing: "<< AUDIO_MANIFEST_PATH << "\n";
        return false;
    }

    out << manifest.dump(4);
    return true;
}

// importing audio
void ImportAudioFile(const std::string& srcPathStr)
{
    namespace fs = std::filesystem;

    fs::path srcPath(srcPathStr);
    if (!fs::exists(srcPath))
    {
        std::cerr << "[ImGui Audio Import] Source does not exist: " << srcPathStr << "\n";
        return;
    }

    const std::string audioRoot = "../Assets/audio/";
    if (!fs::exists(audioRoot))
        fs::create_directories(audioRoot);

    std::string filename = srcPath.filename().string();
    fs::path destPath = fs::path(audioRoot) / filename;

    try
    {
        fs::copy_file(srcPath, destPath, fs::copy_options::overwrite_existing);
    }
    catch (const fs::filesystem_error&)
    {
        return;
    }

    // manifest key n path
    std::string key = MakeAudioKeyFromFilename(filename);

    // "../Assets/audio/<filename>"
    std::string relPath = destPath.generic_string();

    //default
    std::string category = "sfx";
    std::string group = "SFX";

    if (!AddOrUpdateAudioInManifest(key, category, relPath, group))
    {
        std::cerr << "[ImGui Audio Import] Failed to update manifest for: "
            << key << "\n";
        return;
    }

    // refresh AudioCache so new asset appears in AudioComponent
    AudioCache::Shutdown();
    if (!AudioCache::Initialize(AUDIO_MANIFEST_PATH))
    {
        std::cerr << "[ImGui Audio Import] Failed to reinitialize AudioCache.\n";
    }
    else
    {
        std::cout << "[ImGui Audio Import] Imported audio '" << key  << "' at " << relPath << "\n";
    }
}

void DrawSelectionOutline(Entity e, ImVec2 cursor, float w, float h)
{
    if (e == INVALID_ENTITY) return;

    Registry& reg = Registry::Get();
    if (!reg.Has_Component<TransformComponent>(e)) return;

    auto& t = reg.Get_Component<TransformComponent>(e);

    Vec2 minW, maxW;

    // render Component
    if (reg.Has_Component<RenderComponent>(e))
    {
        auto& r = reg.Get_Component<RenderComponent>(e);
        switch (r.GetRenderType())
        {
        case RT_IMAGE:
        {
            Sprite::SpriteComponent& sprite = r.GetSprite();
            const Image& img = sprite.Get_Image();
            //image pixel size to world size based on unitsPerPixel
            Vec2 worldSize = { img.widthPixels * RenderSystem::unitsPerPixel, img.heightPixels * RenderSystem::unitsPerPixel };
            //sprite scale + transform scale
            Vec2 size = worldSize * r.GetSprite().Get_Scale() * t.scale;
            //bounding box
            Vec2 half = size * 0.5f;
            minW = t.pos - half;
            maxW = t.pos + half;
        }
        break;

        case RT_SPRITESHEET:
        {
            Sprite::SpriteSheet& spritesheet    = r.GetSpriteSheet();
            Sprite::SpriteComponent& sprite     = spritesheet.Sprite();
            //const Image& img = sprite.Get_Image();
            // width/height in image space UV difference
            float frameW = spritesheet.u1 - spritesheet.u0;
            float frameH = spritesheet.v1 - spritesheet.v0;

            Vec2 worldSize = { frameW * RenderSystem::unitsPerPixel, frameH * RenderSystem::unitsPerPixel };

            Vec2 size = worldSize * sprite.Get_Scale() * t.scale;

            Vec2 half = size * 0.5f;
            minW = t.pos - half;
            maxW = t.pos + half;
        }
        break;

        case RT_SHAPE:
        {
            Vec2 size = r.GetShape().dimensions * t.scale;
            Vec2 half = size * 0.5f;
            minW = t.pos - half;
            maxW = t.pos + half;
        }
        break;
        }
    }
    //if no render use physics collider
    else if (reg.Has_Component<ColliderComponent>(e))
    {
        auto& col = reg.Get_Component<ColliderComponent>(e);
        AABB a = col.Get_AABB();
        minW = a.min;
        maxW = a.max;
    }
    else return;

    // convert world to panel
    ImVec2 tl = Render::GetInstance().WorldToPanel(minW, cursor, { w, h });
    ImVec2 br = Render::GetInstance().WorldToPanel(maxW, cursor, { w, h });

    ImDrawList* dl = ImGui::GetWindowDrawList();

    //just for funz
    dl->AddRect(tl, br, IM_COL32(0, 255, 255, 60), 0.0f, 0, 6.0f);
    //outline
    dl->AddRect(tl, br, IM_COL32(0, 255, 255, 255), 0.0f, 0, 2.0f);

}

Entity PickEntityAtWorldPos(const Vec2& world)
{
    Registry& reg = Registry::Get();

    Entity best = INVALID_ENTITY;
    //track the closest entity to click
    //FLT_MAX any real distance will be smaller
    float bestDist = FLT_MAX;

    //loop every enetity w transform comp
    for (auto& [e, t] : reg.Get_ComponentMap<TransformComponent>())
    {
        if (!EntityManager::get().isAlive(e))
            continue;

        bool hit = false;

        //check render
        if (reg.Has_Component<RenderComponent>(e))
        {
            auto& r = reg.Get_Component<RenderComponent>(e);
            RenderType rt = r.GetRenderType();
            Vec2 min, max;

            if (rt == RT_IMAGE)
            {
                Sprite::SpriteComponent& sprite = r.GetSprite();
                const Image& img = sprite.Get_Image();
                //same as above
                Vec2 worldSize = { img.widthPixels * RenderSystem::unitsPerPixel, img.heightPixels * RenderSystem::unitsPerPixel };
                Vec2 size = worldSize * r.GetSprite().Get_Scale() * t.scale;

                Vec2 half = size * 0.5f;
                min = t.pos - half;
                max = t.pos + half;
            }
            else if (rt == RT_SPRITESHEET)
            {
                Sprite::SpriteSheet& spritesheet = r.GetSpriteSheet();
                Sprite::SpriteComponent& sprite = spritesheet.Sprite();
                // const Image& img = sprite.Get_Image();

                float frameW = spritesheet.u1 - spritesheet.u0;
                float frameH = spritesheet.v1 - spritesheet.v0;

                Vec2 worldSize = { frameW * RenderSystem::unitsPerPixel, frameH * RenderSystem::unitsPerPixel };

                Vec2 size = worldSize * sprite.Get_Scale();

                Vec2 half = size * 0.5f;
                min = t.pos - half;
                max = t.pos + half;
            }
            else if (rt == RT_SHAPE)
            {
                Vec2 s = r.GetShape().dimensions;
                Vec2 h = s * 0.5f;
                min = t.pos - h;
                max = t.pos + h;
            }
            //aabb test
            hit = (world.x >= min.x && world.x <= max.x && world.y >= min.y && world.y <= max.y);
        }

        //collider as backup again
        else if (reg.Has_Component<ColliderComponent>(e))
        {
            auto& col = reg.Get_Component<ColliderComponent>(e);
            AABB a = col.Get_AABB();
            hit = (world.x >= a.min.x && world.x <= a.max.x && world.y >= a.min.y && world.y <= a.max.y);
        }

        //if click hit this entity
        if (hit)
        {
            //sqrted dist
            float dx = world.x - t.pos.x;
            float dy = world.y - t.pos.y;
            float d = dx * dx + dy * dy;

            //entity closest to click
            if (d < bestDist)
            {
                bestDist = d;
                best = e;
            }
        }
    }
    //winner!!!!!
    return best;
}

inline float AngleToMouse(const Vec2& posWorld, const Vec2& mouseWorld)
{
    //atan angle btwn 2 points 
    return glm::degrees(atan2(mouseWorld.y - posWorld.y,mouseWorld.x - posWorld.x));
}

inline float SmoothDampAngle(float current, float target, float factor)
{
    //rotates smooth -180 to 180
    float diff = glm::mod(target - current + 540.0f, 360.0f) - 180.0f;
    return current + diff * factor;
}

// Widget templates for component data
// ========================================================
#pragma region WIDGETS
// Static variables to track transform edits for undo
static Entity g_editingEntity = INVALID_ENTITY;
static TransformComponent g_transformBeforeEdit;
static bool g_transformEditStarted = false;
static std::map<void*, float> g_floatValuesBefore;

// Call this at the start of editing a transform component
static void BeginTransformEdit(Entity entity)
{
    if (entity == INVALID_ENTITY) return;
    Registry& reg = Registry::Get();
    if (!reg.Has_Component<TransformComponent>(entity)) return;

    g_editingEntity = entity;
    g_transformBeforeEdit = reg.Get_Component<TransformComponent>(entity);
    g_transformEditStarted = true;
    g_floatValuesBefore.clear();
}

// Call this at the end of editing a transform component
static void EndTransformEdit()
{
    if (!g_transformEditStarted || g_editingEntity == INVALID_ENTITY) return;

    Registry& reg = Registry::Get();
    if (!reg.Has_Component<TransformComponent>(g_editingEntity)) {
        g_transformEditStarted = false;
        return;
    }

    TransformComponent& current = reg.Get_Component<TransformComponent>(g_editingEntity);

    // DEBUG OUTPUT
    std::cout << "[Undo] Before: pos=(" << g_transformBeforeEdit.pos.x << "," << g_transformBeforeEdit.pos.y << ")\n";
    std::cout << "[Undo] Current: pos=(" << current.pos.x << "," << current.pos.y << ")\n";

    // Only record if something actually changed
    if (!UndoSystem::StatesEqual(
        UndoSystem::MakeState(g_transformBeforeEdit),
        UndoSystem::MakeState(current)))
    {
        std::cout << "[Undo] States differ - recording!\n";
        UndoSystem::Get().BeginModify(g_editingEntity, g_transformBeforeEdit);
        UndoSystem::Get().EndModify(g_editingEntity, current);
    }
    else
    {
        std::cout << "[Undo] States equal - not recording\n";
    }

    g_transformEditStarted = false;
    g_editingEntity = INVALID_ENTITY;
    g_floatValuesBefore.clear();
}
// Checkbox | For bool
static void MakeWidget(Editor::Widget_Checkmark& w, const std::string& label) {
    if (!w.ref) return;
    ImGui::Checkbox(label.c_str(), w.ref);
}

// Text     | For string
static void MakeWidget(Editor::Widget_String& w, const std::string& label) {
    if (!w.ref) return;
    char buf[128];
    //copy string to buffer
    strncpy(buf, w.ref->c_str(), sizeof(buf));
    if (ImGui::InputText(label.c_str(), buf, IM_ARRAYSIZE(buf))) {
        //copy text back 2 string
        *w.ref = buf;
    }
}

// Dropdown | For enum (e.g image ID, animation states, etc.
static void MakeWidget(Editor::Widget_Dropdown& w, const std::string& label)
{
    if (!w.ref || w.options.empty()) return;
    //imgui need char* array
    std::vector<const char*> items;
    items.reserve(w.options.size());
    for (auto& opt : w.options) {
        items.push_back(opt.c_str());
    }
    // find index of current selected
    int currentIndex = 0;
    for (int i = 0; i < (int)w.options.size(); ++i) {
        if (w.enumMap.at(w.options[i]) == *w.ref) {
            currentIndex = i;
            break;
        }
    }

    if (ImGui::Combo(label.c_str(), &currentIndex, items.data(), (int)items.size())) {
        *w.ref = w.enumMap[w.options[currentIndex]];
    }
}


namespace {
    // undo tracking
    struct WidgetUndoState {
        float floatBefore = 0.0f;
        int intBefore = 0;
        Entity entity = INVALID_ENTITY;
    };

    static std::unordered_map<void*, WidgetUndoState> g_widgetUndoStates;

    //if should record transform undo
    bool ShouldRecordTransformUndo(Entity e, const std::string& fieldName) {
        Registry& reg = Registry::Get();
        if (e == INVALID_ENTITY) return false;
        if (!reg.Has_Component<TransformComponent>(e)) return false;

        //  transform
        return (fieldName == "Position" || fieldName == "Rotation" ||
            fieldName == "Scale" || fieldName == "pos" ||
            fieldName == "rot" || fieldName == "scale");
    }
}

// Float    | For float lol
static void MakeWidget(Editor::Widget_Float& w, const std::string& label) {
    if (!w.ref) return;

    Registry& reg = Registry::Get();
    void* widgetID = w.ref;
    Entity entity = g_currentInspectorEntity;

    
    if (w.slider && w.min != w.max)
        ImGui::SliderFloat(label.c_str(), w.ref, w.min, w.max);
    else
        //ImGui::InputFloat(label.c_str(), w.ref);
        ImGui::DragFloat(label.c_str(), w.ref, 0.1f);

  
    if (ImGui::IsItemActivated()) {
        WidgetUndoState& state = g_widgetUndoStates[widgetID];
        state.floatBefore = *w.ref;
        state.entity = entity;

        // Only call BeginTransformEdit if hvnt already started editing this entity transform
        if (ShouldRecordTransformUndo(entity, label) &&
            reg.Has_Component<TransformComponent>(entity)) {

            // if already editing this entity transform
            if (g_editingEntity != entity || !g_transformEditStarted) {
                BeginTransformEdit(entity);
            }
        }
    }

   
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        auto it = g_widgetUndoStates.find(widgetID);
        if (it != g_widgetUndoStates.end()) {
            WidgetUndoState& state = it->second;

            if (state.floatBefore != *w.ref && state.entity != INVALID_ENTITY) {
                if (ShouldRecordTransformUndo(state.entity, label) &&
                    reg.Has_Component<TransformComponent>(state.entity)) {

                    
                    if (g_editingEntity == state.entity && g_transformEditStarted) {
                        EndTransformEdit();
                    }

                }
            }
            g_widgetUndoStates.erase(it);
        }
    }
}

// Int
static void MakeWidget(Editor::Widget_Int& w, const std::string& label) {
    if (!w.ref) return;

    Registry& reg = Registry::Get();
    void* widgetID = w.ref;
    Entity entity = g_currentInspectorEntity;


    if (w.slider && w.min != w.max)
        ImGui::SliderInt(label.c_str(), w.ref, w.min, w.max);
    else
        ImGui::InputInt(label.c_str(), w.ref);

  
    if (ImGui::IsItemActivated()) {
        WidgetUndoState& state = g_widgetUndoStates[widgetID];
        state.intBefore = *w.ref;
        state.entity = entity;

        if (ShouldRecordTransformUndo(entity, label) &&
            reg.Has_Component<TransformComponent>(entity)) {

            
            if (g_editingEntity != entity || !g_transformEditStarted) {
                BeginTransformEdit(entity);
            }
        }
    }

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        auto it = g_widgetUndoStates.find(widgetID);
        if (it != g_widgetUndoStates.end()) {
            WidgetUndoState& state = it->second;

            if (state.intBefore != *w.ref && state.entity != INVALID_ENTITY) {
                if (ShouldRecordTransformUndo(state.entity, label) &&
                    reg.Has_Component<TransformComponent>(state.entity)) {

                    // Only end if this was the widget that started it
                    if (g_editingEntity == state.entity && g_transformEditStarted) {
                        EndTransformEdit();
                    }

                  
                }
            }
            g_widgetUndoStates.erase(it);
        }
    }
}
//// Int      | For int lol
//static void MakeWidget(Editor::Widget_Int& w, const std::string& label) {
//    if (!w.ref) return;
//    if (w.slider && w.min != w.max)
//        ImGui::SliderInt(label.c_str(), w.ref, w.min, w.max);
//    else
//        ImGui::InputInt(label.c_str(), w.ref);
//}
//
//// Float    | For float lol
//static void MakeWidget(Editor::Widget_Float& w, const std::string& label) {
//    if (!w.ref) return;
//    if (w.slider && w.min != w.max)
//        ImGui::SliderFloat(label.c_str(), w.ref, w.min, w.max);
//    else
//        ImGui::InputFloat(label.c_str(), w.ref);
//}

// AnimatorFrames (SPECIFIC FOR ANIMATOR BC I CANNOT FIGURE OUT HOW TO MAKE IT PROPERLY might remove)
static void MakeWidget(Editor::Widget_Animator& w, const std::string& label) {
    (void)label;
    if (!w.ref) return;

    MakeWidget(w.total_rows, "Total Rows");
    MakeWidget(w.total_cols, "Total Cols");

    MakeWidget(w.current_state, "Animation state");

}

#pragma endregion WIDGETS
// ========================================================

static bool g_needAssetRefresh = true;
void ImGuiSystem::OnFileDropped(const std::string& path)
{
    namespace fs = std::filesystem;

    if (!fs::exists(path)) {
        std::cerr << "[ImGui Drop] File not found: " << path << std::endl;
        return;
    }

    //extension string
    std::string ext = fs::path(path).extension().string();
    // Use a lambda to ensure the result of ::tolower is cast to char
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    //filename = "blabla.PNG"
    std::string filename = fs::path(path).filename().string();

    std::cout << "[ImGui Drop] File dropped: " << filename << " (" << path << ")\n";

    // --- Image loading --- 
    //Image img = CreateImage(path);
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
    {
        std::string src = path; //where file at
        std::string destDir = "../Assets/";
        std::string destPath = destDir + fs::path(src).filename().string();
        // make sure the folder exists
        if (!fs::exists(destDir))
            fs::create_directories(destDir);


        //same name even if diff ext
        //filename without extension
        std::string Name = fs::path(src).stem().string();
        bool SameName = false;
        std::string existingNamePath;

        // loop through asset directory to find same basename
        for (auto& entry : fs::directory_iterator(destDir))
        {
            if (!entry.is_regular_file())
                continue;

            std::string existingName = entry.path().stem().string();
            if (existingName == Name)
            {
                SameName = true;
                existingNamePath = entry.path().string();
                break;
            }
        }

        if (SameName)
        {
            //pending coz popup in update
            pendingImportPath = src;
            pendingDestPath = existingNamePath;
            showReplacePopup = true;
            return;
        }
        // copy the file
        try {
            fs::copy_file(src, destPath, fs::copy_options::overwrite_existing);
            std::cout << "[ImGui Drop] Imported: " << destPath << "\n";
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "[ImGui Drop] Copy failed: " << e.what() << std::endl;
            return;
        }

        //thumbnaillll
        Image img = CreateImage(destPath);

        if (img.IsValid()) {
            g_loadedThumbnails.push_back(img);
            g_imageNames.push_back(fs::path(destPath).filename().string());
            std::cout << "[ImGui Drop] Loaded texture successfully.\n";
        }
        else
            std::cerr << "[ImGui Drop] Failed to load image.\n";
        return;
    }

    //prefabs
    if (ext == ".json")
    {
        Entity ent = JsonLoad::LoadEntityFromFile(path, 300.f, 300.f);
        if (ent != INVALID_ENTITY)
        {
            // Register entity name so it can be found during prefab updates
            std::filesystem::path filepath(path);
            std::string prefabName = filepath.stem().string();
            Registry::Get().Get_EntityNameMap()[ent] = prefabName;
            std::cout << "[ImGui Drop] Spawned prefab entity " << ent << std::endl;
        }
        else
        {
            std::cerr << "[ImGui Drop] Failed to load prefab.\n";
        }
        return;
    }

    // audio
    if (ext == ".wav")
    {
        g_pendingAudioSourcePath = path;
        g_pendingAudioCategory = 0;         
        g_showAudioImportPopup = true;
        return;
    }
    const std::string assetFolder = "../Assets/";
    if (ext == "." || ext == ".ogg" || ext == ".aac" || ext == ".flac")
    {
        g_audioErrorMessage = "Unsupported audio format: " + filename +
            "\n\nOnly .wav files are allowed for import.";
        g_showAudioErrorPopup = true;
        return;
    }

    // video (.mp4)
    if (ext == ".mp4")
    {
        std::string destPath = "../Assets/" + filename;
        try {
            if (!fs::exists("../Assets/")) fs::create_directories("../Assets/");
            fs::copy_file(path, destPath, fs::copy_options::overwrite_existing);
            std::cout << "[ImGui Drop] Imported video: " << destPath << "\n";

            // Add to video list immediately
            g_videoNames.push_back(filename);
            std::sort(g_videoNames.begin(), g_videoNames.end());
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "[ImGui Drop] Video copy failed: " << e.what() << std::endl;
        }
        return;
    }

    //std::string filename = fs::path(path).filename().string();
    std::string destDir = "../Assets/";
    if (ext == ".ttf" || ext == ".json" || ext == ".vert" || ext == ".frag")
    {
        std::string destPath = assetFolder + filename;

        try {
            // Check if directory exists just in case
            if (!fs::exists(assetFolder)) fs::create_directories(assetFolder);

            fs::copy_file(path, destPath, fs::copy_options::overwrite_existing);
            std::cout << "[ImGui Drop] Imported " << ext << " to: " << destPath << "\n";

            // Trigger the UI lists to rebuild
            g_needAssetRefresh = true;
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "[ImGui Drop] Copy failed: " << e.what() << std::endl;
        }
    }

}




void ImGuiSystem::InitializeImGui() {

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); //imgui i/o settings
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; //enable keyboard nav in imgui
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    //enable Docking

    ImGui::StyleColorsDark(); //dark themed lol
    ImGui_ImplGlfw_InitForOpenGL(GLApp::ptr_window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); //init opengl3 backend


    static bool dropCallbackSet = false; //prevent double reg
    if (!dropCallbackSet && GLApp::ptr_window)
    {
        //handle file drop
        // User drags 3 files onto window
        //count = 2
        //    paths[0] = "C:ionno"
        //    paths[1] = "C:bleh"

        glfwSetDropCallback(GLApp::ptr_window, [](GLFWwindow*, int count, const char** paths) {
            for (int i = 0; i < count; ++i)
            {
                std::string path = paths[i];
                ImGuiSystem::OnFileDropped(path);
            }
            });
        dropCallbackSet = true;
    }
}

//init hear so is easier
void ImGuiSystem::Initialize() {


    InitializeImGui();

}


void ImGuiSystem::Update(float dt)
{
    //Render::GetInstance().EndSceneCapture();
    (void)dt;
    if (ImGui::IsKeyPressed(ImGuiKey_W)) gizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E)) gizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R)) gizmoOperation = ImGuizmo::SCALE;

    if (Input::IsKeyPressed(GLFW_KEY_F1))
        guiEnabled = !guiEnabled;
    SceneManager& sm = SceneManager::Get();
    if (guiEnabled && sm.inEditMode)   // must be in editor mode
    {
        ImGuiIO& io = ImGui::GetIO();

        // Ctrl+Z
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z))
        {
            UndoSystem::Get().Undo();
            EnsureSelectionValid();
        }
    }
    if (!guiEnabled) {
        // Fullscreen mode when ImGui is off
        GameViewRect rect;
        rect.x = 0.0f;
        rect.y = 0.0f;
        rect.width = static_cast<float>(GLApp::width);
        rect.height = static_cast<float>(GLApp::height);
        GuiSystem::Get().SetGameViewRect(rect);
        return; // Don't render ImGui
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    //DOCKINGGGG
    ImGuiWindowFlags dockspaceFlags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("Editor", nullptr, dockspaceFlags);
    //prev style
    ImGui::PopStyleVar(3);

    ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
    //dock node
    ImGui::DockSpace(dockspaceID, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

    // setup layout ONCE
    static bool firstDock = true;
    if (firstDock)
    {
        firstDock = false;
        ImGui::DockBuilderRemoveNode(dockspaceID);
        //new dock node
        ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->WorkSize);

        //SPLIT NODES
        //percentages
        ImGuiID dock_left = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.20f, nullptr, &dockspaceID);
        ImGuiID dock_right = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.25f, nullptr, &dockspaceID);
        ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Down, 0.4f, nullptr, &dockspaceID);
        ImGuiID dock_left_top = ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Up, 0.5f, nullptr, &dock_left);
        //wtv left
        ImGuiID dock_left_bottom = dock_left;


        ImGui::DockBuilderDockWindow("Editor Camera", dock_left_top);
        ImGui::DockBuilderDockWindow("Engine Info", dock_left_bottom);
        ImGui::DockBuilderDockWindow("Entity Editor + Prefabs", dock_right);
        ImGui::DockBuilderDockWindow("UI Editor", dock_right);
        ImGui::DockBuilderDockWindow("Collision", dock_bottom);
        ImGui::DockBuilderDockWindow("Scene Manager", dock_bottom);
        ImGui::DockBuilderDockWindow("Assets", dock_bottom);
        ImGui::DockBuilderDockWindow("Game View", dockspaceID);
        ImGui::DockBuilderFinish(dockspaceID);
    }


    if (ImGui::Begin("Editor Camera", nullptr,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        if (/* */!sm.inEditMode) ImGui::BeginDisabled();
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::GetContentRegionAvail();

        // minimum size 
        if (size.x > 50 && size.y > 50) {
            ShowEntityCamera(cursor, size);
            ImGui::Dummy(size);
        }
        else {
            ImGui::TextDisabled("Panel too small to display");
        }
        if (/* */!sm.inEditMode) ImGui::EndDisabled();
    }
    ImGui::End();



    if (ImGui::Begin("Engine Info")) {
        if (/* */!sm.inEditMode) ImGui::BeginDisabled();
        ShowEngineDebugWindow();
        if (/* */!sm.inEditMode) ImGui::EndDisabled();
    }
    ImGui::End();



    if (ImGui::Begin("Collision")) {
        if (/* */!sm.inEditMode) ImGui::BeginDisabled();
        ShowCollisionDebugWindow();
        if (/* */!sm.inEditMode) ImGui::EndDisabled();
    }
    ImGui::End();

    if (ImGui::Begin("Scene Manager")) {
        if (/* */!sm.inEditMode) ImGui::BeginDisabled();
        //ShowCollisionDebugWindow();
        ShowSceneBrowser();
        if (/* */!sm.inEditMode) ImGui::EndDisabled();
    }
    ImGui::End();


    if (ImGui::Begin("Entity Editor + Prefabs")) {
        if (/* */!sm.inEditMode) ImGui::BeginDisabled();
        // MakeNeaterNextTime
        if (ImGui::Button("Add Entity"))
        {
            Entity newEntity = m_registry.Create_Entity();
            selectedEntity = newEntity;

            // Store editor name
            m_registry.Get_EntityNameMap()[newEntity] = "New Entity " + std::to_string(newEntity);
            UndoSystem::Get().RecordAdd(newEntity);
        }

        ImGui::SameLine();
        if (ImGui::Button("Clone Entity") && selectedEntity != INVALID_ENTITY)
        {
            Registry& reg = Registry::Get();
            SavedEntity src = reg.Save_Entity(selectedEntity);

            //filter out empty json data
            for (auto it = src.componentData.begin(); it != src.componentData.end();)
            {
                if (it->second.empty() || it->second == "{}" || it->second == "null")
                {
                    std::cout << "[Clone] Skipping empty component: " << it->first << "\n";
                    it = src.componentData.erase(it);
                }
                else
                    ++it;
            }

            Entity newEntity = reg.Load_Entity(src);
            if (newEntity != INVALID_ENTITY)
            {
                selectedEntity = newEntity;
                UndoSystem::Get().RecordAdd(newEntity);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Delete Entity") && selectedEntity != INVALID_ENTITY)
        {
            SavedEntity snapshot = m_registry.Save_Entity(selectedEntity);
            UndoSystem::Get().RecordRemove(snapshot);
            m_registry.Remove_Entity(selectedEntity);
            selectedEntity = INVALID_ENTITY;
        }

        ImGui::Separator();
        ImGui::Columns(2, "EntityColumns", true);
        ShowHierarchyWindow();     // left: list
        ImGui::NextColumn();
        ShowAddRemoveComponents(); // right: add/remove comp
        ImGui::Columns(1);

        ImGui::Separator();
        ShowInspectorWindow();     //edit

        ImGui::Separator();
        ShowPrefabControls();      //prefabs

        if (/* */!sm.inEditMode) ImGui::EndDisabled();

    }
    ImGui::End();

/**
 *  Renders the UI Editor main window.
 *
 * This function creates the "UI Editor" ImGui window, which allows users to:
 * - View and select UI entities and groups.
 * - Edit properties of selected UI entities (position, size, color, etc.).
 * - Add, duplicate, or delete UI entities and groups.
 * - Save and load UI layouts.
 * - Drag and drop UI elements for reordering or grouping.
 *
 * The window is only rendered if the UI Editor is enabled.
 */

    if (ImGui::Begin("UI Editor", nullptr)) {
        if (/* */!sm.inEditMode) ImGui::BeginDisabled();

        auto& gui = GuiSystem::Get();

        // UI Element Vectors
        auto& buttons = gui.Buttons();
        auto& textboxes = gui.TextBoxes();
        auto& dialogues = gui.Dialogues();
        auto& displays = gui.Displays();

        // Next ID Helper
        auto nextID = [&]() {
            int mx = -1;
            for (auto& b : buttons)   mx = std::max(mx, b.ID);
            for (auto& t : textboxes) mx = std::max(mx, t.ID);
            for (auto& d : dialogues) mx = std::max(mx, d.ID);
            for (auto& dp : displays) mx = std::max(mx, dp.ID);
            return mx + 1;
            };

        // Selection
        static SelectedType uiSelectedType = SelectedType::None;
        static int uiSelectedIndex = -1;

        // UI element counts
        int numBtn = (int)buttons.size();
        int numTxt = (int)textboxes.size();
        int numDlg = (int)dialogues.size();
        int numDisp = (int)displays.size();
        int total = numBtn + numTxt + numDlg + numDisp;

        // Total count of UI elements
        auto totalCount = [&]()
            {
                return (int)buttons.size() + (int)textboxes.size() + (int)dialogues.size() + (int)displays.size();
            };

        auto globalToLocal = [&](int gIdx, SelectedType& outType, int& outLocal)
            {
                const int numBtn = (int)buttons.size();
                const int numTxt = (int)textboxes.size();
                const int numDlg = (int)dialogues.size();
                if (gIdx < 0) { outType = SelectedType::None; outLocal = -1; return; }
                if (gIdx < numBtn) { outType = SelectedType::Button;  outLocal = gIdx; return; }
                if (gIdx < numBtn + numTxt) { outType = SelectedType::TextBox; outLocal = gIdx - numBtn; return; }
                if (gIdx < numBtn + numTxt + numDlg) { outType = SelectedType::Dialogue; outLocal = gIdx - numBtn - numTxt; return; }
                else { outType = SelectedType::Display; outLocal = gIdx - numBtn - numTxt - numDlg; return; }
            };

        auto localToGlobal = [&](SelectedType type, int lIdx)
            {
                const int numBtn = (int)buttons.size();
                const int numTxt = (int)textboxes.size();
                const int numDlg = (int)dialogues.size();
                if (type == SelectedType::Button)   return lIdx;
                if (type == SelectedType::TextBox)  return numBtn + lIdx;
                if (type == SelectedType::Dialogue) return numBtn + numTxt + lIdx;
                if (type == SelectedType::Display)  return numBtn + numTxt + numDlg + lIdx;
                return -1;
            };

        static bool openAddPopup = false;
        static int addTypeIdx = 0;
        const char* addTypeNames[] = { "Button", "TextBox", "Dialogue", "Display"};

        //  Button to add a new UI entity to the scene.
        if (ImGui::Button("Add UI"))
        {
            openAddPopup = true;
            ImGui::OpenPopup("Add UI Type");
        }

        //  Open a popup to choose which UI entity type to add
        if (ImGui::BeginPopupModal("Add UI Type", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Choose UI element type to add:");
            ImGui::Separator();
            ImGui::Combo("Type", &addTypeIdx, addTypeNames, IM_ARRAYSIZE(addTypeNames));
            ImGui::Separator();
            if (ImGui::Button("Create", ImVec2(100, 0)))
            {
                if (addTypeIdx == 0) // Button
                {
                    ButtonConfig b;
                    b.label = "New Button " + std::to_string((int)buttons.size() + 1);
                    b.pos = { 960, 540 };
                    b.size = { 200, 100 };
                    //b.color = { 0.4f, 0.7f, 0.9f };
                    b.textScale = 1.0f;
                    b.basePos = b.pos; b.baseSize = b.size; b.baseTextScale = b.textScale;
                    b.ID = nextID();
                    buttons.push_back(b);

                    // select the new one
                    uiSelectedType = SelectedType::Button;
                    uiSelectedIndex = localToGlobal(SelectedType::Button, (int)buttons.size() - 1);
                }
                else if (addTypeIdx == 1) // TextBox
                {
                    TextBoxConfig t;
                    t.text = "New Text " + std::to_string((int)textboxes.size() + 1);
                    t.pos = { 960, 600 };
                    t.scale = 1.2f;
                    t.color = { 0,0,0 };
                    t.basePos = t.pos; t.baseScale = t.scale;
                    t.ID = nextID();
                    textboxes.push_back(t);

                    uiSelectedType = SelectedType::TextBox;
                    uiSelectedIndex = localToGlobal(SelectedType::TextBox, (int)textboxes.size() - 1);
                }
                else if (addTypeIdx == 2) // Dialogue
                {
                    // Prepare default text box
                    DialogueConfig d;
                    d.textBox.text = "";
                    d.textBox.basePos = { 450, 200 };
                    d.textBox.baseScale = 1.f;
                    d.textBox.pos = d.textBox.basePos;
                    d.textBox.scale = d.textBox.baseScale;
                    d.textBox.color = { 0, 0, 0 };
                    d.textBox.centered = false;
                    d.isVisible = true;
                    d.jsonPath = FilePath::DIALOGUE;
                    d.sceneID = "0";
                    d.ID = nextID();
                    d.owned = std::make_shared<TextSystem::TextComponent>();
                    d.textSpeed = 30.f;
                    d.wrapWidth = 1300.f;
                    if (d.owned) 
                    {
                        TextSystem::LoadDialogueFromJson(d.jsonPath, *d.owned);
                        d.owned->StartTyping();
                    }
                    d.bgEnabled = true;
                    d.bgBaseSize = { 1500, 300 };
                    d.bgRenderData.SetRenderType(UIRenderType::Image);
                    d.profileEnabled = true;
                    d.profileBaseSize = { 300, 300 };
                    d.profileOffset = { 0, 0 };
                    d.profileRenderData.SetRenderType(UIRenderType::Image);
                    dialogues.push_back(std::move(d));

                    uiSelectedType = SelectedType::Dialogue;
                    uiSelectedIndex = localToGlobal(SelectedType::Dialogue, (int)dialogues.size() - 1);
                }
                else if (addTypeIdx == 3) // Display
                {
                    DisplayConfig dp;
                    dp.basePos = dp.pos = { 960, 540 };
                    dp.baseSize = dp.size = { 240, 160 };
                    dp.centered = true;
                    dp.isVisible = true;
                    dp.m_renderData.SetRenderType(UIRenderType::Shape);
                    auto& shp = dp.m_renderData.GetShapeData();
                    shp.shape = ButtonShape::Rectangle;
                    shp.color = { 0.3f, 0.5f, 0.8f };
                    dp.ID = nextID();
                    displays.push_back(dp);

                    uiSelectedType = SelectedType::Display;
                    uiSelectedIndex = localToGlobal(SelectedType::Display, (int)displays.size() - 1);
                }
                ImGui::CloseCurrentPopup();
                openAddPopup = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 0)))
            {
                ImGui::CloseCurrentPopup();
                openAddPopup = false;
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();

        // Snapshot original dialogue box settings before setting Custom Override to true
        // Default Snapshot (per ID)

        struct DialogueOriginalSettings
        {
            // --- TextBox ---
            Vec2 tbBasePos{};
            float tbBaseScale = 1.f;
            Colour3 tbColor{};
            float wrapWidth = 0.f;
            float textSpeed = 60.f;

            // --- Background ---
            bool bgEnabled = false;
            Vec2 bgBaseSize{};
            UI_RenderData bgRenderData{};
            UIRenderType bgType;

            // Background UIRenderType::Shape
            ButtonShape bgShape;
            Colour3 bgShapeColor{};

            // Background UIRenderType::CustomQuad
            Vec2 bgV0{}, bgV1{}, bgV2{}, bgV3{};
            Colour3 bgQuadColor{};

            // Background UIRenderType::ImageSheet
            std::string bgTexName;
            GLuint bgTexID = 0;
            unsigned bgRows = 1;
            unsigned bgCols = 1;
            UIImageSheetRender::FrameIndex bgFrameIndex{};
            std::unordered_map<unsigned int, UIImageSheetRender::FrameIndex> bgFrames{};

            // --- Profile ---
            bool profileEnabled = false;
            Vec2 profileBaseSize{};
            Vec2 profileOffset{};
            UI_RenderData profileRenderData{};
            UIRenderType profileType;

            // Profile UIRenderType::Shape
            ButtonShape profileShape;
            Colour3 profileShapeColor{};

            // Profile UIRenderType::CustomQuad
            Vec2 pv0{}, pv1{}, pv2{}, pv3{};
            Colour3 pQuadColor{};

            // Profile UIRenderType::ImageSheet
            std::string profileTexName;
            GLuint profileTexID = 0;
            unsigned pRows = 1;
            unsigned pCols = 1;
            UIImageSheetRender::FrameIndex pFrameIndex{};
            std::unordered_map<unsigned int, UIImageSheetRender::FrameIndex> pFrames{};
        };

        // static maps for accessing Dialogue properties for clones
        static std::unordered_map<int, DialogueOriginalSettings> perDlgOriginalSettingsById; // key by d.ID
        static std::unordered_map<int, bool> perDlgOriginalSettingsCapturedById; // key by d.ID
        static std::unordered_map<int, int> perDlgSceneIdxById; // key by d.ID
        static std::unordered_map<int, bool> perDlgColorOverrideById; // key by d.ID (true = dialogue text color can be overriden)
        static std::unordered_map<int, bool> perDlgCustomOverrideById; // key by d.ID (true = dialogue box is user-customizable)
        static std::unordered_map<int, bool> perDlgPrevCustomOverrideById;

        //  Button to duplicate the currently selected UI entity.
        if (ImGui::Button("Clone UI") && uiSelectedIndex >= 0)
        {
            SelectedType type; int lIdx;
            globalToLocal(uiSelectedIndex, type, lIdx);
            if (type == SelectedType::Button && lIdx >= 0 && lIdx < (int)buttons.size())
            {
                ButtonConfig src = buttons[lIdx];
                src.ID = nextID();                   // new ID
                src.label += " (Copy)";
                buttons.push_back(src);
                uiSelectedType = SelectedType::Button;
                uiSelectedIndex = localToGlobal(SelectedType::Button, (int)buttons.size() - 1);
            }
            else if (type == SelectedType::TextBox && lIdx >= 0 && lIdx < (int)textboxes.size())
            {
                TextBoxConfig src = textboxes[lIdx];
                src.ID = nextID();                   // new ID
                src.text += " (Copy)";
                textboxes.push_back(src);
                uiSelectedType = SelectedType::TextBox;
                uiSelectedIndex = localToGlobal(SelectedType::TextBox, (int)textboxes.size() - 1);
            }
            else if (type == SelectedType::Dialogue && lIdx >= 0 && lIdx < (int)dialogues.size())
            {
                const auto& src = dialogues[lIdx];
                DialogueConfig dst;

                dst.ID = nextID();                   // new ID
                dst.textBox = src.textBox;
                dst.isVisible = src.isVisible;
                dst.jsonPath = src.jsonPath;
                dst.sceneID = src.sceneID;
                
                {
                    const bool srcColorOverriden = perDlgColorOverrideById.contains(src.ID) ? perDlgColorOverrideById[src.ID] : false;
                    perDlgColorOverrideById[dst.ID] = srcColorOverriden;

                    const bool srcCustomOverriden = perDlgCustomOverrideById.contains(src.ID) ? perDlgCustomOverrideById[src.ID] : false;
                    perDlgCustomOverrideById[dst.ID] = srcCustomOverriden;

                    if (perDlgSceneIdxById.contains(src.ID))
                    {
                        perDlgSceneIdxById[dst.ID] = perDlgSceneIdxById[src.ID];
                    }
                }

                const TextSystem::TextComponent* sc = src.textComp ? src.textComp : src.owned.get();
                if (sc)
                {
                    dst.owned = std::make_shared<TextSystem::TextComponent>();
                    dst.owned->SetLines(sc->GetLines());
                    dst.owned->SetCurrentLineIndex(sc->GetCurrentLineIndex());
                    dst.textComp = dst.owned.get();

                    dst.owned->SetColor({ dst.textBox.color.r, dst.textBox.color.g, dst.textBox.color.b });
                }
                else
                {
                    dst.textComp = nullptr;
                }

                dialogues.push_back(std::move(dst));
                uiSelectedType = SelectedType::Dialogue;
                uiSelectedIndex = localToGlobal(SelectedType::Dialogue, (int)dialogues.size() - 1);
            }
            else if (type == SelectedType::Display && lIdx >= 0 && lIdx < (int)displays.size())
            {
                DisplayConfig src = displays[lIdx];
                src.ID = nextID();
                displays.push_back(src);
                uiSelectedType = SelectedType::Display;
                uiSelectedIndex = localToGlobal(SelectedType::Display, (int)displays.size() - 1);
            }
        }

        ImGui::SameLine();

        //  Button to delete the currently selected UI entity.
        if (ImGui::Button("Delete UI") && uiSelectedIndex >= 0)
        {
            SelectedType type; int lIdx;
            globalToLocal(uiSelectedIndex, type, lIdx);
            if (type == SelectedType::Button)
            {
                if (lIdx >= 0 && lIdx < (int)buttons.size())
                {
                    buttons.erase(buttons.begin() + lIdx);
                }
            }
            else if (type == SelectedType::TextBox)
            {
                if (lIdx >= 0 && lIdx < (int)textboxes.size())
                {
                    textboxes.erase(textboxes.begin() + lIdx);
                }
            }
            else if (type == SelectedType::Dialogue)
            {
                if (lIdx >= 0 && lIdx < (int)dialogues.size())
                {
                    const int erasedID = dialogues[lIdx].ID;
                    dialogues.erase(dialogues.begin() + lIdx);;
                    perDlgSceneIdxById.erase(erasedID);
                    perDlgColorOverrideById.erase(erasedID);
                    perDlgCustomOverrideById.erase(erasedID);
                    perDlgOriginalSettingsCapturedById.erase(erasedID);
                    perDlgOriginalSettingsById.erase(erasedID);
                }
            }
            else if (type == SelectedType::Display)
            {
                if (lIdx >= 0 && lIdx < (int)displays.size())
                {
                    displays.erase(displays.begin() + lIdx);
                }
            }
            // Reset selection
            uiSelectedType = SelectedType::None;
            uiSelectedIndex = -1;
        }

        ImGui::Separator();
        //ShowHierarchyWindow();     // left: list

        // Refresh UI element counts
        numBtn = (int)buttons.size();
        numTxt = (int)textboxes.size();
        numDlg = (int)dialogues.size();
        total = numBtn + numTxt + numDlg;

        // Hierarchy of UI elements listed by order of ID
        ImGui::SeparatorText("UI Elements");

        // Build flat list sorted by ID
        struct Row { SelectedType type; int local; int id; };
        std::vector<Row> flat;
        flat.reserve(buttons.size() + textboxes.size() + dialogues.size() + displays.size());
        for (int i = 0; i < (int)buttons.size(); ++i) flat.push_back({ SelectedType::Button,   i, buttons[i].ID });
        for (int i = 0; i < (int)textboxes.size(); ++i) flat.push_back({ SelectedType::TextBox,  i, textboxes[i].ID });
        for (int i = 0; i < (int)dialogues.size(); ++i) flat.push_back({ SelectedType::Dialogue, i, dialogues[i].ID });
        for (int i = 0; i < (int)displays.size(); ++i) flat.push_back({ SelectedType::Display, i, displays[i].ID });

        std::sort(flat.begin(), flat.end(), [](const Row& a, const Row& b) { return a.id < b.id; });

        // Resolve current selection to (type, local)
        SelectedType curSelType = uiSelectedType;
        int          curSelLocal = -1;
        if (uiSelectedIndex >= 0) {
            globalToLocal(uiSelectedIndex, curSelType, curSelLocal);
        }

        // Display list of UI entities
        ImGui::BeginChild("List", ImVec2(0, 220), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (int i = 0; i < (int)flat.size(); ++i) 
        {
            const Row& r = flat[i];

            std::string label;
            // Buttons
            if (r.type == SelectedType::Button) 
            {
                const auto& b = buttons[r.local];
                label = "Button (" + b.actionName + "): " + " [ID " + std::to_string(b.ID) + "]";
            }
            // Textboxes
            else if (r.type == SelectedType::TextBox) 
            {
                const auto& tb = textboxes[r.local];
                label = "Text: " + tb.text + " [ID " + std::to_string(tb.ID) + "]";
            }
            // Dialogues
            else if (r.type == SelectedType::Dialogue)
            {
                const auto& d = dialogues[r.local];
                auto* comp = d.textComp ? d.textComp : d.owned.get();
                label = "Dialogue";
                if (comp && !comp->GetLines().empty()) 
                {
                    const auto& ln = comp->GetLines()[comp->GetCurrentLineIndex()];
                    label += ": " + (ln.size() > 24 ? (ln.substr(0, 24) + "...") : ln);
                }
                label += " [ID " + std::to_string(d.ID) + "]";
            }
            // Displays
            else if (r.type == SelectedType::Display)
            {
                const auto& dp = displays[r.local];
                const char* dpType = "None";
                switch (dp.m_renderData.GetRenderType())
                {
                case UIRenderType::Shape: dpType = "Shape"; break;
                case UIRenderType::Image: dpType = "Image"; break;
                case UIRenderType::ImageSheet: dpType = "ImageSheet"; break;
                case UIRenderType::CustomQuad: dpType = "Quad"; break;
                default: break;
                }
                label = std::string("Display (") + dpType + "): [ID " + std::to_string(dp.ID) + "]";
            }

            bool selected = (r.type == curSelType && r.local == curSelLocal);
            if (ImGui::Selectable(label.c_str(), selected)) 
            {
                uiSelectedType = r.type;
                uiSelectedIndex = localToGlobal(r.type, r.local);

                // Re-arm dialogue typing on selection
                if (r.type == SelectedType::Dialogue) 
                {
                    auto& d = dialogues[r.local];
                    auto* tc = d.textComp ? d.textComp : d.owned.get();
                    if (tc) GuiSystem::Get().SetDialogueLine(d, tc->GetCurrentLineIndex(), !SceneManager::Get().inEditMode);
                }
                curSelType = r.type;
                curSelLocal = r.local;
            }
        }
        ImGui::EndChild();

        int selectedLocal = -1;
        if (uiSelectedIndex >= 0)
        {
            globalToLocal(uiSelectedIndex, uiSelectedType, selectedLocal);
        }

        // Find current flat position
        auto findFlatPosOfSelection = [&]() -> int 
            {
            for (int i = 0; i < (int)flat.size(); ++i) 
            {
                if (flat[i].type == curSelType && flat[i].local == curSelLocal) return i;
            }
            return -1;
            };

        // Swap IDs for cross-type reorder
        auto swapIDs = [&](const Row& A, const Row& B) 
            {
            int* idA = nullptr, * idB = nullptr;
            if (A.type == SelectedType::Button)         idA = &buttons[A.local].ID;
            else if (A.type == SelectedType::TextBox)   idA = &textboxes[A.local].ID;
            else if (A.type == SelectedType::Dialogue)  idA = &dialogues[A.local].ID;
            else if (A.type == SelectedType::Display)   idA = &displays[A.local].ID;

            if (B.type == SelectedType::Button)         idB = &buttons[B.local].ID;
            else if (B.type == SelectedType::TextBox)   idB = &textboxes[B.local].ID;
            else if (B.type == SelectedType::Dialogue)  idB = &dialogues[B.local].ID;
            else if (B.type == SelectedType::Display)   idB = &displays[B.local].ID;

            if (idA && idB)
            {
                std::swap(*idA, *idB);
            }
            };

        // pointer getter for an element's ID (works across all types)
        auto idPtrOf = [&](const Row& r) -> int* {
            if (r.type == SelectedType::Button)     return &buttons[r.local].ID;
            if (r.type == SelectedType::TextBox)    return &textboxes[r.local].ID;
            if (r.type == SelectedType::Dialogue)   return &dialogues[r.local].ID;
            if (r.type == SelectedType::Display)    return &displays[r.local].ID;
            else return 0;
            };

        int selPos = findFlatPosOfSelection();

        // Move Up: swap IDs with previous row
        ImGui::BeginDisabled(selPos <= 0);
        if (ImGui::Button("Move Up")) {
            const Row& A = flat[selPos];
            const Row& B = flat[selPos - 1];

            int* idA = idPtrOf(A);
            int* idB = idPtrOf(B);
            std::swap(*idA, *idB);  // this changes the ID-sorted order across ANY types

            // selection still refers to same object; keep uiSelectedIndex consistent
            uiSelectedIndex = localToGlobal(curSelType, curSelLocal);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        // Move Down: swap IDs with next row
        ImGui::BeginDisabled(selPos < 0 || selPos + 1 >= (int)flat.size());
        if (ImGui::Button("Move Down")) {
            const Row& A = flat[selPos];
            const Row& C = flat[selPos + 1];

            int* idA = idPtrOf(A);
            int* idC = idPtrOf(C);
            std::swap(*idA, *idC);

            uiSelectedIndex = localToGlobal(curSelType, curSelLocal);
        }
        ImGui::EndDisabled();

        // UI Properties Window for selected UI entity
        ImGui::SeparatorText("UI Properties");
        ImGui::BeginChild("Properties", ImVec2(0, 300), true);

        if (uiSelectedIndex < 0 || uiSelectedType == SelectedType::None)
        {
            ImGui::TextDisabled("No UI element selected.");
        }
        else
        {
            // Clamp selection after erasure
            auto clampSelectionAfterErase = [&]()
                {
                    int nB = (int)buttons.size();
                    int nT = (int)textboxes.size();
                    int nD = (int)dialogues.size();
                    int totalNow = nB + nT + nD;
                    if (totalNow == 0) { uiSelectedIndex = -1; uiSelectedType = SelectedType::None; return; }
                    if (uiSelectedIndex >= totalNow) uiSelectedIndex = totalNow - 1;
                    SelectedType t2; int li2;
                    if (uiSelectedIndex < nB) { t2 = SelectedType::Button; li2 = uiSelectedIndex; }
                    else if (uiSelectedIndex < nB + nT) { t2 = SelectedType::TextBox; li2 = uiSelectedIndex - nB; }
                    else { t2 = SelectedType::Dialogue; li2 = uiSelectedIndex - nB - nT; }
                    uiSelectedType = t2;
                    selectedLocal = li2;
                };

            // Button Properties
            if (uiSelectedType == SelectedType::Button)
            {
                if (selectedLocal >= 0 && selectedLocal < (int)buttons.size())
                {
                    auto& b = buttons[selectedLocal];

                    // Label
                    {
                        char buf[128];
                        std::snprintf(buf, sizeof(buf), "%s", b.label.c_str());
                        if (ImGui::InputText("Label", buf, IM_ARRAYSIZE(buf)))
                        {
                            b.label = buf;
                        }
                    }
                    ImGui::Separator();
                    // Action new
                    {
                        std::vector<const char*> actionNames;
                        int selected = 0;

                        const auto& ordered = GuiSystem::Get().GetActionOrder();
                        actionNames.reserve(ordered.size());
                        for (const auto& s : ordered) actionNames.push_back(s.c_str());

                        for (int i = 0; i < (int)actionNames.size(); ++i)
                        {
                            if (b.actionName == actionNames[i]) { selected = i; break; }
                        }

                        if (ImGui::Combo("Click Action", &selected, actionNames.data(), static_cast<int>(actionNames.size())))
                        {
                            b.actionName = actionNames[selected];
                            b.actionParams.clear();
                        }

                        if (b.actionName == "LoadScene")
                        {
                            ImGui::Indent();
                            // Check if params assigned
                            if (!b.actionParams.contains("scene") || !b.actionParams.contains("duration"))
                            {
                                json j{
                                    {"scene", "fakescenename"},
                                    {"duration", 2.f}
                                };
                                b.actionParams = j;
                            }


                            // Scene param
                            int selectedScene = -1;
                            std::vector<std::string> sceneFiles;
                            const std::string sceneFolder = "../Assets/saves/";

                            sceneFiles.clear();

                            for (auto& entry : fs::directory_iterator(sceneFolder))
                            {
                                if (entry.path().stem().string() == "temp") continue;

                                if (entry.path().extension() == ".json")
                                {
                                    sceneFiles.push_back(entry.path().stem().string());
                                }
                            }

                            std::vector<const char*> sceneFilePtr; //bruh cannnot const char at start else dangling pointer so i make new vector
                            sceneFilePtr.reserve(sceneFiles.size());
                            for (auto& file : sceneFiles)
                            {
                                sceneFilePtr.push_back(file.c_str());
                            }
                            // check scene index for selected scene
                            std::string currScene = b.actionParams["scene"];
                            for (size_t i = 0; i < sceneFilePtr.size(); i++)
                            {
                                if (currScene == sceneFilePtr[i])
                                {
                                    selectedScene = static_cast<int>(i);
                                    break;
                                }
                            }

                            if (ImGui::Combo("Go scene", &selectedScene, sceneFilePtr.data(), static_cast<int>(sceneFilePtr.size())))
                            {
                                b.actionParams["scene"] = sceneFilePtr[selectedScene];
                            }

                            float currDuration = b.actionParams["duration"];
                            if (ImGui::InputFloat("Fade duration", &currDuration))
                            {
                                b.actionParams["duration"] = currDuration;
                            }
                            ImGui::Unindent();
                        }
                    }
                    ImGui::Separator();
                    // Button sound
                    {
                        static const char* ButtonSoundLabels[] = {
                            "Play",
                            "Quit",
                            "Click"
                        };
                        int currentSound = static_cast<int>(b.soundType);
                        if (ImGui::Combo("Button Sound", &currentSound, ButtonSoundLabels, static_cast<int>(AudioSystem::ButtonType::COUNT)))
                        {
                            b.soundType = static_cast<AudioSystem::ButtonType>(currentSound);
                        }
                    }
                    ImGui::Separator();
                    // Render data
                    {
                        ImGui::SeparatorText("Render");
                        const char* typeNames[] = { "None", "Shape", "CustomQuad", "Image", "ImageSheet" };
                        int currType = (int)b.m_renderData.GetRenderType();
                        if (ImGui::Combo("RenderType", &currType, typeNames, IM_ARRAYSIZE(typeNames)))
                        {
                            b.m_renderData.SetRenderType((UIRenderType)currType);
                        }

                        switch (b.m_renderData.GetRenderType())
                        {

                        // Shape Render
                        case UIRenderType::Shape:
                        {
                            auto& shape = b.m_renderData.GetShapeData();

                            const char* shapeNames[] = { "Rectangle", "Rounded", "Circle" };
                            int currShape = (int)shape.shape;
                            if (ImGui::Combo("Shape", &currShape, shapeNames, IM_ARRAYSIZE(shapeNames)))
                            {
                                shape.shape = (ButtonShape)currShape;
                            }

                            float col[3] = { shape.color.r, shape.color.g, shape.color.b };
                            if (ImGui::ColorEdit3("Color", col))
                            {
                                shape.color.r = col[0]; shape.color.g = col[1]; shape.color.b = col[2];
                            }
                        }
                        break; //end case SHAPE

                        // Custom Quad Render
                        case UIRenderType::CustomQuad:
                        {
                            auto& quad = b.m_renderData.GetQuadData();

                            ImGui::InputFloat2("v0", &quad.v0.x);
                            ImGui::InputFloat2("v1", &quad.v1.x);
                            ImGui::InputFloat2("v2", &quad.v2.x);
                            ImGui::InputFloat2("v3", &quad.v3.x);

                            if (ImGui::Button("Automatic quad by size"))
                            {
                                quad.v0 = { b.basePos.x - (0.5f * b.baseSize.x), b.basePos.y - (0.5f * b.baseSize.y) };
                                quad.v1 = { b.basePos.x + (0.5f * b.baseSize.x), b.basePos.y - (0.5f * b.baseSize.y) };
                                quad.v2 = { b.basePos.x + (0.5f * b.baseSize.x), b.basePos.y + (0.5f * b.baseSize.y) };
                                quad.v3 = { b.basePos.x - (0.5f * b.baseSize.x), b.basePos.y + (0.5f * b.baseSize.y) };
                            }

                            float col[3] = { quad.color.r, quad.color.g, quad.color.b };
                            if (ImGui::ColorEdit3("Color", col))
                            {
                                quad.color.r = col[0]; quad.color.g = col[1]; quad.color.b = col[2];
                            }
                        }
                        break; // end case CUSTOMQUAD

                        // Image Render
                        case UIRenderType::Image:
                        {
                            auto& imgData = b.m_renderData.GetImageData();
                            Image& img = imgData.m_image;

                            const auto& cache = Render::GetInstance().Get_TextureCache();

                            static int selected = 0;
                            std::vector<const char*> imgOptions;
                            std::vector<GLuint> textureIDs;
                            imgOptions.reserve(cache.size());
                            for (auto const& [optname, optval] : cache)
                            {
                                imgOptions.push_back(optname.c_str());
                                textureIDs.push_back(optval);
                            }
                            for (size_t i = 0; i < textureIDs.size(); i++) {
                                if (textureIDs[i] == img.textureID) {
                                    selected = static_cast<int>(i);
                                    break;
                                }
                            }

                            // Image texture
                            if (ImGui::Combo("textureIDnew", &selected, imgOptions.data(), static_cast<int>(imgOptions.size())))
                            {
                                img = CreateImage(imgOptions[selected]);
                            }

                            // display
                            {
                                GLuint texID = img.textureID;
                                const float texW = (float)img.widthPixels;
                                const float texH = (float)img.heightPixels;

                                ImVec2 avail = ImGui::GetContentRegionAvail();

                                float scale = (texW != 0) ? avail.x / texW : 0.f;

                                ImVec2 displaySize(texW * scale, texH * scale);

                                ImGui::Image((ImTextureID)(intptr_t)texID, displaySize, ImVec2(0, 1), ImVec2(1, 0));

                            }

                            // Resize image based on ratio
                            ImGui::Text("Resize to aspect ratio:");
                            if (ImGui::Button("Width"))
                            {
                                b.baseSize.x = b.baseSize.y * ((float)img.widthPixels / (float)img.heightPixels);
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("Height"))
                            {
                                b.baseSize.y = b.baseSize.x * ((float)img.heightPixels / (float)img.widthPixels);
                            }
                        }
                        break; // end case IMAGE

                        // Image Sheet Render
                        case UIRenderType::ImageSheet:
                        {
                            auto& imgsheet = b.m_renderData.GetImageSheetData();

                            Image& img = imgsheet.m_image;

                            const auto& cache = Render::GetInstance().Get_TextureCache();

                            static int selected = 0;
                            std::vector<const char*> imgOptions;
                            std::vector<GLuint> textureIDs;
                            imgOptions.reserve(cache.size());
                            for (auto const& [optname, optval] : cache)
                            {
                                imgOptions.push_back(optname.c_str());
                                textureIDs.push_back(optval);
                            }
                            for (size_t i = 0; i < textureIDs.size(); i++) {
                                if (textureIDs[i] == img.textureID) {
                                    selected = static_cast<int>(i);
                                    break;
                                }
                            }

                            // Image texture
                            if (ImGui::Combo("Texture Drawn", &selected, imgOptions.data(), static_cast<int>(imgOptions.size())))
                            {
                                img = CreateImage(imgOptions[selected]);
                            }

                            // display
                            {
                                GLuint texID = img.textureID;
                                const float texW = (float)img.widthPixels;
                                const float texH = (float)img.heightPixels;

                                ImVec2 avail = ImGui::GetContentRegionAvail();

                                float scale = (texW != 0) ? avail.x / texW : 0.f;

                                ImVec2 displaySize(texW * scale, texH * scale);
                                ImVec2 imagePos = ImGui::GetCursorScreenPos();

                                ImGui::Image((ImTextureID)(intptr_t)texID, displaySize, ImVec2(0, 1), ImVec2(1, 0));

                                ImDrawList* draw = ImGui::GetWindowDrawList();

                                int rows = imgsheet.total_rows;
                                int cols = imgsheet.total_cols;

                                float cellW = displaySize.x / cols;
                                float cellH = displaySize.y / rows;

                                // vertical lines
                                for (int c = 1; c < cols; ++c)
                                {
                                    float x = imagePos.x + c * cellW;
                                    draw->AddLine(ImVec2(x, imagePos.y),
                                        ImVec2(x, imagePos.y + displaySize.y),
                                        IM_COL32(255, 255, 255, 128));
                                }

                                // horizontal lines
                                for (int r = 1; r < rows; ++r)
                                {
                                    float y = imagePos.y + r * cellH;
                                    draw->AddLine(ImVec2(imagePos.x, y),
                                        ImVec2(imagePos.x + displaySize.x, y),
                                        IM_COL32(255, 255, 255, 128));
                                }

                                // highlight
                                int r = imgsheet.frame_index.curr_row;
                                int c = imgsheet.frame_index.curr_col;

                                ImVec2 p0(
                                    imagePos.x + c * cellW,
                                    imagePos.y + r * cellH
                                );
                                ImVec2 p1(
                                    imagePos.x + (c + 1) * cellW,
                                    imagePos.y + (r + 1) * cellH
                                );

                                // Filled highlight
                                draw->AddRectFilled(p0, p1, IM_COL32(255, 255, 0, 80));

                                // Outline
                                draw->AddRect(p0, p1, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
                            }

                            // Render image based on selected row and column of the image sheet
                            ImGui::InputInt("Rows", (int*)&imgsheet.total_rows);
                            if (imgsheet.total_rows < 1) { imgsheet.total_rows = 1; }
                            ImGui::InputInt("Columns", (int*)&imgsheet.total_cols);
                            if (imgsheet.total_cols < 1) { imgsheet.total_cols = 1; }

                            ImGui::Separator();

                            ImGui::SliderInt("Current Row", (int*)&imgsheet.frame_index.curr_row, 0, (int)imgsheet.total_rows - 1);
                            ImGui::SliderInt("Current Col", (int*)&imgsheet.frame_index.curr_col, 0, (int)imgsheet.total_cols - 1);


                            imgsheet.total_cols = std::max(1u, imgsheet.total_cols);
                            imgsheet.total_rows = std::max(1u, imgsheet.total_rows);

                            // State Frames
                            if (ImGui::TreeNode("State Frames"))
                            {
                                for (auto& [state, idx] : imgsheet.frames)
                                {
                                    ImGui::PushID(state);

                                    ImGui::Text("State %u", state);

                                    ImGui::InputInt("Row", (int*)&idx.curr_row);
                                    ImGui::InputInt("Col", (int*)&idx.curr_col);

                                    idx.curr_row = std::clamp(idx.curr_row, 0u, imgsheet.total_rows - 1);
                                    idx.curr_col = std::clamp(idx.curr_col, 0u, imgsheet.total_cols - 1);

                                    if (ImGui::Button("Delete"))
                                        imgsheet.frames.erase(state);

                                    ImGui::Separator();
                                    ImGui::PopID();
                                }

                                static int newStateID = 0;
                                ImGui::InputInt("Add State", &newStateID);
                                if (ImGui::Button("Add")) {
                                    imgsheet.frames[newStateID] = { 0,0 };
                                }

                                ImGui::TreePop();
                            }

                            int frameW = img.widthPixels / imgsheet.total_cols;
                            int frameH = img.heightPixels / imgsheet.total_rows;
                            ImGui::Text("Resize to aspect ratio:");
                            if (ImGui::Button("Width"))
                            {
                                b.baseSize.x = b.baseSize.y * (float(frameW) / float(frameH));
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("Height"))
                            {
                                b.baseSize.y = b.baseSize.x * (float(frameH) / float(frameW));
                            }

                        }
                        break; // end case IMAGESHEET

                        case UIRenderType::None:
                            ImGui::Text("No render information");
                            break;
                        default:
                            break;
                        }
                    }
                    ImGui::Separator();
                    // Position / Size / Text scale
                    {
                        ImGui::SeparatorText("Transform");
                        ImGui::DragFloat("posX", &b.basePos.x, 1.f);
                        ImGui::DragFloat("posY", &b.basePos.y, 1.f);
                        ImGui::DragFloat("sizeX", &b.baseSize.x, 1.f);
                        ImGui::DragFloat("sizeY", &b.baseSize.y, 1.f);
                        ImGui::DragFloat("textScale", &b.baseTextScale, 0.1f);
                    }
                    ImGui::Separator();

                    // Delete Button
                    if (ImGui::Button("Delete This Button"))
                    {
                        buttons.erase(buttons.begin() + selectedLocal);
                        uiSelectedIndex = -1;
                        uiSelectedType = SelectedType::None;
                        clampSelectionAfterErase();
                        goto _UI_PROPS_END_;
                    }
                }
            }

            // TextBox Properties
            else if (uiSelectedType == SelectedType::TextBox)
            {
                if (selectedLocal >= 0 && selectedLocal < (int)textboxes.size())
                {
                    auto& t = textboxes[selectedLocal];

                    // Text
                    {
                        char buf[256];
                        std::snprintf(buf, sizeof(buf), "%s", t.text.c_str());
                        if (ImGui::InputText("Text", buf, IM_ARRAYSIZE(buf))) {
                            t.text = buf;
                        }
                    }
                    ImGui::Separator();
                    ImGui::Checkbox("Visible", &t.isVisible);

                    // Transform
                    ImGui::Separator();
                    ImGui::Text("Transform");
                    ImGui::DragFloat("posX", &t.basePos.x, 1.f);
                    ImGui::DragFloat("posY", &t.basePos.y, 1.f);
                    ImGui::DragFloat("scale", &t.baseScale, 0.1f);

                    // Color
                    ImGui::Separator();
                    ImGui::Text("Color");
                    float col[3] = { t.color.r, t.color.g, t.color.b };
                    if (ImGui::ColorEdit3("Text Color", col)) {
                        t.color.r = col[0]; t.color.g = col[1]; t.color.b = col[2];
                    }

                    ImGui::Separator();

                    // Delete TextBox
                    if (ImGui::Button("Delete This Text")) {
                        textboxes.erase(textboxes.begin() + selectedLocal);
                        uiSelectedIndex = -1;
                        uiSelectedType = SelectedType::None;
                        clampSelectionAfterErase();
                        goto _UI_PROPS_END_;
                    }
                }
            }
            // Dialogue Properties
            else if (uiSelectedType == SelectedType::Dialogue)
            {
                if (selectedLocal >= 0 && selectedLocal < (int)dialogues.size())
                {
                    auto& d = dialogues[selectedLocal];

                    TextSystem::TextComponent* comp = d.textComp ? d.textComp : d.owned.get();
                    if (!comp)
                    {
                        if (!d.owned) d.owned = std::make_shared<TextSystem::TextComponent>();
                        comp = d.owned.get();
                    }

                    // Load data from Dialogue.json (shared per editor session)
                    static json dlgDoc;
                    static bool dlgLoaded = false;
                    static std::vector<std::string> sceneIDs;

                    auto loadDlgDocIfNeeded = [&]()
                        {
                            if (dlgLoaded) return;

                            try
                            {
                                std::ifstream ifs(d.jsonPath);
                                if (!ifs)
                                {
                                    dlgLoaded = false;
                                    sceneIDs.clear();
                                    sceneIDs.push_back("0");
                                    return;
                                }

                                try { ifs >> dlgDoc; }
                                catch (...)
                                {
                                    ifs.clear();
                                    ifs.seekg(0);
                                    std::string raw((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                                    std::string cleaned; cleaned.reserve(raw.size());
                                    for (size_t i = 0; i < raw.size(); ++i)
                                    {
                                        if (raw[i] == ',' && i + 1 < raw.size() && (raw[i + 1] == '\n' || raw[i + 1] == '\r')) continue;
                                        cleaned.push_back(raw[i]);
                                    }
                                    dlgDoc = json::parse(cleaned);
                                }

                                sceneIDs.clear();
                                if (dlgDoc.contains("scenes") && dlgDoc["scenes"].is_array())
                                {
                                    for (auto& sc : dlgDoc["scenes"])
                                        sceneIDs.push_back(sc.value("sceneID", std::string("0")));
                                }

                                if (sceneIDs.empty()) sceneIDs.push_back("0");
                                dlgLoaded = true;
                            }
                            catch (...)
                            {
                                dlgLoaded = false;
                                sceneIDs.clear();
                                sceneIDs.push_back("0");
                            }
                        };

                    loadDlgDocIfNeeded();

                    auto findSceneIndexByID = [&](const std::string& sid) -> int
                        {
                            for (int i = 0; i < (int)sceneIDs.size(); ++i)
                                if (sceneIDs[i] == sid) return i;
                            return 0;
                        };

                    //// per-dialogue editor state
                    //static std::unordered_map<int, int> perDlgSceneIdxById;
                    //static std::unordered_map<int, bool> perDlgColorOverrideById;
                    //static std::unordered_map<int, bool> perDlgCustomOverrideById;
                    //static std::unordered_map<int, bool> perDlgPrevCustomOverrideById;

                    // New: selected speaker in editor (custom mode)
                    static std::unordered_map<int, int> perDlgSelectedSpeakerById;

                    if (d.sceneID.empty()) d.sceneID = "0";

                    if (!perDlgSceneIdxById.contains(d.ID))
                        perDlgSceneIdxById[d.ID] = std::clamp(findSceneIndexByID(d.sceneID), 0, std::max(0, (int)sceneIDs.size() - 1));

                    if (!perDlgColorOverrideById.contains(d.ID))
                        perDlgColorOverrideById[d.ID] = false;

                    if (!perDlgCustomOverrideById.contains(d.ID))
                        perDlgCustomOverrideById[d.ID] = d.customOverride;

                    if (!perDlgPrevCustomOverrideById.contains(d.ID))
                        perDlgPrevCustomOverrideById[d.ID] = perDlgCustomOverrideById[d.ID];

                    if (!perDlgSelectedSpeakerById.contains(d.ID))
                        perDlgSelectedSpeakerById[d.ID] = 0;

                    int& activeSceneIdx = perDlgSceneIdxById[d.ID];
                    bool& colorOverriden = perDlgColorOverrideById[d.ID];
                    bool& customOverriden = perDlgCustomOverrideById[d.ID];
                    bool& prevCustomState = perDlgPrevCustomOverrideById[d.ID];
                    int& selectedSpeakerID = perDlgSelectedSpeakerById[d.ID];

                    if (!sceneIDs.empty()) activeSceneIdx = std::clamp(activeSceneIdx, 0, (int)sceneIDs.size() - 1);
                    else activeSceneIdx = 0;

                    // -------------------------
                    // Helpers
                    // -------------------------
                    auto clampCurrentLine = [&]()
                        {
                            auto lines = comp->GetLines();
                            if (lines.empty())
                            {
                                lines.push_back("");
                                comp->SetLines(lines);
                            }
                            int idx = comp->GetCurrentLineIndex();
                            idx = std::clamp(idx, 0, (int)lines.size() - 1);
                            comp->SetCurrentLineIndex(idx);
                        };

                    auto applySceneToComp = [&](int sceneIdx, bool applyColor, bool applyTextBoxSettings)
                        {
                            if (!dlgDoc.contains("scenes") || !dlgDoc["scenes"].is_array()) return;
                            if (sceneIdx < 0 || sceneIdx >= (int)dlgDoc["scenes"].size()) return;

                            const auto& sc = dlgDoc["scenes"][sceneIdx];

                            std::vector<std::string> lines;
                            if (sc.contains("script") && sc["script"].is_array())
                            {
                                for (const auto& node : sc["script"])
                                    lines.push_back(node.value("line", std::string("")));
                            }
                            if (lines.empty()) lines.push_back("");

                            comp->SetLines(lines);
                            comp->SetCurrentLineIndex(0);
                            comp->SetIsTyping(false);

                            comp->SetFontPath(sc.value("fontPath", std::string("../Assets/fonts/Marker.ttf")));
                            comp->SetFontSize(sc.value("fontSize", 1.0f));

                            if (applyTextBoxSettings)
                            {
                                comp->SetTextSpeed(sc.value("textSpeed", 60.f));
                                d.wrapWidth = sc.value("wrapWidth", d.wrapWidth);

                                if (applyColor && sc.contains("color") && sc["color"].is_array() && sc["color"].size() == 3)
                                {
                                    Vec3 c{ sc["color"][0].get<float>(), sc["color"][1].get<float>(), sc["color"][2].get<float>() };
                                    comp->SetColor(c);
                                    d.textBox.color = { c.x, c.y, c.z };
                                }
                                else
                                {
                                    comp->SetColor({ d.textBox.color.r, d.textBox.color.g, d.textBox.color.b });
                                }
                            }
                            else
                            {
                                comp->SetTextSpeed(comp->GetTextSpeed());
                                comp->SetColor({ d.textBox.color.r, d.textBox.color.g, d.textBox.color.b });
                            }

                            comp->SetVisibleChars(0);
                            comp->StartTyping();
                        };

                    auto getSceneSpeakerIDs = [&](int sceneIdx, std::vector<int>& outSpeakerIDs)
                        {
                            outSpeakerIDs.clear();
                            if (!dlgDoc.contains("scenes") || !dlgDoc["scenes"].is_array()) return;
                            if (sceneIdx < 0 || sceneIdx >= (int)dlgDoc["scenes"].size()) return;
                            const auto& sc = dlgDoc["scenes"][sceneIdx];
                            if (!sc.contains("script") || !sc["script"].is_array()) return;

                            for (const auto& node : sc["script"])
                                outSpeakerIDs.push_back(std::max(0, node.value("speakerID", 0)));
                        };

                    auto ensureSkinExists = [&](int speakerID)
                        {
                            speakerID = std::max(0, speakerID);
                            if (!d.speakerSkins.contains(speakerID))
                            {
                                DialogueSpeakerSkin skin{};
                                // Seed from current legacy single-skin so it "just works"
                                skin.bgEnabled = d.bgEnabled;
                                skin.bgBaseSize = d.bgBaseSize;
                                skin.bgRenderData = d.bgRenderData;

                                skin.profileEnabled = d.profileEnabled;
                                skin.profileBaseSize = d.profileBaseSize;
                                skin.profileOffset = d.profileOffset;
                                skin.profileRenderData = d.profileRenderData;

                                d.speakerSkins[speakerID] = skin;
                            }
                        };

                    // RenderData editor (parametric) - avoids duplicating bg/profile logic for each speaker skin.
                    auto drawUIRenderDataEditor = [&](const char* labelPrefix,
                        UI_RenderData& rd,
                        Vec2& baseSize,
                        const Vec2& anchorPos,
                        int uniqueKey,
                        bool showAspectHelpers)
                        {
                            ImGui::PushID(uniqueKey);

                            const char* typeNames[] = { "None", "Shape", "CustomQuad", "Image", "ImageSheet" };
                            int currType = static_cast<int>(rd.GetRenderType());
                            if (ImGui::Combo((std::string(labelPrefix) + " Render Type").c_str(), &currType, typeNames, IM_ARRAYSIZE(typeNames)))
                                rd.SetRenderType((UIRenderType)currType);

                            switch (rd.GetRenderType())
                            {
                            case UIRenderType::Shape:
                            {
                                auto& shape = rd.GetShapeData();
                                const char* shapeNames[] = { "Rectangle", "Rounded", "Circle" };
                                int currShape = (int)shape.shape;
                                if (ImGui::Combo((std::string(labelPrefix) + " Shape").c_str(), &currShape, shapeNames, IM_ARRAYSIZE(shapeNames)))
                                    shape.shape = (ButtonShape)currShape;

                                float currCol[3] = { shape.color.r, shape.color.g, shape.color.b };
                                if (ImGui::ColorEdit3((std::string(labelPrefix) + " Color").c_str(), currCol))
                                    shape.color = { currCol[0], currCol[1], currCol[2] };
                            } break;

                            case UIRenderType::CustomQuad:
                            {
                                auto& quad = rd.GetQuadData();
                                ImGui::InputFloat2((std::string(labelPrefix) + " v0").c_str(), &quad.v0.x);
                                ImGui::InputFloat2((std::string(labelPrefix) + " v1").c_str(), &quad.v1.x);
                                ImGui::InputFloat2((std::string(labelPrefix) + " v2").c_str(), &quad.v2.x);
                                ImGui::InputFloat2((std::string(labelPrefix) + " v3").c_str(), &quad.v3.x);

                                if (ImGui::Button((std::string("Auto Quad##") + labelPrefix).c_str()))
                                {
                                    quad.v0 = { anchorPos.x - (0.5f * baseSize.x), anchorPos.y - (0.5f * baseSize.y) };
                                    quad.v1 = { anchorPos.x + (0.5f * baseSize.x), anchorPos.y - (0.5f * baseSize.y) };
                                    quad.v2 = { anchorPos.x + (0.5f * baseSize.x), anchorPos.y + (0.5f * baseSize.y) };
                                    quad.v3 = { anchorPos.x - (0.5f * baseSize.x), anchorPos.y + (0.5f * baseSize.y) };
                                }

                                float quadCol[3] = { quad.color.r, quad.color.g, quad.color.b };
                                if (ImGui::ColorEdit3((std::string(labelPrefix) + " Quad Color").c_str(), quadCol))
                                    quad.color = { quadCol[0], quadCol[1], quadCol[2] };
                            } break;

                            case UIRenderType::Image:
                            {
                                auto& imgData = rd.GetImageData();
                                Image& img = imgData.m_image;
                                const auto& cache = Render::GetInstance().Get_TextureCache();

                                static std::unordered_map<int, int> selectedImageByKey;
                                int& selected = selectedImageByKey[uniqueKey];

                                std::vector<const char*> imgOptions;
                                std::vector<GLuint> textureIDs;
                                imgOptions.reserve(cache.size());
                                textureIDs.reserve(cache.size());
                                for (auto const& [optname, optval] : cache)
                                {
                                    imgOptions.push_back(optname.c_str());
                                    textureIDs.push_back(optval);
                                }

                                for (size_t i = 0; i < textureIDs.size(); i++)
                                    if (textureIDs[i] == img.textureID) { selected = (int)i; break; }

                                if (!imgOptions.empty() && ImGui::Combo((std::string(labelPrefix) + " Texture").c_str(), &selected, imgOptions.data(), (int)imgOptions.size()))
                                    img = CreateImage(imgOptions[selected]);

                                if (img.textureID)
                                {
                                    ImVec2 avail = ImGui::GetContentRegionAvail();
                                    float scale = img.widthPixels ? (avail.x / (float)img.widthPixels) : 0.f;
                                    ImVec2 displaySize(img.widthPixels * scale, img.heightPixels * scale);
                                    ImGui::Image((ImTextureID)(intptr_t)img.textureID, displaySize, ImVec2(0, 1), ImVec2(1, 0));

                                    if (showAspectHelpers && img.widthPixels > 0 && img.heightPixels > 0)
                                    {
                                        ImGui::TextUnformatted("Resize to aspect ratio:");
                                        if (ImGui::Button((std::string("Width##") + labelPrefix).c_str()))
                                            baseSize.x = baseSize.y * ((float)img.widthPixels / (float)img.heightPixels);
                                        ImGui::SameLine();
                                        if (ImGui::Button((std::string("Height##") + labelPrefix).c_str()))
                                            baseSize.y = baseSize.x * ((float)img.heightPixels / (float)img.widthPixels);
                                    }
                                }
                            } break;

                            case UIRenderType::ImageSheet:
                            {
                                auto& sheet = rd.GetImageSheetData();
                                Image& img = sheet.m_image;

                                const auto& cache = Render::GetInstance().Get_TextureCache();

                                static std::unordered_map<int, int> selectedSheetByKey;
                                int& selected = selectedSheetByKey[uniqueKey];

                                std::vector<const char*> imgOptions;
                                std::vector<GLuint> textureIDs;
                                imgOptions.reserve(cache.size());
                                textureIDs.reserve(cache.size());
                                for (auto const& [optname, optval] : cache)
                                {
                                    imgOptions.push_back(optname.c_str());
                                    textureIDs.push_back(optval);
                                }

                                for (size_t i = 0; i < textureIDs.size(); i++)
                                    if (textureIDs[i] == img.textureID) { selected = (int)i; break; }

                                if (!imgOptions.empty() && ImGui::Combo((std::string(labelPrefix) + " Sheet Texture").c_str(), &selected, imgOptions.data(), (int)imgOptions.size()))
                                    img = CreateImage(imgOptions[selected]);

                                ImGui::InputInt((std::string(labelPrefix) + " Rows").c_str(), (int*)&sheet.total_rows);
                                ImGui::InputInt((std::string(labelPrefix) + " Columns").c_str(), (int*)&sheet.total_cols);
                                sheet.total_rows = std::max(1u, sheet.total_rows);
                                sheet.total_cols = std::max(1u, sheet.total_cols);

                                ImGui::SliderInt((std::string(labelPrefix) + " Current Row").c_str(), (int*)&sheet.frame_index.curr_row, 0, (int)sheet.total_rows - 1);
                                ImGui::SliderInt((std::string(labelPrefix) + " Current Col").c_str(), (int*)&sheet.frame_index.curr_col, 0, (int)sheet.total_cols - 1);

                                if (img.textureID)
                                {
                                    const float texW = (float)img.widthPixels;
                                    const float texH = (float)img.heightPixels;

                                    ImVec2 avail = ImGui::GetContentRegionAvail();
                                    float scale = (texW != 0) ? avail.x / texW : 0.f;
                                    ImVec2 displaySize(texW * scale, texH * scale);

                                    ImVec2 imagePos = ImGui::GetCursorScreenPos();
                                    ImGui::Image((ImTextureID)(intptr_t)img.textureID, displaySize, ImVec2(0, 1), ImVec2(1, 0));

                                    ImDrawList* draw = ImGui::GetWindowDrawList();
                                    int rows = (int)sheet.total_rows;
                                    int cols = (int)sheet.total_cols;

                                    float cellW = displaySize.x / cols;
                                    float cellH = displaySize.y / rows;

                                    for (int c = 1; c < cols; ++c)
                                    {
                                        float x = imagePos.x + c * cellW;
                                        draw->AddLine(ImVec2(x, imagePos.y), ImVec2(x, imagePos.y + displaySize.y), IM_COL32(255, 255, 255, 128));
                                    }
                                    for (int r = 1; r < rows; ++r)
                                    {
                                        float y = imagePos.y + r * cellH;
                                        draw->AddLine(ImVec2(imagePos.x, y), ImVec2(imagePos.x + displaySize.x, y), IM_COL32(255, 255, 255, 128));
                                    }

                                    int rr = (int)sheet.frame_index.curr_row;
                                    int cc = (int)sheet.frame_index.curr_col;
                                    ImVec2 p0(imagePos.x + cc * cellW, imagePos.y + rr * cellH);
                                    ImVec2 p1(imagePos.x + (cc + 1) * cellW, imagePos.y + (rr + 1) * cellH);

                                    draw->AddRectFilled(p0, p1, IM_COL32(255, 255, 0, 80));
                                    draw->AddRect(p0, p1, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);

                                    if (showAspectHelpers)
                                    {
                                        int frameW = img.widthPixels / std::max(1u, sheet.total_cols);
                                        int frameH = img.heightPixels / std::max(1u, sheet.total_rows);

                                        if (frameW > 0 && frameH > 0)
                                        {
                                            ImGui::TextUnformatted("Resize to aspect ratio:");
                                            if (ImGui::Button((std::string("Width##") + labelPrefix).c_str()))
                                                baseSize.x = baseSize.y * (float(frameW) / float(frameH));
                                            ImGui::SameLine();
                                            if (ImGui::Button((std::string("Height##") + labelPrefix).c_str()))
                                                baseSize.y = baseSize.x * (float(frameH) / float(frameW));
                                        }
                                    }
                                }
                            } break;

                            default:
                                ImGui::TextDisabled("No render info.");
                                break;
                            }

                            ImGui::PopID();
                        };

                    // Capture and restore original settings when leaving custom mode
                    {
                        bool& captured = perDlgOriginalSettingsCapturedById[d.ID];
                        if (!captured)
                        {
                            DialogueOriginalSettings ogs{};
                            ogs.tbBasePos = d.textBox.basePos;
                            ogs.tbBaseScale = d.textBox.baseScale;
                            ogs.tbColor = d.textBox.color;
                            ogs.wrapWidth = d.wrapWidth;
                            ogs.textSpeed = comp->GetTextSpeed();

                            ogs.bgEnabled = d.bgEnabled;
                            ogs.bgBaseSize = d.bgBaseSize;
                            ogs.bgRenderData = d.bgRenderData;
                            ogs.bgType = d.bgRenderData.GetRenderType();

                            ogs.profileEnabled = d.profileEnabled;
                            ogs.profileBaseSize = d.profileBaseSize;
                            ogs.profileOffset = d.profileOffset;
                            ogs.profileRenderData = d.profileRenderData;
                            ogs.profileType = d.profileRenderData.GetRenderType();

                            perDlgOriginalSettingsById[d.ID] = ogs;
                            captured = true;
                        }
                    }

                    // Custom Override / Scene-Driven Mode Select
                    {
                        ImGui::SeparatorText("Dialogue Mode");
                        const bool changedCustomMode = ImGui::Checkbox("Custom Override", &customOverriden);
                        d.customOverride = customOverriden;

                        ImGui::SameLine();
                        ImGui::TextDisabled(customOverriden ? "(Editable)" : "(Scene-driven)");

                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip(customOverriden
                                ? "Lines + speaker skins can be edited."
                                : "Lines + speaker skins are derived by game scene data.");

                        if (changedCustomMode && customOverriden)
                        {
                            // Initialize per-line speaker IDs to match current lines.
                            const int n = (int)comp->GetLines().size();
                            d.customLineSpeakerIDs.resize(n, 0);

                            // Seed speaker 0 so there's always something to edit.
                            ensureSkinExists(0);
                            selectedSpeakerID = 0;
                        }

                        // Leaving custom => restore original settings, re-apply scene
                        if (changedCustomMode && prevCustomState && !customOverriden)
                        {
                            if (auto it = perDlgOriginalSettingsById.find(d.ID); it != perDlgOriginalSettingsById.end())
                            {
                                const DialogueOriginalSettings& ogs = it->second;

                                d.textBox.basePos = ogs.tbBasePos;
                                d.textBox.baseScale = ogs.tbBaseScale;
                                d.textBox.color = ogs.tbColor;
                                d.wrapWidth = ogs.wrapWidth;

                                comp->SetTextSpeed(ogs.textSpeed);
                                comp->SetColor({ ogs.tbColor.r, ogs.tbColor.g, ogs.tbColor.b });

                                d.bgEnabled = ogs.bgEnabled;
                                d.bgBaseSize = ogs.bgBaseSize;
                                d.bgRenderData = ogs.bgRenderData;

                                d.profileEnabled = ogs.profileEnabled;
                                d.profileBaseSize = ogs.profileBaseSize;
                                d.profileOffset = ogs.profileOffset;
                                d.profileRenderData = ogs.profileRenderData;
                            }

                            colorOverriden = false;
                            d.sceneID = sceneIDs.empty() ? "0" : sceneIDs[activeSceneIdx];
                            applySceneToComp(activeSceneIdx, true, true);
                        }

                        prevCustomState = customOverriden;
                    }

                    // Scene selection (always visible; only applies in scene-driven mode)
                    {
                        ImGui::SeparatorText("Scene Source");

                        std::vector<const char*> sceneItems;
                        sceneItems.reserve(sceneIDs.size());
                        for (auto& s : sceneIDs) sceneItems.push_back(s.c_str());

                        const int prevScene = activeSceneIdx;
                        if (customOverriden) ImGui::BeginDisabled(true);
                        ImGui::Combo("Scene ID", &activeSceneIdx, sceneItems.data(), (int)sceneItems.size());
                        if (customOverriden) ImGui::EndDisabled();

                        if (customOverriden && ImGui::IsItemHovered())
                            ImGui::SetTooltip("Scene selection only drives dialogue when Custom Override is OFF.");

                        if (!sceneItems.empty()) activeSceneIdx = std::clamp(activeSceneIdx, 0, (int)sceneItems.size() - 1);
                        else activeSceneIdx = 0;

                        if (activeSceneIdx != prevScene)
                        {
                            d.sceneID = sceneIDs.empty() ? "0" : sceneIDs[activeSceneIdx];
                            if (!customOverriden)
                            {
                                colorOverriden = false;
                                applySceneToComp(activeSceneIdx, true, true);
                            }
                        }

                        // Ensure component has lines in scene-driven mode
                        if (!customOverriden && comp->GetLines().empty())
                        {
                            applySceneToComp(activeSceneIdx, true, true);
                        }
                    }

                    // Textbox (editable only in custom mode)
                    {
                        ImGui::SeparatorText("Text Box");

                        ImGui::DragFloat("Pos X", &d.textBox.basePos.x, 1.f);
                        ImGui::DragFloat("Pos Y", &d.textBox.basePos.y, 1.f);
                        ImGui::DragFloat("Scale", &d.textBox.baseScale, 0.1f);

                        float col[3] = { d.textBox.color.r, d.textBox.color.g, d.textBox.color.b };
                        if (ImGui::ColorEdit3("Text Color", col))
                        {
                            d.textBox.color = { col[0], col[1], col[2] };
                            comp->SetColor({ col[0], col[1], col[2] });
                            colorOverriden = true;
                        }

                        ImGui::DragFloat("Wrap Width", &d.wrapWidth, 1.f);

                        float spd = comp->GetTextSpeed();
                        if (ImGui::DragFloat("Text Speed", &spd, 1.f))
                        {
                            comp->SetTextSpeed(spd);
                        }
                    }

                    // Lines
                    {
                        ImGui::SeparatorText("Lines");

                        clampCurrentLine();

                        if (!customOverriden)
                        {
                            // Scene-driven: pick a line to preview; show its speakerID too.
                            const auto& lines = comp->GetLines();
                            int idx = comp->GetCurrentLineIndex();
                            idx = std::clamp(idx, 0, std::max(0, (int)lines.size() - 1));

                            std::vector<int> sceneSpeakerIDs;
                            getSceneSpeakerIDs(activeSceneIdx, sceneSpeakerIDs);

                            ImGui::Text("Current Line: %d / %d", lines.empty() ? 0 : (idx + 1), (int)lines.size());

                            if (!lines.empty())
                            {
                                static std::vector<std::string> lbls;
                                lbls.clear();
                                lbls.reserve(lines.size());
                                for (const auto& s : lines)
                                    lbls.push_back(s.size() > 96 ? (s.substr(0, 96) + "...") : s);

                                std::vector<const char*> citems;
                                citems.reserve(lbls.size());
                                for (auto& s : lbls) citems.push_back(s.c_str());

                                int prevIdx = idx;
                                ImGui::Combo("Line", &idx, citems.data(), (int)citems.size());
                                if (idx != prevIdx)
                                {
                                    idx = std::clamp(idx, 0, (int)lines.size() - 1);
                                    comp->SetCurrentLineIndex(idx);
                                    comp->StartTyping();
                                }

                                int sid = 0;
                                if (idx >= 0 && idx < (int)sceneSpeakerIDs.size())
                                    sid = sceneSpeakerIDs[idx];

                                ImGui::Text("SpeakerID: %d", sid);
                                //ImGui::BeginDisabled(true);
                                //ImGui::InputTextMultiline("Text (read-only)", (char*)lines[idx].c_str(), lines[idx].size() + 1, ImVec2(-1, 80));
                                //ImGui::EndDisabled();
                            }
                            else
                            {
                                ImGui::TextDisabled("No lines in this scene.");
                            }
                        }
                        else
                        {
                            // Custom: edit all lines + their speakerIDs in one list (simple mental model).
                            auto lines = comp->GetLines();
                            if (lines.empty()) lines.push_back("");

                            d.customLineSpeakerIDs.resize(lines.size(), 0);

                            ImGui::Text("Number of Lines: %d", (int)lines.size());

                            for (int i = 0; i < (int)lines.size(); ++i)
                            {
                                ImGui::PushID(i);

                                // Select speaker per line
                                ImGui::Text("Line #%d", i + 1);

                                // Build dropdown options from existing speaker skins (custom mode)
                                std::vector<int> speakerIDs;
                                speakerIDs.reserve(d.speakerSkins.size());
                                for (const auto& [sid, _] : d.speakerSkins) speakerIDs.push_back(sid);
                                if (speakerIDs.empty()) speakerIDs.push_back(0);
                                std::sort(speakerIDs.begin(), speakerIDs.end());
                                speakerIDs.erase(std::unique(speakerIDs.begin(), speakerIDs.end()), speakerIDs.end());

                                // --- NEW: try to pull speaker names from Dialogue.json (scene-driven source) ---
                                std::vector<std::string> speakerNamesFromJson;
                                if (dlgLoaded && dlgDoc.contains("scenes") && dlgDoc["scenes"].is_array() &&
                                    activeSceneIdx >= 0 && activeSceneIdx < (int)dlgDoc["scenes"].size())
                                {
                                    const auto& sc = dlgDoc["scenes"][activeSceneIdx];
                                    if (sc.contains("speakers") && sc["speakers"].is_array())
                                    {
                                        speakerNamesFromJson.reserve(sc["speakers"].size());
                                        for (const auto& s : sc["speakers"])
                                            speakerNamesFromJson.push_back(s.get<std::string>());
                                    }
                                }

                                // Labels: "0 - Rock", "1 - Ghoul", ...
                                std::vector<std::string> speakerLabels;
                                speakerLabels.reserve(speakerIDs.size());
                                for (int sid : speakerIDs)
                                {
                                    std::string label = std::to_string(sid);

                                    // Take speaker names from overrided values
                                    if (!d.speakerNameOverrides.empty())
                                    {
                                        if (sid >= 0 && sid < (int)d.speakerNameOverrides.size() && d.speakerNameOverrides[sid] != "")
                                        {
                                            label += " - " + d.speakerNameOverrides[sid];
                                        }
                                        else
                                        {
                                            label += " - (Untitled)";
                                        }
                                    }
                                    // Take speaker names from Dialogue.json
                                    else
                                    {
                                        if (sid >= 0 && sid < (int)speakerNamesFromJson.size() && speakerNamesFromJson[sid] != "")
                                        {
                                            label += " - " + speakerNamesFromJson[sid];
                                        }
                                        else
                                        {
                                            label += " - (Untitled)";
                                        }
                                    }

                                    speakerLabels.push_back(std::move(label));
                                }

                                std::vector<const char*> speakerItems;
                                speakerItems.reserve(speakerLabels.size());
                                for (auto& s : speakerLabels) speakerItems.push_back(s.c_str());

                                // Current selection -> index
                                int currentSID = d.customLineSpeakerIDs[i];
                                auto it = std::find(speakerIDs.begin(), speakerIDs.end(), currentSID);
                                int selIdx = (it == speakerIDs.end()) ? 0 : (int)std::distance(speakerIDs.begin(), it);

                                ImGui::SetNextItemWidth(100.f);
                                
                                // Dropdown for speaker ID per line
                                if (ImGui::Combo("Speaker", &selIdx, speakerItems.data(), (int)speakerItems.size()))
                                {
                                    d.customLineSpeakerIDs[i] = speakerIDs[selIdx];
                                }

                                // Multi-line editable field for line
                                {
                                    std::string lbl = "##LineText";
                                    char buf[512];
                                    strncpy(buf, lines[i].c_str(), sizeof(buf));
                                    buf[sizeof(buf) - 1] = '\0';

                                    if (ImGui::InputTextMultiline(lbl.c_str(), buf, IM_ARRAYSIZE(buf), ImVec2(-1, 60)))
                                    {
                                        lines[i] = buf;
                                        comp->SetLines(lines);
                                    }
                                }


                                ImGui::Separator();
                                ImGui::PopID();
                            }

                            if (ImGui::Button("Add Line"))
                            {
                                lines.push_back("");
                                comp->SetLines(lines);
                                d.customLineSpeakerIDs.push_back(0);
                                ensureSkinExists(0);
                            }
                            ImGui::SameLine();
                            ImGui::BeginDisabled(lines.size() <= 1);
                            if (ImGui::Button("Remove Last Line"))
                            {
                                lines.pop_back();
                                comp->SetLines(lines);

                                if (!d.customLineSpeakerIDs.empty())
                                    d.customLineSpeakerIDs.pop_back();
                            }
                            ImGui::EndDisabled();
                        }
                    }

                    // Speaker Skin Editor (custom mode only)
                    {
                        ImGui::SeparatorText("Selected Speaker Properties");

                        if (!customOverriden)
                        {
                            ImGui::TextDisabled("Locked (scene-driven).");
                        }
                        else
                        {
                            // Build list of speaker IDs
                            std::vector<int> speakerIDs;
                            speakerIDs.reserve(d.speakerSkins.size());
                            for (auto& [sid, _] : d.speakerSkins)
                            {
                                speakerIDs.push_back(sid);
                            }
                            if (speakerIDs.empty())
                            {
                                ensureSkinExists(0);
                                speakerIDs.push_back(0);
                            }
                            std::sort(speakerIDs.begin(), speakerIDs.end());

                            // Clamp selection
                            if (std::find(speakerIDs.begin(), speakerIDs.end(), selectedSpeakerID) == speakerIDs.end())
                                selectedSpeakerID = speakerIDs.front();

                            // Speaker selector
                            {
                                std::vector<std::string> speakerLabels;
                                speakerLabels.reserve(speakerIDs.size());
                                for (int sid : speakerIDs)
                                    speakerLabels.push_back(std::to_string(sid));

                                std::vector<const char*> items;
                                items.reserve(speakerLabels.size());
                                for (auto& s : speakerLabels) items.push_back(s.c_str());

                                int selIndex = 0;
                                for (int i = 0; i < (int)speakerIDs.size(); ++i)
                                    if (speakerIDs[i] == selectedSpeakerID) { selIndex = i; break; }

                                if (ImGui::Combo("SpeakerID", &selIndex, items.data(), (int)items.size()))
                                {
                                    selectedSpeakerID = speakerIDs[selIndex];
                                }

                                std::string& name = d.speakerNameOverrides[selectedSpeakerID];
                                char buf[128];
                                strncpy(buf, name.c_str(), sizeof(buf));
                                buf[sizeof(buf) - 1] = '\0';

                                ImGui::SetNextItemWidth(100.f);
                                if (ImGui::InputText("Speaker Name", buf, IM_ARRAYSIZE(buf)))
                                {
                                    name = buf;
                                }

                                ImGui::SameLine();

                                if (ImGui::Button("Add Speaker"))
                                {
                                    int newID = 0;
                                    while (d.speakerSkins.contains(newID)) ++newID;
                                    ensureSkinExists(newID);
                                    selectedSpeakerID = newID;
                                }

                                ImGui::SameLine();
                                ImGui::BeginDisabled(d.speakerSkins.size() <= 1);
                                if (ImGui::Button("Remove Selected"))
                                {
                                    d.speakerSkins.erase(selectedSpeakerID);
                                    if (d.speakerSkins.empty()) ensureSkinExists(0);
                                    selectedSpeakerID = d.speakerSkins.begin()->first;
                                }
                                ImGui::EndDisabled();
                            }

                            ensureSkinExists(selectedSpeakerID);
                            auto& skin = d.speakerSkins[selectedSpeakerID];

                            // Background
                            ImGui::Separator();
                            ImGui::Text("Background Render");
                            ImGui::Checkbox("BG Enabled", &skin.bgEnabled);
                            if (skin.bgEnabled)
                            {
                                ImGui::DragFloat("BG Width", &skin.bgBaseSize.x, 1.f);
                                ImGui::DragFloat("BG Height", &skin.bgBaseSize.y, 1.f);
                                drawUIRenderDataEditor("BG", skin.bgRenderData, skin.bgBaseSize, d.textBox.basePos, (d.ID * 100000) + selectedSpeakerID * 10 + 1, true);
                            }

                            // Profile
                            ImGui::Separator();
                            ImGui::Text("Profile Render");
                            ImGui::Checkbox("Profile Enabled", &skin.profileEnabled);
                            if (skin.profileEnabled)
                            {
                                ImGui::DragFloat("Profile Width", &skin.profileBaseSize.x, 1.f);
                                ImGui::DragFloat("Profile Height", &skin.profileBaseSize.y, 1.f);
                                ImGui::DragFloat("Profile Offset X", &skin.profileOffset.x, 1.f);
                                ImGui::DragFloat("Profile Offset Y", &skin.profileOffset.y, 1.f);

                                const char* sideItems[] = { "Auto (from speaker)", "Left", "Right" };
                                int displaySide = (int)d.profileDisplaySide;
                                if (ImGui::Combo("Profile Display Side", &displaySide, sideItems, IM_ARRAYSIZE(sideItems)))
                                    d.profileDisplaySide = (DialogueConfig::ProfileDisplaySide)displaySide;

                                drawUIRenderDataEditor("Profile", skin.profileRenderData, skin.profileBaseSize, d.textBox.basePos, (d.ID * 100000) + selectedSpeakerID * 10 + 2, true);
                            }

                            d.bgEnabled = skin.bgEnabled;
                            d.bgBaseSize = skin.bgBaseSize;
                            d.bgRenderData = skin.bgRenderData;

                            d.profileEnabled = skin.profileEnabled;
                            d.profileBaseSize = skin.profileBaseSize;
                            d.profileOffset = skin.profileOffset;
                            d.profileRenderData = skin.profileRenderData;
                        }
                    }

                    ImGui::Separator();

                    // Delete Dialogue
                    if (ImGui::Button("Delete This Dialogue"))
                    {
                        const int erasedID = dialogues[selectedLocal].ID;
                        dialogues.erase(dialogues.begin() + selectedLocal);

                        perDlgSceneIdxById.erase(erasedID);
                        perDlgColorOverrideById.erase(erasedID);
                        perDlgCustomOverrideById.erase(erasedID);
                        perDlgPrevCustomOverrideById.erase(erasedID);
                        perDlgSelectedSpeakerById.erase(erasedID);

                        perDlgOriginalSettingsCapturedById.erase(erasedID);
                        perDlgOriginalSettingsById.erase(erasedID);

                        uiSelectedIndex = -1;
                        uiSelectedType = SelectedType::None;
                        clampSelectionAfterErase();
                        goto _UI_PROPS_END_;
                    }
                }
            }

            // Display Properties
            else if (uiSelectedType == SelectedType::Display)
            {
                if (selectedLocal >= 0 && selectedLocal < (int)displays.size())
                {
                    auto& dp = displays[selectedLocal];

                    // Transform
                    {
                        ImGui::SeparatorText("Transform");
                        ImGui::DragFloat("posX", &dp.basePos.x, 1.f);
                        ImGui::DragFloat("posY", &dp.basePos.y, 1.f);
                        ImGui::DragFloat("sizeX", &dp.baseSize.x, 1.f);
                        ImGui::DragFloat("sizeY", &dp.baseSize.y, 1.f);
                        ImGui::Checkbox("Centered", &dp.centered);
                    }

                    // Render
                    {
                        ImGui::SeparatorText("Render");
                        const char* typeNames[] = { "None", "Shape", "CustomQuad", "Image", "ImageSheet" };
                        int currType = (int)dp.m_renderData.GetRenderType();
                        if (ImGui::Combo("RenderType", &currType, typeNames, IM_ARRAYSIZE(typeNames)))
                        {
                            dp.m_renderData.SetRenderType((UIRenderType)currType);
                        }

                        switch (dp.m_renderData.GetRenderType())
                        {
                        // Shape Render
                        case UIRenderType::Shape:
                        {
                            auto& shape = dp.m_renderData.GetShapeData();
                            const char* shapeNames[] = { "Rectangle", "Rounded", "Circle" };
                            int currShape = (int)shape.shape;
                            if (ImGui::Combo("Shape", &currShape, shapeNames, IM_ARRAYSIZE(shapeNames)))
                            {
                                shape.shape = (ButtonShape)currShape;
                            }

                            float col[3] = { shape.color.r, shape.color.g, shape.color.b };
                            if (ImGui::ColorEdit3("Color", col))
                            {
                                shape.color = { col[0], col[1], col[2] };
                            }
                        } break;

                        // Custom Quad Render
                        case UIRenderType::CustomQuad:
                        {
                            auto& quad = dp.m_renderData.GetQuadData();
                            ImGui::InputFloat2("v0", &quad.v0.x);
                            ImGui::InputFloat2("v1", &quad.v1.x);
                            ImGui::InputFloat2("v2", &quad.v2.x);
                            ImGui::InputFloat2("v3", &quad.v3.x);

                            if (ImGui::Button("Automatic quad by size"))
                            {
                                quad.v0 = { dp.basePos.x - (0.5f * dp.baseSize.x), dp.basePos.y - (0.5f * dp.baseSize.y) };
                                quad.v1 = { dp.basePos.x + (0.5f * dp.baseSize.x), dp.basePos.y - (0.5f * dp.baseSize.y) };
                                quad.v2 = { dp.basePos.x + (0.5f * dp.baseSize.x), dp.basePos.y + (0.5f * dp.baseSize.y) };
                                quad.v3 = { dp.basePos.x - (0.5f * dp.baseSize.x), dp.basePos.y + (0.5f * dp.baseSize.y) };
                            }
                            float col[3] = { quad.color.r, quad.color.g, quad.color.b };
                            if (ImGui::ColorEdit3("Color", col))
                            {
                                quad.color = { col[0], col[1], col[2] };
                            }
                        } break;

                        // Image Render
                        case UIRenderType::Image:
                        {
                            auto& imgData = dp.m_renderData.GetImageData();
                            Image& img = imgData.m_image;
                            const auto& cache = Render::GetInstance().Get_TextureCache();

                            static int selected = 0;
                            std::vector<const char*> imgOptions;
                            std::vector<GLuint> textureIDs;
                            imgOptions.reserve(cache.size());
                            for (auto const& [optname, optval] : cache)
                            {
                                imgOptions.push_back(optname.c_str());
                                textureIDs.push_back(optval);
                            }
                            for (size_t i = 0; i < textureIDs.size(); i++)
                            {
                                if (textureIDs[i] == img.textureID) { selected = (int)i; break; }
                            }
                            if (ImGui::Combo("Texture", &selected, imgOptions.data(), (int)imgOptions.size()))
                            {
                                img = CreateImage(imgOptions[selected]);
                            }

                            {
                                GLuint texID = img.textureID;
                                ImVec2 avail = ImGui::GetContentRegionAvail();
                                float scale = img.widthPixels ? (avail.x / img.widthPixels) : 0.f;
                                ImVec2 displaySize(img.widthPixels * scale, img.heightPixels * scale);
                                ImGui::Image((ImTextureID)(intptr_t)texID, displaySize, ImVec2(0, 1), ImVec2(1, 0));
                            }

                            ImGui::Text("Resize to aspect ratio:");
                            if (ImGui::Button("Width"))  dp.baseSize.x = dp.baseSize.y * ((float)img.widthPixels / (float)img.heightPixels);
                            ImGui::SameLine();
                            if (ImGui::Button("Height")) dp.baseSize.y = dp.baseSize.x * ((float)img.heightPixels / (float)img.widthPixels);
                        } break;

                        // Image Sheet Render
                        case UIRenderType::ImageSheet:
                        {
                            auto& sheet = dp.m_renderData.GetImageSheetData();
                            Image& img = sheet.m_image;

                            const auto& cache = Render::GetInstance().Get_TextureCache();
                            static int selected = 0;
                            std::vector<const char*> imgOptions;
                            std::vector<GLuint> textureIDs;
                            imgOptions.reserve(cache.size());
                            for (auto const& [optname, optval] : cache)
                            {
                                imgOptions.push_back(optname.c_str());
                                textureIDs.push_back(optval);
                            }
                            for (size_t i = 0; i < textureIDs.size(); i++)
                            {
                                if (textureIDs[i] == img.textureID) { selected = (int)i; break; }
                            }
                            if (ImGui::Combo("Texture Drawn", &selected, imgOptions.data(), (int)imgOptions.size()))
                            {
                                img = CreateImage(imgOptions[selected]);
                            }

                            // display
                            {
                                GLuint texID = img.textureID;
                                const float texW = (float)img.widthPixels;
                                const float texH = (float)img.heightPixels;

                                ImVec2 avail = ImGui::GetContentRegionAvail();

                                float scale = (texW != 0) ? avail.x / texW : 0.f;

                                ImVec2 displaySize(texW * scale, texH * scale);
                                ImVec2 imagePos = ImGui::GetCursorScreenPos();

                                ImGui::Image((ImTextureID)(intptr_t)texID, displaySize, ImVec2(0, 1), ImVec2(1, 0));

                                ImDrawList* draw = ImGui::GetWindowDrawList();

                                int rows = sheet.total_rows;
                                int cols = sheet.total_cols;

                                float cellW = displaySize.x / cols;
                                float cellH = displaySize.y / rows;

                                // vertical lines
                                for (int c = 1; c < cols; ++c)
                                {
                                    float x = imagePos.x + c * cellW;
                                    draw->AddLine(ImVec2(x, imagePos.y),
                                        ImVec2(x, imagePos.y + displaySize.y),
                                        IM_COL32(255, 255, 255, 128));
                                }

                                // horizontal lines
                                for (int r = 1; r < rows; ++r)
                                {
                                    float y = imagePos.y + r * cellH;
                                    draw->AddLine(ImVec2(imagePos.x, y),
                                        ImVec2(imagePos.x + displaySize.x, y),
                                        IM_COL32(255, 255, 255, 128));
                                }

                                // highlight
                                int r = sheet.frame_index.curr_row;
                                int c = sheet.frame_index.curr_col;

                                ImVec2 p0(
                                    imagePos.x + c * cellW,
                                    imagePos.y + r * cellH
                                );
                                ImVec2 p1(
                                    imagePos.x + (c + 1) * cellW,
                                    imagePos.y + (r + 1) * cellH
                                );

                                // Filled highlight
                                draw->AddRectFilled(p0, p1, IM_COL32(255, 255, 0, 80));

                                // Outline
                                draw->AddRect(p0, p1, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
                            }

                            // Render image based on selected row and column of the image sheet
                            ImGui::InputInt("Rows", (int*)&sheet.total_rows);
                            if (sheet.total_rows < 1) { sheet.total_rows = 1; }
                            ImGui::InputInt("Columns", (int*)&sheet.total_cols);
                            if (sheet.total_cols < 1) { sheet.total_cols = 1; }

                            ImGui::Separator();

                            ImGui::SliderInt("Current Row", (int*)&sheet.frame_index.curr_row, 0, (int)sheet.total_rows - 1);
                            ImGui::SliderInt("Current Col", (int*)&sheet.frame_index.curr_col, 0, (int)sheet.total_cols - 1);


                            sheet.total_cols = std::max(1u, sheet.total_cols);
                            sheet.total_rows = std::max(1u, sheet.total_rows);

                            // State Frames
                            if (ImGui::TreeNode("State Frames"))
                            {
                                for (auto& [state, idx] : sheet.frames)
                                {
                                    ImGui::PushID(state);

                                    ImGui::Text("State %u", state);

                                    ImGui::InputInt("Row", (int*)&idx.curr_row);
                                    ImGui::InputInt("Col", (int*)&idx.curr_col);

                                    idx.curr_row = std::clamp(idx.curr_row, 0u, sheet.total_rows - 1);
                                    idx.curr_col = std::clamp(idx.curr_col, 0u, sheet.total_cols - 1);

                                    if (ImGui::Button("Delete"))
                                        sheet.frames.erase(state);

                                    ImGui::Separator();
                                    ImGui::PopID();
                                }

                                static int newStateID = 0;
                                ImGui::InputInt("Add State", &newStateID);
                                if (ImGui::Button("Add"))
                                {
                                    sheet.frames[newStateID] = { 0,0 };
                                }

                                ImGui::TreePop();
                            }

                            // Aspect helpers (use frame size)
                            int frameW = img.widthPixels / std::max(1u, sheet.total_cols);
                            int frameH = img.heightPixels / std::max(1u, sheet.total_rows);
                            ImGui::Text("Resize to aspect ratio:");
                            if (ImGui::Button("Width"))  dp.baseSize.x = dp.baseSize.y * (float(frameW) / float(frameH));
                            ImGui::SameLine();
                            if (ImGui::Button("Height")) dp.baseSize.y = dp.baseSize.x * (float(frameH) / float(frameW));
                        } break;

                        default:
                            ImGui::TextDisabled("No render info.");
                            break;
                        }
                    }
                    ImGui::Separator();

                    // Delete Display
                    if (ImGui::Button("Delete This Display"))
                    {
                        displays.erase(displays.begin() + selectedLocal);
                        uiSelectedIndex = -1;
                        uiSelectedType = SelectedType::None;
                        clampSelectionAfterErase();
                        goto _UI_PROPS_END_;
                    }
                }
            }
        }
    _UI_PROPS_END_:
        ImGui::EndChild();

        // UI GROUPS
        // Manage groups of UI entities
        ImGui::SeparatorText("UI Groups");
        ImGui::BeginChild("Group Properties", ImVec2(0, 0), true);

        // Helper to work with IDs of various UI types
        auto getIDFromGlobal = [&](int globalIdx) -> int
            {
                SelectedType t; int li;
                globalToLocal(globalIdx, t, li);
                if (t == SelectedType::Button) { if (li >= 0 && li < (int)buttons.size())   return buttons[li].ID; }
                if (t == SelectedType::TextBox) { if (li >= 0 && li < (int)textboxes.size()) return textboxes[li].ID; }
                if (t == SelectedType::Dialogue) { if (li >= 0 && li < (int)dialogues.size()) return dialogues[li].ID; }
                if (t == SelectedType::Display) { if (li >= 0 && li < (int)displays.size()) return displays[li].ID; }
                return -1;
            };

        struct FoundRef 
        { 
            SelectedType type = SelectedType::None; 
            int local = -1; 
        };

        // Find UI element by ID
        auto findByID = [&](int id) -> FoundRef
            {
                if (id <= 0) return {};
                for (int i = 0; i < (int)buttons.size(); ++i)   if (buttons[i].ID == id)    return { SelectedType::Button,   i };
                for (int i = 0; i < (int)textboxes.size(); ++i) if (textboxes[i].ID == id)  return { SelectedType::TextBox,  i };
                for (int i = 0; i < (int)dialogues.size(); ++i) if (dialogues[i].ID == id)  return { SelectedType::Dialogue, i };
                for (int i = 0; i < (int)displays.size(); ++i) if (displays[i].ID == id)    return { SelectedType::Display,  i };
                return {};
            };

        // UI Groups

        std::vector<UIGroup>& s_groups = GuiSystem::Get().GetGroups();
        static int s_activeGroup = -1;
        static int s_nextGroupID = 1;

        // Prune stale members (elements deleted externally)
        auto pruneMissingMembers = [&](UIGroup& g)
            {
                g.memberIDs.erase(std::remove_if(g.memberIDs.begin(), g.memberIDs.end(),
                    [&](int id) { return findByID(id).type == SelectedType::None; }), g.memberIDs.end());
            };

        // Row: create/select group
        {
            if (ImGui::Button("New Group from Selected"))
            {
                int selID = getIDFromGlobal(uiSelectedIndex);
                if (selID > 0)
                {
                    UIGroup g;
                    g.groupID = s_nextGroupID++;
                    g.groupName = "Group " + std::to_string(g.groupID);
                    g.memberIDs.push_back(selID);
                    s_groups.push_back(std::move(g));
                    s_activeGroup = (int)s_groups.size() - 1;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Ungroup") && s_activeGroup >= 0 && s_activeGroup < (int)s_groups.size())
            {
                s_groups.erase(s_groups.begin() + s_activeGroup);
                s_activeGroup = -1;
            }

            // Group selector
            {
                std::vector<std::string> gnames; gnames.reserve(s_groups.size());
                for (auto& g : s_groups) gnames.push_back(g.groupName);
                std::vector<const char*> gitems; gitems.reserve(gnames.size());
                for (auto& s : gnames) gitems.push_back(s.c_str());

                int curr = s_activeGroup;
                ImGui::SetNextItemWidth(100.f);
                if (ImGui::Combo("##GroupSelect", &curr, gitems.data(), (int)gitems.size()))
                    s_activeGroup = curr;
                ImGui::SameLine();
                ImGui::TextUnformatted("Select Group");
            }
        }

        // Active group panel
        if (s_activeGroup >= 0 && s_activeGroup < (int)s_groups.size())
        {
            UIGroup& grp = s_groups[s_activeGroup];
            pruneMissingMembers(grp);

            // Editable name
            {
                char buf[128];
                std::snprintf(buf, sizeof(buf), "%s", grp.groupName.c_str());
                if (ImGui::InputText("Group Name", buf, IM_ARRAYSIZE(buf)))
                    grp.groupName = buf;
            }

            // Membership controls
            {
                if (ImGui::Button("Add Selected to Group"))
                {
                    int selID = getIDFromGlobal(uiSelectedIndex);
                    if (selID > 0)
                    {
                        if (std::find(grp.memberIDs.begin(), grp.memberIDs.end(), selID) == grp.memberIDs.end())
                            grp.memberIDs.push_back(selID);
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove Selected from Group"))
                {
                    int selID = getIDFromGlobal(uiSelectedIndex);
                    if (selID > 0)
                    {
                        auto it = std::remove(grp.memberIDs.begin(), grp.memberIDs.end(), selID);
                        grp.memberIDs.erase(it, grp.memberIDs.end());
                    }
                }

                // Member list
                if (ImGui::TreeNode("Members"))
                {
                    int removeAt = -1;
                    for (int i = 0; i < (int)grp.memberIDs.size(); ++i)
                    {
                        int id = grp.memberIDs[i];
                        FoundRef found = findByID(id);
                        std::string label = "#" + std::to_string(id) + "  ";
                        if (found.type == SelectedType::Button)   label += "[Button]  " + buttons[found.local].label;
                        else if (found.type == SelectedType::TextBox)  label += "[Text]    " + textboxes[found.local].text;
                        else if (found.type == SelectedType::Dialogue) label += "[Dialogue]";
                        else if (found.type == SelectedType::Display) label += "[Display]";
                        else label += "(missing)";

                        ImGui::BulletText("%s", label.c_str());
                        ImGui::SameLine();
                        ImGui::PushID(i);
                        if (ImGui::SmallButton("Select"))
                        {
                            // Jump selection to this element
                            if (found.type != SelectedType::None)
                                uiSelectedIndex = localToGlobal(found.type, found.local);
                        }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("Remove")) removeAt = i;
                        ImGui::PopID();
                    }
                    if (removeAt >= 0 && removeAt < (int)grp.memberIDs.size())
                        grp.memberIDs.erase(grp.memberIDs.begin() + removeAt);

                    ImGui::TreePop();
                }
            }

            ImGui::Separator();

            // ===== Bulk edit =====
            ImGui::TextDisabled("Bulk Edit");

            // Visibility toggle (apply immediately)
            if (ImGui::Button("Toggle Visibility (All Members)"))
            {
                // Compute new state: if any visible -> hide all, else show all
                bool anyVisible = false;
                for (int id : grp.memberIDs)
                {
                    FoundRef found = findByID(id);
                    if (found.type == SelectedType::Button) { if (buttons[found.local].isVisible) { anyVisible = true; break; } }
                    if (found.type == SelectedType::TextBox) { if (textboxes[found.local].isVisible) { anyVisible = true; break; } }
                    if (found.type == SelectedType::Dialogue) { if (dialogues[found.local].isVisible) { anyVisible = true; break; } }
                    if (found.type == SelectedType::Display) { if (displays[found.local].isVisible) { anyVisible = true; break; } }
                }
                bool newState = !anyVisible;
                for (int id : grp.memberIDs)
                {
                    FoundRef found = findByID(id);
                    if (found.type == SelectedType::Button) buttons[found.local].isVisible = newState;
                    else if (found.type == SelectedType::TextBox)  textboxes[found.local].isVisible = newState;
                    else if (found.type == SelectedType::Dialogue) dialogues[found.local].isVisible = newState;
                    else if (found.type == SelectedType::Display) displays[found.local].isVisible = newState;
                }
            }

            // Transform offsets (applied on click)
            static float g_dx = 0.f, g_dy = 0.f;
            static float g_uniformScale = 1.f;
            static float g_textScaleMul = 1.f;
            ImGui::DragFloat("Transform X", &g_dx, 1.0f);
            ImGui::DragFloat("Transform Y", &g_dy, 1.0f);
            ImGui::DragFloat("Uniform Scale", &g_uniformScale, 0.01f, 0.1f, 10.0f);
            ImGui::DragFloat("Text Scale", &g_textScaleMul, 0.01f, 0.1f, 10.0f);
            if (ImGui::Button("Apply Transform to Group"))
            {
                for (int id : grp.memberIDs)
                {
                    FoundRef found = findByID(id);
                    switch (found.type)
                    {
                    case SelectedType::Button:
                    {
                        auto& b = buttons[found.local];
                        b.basePos.x += g_dx; b.basePos.y += g_dy;
                        b.baseSize.x *= g_uniformScale; b.baseSize.y *= g_uniformScale;
                        b.baseTextScale *= g_textScaleMul;
                    } break;
                    case SelectedType::TextBox:
                    {
                        auto& t = textboxes[found.local];
                        t.basePos.x += g_dx; t.basePos.y += g_dy;
                        t.baseScale *= g_uniformScale;
                        t.baseScale *= g_textScaleMul;
                    } break;
                    case SelectedType::Dialogue:
                    {
                        auto& d = dialogues[found.local];
                        d.textBox.basePos.x += g_dx; d.textBox.basePos.y += g_dy;
                        d.textBox.baseScale *= g_uniformScale;
                        d.textBox.baseScale *= g_textScaleMul;
                    } break;
                    case SelectedType::Display:
                    {
                        auto& dp = displays[found.local];
                        dp.basePos.x += g_dx; dp.basePos.y += g_dy;
                        dp.baseSize.x *= g_uniformScale; dp.baseSize.y *= g_uniformScale;
                    } break;
                    default: break;
                    }
                }
            }

            ImGui::Separator();

            // ===== Group actions =====
            // CLONE GROUP
            if (ImGui::Button("Clone Group"))
            {
                // auto& gui = GuiSystem::Get();

                gui.ReseedNextUIID();

                const float kOffX = 20.f, kOffY = -20.f;

                // collect IDs of cloned elements to form new group (cloned)
                std::vector<int> newMemberIDs;
                newMemberIDs.reserve(grp.memberIDs.size());

                for (int id : grp.memberIDs)
                {
                    FoundRef found = findByID(id);
                    switch (found.type)
                    {
                    case SelectedType::Button:
                    {
                        ButtonConfig c = buttons[found.local];
                        c.ID = 0;
                        c.label += " (Copy)";
                        c.basePos.x += kOffX; c.basePos.y += kOffY;
                        gui.AddButton(c);
                        newMemberIDs.push_back(gui.Buttons().back().ID);
                    } break;

                    case SelectedType::TextBox:
                    {
                        TextBoxConfig t = textboxes[found.local];
                        t.ID = 0;
                        t.text += " (Copy)";
                        t.basePos.x += kOffX; t.basePos.y += kOffY;
                        gui.AddTextBox(t);
                        newMemberIDs.push_back(gui.TextBoxes().back().ID);
                    } break;

                    case SelectedType::Dialogue:
                    {
                        const auto& src = dialogues[found.local];
                        DialogueConfig dst = src;
                        dst.ID = 0;

                        // Deep copy text component if present
                        const TextSystem::TextComponent* sc = src.textComp ? src.textComp : src.owned.get();
                        if (sc)
                        {
                            dst.owned = std::make_shared<TextSystem::TextComponent>();
                            dst.owned->SetLines(sc->GetLines());
                            dst.owned->SetCurrentLineIndex(sc->GetCurrentLineIndex());
                            dst.owned->SetIsTyping(false);
                            dst.textComp = dst.owned.get();

                            dst.bgEnabled = src.bgEnabled;
                            dst.bgBaseSize = src.bgBaseSize;
                            dst.bgRenderData = src.bgRenderData;

                            dst.profileEnabled = src.profileEnabled;
                            dst.profileBaseSize = src.profileBaseSize;
                            dst.profileOffset = src.profileOffset;
                            dst.profileRenderData = src.profileRenderData;
                        }
                        else
                        {
                            dst.textComp = nullptr;
                        }

                        dst.textBox.basePos.x += kOffX; dst.textBox.basePos.y += kOffY;
                        gui.AddDialogue(dst);
                        newMemberIDs.push_back(gui.Dialogues().back().ID);
                    } break;

                    case SelectedType::Display:
                    {
                        DisplayConfig dp = displays[found.local];
                        dp.ID = 0;
                        dp.basePos.x += kOffX; dp.basePos.y += kOffY;
                        gui.AddDisplay(dp);
                        newMemberIDs.push_back(gui.Displays().back().ID);
                    } break;

                    default: break;
                    }
                }

                // Create a new group for the cloned UI elements
                if (!newMemberIDs.empty())
                {
                    UIGroup newGrp;
                    newGrp.groupID = s_nextGroupID++;
                    newGrp.groupName = grp.groupName.empty() ? 
                        ("Group " + std::to_string(newGrp.groupID)) :(grp.groupName + " (Copy)");
                    newGrp.memberIDs = std::move(newMemberIDs);
                    s_groups.push_back(std::move(newGrp));
                    s_activeGroup = (int)s_groups.size() - 1;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete Group"))
            {
                // Buttons
                for (int i = (int)buttons.size() - 1; i >= 0; --i)
                {
                    if (std::find(grp.memberIDs.begin(), grp.memberIDs.end(), buttons[i].ID) != grp.memberIDs.end())
                        buttons.erase(buttons.begin() + i);
                }
                // TextBoxes
                for (int i = (int)textboxes.size() - 1; i >= 0; --i)
                {
                    if (std::find(grp.memberIDs.begin(), grp.memberIDs.end(), textboxes[i].ID) != grp.memberIDs.end())
                        textboxes.erase(textboxes.begin() + i);
                }
                // Dialogues
                for (int i = (int)dialogues.size() - 1; i >= 0; --i)
                {
                    if (std::find(grp.memberIDs.begin(), grp.memberIDs.end(), dialogues[i].ID) != grp.memberIDs.end())
                        dialogues.erase(dialogues.begin() + i);
                }
                // Displays
                for (int i = (int)displays.size() - 1; i >= 0; --i)
                {
                    if (std::find(grp.memberIDs.begin(), grp.memberIDs.end(), dialogues[i].ID) != grp.memberIDs.end())
                        displays.erase(displays.begin() + i);
                }

                // Clear group selection & definition (since the elements are gone)
                s_groups.erase(s_groups.begin() + s_activeGroup);
                s_activeGroup = -1;

                uiSelectedType = SelectedType::None;
                uiSelectedIndex = -1;
            }
        }
        else
        {
            ImGui::TextDisabled("No active group.");
        }
        ImGui::EndChild();

        // Refresh UI element counts
        numBtn = (int)buttons.size();
        numTxt = (int)textboxes.size();
        numDlg = (int)dialogues.size();
        numDisp = (int)displays.size();
        total = numBtn + numTxt + numDlg + numDisp;
        {
            // Build a flat list of all UI elements for the dropdown
            static int comboIndex = -1;
            total = totalCount();

            static std::vector<std::string> labels;
            labels.clear();
            labels.reserve(total);

            // Buttons
            for (int i = 0; i < (int)buttons.size(); ++i)
            {
                std::string name = "Button: " + buttons[i].label;
                labels.push_back(std::move(name));
            }

            // TextBoxes
            for (int i = 0; i < (int)textboxes.size(); ++i)
            {
                std::string name = "Text: " + textboxes[i].text;
                labels.push_back(std::move(name));
            }

            // Dialogues
            for (int i = 0; i < (int)dialogues.size(); ++i)
            {
                std::string label = "Dialogue";
                auto* comp = dialogues[i].textComp ? dialogues[i].textComp : dialogues[i].owned.get();
                if (comp && !comp->GetLines().empty())
                {
                    const auto& ln = comp->GetLines()[comp->GetCurrentLineIndex()];
                    label += ": " + (ln.size() > 12 ? (ln.substr(0, 12) + "...") : ln);
                }
                labels.push_back(std::move(label));
            }

            std::vector<const char*> items;
            items.reserve(labels.size());
            for (auto& s : labels) items.push_back(s.c_str());

            if (uiSelectedIndex >= 0 && uiSelectedIndex < (int)items.size())
                comboIndex = uiSelectedIndex;
            else if (total == 0)
                comboIndex = -1;

            // UI Elements
            if (ImGui::Combo("UI Elements", &comboIndex,
                items.data(), (int)items.size()))
            {
                if (comboIndex >= 0 && comboIndex < total)
                {
                    uiSelectedIndex = comboIndex;
                    SelectedType type; int lIdx;
                    globalToLocal(uiSelectedIndex, type, lIdx);
                    uiSelectedType = type;
                }
                else
                {
                    uiSelectedType = SelectedType::None;
                    uiSelectedIndex = -1;
                }
            }
        }
        // Refresh UI element counts
        numBtn = (int)buttons.size();
        numTxt = (int)textboxes.size();
        numDlg = (int)dialogues.size();
        numDisp = (int)displays.size();
        total = numBtn + numTxt + numDlg + numDisp;

        ImGui::NextColumn();
        //ShowAddRemoveComponents(); // right: add/remove comp
        // UI COMPONENTS?
        ImGui::Columns(1);

        ImGui::Separator();
        //ShowInspectorWindow();     //edit
        // UI EDITOR

        ImGui::Separator();
        //ShowPrefabControls();      //prefabs
        if ( !sm.inEditMode) ImGui::EndDisabled();
    }

    ImGui::End();

    if (ImGui::Begin("Assets")) {
        if ( !sm.inEditMode) ImGui::BeginDisabled();
        ShowAssetPreviewWindow();
        if ( !sm.inEditMode) ImGui::EndDisabled();

    }
    ImGui::End();


    // game view panel
    if (ImGui::Begin("Game View")) {
        bool isPlaying =  !sm.inEditMode;
        ShowSimulationControls(isPlaying);
        isPlaying =  !sm.inEditMode;
        ImVec2 avail = ImGui::GetContentRegionAvail();

        //16:9 ratio
        float aspect = 16.0f / 9.0f;
        float newWidth = avail.x;
        float newHeight = newWidth / aspect;
        if (newHeight > avail.y) {
            newHeight = avail.y;
            newWidth = newHeight * aspect;
        }

        ImVec2 cursor = ImGui::GetCursorScreenPos();

        //texture from renderer
        ::Render& renderer = Render::GetInstance();
        GLuint texID = renderer.GetSceneTexture();

        // show it
        ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(newWidth, newHeight), ImVec2(0, 1), ImVec2(1, 0));

        DrawSelectionOutline(selectedEntity, cursor, newWidth, newHeight);

        //gizmooooo
        ImGuizmo::BeginFrame();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(cursor.x, cursor.y, newWidth, newHeight);

        static glm::vec3 lastScale(1.0f);
        static bool wasGizmoUsing = false;
        Registry& reg = Registry::Get();
        if (selectedEntity != INVALID_ENTITY &&
            reg.Has_Component<TransformComponent>(selectedEntity))
        {
            TransformComponent& t = reg.Get_Component<TransformComponent>(selectedEntity);

            // Build model matrix TRSSSS
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(t.pos.x, t.pos.y, 0.0f));
            model = glm::rotate(model, glm::radians(t.rot), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(t.scale.x, t.scale.y, 1.0f));

            //cam for gizmo
            Camera& cam = Camera::GetInstance();
            glm::mat4 view = cam.GetViewMatrix();
            glm::mat4 proj = cam.GetProjMatrix();

            ImGuizmo::SetOrthographic(true);


            ImGuizmo::Manipulate(
                glm::value_ptr(view),
                glm::value_ptr(proj),
                gizmoOperation, // move/rot/scale
                gizmoMode, //local/world
                glm::value_ptr(model)
            );
            //check if using gizmo
            bool isUsingNow = ImGuizmo::IsUsing();

            //UNDOOOOO
            if (isUsingNow && !wasGizmoUsing)
            {
                UndoSystem::Get().BeginModify(selectedEntity, t);
                lastScale = glm::vec3(1.0f);
            }


            if (isUsingNow)
            {
                glm::vec3 translation, rotation3, scaleAbs;
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(model),
                    glm::value_ptr(translation),
                    glm::value_ptr(rotation3),
                    glm::value_ptr(scaleAbs)
                );

                // pos
                t.pos.x = translation.x;
                t.pos.y = translation.y;

                //rototo
                if (gizmoOperation == ImGuizmo::ROTATE)
                {
                    ImVec2 mouseScreen = ImGui::GetMousePos();
                    //mouse from screen to NDC
                    float u = (mouseScreen.x - cursor.x) / newWidth;
                    float v = (mouseScreen.y - cursor.y) / newHeight;

                    float ndcX = u * 2.0f - 1.0f;
                    float ndcY = (1.0f - v) * 2.0f - 1.0f;

                    //NDC to world
                    Vec2 mouseWorld = cam.NDCToWorld({ ndcX, ndcY });
                    //mouse rototo angle stuff
                    float targetRot = AngleToMouse(t.pos, mouseWorld);
                    t.rot = SmoothDampAngle(t.rot, targetRot, 0.18f);
                }
                //scale
                t.scale.x = scaleAbs.x;
                t.scale.y = scaleAbs.y;

                //collider nid to sync
                if (reg.Has_Component<ColliderComponent>(selectedEntity))
                {
                    auto& col = reg.Get_Component<ColliderComponent>(selectedEntity);
                    col.UpdateHitboxes(t.pos, t.scale);
                }
            }

            if (!isUsingNow && wasGizmoUsing)
            {
                UndoSystem::Get().EndModify(selectedEntity, t);
            }
            //track prev frame state
            wasGizmoUsing = isUsingNow;
        }



        if (!isPlaying && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (!ImGuizmo::IsUsing())
            {
                // mouse position to world position
                ImVec2 mousePos = ImGui::GetMousePos();

                ImVec2 imgMin = cursor;
                ImVec2 imgMax = ImVec2(cursor.x + newWidth, cursor.y + newHeight);

                //if the mouse is actually inside the scene preview rectangle
                if (mousePos.x >= imgMin.x && mousePos.x <= imgMax.x && mousePos.y >= imgMin.y && mousePos.y <= imgMax.y)
                {
                    //conversion again
                    float u = (mousePos.x - imgMin.x) / (imgMax.x - imgMin.x);
                    float v = (mousePos.y - imgMin.y) / (imgMax.y - imgMin.y);

                    //imgui top left so converttt
                    float ndcX = u * 2.0f - 1.0f;
                    float ndcY = (1.0f - v) * 2.0f - 1.0f;

                    Camera& cam = Camera::GetInstance();
                    Vec2 world = cam.NDCToWorld({ ndcX, ndcY });

                    selectedEntity = PickEntityAtWorldPos(world);
                }
            }
        }


        // =============================================
        // DRAG-DROP TARGET: Drop images into Game View
        // =============================================
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_IMAGE"))
            {
                const char* droppedPath = (const char*)payload->Data;
                std::string texPath = droppedPath;

                std::cout << "[Editor] Dropped image: " << texPath << "\n";

                // --- Convert mouse position (on Game View image) -> world coords ---
                ImVec2 mouse = ImGui::GetMousePos();
                ImVec2 imgMin = cursor; // top-left of ImGui::Image
                ImVec2 imgMax = ImVec2(cursor.x + newWidth, cursor.y + newHeight);

                float u = (mouse.x - imgMin.x) / (imgMax.x - imgMin.x);
                float v = (mouse.y - imgMin.y) / (imgMax.y - imgMin.y);

                // to NDC, flip Y
                float ndcX = u * 2.0f - 1.0f;
                float ndcY = (1.0f - v) * 2.0f - 1.0f;

                Camera& cam = Camera::GetInstance();
                Vec2 world = cam.NDCToWorld({ ndcX, ndcY });

                // --- Create entity + components ---
                Entity e = reg.Create_Entity();

                // Transform
                TransformComponent t;
                t.pos = world;
                reg.Add_Component<TransformComponent>(e, t);
                RenderComponent rc;
                rc.SetRenderType(RT_IMAGE);
                // Sprite (uses Set_Image / Set_Size)
                Sprite::SpriteComponent sprite;

                Image img = CreateImage(texPath);
                if (img.IsValid())
                {
                    sprite.Set_Image(img);
                    // Auto-size to the actual texture size
                    //sprite.Set_Size(Vec2((float)img.widthPixels, (float)img.heightPixels));
                    sprite.Set_Size({ 10.f,10.f });
                }
                else
                {
                    // Fallback size if texture load fails
                    sprite.Set_Size(Vec2(100.f, 100.f));
                    std::cerr << "[Editor] Warning: Failed to load image; using default 100x100.\n";
                }
                rc.SetSprite(sprite);
                reg.Add_Component<RenderComponent>(e, rc);
               // reg.Add_Component<Sprite::SpriteComponent>(e, sprite);

                ColliderComponent col;

                // image size in world units
                Vec2 imgPixels = sprite.Get_ImageSize(); // pixel
                Vec2 baseWorldSize = imgPixels * RenderSystem::unitsPerPixel * 10;  // convert to world units

                col.aabbhitbox.dim = baseWorldSize;

                // UpdateHitboxes apply TransformComponent scale and set min/max
                //col.UpdateHitboxes(t.pos, t.scale);

                col.SetActive(true);
                col.SetTrigger(false);

                reg.Add_Component<ColliderComponent>(e, col);


                // Sync hitboxes
                auto& cRef = reg.Get_Component<ColliderComponent>(e);
                cRef.UpdateHitboxes(t.pos, t.scale);

                // Optional: give a friendly name
                reg.Get_EntityNameMap()[e] = std::filesystem::path(texPath).stem().string();

                // Select it
                ImGuiSystem::selectedEntity = e;
                UndoSystem::Get().RecordAdd(e);
                std::cout << "[Editor] Spawned image entity at (" << world.x << ", " << world.y << ")\n";
            }

            ImGui::EndDragDropTarget();
        }
        
        //GUISYSTEM STUFF SCALE FOR MOUSE
        //if (!isPlaying) {
        if (guiEnabled) {
            GameViewRect rect;
            rect.x = cursor.x;
            rect.y = cursor.y;
            rect.width = newWidth;
            rect.height = newHeight;
            GuiSystem::Get().SetGameViewRect(rect);
        }

        //if (dragEditMode) {
        //    DrawEntityDragHandles(cursor, ImVec2(newWidth, newHeight));
        //}
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && selectedEntity != INVALID_ENTITY)
        {
            SavedEntity snapshot = reg.Save_Entity(selectedEntity);
            UndoSystem::Get().RecordRemove(snapshot);
            reg.Remove_Entity(selectedEntity);

            std::cout << "[Editor] Deleted entity " << selectedEntity << " via Delete key\n";
            selectedEntity = INVALID_ENTITY;
        }
        // ----------------------
        // Prefab Mouse Spawner
        // ----------------------

        if (isPlacingPrefab && ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                // mouse in screen space
                ImVec2 mouse = ImGui::GetMousePos();

                // the image rect in screen space
                ImVec2 imgMin = cursor; // top-left of the ImGui::Image
                ImVec2 imgMax = ImVec2(cursor.x + newWidth, cursor.y + newHeight);   // bottom-right

                // panel-local uv in [0,1]
                float u = (mouse.x - imgMin.x) / (imgMax.x - imgMin.x);
                float v = (mouse.y - imgMin.y) / (imgMax.y - imgMin.y);

                //texture with flipped UVs (0,1) ->(1,0),
                //v grows downward in screen space but NDC up to be y flip v here.
                float ndcX = u * 2.0f - 1.0f; // 0,1 to -1,1
                float ndcY = (1.0f - v) * 2.0f - 1.0f;

                //ndc to world
                Camera& cam = Camera::GetInstance();

                Vec2 world = cam.NDCToWorld({ ndcX, ndcY });
                float worldX = world.x;
                float worldY = world.y;


                //loads prefab and spawn.
                Entity newEnt = JsonLoad::LoadEntityFromFile(pendingPrefabPath, worldX, worldY);
                if (newEnt != INVALID_ENTITY)
                {
                    selectedEntity = newEnt;
                    UndoSystem::Get().RecordAdd(newEnt);
                    // Register entity name so it can be found during prefab updates
                    std::filesystem::path path(pendingPrefabPath);
                    std::string prefabName = path.stem().string();
                    Registry::Get().Get_EntityNameMap()[newEnt] = prefabName;
                    std::cout << "[Editor] Spawned prefab at (" << worldX << ", " << worldY << ")\n";
                }
                else
                {
                    std::cerr << "[Editor] Failed to spawn prefab!\n";
                }
            }

            if (Input::IsKeyPressed(GLFW_KEY_0))
            {
                isPlacingPrefab = false;
                pendingPrefabPath.clear();
                std::cout << "[Editor] Prefab placement canceled.\n";
            }
        }


    }
    ImGui::End();

    ImGui::End();
}


// called every frame after Update() to actually render ImGui to screen in core engine!!!!!!!!!!!!!!!!!!!!
void ImGuiSystem::Render() {
    if (!guiEnabled) return; //skip if GUI is off
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); //draw to screen
}

void ImGuiSystem::ShowEngineDebugWindow() {

    CoreEngine& engine = CoreEngine::GetInstance();

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();


    ImGui::SeparatorText("Update Systems");

    double totalUpdate = 0.0;
    for (auto& [name, t] : engine.GetUpdateTimings())
        totalUpdate += t;

    // Copy into sortable vector
    std::vector<std::pair<std::string, double>> sortedUpdate(
        engine.GetUpdateTimings().begin(),
        engine.GetUpdateTimings().end()
    );

    // sort by system name
    std::sort(sortedUpdate.begin(), sortedUpdate.end(),
        [](auto& a, auto& b) { return a.first < b.first; }
    );

    // Draw update timings
    for (auto& [name, t] : sortedUpdate)
    {
        double pct = (totalUpdate > 0.0) ? (t / totalUpdate) * 100.0 : 0.0;

        ImGui::Text("%s: %.3f ms (%.1f%%)", name.c_str(), t, pct);
        ImGui::ProgressBar((float)(pct / 100.0f), ImVec2(-1, 0));
    }

    ImGui::SeparatorText("Render Systems");

    double totalRender = 0.0;
    for (auto& [name, t] : engine.GetRenderTimings())
        totalRender += t;

    // Copy into sortable vector
    std::vector<std::pair<std::string, double>> sortedRender(
        engine.GetRenderTimings().begin(),
        engine.GetRenderTimings().end()
    );

    // sort by system name
    std::sort(sortedRender.begin(), sortedRender.end(),
        [](auto& a, auto& b) { return a.first < b.first; }
    );

    // Draw render timings
    for (auto& [name, t] : sortedRender)
    {
        double pct = (totalRender > 0.0) ? (t / totalRender) * 100.0 : 0.0;

        ImGui::Text("%s: %.3f ms (%.1f%%)", name.c_str(), t, pct);
        ImGui::ProgressBar((float)(pct / 100.0f), ImVec2(-1, 0));
    }


    /*ImGui::Separator();
    ImGui::Checkbox("Enable Drag Handles", &dragEditMode);*/
    ImGui::Separator();
    ImGui::SeparatorText("Window Mode");

    bool isFullscreen = GLApp::IsFullscreen();
    if (ImGui::Checkbox("Fullscreen (Alt+Enter)", &isFullscreen))
    {
        GLApp::ToggleFullscreen();
    }

    ImGui::Separator();
}

//collili
void ImGuiSystem::ShowCollisionDebugWindow() {
    // count active colliders
    int activeColliders = 0;
    int totalColliders = 0;

    for (auto& [entity, collider] : m_registry.Get_ComponentMap<ColliderComponent>()) {
        totalColliders++;
        if (collider.IsActive()) {
            activeColliders++;
        }
    }

    ImGui::Columns(2);

    ImGui::Text("AABB Colliders: %d / %d active", activeColliders, totalColliders);
    ImGui::Checkbox("Show Colliders", &showColliders);
    ImGui::Checkbox("Show Velocity", &showVelocity);
    ImGui::Checkbox("Show Spatial Grid", &showSpatialGrid);

    // Spatial Grid Statistics
    // ImGui::Separator();
    ImGui::NextColumn();
    ImGui::Text("Spatial Grid (Broad Phase):");
    SpatialGrid* grid = CollisionManager::GetSpatialGrid();
    if (grid) {
        auto stats = grid->GetStats();
        size_t collisionCellCount = grid->GetCollisionCellCount();
        // Cell size is 150.0 by default
        ImGui::Text("  Cell Size: 150.0 units");
        ImGui::Text("  Occupied Cells: %llu", (unsigned long long)stats.occupiedCells);
        ImGui::Text("  Total Entities: %llu", (unsigned long long)stats.totalEntities);
        ImGui::Text("  Avg per Cell: %.2f", stats.avgEntitiesPerCell);
        ImGui::Text("  Max in Cell: %llu", (unsigned long long)stats.maxEntitiesInCell);

        // Collision cells (shown in red when visualized)
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
            "  Cells with Collisions: %llu", (unsigned long long)collisionCellCount);

        // Performance estimation
        int naiveChecks = (totalColliders * (totalColliders - 1)) / 2;
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
            "  Optimization: %d -> ~%d checks",
            naiveChecks,
            (int)(stats.occupiedCells * stats.avgEntitiesPerCell * 9));
    }
    ImGui::Columns(1);
    //ImGui::Separator(); 
    ShowLayerCollisionMatrix(); 
    ImGui::Separator();
    //ImGui::Text("Scene Management:");
    //ShowSceneBrowser();

}

void ImGuiSystem::ShowSceneBrowser()
{
    ImGui::Columns(2);
    ImGui::SeparatorText("Scene Management");
    //same as earlier thing
    namespace fs = std::filesystem;
    const std::string sceneFolder = "../Assets/saves/";

    if (!fs::exists(sceneFolder))
        fs::create_directories(sceneFolder);

    static std::vector<std::string> sceneFiles;
    static bool needRefresh = true;


    //refresh button
    if (ImGui::Button("Refresh Scenes"))
    {
        needRefresh = true;
    }

    //ImGui::Separator();

    if (needRefresh)
    {
        sceneFiles.clear();
        for (auto& entry : fs::directory_iterator(sceneFolder))
        {
            if (entry.path().extension() == ".json")
                sceneFiles.push_back(entry.path().stem().string());
        }
        needRefresh = false;
        std::cout << "[Editor] Scene list rebuilt (" << sceneFiles.size() << " found)\n";
    }
    static int selectedScene = -1;

    // scene list
    ImGui::Text("Loaded Scenes:");

    static std::string pendingDeleteScene;
    static bool DeleteScenePopup = false;

    //all scenes in scroll
    ImGui::BeginChild("SceneList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (int i = 0; i < static_cast<int>(sceneFiles.size()); ++i)
    {
        const std::string& sceneName = sceneFiles[i];
        if (sceneName == "temp") continue;
        bool selected = (selectedScene == i);

        ImGui::PushID(sceneName.c_str());
        if (ImGui::Selectable(sceneName.c_str(), selected))
        {
            selectedScene = i;
            std::cout << "[Editor] Selected scene: " << sceneName << std::endl;
        }

        // right click context menu
        if (ImGui::BeginPopupContextItem("SceneContext"))
        {
            if (ImGui::MenuItem("Delete Scene"))
            {
                pendingDeleteScene = sceneName;
                DeleteScenePopup = true;
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();
    }
    ImGui::EndChild();

    // deletepop up
    if (DeleteScenePopup)
    {
        ImGui::OpenPopup("Delete Scene Confirmation");
        DeleteScenePopup = false;
    }
    if (ImGui::BeginPopupModal("Delete Scene Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to delete scene '%s'?", pendingDeleteScene.c_str());
        ImGui::Separator();

        if (ImGui::Button("Yes, Delete", ImVec2(130, 0)))
        {
            try
            {
                std::string filePath = sceneFolder + pendingDeleteScene + ".json";
                std::filesystem::remove(filePath);
                std::cout << "[Editor] Deleted scene: " << filePath << "\n";
                needRefresh = true;
                selectedScene = -1;
            }
            catch (const std::exception& e)
            {
                std::cerr << "[Editor] Failed to delete scene: " << e.what() << "\n";
            }
            pendingDeleteScene.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            pendingDeleteScene.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Buttons
    // LOAD SCENE
    if (ImGui::Button("Load Scene") && selectedScene >= 0)
    {
        std::string sceneName = sceneFiles[selectedScene];        // "Level2"
        std::string path = sceneFolder + sceneName + ".json";     //  "../Assets/saves/Level2.json"

        // Always return to editor mode when loading scenes
        //sm.EnterEditMode();
        SceneManager& sm = SceneManager::Get();
        // Load the selected scene (real file, not temp)
        sm.LoadScene(path);

        // Clear selection in hierarchy
        ImGuiSystem::selectedEntity = INVALID_ENTITY;

        std::cout << "[Editor] Loaded scene: " << path << "\n";
    }

    // SAVE SCENE
    ImGui::SameLine();
    if (ImGui::Button("Save Current Scene") && selectedScene >= 0)
    {
        std::string path = sceneFolder + sceneFiles[selectedScene] + ".json";
        SavedScene::SavedCamera camSet;
        camSet.pos = camSpawnPos;
        camSet.zoom = camSpawnZoom;
        camSet.worldMin = sceneMin;
        camSet.worldMax = sceneMax;
        JsonLoad::SaveCurrentScene(path, sceneFiles[selectedScene], camSet);
        std::cout << "[Editor] Saved scene: " << path << "\n";
    }

    // NEW SCENE
    static char newSceneName[64] = "";
    ImGui::InputText("New Scene Name", newSceneName, IM_ARRAYSIZE(newSceneName));
    if (ImGui::Button("Create New Scene"))
    {
        std::string sceneName = newSceneName;
        if (sceneName.empty())
        {
            std::cout << "[Editor] Scene name cannot be empty!\n";
        }
        else
        {
            std::string newPath = sceneFolder + sceneName + ".json";
            if (fs::exists(newPath))
            {
                std::cout << "[Editor] Scene already exists.\n";
            }
            else
            {
                SavedScene empty;
                empty.sceneName = sceneName;
                JsonLoad::SaveCurrentScene(newPath, sceneName);
                std::cout << "[Editor] Created new empty scene: " << newPath << "\n";
                needRefresh = true;
            }
        }
    }

    ImGui::NextColumn();

    // Scene settings
    Camera& cam = Camera::GetInstance();
    SceneManager& sm = SceneManager::Get();
    std::string sceneSetLabel = "Scene Settings : [" + sm.GetCurrentSceneName() + "]";
    ImGui::SeparatorText(sceneSetLabel.c_str());

    ImGui::Indent(20.f);
    // WORLD SETTINGS
    // - Scene type
    // - World size
    // ------------------------------------------------------------
#pragma region WORLD SETTINGS
    ImGui::SeparatorText("World settings");

    static const char* sceneTypes[] = { "Menu", "Game", "Cutscene" };
    int currType = static_cast<int>(SceneManager::Get().GetSceneType());
    if (ImGui::Combo("Scene Type", &currType, sceneTypes, IM_ARRAYSIZE(sceneTypes)))
    {
        SceneManager::Get().SetSceneType(static_cast<SceneType>(currType));
    }

    if (currType != ST_GAME) { ImGui::BeginDisabled(); }
    ImGui::InputFloat2("Size", &sceneSize.x);
    if (ImGui::Button("Set size"))
    {
        if (sceneSize.x != 0 && sceneSize.y != 0)
        {
            sceneMin.x = 990 - sceneSize.x / 2;
            sceneMax.x = 990 + sceneSize.x / 2;
            sceneMin.y = 540 - sceneSize.y / 2;
            sceneMax.y = 540 + sceneSize.y / 2;
        }
        cam.SetBounds(sceneMin, sceneMax);
    }
#pragma endregion                                       WORLD SETTINGS
    // ------------------------------------------------------------

    // CAMERA SETTINGS
    // - Camera spawn position and zoom
    // - Reset to center
    // ------------------------------------------------------------
#pragma region CAMERA SETTINGS
    ImGui::SeparatorText("Camera spawn settings");
    // Spawn position
    ImGui::InputFloat2("Position", &camSpawnPos.x);
    // Zoom value
    ImGui::InputFloat("Zoom", &camSpawnZoom);

    if (ImGui::Button("Set spawn to current setting"))
    {
        camSpawnZoom = cam.GetZoom();
        camSpawnPos = cam.GetPosition();
    }

    ImGui::SameLine();
    if (ImGui::Button("Go to spawn camera"))
    {
        cam.SetZoom(camSpawnZoom);
        cam.SetPosition(camSpawnPos);
    }
    if (currType != ST_GAME) { ImGui::EndDisabled(); }
#pragma endregion                                     CAMERA SETTINGS
    // ------------------------------------------------------------

    // BACKGROUND MUSIC
    // - Music bound
    // - Fade out timing
    // ------------------------------------------------------------
#pragma region BGM
    ImGui::SeparatorText("Background Music");
    std::string bgmName = sm.GetCurrentSceneBGM();
    static int selectedTrack = 0;

    std::vector<const char*> musicTrackList;

    for (const auto& [name, asset] : AudioCache::GetAllAssets())
    {
        if (asset.group == "Music")
        {
            musicTrackList.push_back(asset.name.c_str());
            if (asset.name == bgmName)
            {
                selectedTrack = static_cast<int>(musicTrackList.size()) - 1;
            }
        }
    }

    if (ImGui::Combo("BGM Track", &selectedTrack, musicTrackList.data(), static_cast<int>(musicTrackList.size())))
    {
        //std::string trackName = AudioCache::GetAssetName(selectedTrack);
        std::string trackName = musicTrackList[selectedTrack];
        sm.SetCurrentSceneBGM(trackName);
    }
#pragma endregion                                                             BGM
    // ------------------------------------------------------------

    // CUTSCENE STUFF
    // - FPS
    // - Next Scene?
#pragma region CUTSCENE
    // ------------------------------------------------------------
    if (currType == ST_CUTSCENE){
        ImGui::SeparatorText("Cutscene Settings");

        // FPS
        int fps = static_cast<int>(RenderSystem::GetCutsceneFPS());
        if (ImGui::InputInt("FPS", &fps))
        {
            RenderSystem::SetCutsceneFPS(static_cast<double>(fps));
        }

        // Video selector
        {
            //mp4 files in assset browser
            if (g_videoNames.empty())
            {
                const std::string assetFolder = "../Assets/";
                if (std::filesystem::exists(assetFolder))
                {
                    auto options = std::filesystem::directory_options::skip_permission_denied;
                    try {
                        for (auto& entry : std::filesystem::recursive_directory_iterator(assetFolder, options))
                        {
                            if (!entry.is_regular_file()) continue;
                            std::string ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(),
                                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                            if (ext == ".mp4")
                                g_videoNames.push_back(entry.path().filename().string());
                        }
                        std::sort(g_videoNames.begin(), g_videoNames.end());
                    }
                    catch (...) {}
                }
            }

            std::string currentVideo = RenderSystem::GetCutscenePath();
            static int selectedVideo = -1;

            if (!g_videoNames.empty())
            {
                std::vector<const char*> videoItems;
                videoItems.reserve(g_videoNames.size());
                for (auto& v : g_videoNames)
                    videoItems.push_back(v.c_str());

                // selection with current cutscene path
                // "../Assets/file.mp4" -> g_videoName is jst th ename lol
                std::string currentFilename = currentVideo;
                auto pos = currentFilename.find_last_of("/\\");
                if (pos != std::string::npos)
                    currentFilename = currentFilename.substr(pos + 1);

                for (int i = 0; i < (int)g_videoNames.size(); ++i) {
                    if (g_videoNames[i] == currentFilename) {
                        selectedVideo = i;
                    }
                }

                if (selectedVideo < 0)
                    selectedVideo = 0;

                if (ImGui::Combo("Video", &selectedVideo,
                    videoItems.data(),
                    static_cast<int>(videoItems.size())))
                {
                    RenderSystem::SetCutscenePath("../Assets/" + g_videoNames[selectedVideo]);
                }
            }
            else
            {
                ImGui::TextDisabled("No videos found");
            }
        }

        // Next Scene
        std::string nextScene = RenderSystem::GetNextScene();
        static int selectedNextScene = -1;

        if (!sceneFiles.empty())
        {
            // Build items
            std::vector<const char*> sceneItems;
            sceneItems.reserve(sceneFiles.size());
            for (auto& s : sceneFiles)
                sceneItems.push_back(s.c_str());


            for (int i = 0; i < (int)sceneFiles.size(); ++i) {
                if (sceneFiles[i] == nextScene) {
                    selectedNextScene = i;
                }
            }

            if (selectedNextScene < 0)
                selectedNextScene = 0;

            if (ImGui::Combo("Next Scene", &selectedNextScene,
                sceneItems.data(),
                static_cast<int>(sceneItems.size())))
            {
                RenderSystem::SetNextScene(sceneFiles[selectedNextScene]);
            }
        }
        else
        {
            ImGui::TextDisabled("No scenes available");
        }


    }
#pragma endregion                                          CUTSCENE
    // ------------------------------------------------------------

    ImGui::Columns(1);
}

void ImGuiSystem::ShowHierarchyWindow() {

    EnsureSelectionValid();

    ImGui::Text("Entities");

    // show all known entities
    auto& nameMap = m_registry.Get_EntityNameMap();

    for (auto& [entity, name] : nameMap)
    {
        if (!EntityManager::get().isAlive(entity))
            continue;

        bool selected = (entity == selectedEntity);
        //prev problem cant select old entity after clone
        //Use ImGui::PushID(entity) so diff imguiID
        ImGui::PushID((int)entity);
        if (ImGui::Selectable(name.c_str(), selected))
            selectedEntity = entity;
        ImGui::PopID();
    }


}

void ImGuiSystem::ShowInspectorWindow()
{
    EnsureSelectionValid();
    if (selectedEntity == INVALID_ENTITY)
    {
        g_currentInspectorEntity = INVALID_ENTITY;
        ImGui::Text("No entity selected");
        ImGui::Columns(1); //reset
        return;
    }
    g_currentInspectorEntity = selectedEntity;
    ImGui::Text("Entity %d", selectedEntity);
    ImGui::Separator();

    // show all editable components for the selected entity
    // Get entity data in a vector of ComponentData. ComponentData holds name and a map of <DataName(std::string), WidgetData>
    std::vector<Editor::ComponentData> compDataVec = m_registry.Get_EntityData(selectedEntity);
    for (auto& componentData : compDataVec)
    {
        const std::string& compName = componentData.GetComponentName();

        // Custom data fields for render component
        if (compName == "Render")
        {
            RenderComponent& rendercomp = m_registry.Get_Component<RenderComponent>(selectedEntity);
            if (ImGui::TreeNode(compName.c_str()))
            {
                // For m_type and m_layer
                for (auto& [name, widgetVariant] : componentData.GetDataMap())
                {
                    ImGui::PushID(name.c_str());
                    std::visit([&](auto& widget)
                        { MakeWidget(widget, name); }, widgetVariant);
                    ImGui::PopID();
                }

                RenderType startRenderType = rendercomp.GetRenderType();
                RenderType type = rendercomp.GetRenderType();
                // RenderType dropdown
                {
                    Editor::Widget_Dropdown dropdown;
                    dropdown.ref = reinterpret_cast<unsigned int*>(&type);
                    dropdown.Add_Option("Shape", static_cast<unsigned int>(RT_SHAPE));
                    dropdown.Add_Option("Image", static_cast<unsigned int>(RT_IMAGE));
                    dropdown.Add_Option("Spritesheet", static_cast<unsigned int>(RT_SPRITESHEET));
                    MakeWidget(dropdown, "Type");
                }

                bool typeChanged = (type != startRenderType);

                // switch m_type for displays
                switch (type)
                {
                case RT_SHAPE:
                {
                    ImGui::SeparatorText("Shape");

                    // if (typeChanged) { rendercomp.m_renderData = ShapeComponent(); }

                    if (typeChanged) { rendercomp.SetRenderType(type); }
                    ShapeComponent& shape = rendercomp.GetShape();
                    Editor::ComponentData shapedata = shape.EditorData();
                    for (auto& [name, widgetVariant] : shapedata.GetDataMap())
                    {
                        ImGui::PushID(name.c_str());
                        std::visit([&](auto& widget)
                            { MakeWidget(widget, name); }, widgetVariant);
                        ImGui::PopID();
                    }
                }
                break; // end RT_SHAPE

                case RT_IMAGE:
                {
                    ImGui::SeparatorText("Sprite");

                    if (typeChanged) {
                        if (startRenderType == RT_SPRITESHEET) {
                            Sprite::SpriteComponent temp = rendercomp.GetSpriteSheet().Sprite();
                            rendercomp.SetSprite(temp);
                        }
                        else
                        {
                            rendercomp.SetRenderType(type);
                        }
                    }
                    Sprite::SpriteComponent& sprite = rendercomp.GetSprite();

                    Editor::ComponentData spritedata = sprite.EditorData();
                    for (auto& [name, widgetVariant] : spritedata.GetDataMap())
                    {
                        ImGui::PushID(name.c_str());
                        std::visit([&](auto& widget)
                            { MakeWidget(widget, name); }, widgetVariant);
                        ImGui::PopID();
                    }
                    // Display the sprite
                    {
                        GLuint texID = sprite.Get_Image().textureID;
                        const float texW = (float)sprite.Get_Image().widthPixels;
                        const float texH = (float)sprite.Get_Image().heightPixels;

                        ImVec2 avail = ImGui::GetContentRegionAvail();

                        float scale = (texW != 0) ? avail.x / texW : 0.f;

                        ImVec2 displaySize(texW * scale, texH * scale);

                        ImGui::Image((ImTextureID)(intptr_t)texID, displaySize, ImVec2(0, 1), ImVec2(1, 0));
                    }
                }
                break; // end RT_IMAGE

                case RT_SPRITESHEET:
                {
                    ImGui::SeparatorText("SpriteSheet");

                    if (typeChanged) {
                        if (startRenderType == RT_IMAGE) {
                            Sprite::SpriteComponent tempsprite = rendercomp.GetSprite();
                            rendercomp.SetRenderType(type);
                            rendercomp.GetSpriteSheet().Sprite() = tempsprite;
                        }
                        else
                        {
                            rendercomp.SetRenderType(type);
                        }
                    }
                    Sprite::SpriteSheet& spritesheet = rendercomp.GetSpriteSheet();
                    Sprite::SpriteComponent& sprite = spritesheet.Sprite();

                    // Sprite stuff

                    Editor::ComponentData spritedata = sprite.EditorData();
                    for (auto& [name, widgetVariant] : spritedata.GetDataMap())
                    {
                        ImGui::PushID(name.c_str());
                        std::visit([&](auto& widget)
                            { MakeWidget(widget, name); }, widgetVariant);
                        ImGui::PopID();
                    }

                    spritesheet.UpdateUV();
                    ActiveFrames& temp = spritesheet.GetFrames();
                    ImVec2 displaySize{};
                    // Display the sprite
                    {
                        GLuint texID = sprite.Get_Image().textureID;
                        const float texW = (float)sprite.Get_Image().widthPixels;
                        const float texH = (float)sprite.Get_Image().heightPixels;

                        ImVec2 avail = ImGui::GetContentRegionAvail();

                        float scale = (texW != 0) ? avail.x / texW : 0.f;

                        displaySize = { texW * scale, texH * scale };

                        ImVec2 imagePos = ImGui::GetCursorScreenPos();

                        ImGui::Image((ImTextureID)(intptr_t)texID, displaySize, ImVec2(0, 1), ImVec2(1, 0));

                        ImDrawList* grid = ImGui::GetWindowDrawList();


                        int rows = spritesheet.GetTotalRows();
                        int cols = spritesheet.GetTotalCols();

                        float cellW = displaySize.x / cols;
                        float cellH = displaySize.y / rows;

                        for (int c = 1; c < cols; ++c)
                        {
                            float x = imagePos.x + c * cellW;
                            grid->AddLine(ImVec2(x, imagePos.y), ImVec2(x, imagePos.y + displaySize.y), IM_COL32(255, 255, 255, 128));
                        }

                        for (int r = 1; r < rows; ++r)
                        {
                            float y = imagePos.y + r * cellH;
                            grid->AddLine(ImVec2(imagePos.x, y), ImVec2(imagePos.x + displaySize.x, y), IM_COL32(255, 255, 255, 128));
                        }

                        int r = temp.row;
                        int cstart = temp.col_start;
                        int cend = temp.col_end;

                        for (int c = cstart; c <= cend; ++c)
                        {
                            ImVec2 p0(imagePos.x + c * cellW,
                                imagePos.y + r * cellH);

                            ImVec2 p1(imagePos.x + (c + 1) * cellW,
                                imagePos.y + (r + 1) * cellH);

                            // Draw filled highlight
                            grid->AddRectFilled(p0, p1, IM_COL32(255, 255, 0, 80));

                            // Draw border
                            grid->AddRect(p0, p1, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
                        }
                    }

                    // Spritesheet stuff
                    Editor::ComponentData spritesheetdata = spritesheet.EditorData();
                    for (auto& [name, widgetVariant] : spritesheetdata.GetDataMap())
                    {
                        ImGui::PushID(name.c_str());
                        std::visit([&](auto& widget) {
                            MakeWidget(widget, name);
                            }, widgetVariant);
                        ImGui::PopID();
                    }
                    spritesheet.SetState(spritesheet.GetCurrentState());

                    Editor::Widget_Int widgetrow(&temp.row, true, 0, spritesheet.GetTotalRows());
                    Editor::Widget_Int widgetcolstart(&temp.col_start, true, 0, spritesheet.GetTotalCols()-1);
                    Editor::Widget_Int widgetcolend(&temp.col_end, true, temp.col_start, spritesheet.GetTotalCols()-1);
                    Editor::Widget_Float widgetduration(&temp.duration);
                    Editor::Widget_Checkmark widgetloop(&temp.willLoop);
                    //ImGui::PushItemWidth(displaySize.x/4);
                    MakeWidget(widgetrow, "row");
                    //ImGui::SameLine();
                    MakeWidget(widgetcolstart, "first col");
                    //ImGui::SameLine();
                    MakeWidget(widgetcolend, "last col");
                    //ImGui::PopItemWidth();
                    MakeWidget(widgetduration, "duration");
                    
                    MakeWidget(widgetloop, "loop");

                    ImGui::Separator();
                    if (ImGui::SmallButton("Add State"))
                    {
                        spritesheet.AddState(static_cast<int>(spritesheet.GetStateFrames().size()), { 0,0,0 });
                    }

                    spritesheet.EditFrames(temp.row, temp.col_start, temp.col_end, temp.duration, temp.willLoop);
                }
                break; // end RT_SPRITESHEET

                default:
                    // this is wrong...
                    break;
                }

                ImGui::TreePop();
            }
        }
        // Custom data fields for Behaviour component (C++ scripts)
        else if (compName == "Behaviour")
        {
            BehaviourComponent& behaviourComp = m_registry.Get_Component<BehaviourComponent>(selectedEntity);
            if (ImGui::TreeNode(compName.c_str()))
            {
                LogicSystem& logicSys = LogicSystem::Get();
                auto& map = logicSys.GetIDToNameMap();

                if (logicSys.GetIDToNameMap().empty()) return;

                std::vector<const char*> items;
                size_t optSize = logicSys.GetIDToNameMap().size();
                items.reserve(optSize);
                for (auto& [scriptname, scriptID] : logicSys.GetNameToIDMap()) items.push_back(scriptname.c_str());
                // find index of current value

                static int currentIndex = 0;

                std::string selectedScript(items[currentIndex]);

                if (ImGui::Combo("Select Script", &currentIndex, items.data(), (int)items.size())) {
                    //*w.ref = w.enumMap[w.options[currentIndex]];
                    selectedScript = (items[currentIndex]);
                }

                bool disable = behaviourComp.HasScript(selectedScript);

                std::string AddLabel = "Add Script";
                if (disable)
                {
                    AddLabel += " (Script already registered!)";
                    ImGui::BeginDisabled();
                }
                if (ImGui::Button(AddLabel.c_str()))
                {
                    behaviourComp.AddScript(selectedScript);
                }
                if (disable) ImGui::EndDisabled();

                ImGui::SeparatorText("Registered Scripts:");
                int id = 0;
                for (ScriptID sID : behaviourComp.scriptIDList)
                {
                    ImGui::PushID(id++);
                    ImGui::Text(map.find(sID)->second.c_str());

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove"))
                    {
                        behaviourComp.RemoveScript(sID);
                    }
                    ImGui::PopID();
                }

                ImGui::TreePop();
            }
        }
        // Custom data fields for Monoscript component (C# scripts)
        else if (compName == "Mono Script")
        {
            MonoScriptComponent& monoComp =
                m_registry.Get_Component<MonoScriptComponent>(selectedEntity);

            if (ImGui::TreeNode(compName.c_str()))
            {
                MonoSystem& monoSys = MonoSystem::Get();

                // ----------------------------------------------
                // 1. Build list of available script names
                // ----------------------------------------------
                const auto& nameToID = monoSys.GetNameToIDMap();

                if (nameToID.empty())
                {
                    ImGui::TextDisabled("No C# scripts registered.");
                    ImGui::TreePop();
                    return;
                }

                std::vector<const char*> items;
                items.reserve(nameToID.size());
                for (auto& [name, id] : nameToID)
                    items.push_back(name.c_str());

                static int currentIndex = 0;
                std::string selectedScript = items[currentIndex];

                // dropdown
                if (ImGui::Combo("Select C# Script",
                    &currentIndex, items.data(), (int)items.size()))
                {
                    selectedScript = items[currentIndex];
                }

                // ----------------------------------------------
                // 2. Add Script button
                // ----------------------------------------------
                bool disableAdd = monoComp.HasScript(selectedScript);

                std::string addLabel = "Add Script";
                if (disableAdd)
                {
                    addLabel += " (already added)";
                    ImGui::BeginDisabled();
                }

                if (ImGui::Button(addLabel.c_str()))
                {
                    monoComp.AddScript(selectedScript);
                }

                if (disableAdd) ImGui::EndDisabled();

                ImGui::SeparatorText("Attached Scripts");

                // ----------------------------------------------
                // 3. Show list of attached scripts
                // ----------------------------------------------
                int id = 0;
                for (MonoScriptID scriptID : monoComp.scriptIDList)
                {
                    ImGui::PushID(id++);

                    const std::string& name = monoSys.GetScriptName(scriptID);
                    ImGui::Text("%s", name.c_str());

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove"))
                    {
                        monoComp.RemoveScript(scriptID);
                    }

                    ImGui::PopID();
                }

                ImGui::TreePop();
            }
        }
        // Custom data fields for collider component (collision layers)
        else if (compName == "Collider")
        {
            if (ImGui::TreeNode(compName.c_str()))
            {
                // Get component reference inside TreeNode to avoid stale references
                ColliderComponent& collider = m_registry.Get_Component<ColliderComponent>(selectedEntity);

                {
                    std::vector<uint32_t> layerBitsOrdered;
                    std::vector<const char*> layerNames;
                    layerBitsOrdered.reserve(CollisionLayerManager::Get().GetLayerNameMap().size());


                    for (auto& [bit, layername] : CollisionLayerManager::Get().GetLayerNameMap())
                    {
                        layerBitsOrdered.push_back(bit);
                        layerNames.push_back(layername.c_str());
                    }

                    uint32_t currBit = (int)collider.GetLayer();
                    int currIndex = 0;

                    for (int i = 0; i < (int)layerBitsOrdered.size(); i++)
                    {
                        if (layerBitsOrdered[i] == currBit)
                        {
                            currIndex = i;
                            break;
                        }
                    }

                    if (ImGui::Combo("Layer", &currIndex, layerNames.data(), (int)layerNames.size()))
                    {
                        uint32_t newBit = layerBitsOrdered[currIndex];
                        collider.SetLayer(newBit);
                    }

                }

                // Show default component fields (AABB, Circle, isTrigger, isActive)
                for (auto& [name, widgetVariant] : componentData.GetDataMap())
                {
                    ImGui::PushID(name.c_str());
                    std::visit([&](auto& widget)
                        { MakeWidget(widget, name); }, widgetVariant);
                    ImGui::PopID();
                }

                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Collision Layers");

                // Create checkboxes for each registered layer
                std::vector<std::string> layers = { "Player", "Enemy", "Projectile", "Wall" };

                ImGui::Text("Collides With:");

                // Get current mask once
                uint32_t currentMask = CollisionLayerManager::Get().GetLayerMask(collider.GetLayer());

                // Debug: show current layer and mask values
                ImGui::Text("Layer: 0x%X", collider.GetLayer());
                ImGui::Text("Mask: 0x%X", currentMask);
                ImGui::Text("Is Trigger: %s", collider.IsTrigger() ? "YES" : "NO");
                ImGui::Text("Is Active: %s", collider.IsActive() ? "YES" : "NO");

                // Show if entity has a tag
                //if (m_registry.Has_Component<TagComponent>(selectedEntity)) {
                //    auto& tag = m_registry.Get_Component<TagComponent>(selectedEntity);
                //    const char* tagNames[] = {"PLAYER", "ENEMY", "PROJECTILE", "WALL", "INSTANCING", "BATCH", "TEST", "OBJECT"};
                //    if (tag.tag < 8) {
                //        ImGui::Text("Tag: %s", tagNames[tag.tag]);
                //    }

                //    // Button to fix layers based on tag
                //    if (ImGui::Button("Auto-Setup Layer from Tag")) {
                //        CollisionManager::SetupCollisionLayers();
                //        std::cout << "[Editor] Re-ran SetupCollisionLayers for all entities" << std::endl;
                //    }
                //} else {
                //    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No TagComponent!");
                //}

                //int layerIndex = 0;
                //for (const std::string& layerName : layers)
                //{
                //    ImGui::PushID(layerIndex++);

                //    uint32_t layerBit = CollisionLayerManager::Get().GetLayerBit(layerName);
                //    bool isSet = (currentMask & layerBit) != 0;
                //    bool previousState = isSet;

                //    ImGui::Checkbox(layerName.c_str(), &isSet);

                //    // Only update if the state actually changed
                //    if (isSet != previousState)
                //    {
                //        // Get fresh reference BEFORE making any changes
                //        auto& freshCollider = m_registry.Get_Component<ColliderComponent>(selectedEntity);

                //        if (isSet) {
                //            currentMask |= layerBit;
                //        }
                //        else {
                //            currentMask &= ~layerBit;
                //        }
                //        //freshCollider.SetMask(currentMask);
                //    }

                //    ImGui::PopID();
                //}

                ImGui::TreePop();
            }
        }

        else if (compName == "Transform")
        {
            if (ImGui::TreeNode(compName.c_str()))
            {
                TransformComponent& transform = m_registry.Get_Component<TransformComponent>(selectedEntity);

                // Track editing state for the entire Transform component
                static bool wasEditingTransform = false;
                bool isEditingNow = false;

                // For each transform widget
                for (auto& [name, widgetVariant] : componentData.GetDataMap())
                {
                    ImGui::PushID(name.c_str());

                    
                    std::visit([&](auto& widget) {
                        MakeWidget(widget, name);

                        // Check if is currently being edited
                        if (ImGui::IsItemActive()) {
                            isEditingNow = true;
                        }
                        }, widgetVariant);

                    ImGui::PopID();
                }

            
                if (isEditingNow && !wasEditingTransform)
                {
                    
                    BeginTransformEdit(selectedEntity);
                }
                else if (!isEditingNow && wasEditingTransform)
                {
                    
                    EndTransformEdit();
                }

                wasEditingTransform = isEditingNow;

                // collider
                if (selectedEntity != INVALID_ENTITY &&
                    m_registry.Has_Component<ColliderComponent>(selectedEntity))
                {
                    auto& collider = m_registry.Get_Component<ColliderComponent>(selectedEntity);
                    collider.UpdateHitboxes(transform.pos, transform.scale);
                }

                ImGui::TreePop();
            }
        }
        // Custom data fields for Portal component
        else if (compName == "Portal")
        {
            PortalComponent& portal = m_registry.Get_Component<PortalComponent>(selectedEntity);
            if (ImGui::TreeNode(compName.c_str()))
            {
                for (auto& [name, widgetVariant] : componentData.GetDataMap())
                {
                    ImGui::PushID(name.c_str());
                    std::visit([&](auto& widget)
                        { MakeWidget(widget, name); }, widgetVariant);
                    ImGui::PopID();
                }

                // Scene selector dropdown
                {
                    namespace fs = std::filesystem;
                    const std::string sceneFolder = "../Assets/saves/";

                    std::vector<std::string> sceneFiles;
                    if (fs::exists(sceneFolder))
                    {
                        for (auto& entry : fs::directory_iterator(sceneFolder))
                        {
                            if (entry.path().extension() == ".json" && entry.path().stem().string() != "temp")
                            {
                                sceneFiles.push_back(entry.path().stem().string());
                            }
                        }
                    }

                    std::vector<const char*> sceneFilePtr;
                    sceneFilePtr.reserve(sceneFiles.size());
                    for (auto& file : sceneFiles)
                    {
                        sceneFilePtr.push_back(file.c_str());
                    }

                    // Find currently selected scene idx
                    int selectedScene = -1;
                    std::string currSceneStem = fs::path(portal.nextScene).stem().string();
                    for (size_t i = 0; i < sceneFiles.size(); i++)
                    {
                        if (currSceneStem == sceneFiles[i])
                        {
                            selectedScene = static_cast<int>(i);
                            break;
                        }
                    }

                    if (ImGui::Combo("Scene", &selectedScene, sceneFilePtr.data(), static_cast<int>(sceneFilePtr.size())))
                    {
                        portal.nextScene = sceneFolder + sceneFilePtr[selectedScene] + ".json";
                    }
                }

                ImGui::TreePop();
            }
        }
        // Default data fields for other components
        else
            if (ImGui::TreeNode(compName.c_str()))
            {
                for (auto& [name, widgetVariant] : componentData.GetDataMap())
                {
                    ImGui::PushID(name.c_str());
                    std::visit([&](auto& widget)
                        { MakeWidget(widget, name); }, widgetVariant);
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }

        if (selectedEntity != INVALID_ENTITY &&
            m_registry.Has_Component<ColliderComponent>(selectedEntity) &&
            m_registry.Has_Component<TransformComponent>(selectedEntity))
        {
            auto& collider = m_registry.Get_Component<ColliderComponent>(selectedEntity);
            auto& t = m_registry.Get_Component<TransformComponent>(selectedEntity);
            collider.UpdateHitboxes(t.pos, t.scale);
        }
    }

}


void ImGuiSystem::ShowAddRemoveComponents()
{
    ImGui::Text("Add / Remove Components");

    ImGui::Separator();

    if (selectedEntity == INVALID_ENTITY)
    {
        ImGui::TextDisabled("No entity selected");
        return;
    }

    Entity eID = selectedEntity;
    for (std::type_index typeID : m_registry.Get_ComponentOrder())
    {
        std::string compName = m_registry.Get_ComponentTypeName(typeID);
        bool hasComp = m_registry.Has_Component(eID, typeID);
        bool depsOK = m_registry.HasDependencies(eID, typeID);

        if (hasComp)
        {
            ImGui::BeginDisabled();
            ImGui::Button((compName + " (Has)").c_str());
            ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::SmallButton(("Remove##" + compName).c_str()))
            {
                //std::string compName = m_registry.Get_ComponentTypeName(typeID);
                /*if (compName == "Animator" &&
                    m_registry.Has_Component<Sprite::SpriteComponent>(eID))
                {
                    auto& sprite = m_registry.Get_Component<Sprite::SpriteComponent>(eID);
                    sprite.Set_Animated(false);
                }*/

                m_registry.Remove_Component(eID, typeID);
                std::cout << "[Editor] Removed component from entity " << eID << "\n";
            }
        }
        else
        {
            if (!depsOK) ImGui::BeginDisabled();
            if (ImGui::SmallButton(("Add " + compName).c_str()))
            {
                m_registry.Add_Component(eID, typeID);
                std::cout << "[Editor] Added " << compName << " to entity " << eID << "\n";
            }
            if (!depsOK) ImGui::EndDisabled();
        }
    }
}


static const std::unordered_set<std::string> PROTECTED_ASSETS = {
    "vertexSrc.vert", "fragmentSrc.frag",
    "batch.vert",   "batch.frag", "PermanentMarker-Regular.ttf"
    
};

static bool g_showProtectedFileError = false;
static std::string g_protectedFileName;

void ImGuiSystem::ShowAssetPreviewWindow()
{
    ImGui::Text("Asset Preview Browser");
    ImGui::Separator();

    if (ImGui::BeginTabBar("AssetsTabs"))
    {
        // ====================================================
        // IMAGES TAB
        // ====================================================
        if (ImGui::BeginTabItem("Images"))
        {
            static bool initialized = false;

            if (ImGui::Button("Refresh Images"))
            {
                RefreshImageAssets();
                initialized = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("Rescans Assets/ for images");
            ImGui::Separator();

            if (!initialized)
            {
                g_loadedThumbnails.clear();
                g_imageNames.clear();

                const std::string assetFolder = "../Assets/";
                //same as earlier loop thru all
                for (auto& entry : std::filesystem::recursive_directory_iterator(assetFolder))
                {
                    if (!entry.is_regular_file())
                        continue;
                    std::string path = entry.path().string();
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(),
                        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                    {

                        try {
                            Image img = CreateImage(path);
                            if (img.IsValid()) {
                                g_loadedThumbnails.push_back(img);
                                g_imageNames.push_back(entry.path().filename().string());
                            }
                        }
                        catch (...) {
                            std::cerr << "Failed to load " << entry.path() << "\n";
                        }
                    }
                }
                initialized = true;
            }

            if (g_loadedThumbnails.empty()) {
                ImGui::TextDisabled("No images found in Assets folder.");
            }
            else
            {
                const float thumbSize = 100.0f;
                const float padding = 16.0f;
                float windowWidth = ImGui::GetContentRegionAvail().x;
                int columns = static_cast<int>(windowWidth / (thumbSize + padding));
                if (columns < 1) columns = 1;  // zero or negative
                ImGui::BeginChild("ImageScroll", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

                int count = 0;
                for (size_t i = 0; i < g_loadedThumbnails.size(); ++i)
                {
                    ImGui::BeginGroup();
                    GLuint texID = g_loadedThumbnails[i].textureID;

                    //same from earlier stuff
                    ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(thumbSize, thumbSize), ImVec2(0, 1), ImVec2(1, 0));

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        const std::string fullPath = "../Assets/" + g_imageNames[i];
                        ImGui::SetDragDropPayload("ASSET_IMAGE", fullPath.c_str(), fullPath.size() + 1);
                        ImGui::Text("Drag %s", g_imageNames[i].c_str());
                        ImGui::EndDragDropSource();
                    }
                    // right click on image thumbnail
                    ImGui::PushID((int)i);
                    if (ImGui::BeginPopupContextItem("##ImgCtx"))
                    {
                        ImGui::Text("%s", g_imageNames[i].c_str());
                        ImGui::Separator();
                        if (ImGui::MenuItem("Delete Image"))
                        {
                            g_pendingDeleteImageName = g_imageNames[i];
                            g_showDeleteImagePopup = true;
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                    // filename
                    ImGui::TextWrapped("%s", g_imageNames[i].c_str());
                    ImGui::EndGroup();
                    //grid
                    if (++count % columns != 0)
                        ImGui::SameLine();
                    //horizontal line until a full row wraps when count % columns == 0.
                }

                ImGui::EndChild();
            }

            // Delete image confirmation modal
            if (g_showDeleteImagePopup)
            {
                ImGui::OpenPopup("Delete Image Confirmation");
                g_showDeleteImagePopup = false;
            }
            if (ImGui::BeginPopupModal("Delete Image Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Are you sure you want to delete:\n%s ?", g_pendingDeleteImageName.c_str());
                ImGui::Separator();

                if (ImGui::Button("Yes, Delete", ImVec2(120, 0)))
                {
                    // search Assets for the file by name and delete it
                    namespace fs = std::filesystem;
                    const std::string assetFolder = "../Assets/";
                    bool deleted = false;
                    for (auto& entry : fs::recursive_directory_iterator(assetFolder))
                    {
                        if (entry.is_regular_file() && entry.path().filename().string() == g_pendingDeleteImageName)
                        {
                            try
                            {
                                fs::remove(entry.path());
                                std::cout << "[AssetBrowser] Deleted image: " << entry.path() << "\n";
                                deleted = true;
                            }
                            catch (const std::exception& e)
                            {
                                std::cerr << "[AssetBrowser] Failed to delete: " << e.what() << "\n";
                            }
                            break;
                        }
                    }

                    if (deleted)
                    {
                        // remove from thumbnail cache so it disappears immediately
                        for (size_t j = 0; j < g_imageNames.size(); ++j)
                        {
                            if (g_imageNames[j] == g_pendingDeleteImageName)
                            {
                                g_imageNames.erase(g_imageNames.begin() + j);
                                g_loadedThumbnails.erase(g_loadedThumbnails.begin() + j);
                                break;
                            }
                        }
                    }

                    g_pendingDeleteImageName.clear();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    g_pendingDeleteImageName.clear();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
            
            if (showReplacePopup)
            {
                ImGui::OpenPopup("Replace Existing Asset?");
            }

            if (ImGui::BeginPopupModal("Replace Existing Asset?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("File already exists:\n%s\n", pendingDestPath.c_str());
                ImGui::Text("Do you want to replace it with:\n%s ?", pendingImportPath.c_str());
                ImGui::Separator();

                if (ImGui::Button("Yes, Replace", ImVec2(120, 0)))
                {
                    try {
                        namespace fs = std::filesystem;

                        const std::string assetFolder = "../Assets/";
                        const std::string newName = fs::path(pendingImportPath).filename().string(); // Luke2023.jpg
                        const std::string newStem = fs::path(pendingImportPath).stem().string();     // Luke2023
                        const std::string destNewPath = assetFolder + newName;

                        //remove all existing variants (by stem)
                        for (auto& entry : fs::directory_iterator(assetFolder))
                        {
                            if (!entry.is_regular_file())
                                continue;
                            if (entry.path().stem().string() == newStem)
                            {
                                fs::remove(entry.path());
                                std::cout << "[ImGui Replace] Deleted old: " << entry.path().filename().string() << "\n";
                            }
                        }

                        //copy new file using own filename (so extension preserved)
                        fs::copy_file(pendingImportPath, destNewPath, fs::copy_options::overwrite_existing);
                        std::cout << "[ImGui Replace] Installed: " << destNewPath << "\n";

                        //thumbnail for new file
                        if (Image newImg = CreateImage(destNewPath); newImg.IsValid())
                        {
                            g_loadedThumbnails.push_back(newImg);
                            g_imageNames.push_back(fs::path(destNewPath).filename().string());
                        }

                        //refresh
                        g_loadedThumbnails.clear();
                        g_imageNames.clear();
                        for (auto& entry : fs::recursive_directory_iterator(assetFolder))
                        {
                            if (!entry.is_regular_file())
                                continue;
                            auto ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(),
                                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                            {
                                if (Image img = CreateImage(entry.path().string()); img.IsValid())
                                {
                                    g_loadedThumbnails.push_back(img);
                                    g_imageNames.push_back(entry.path().filename().string());
                                }
                            }
                        }

                        std::cout << "[Replace] Asset preview refreshed.\n";
                    }
                    catch (const std::filesystem::filesystem_error& e)
                    {
                        std::cerr << "[Replace] Failed: " << e.what() << std::endl;
                    }

                    showReplacePopup = false;
                    ImGui::CloseCurrentPopup();
                }



                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    showReplacePopup = false;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::EndTabItem();




        }

        // ====================================================
        // MUSIC TAB
        // ====================================================

        if (ImGui::BeginTabItem("Audio"))
        {
            //storing audio assets
            static std::unordered_map<std::string, AudioAsset> audioList;
            auto& all = audioList;

            if (ImGui::Button("Refresh Audio"))
            {
                audioList = RefreshAudioAssets();
            }
            ImGui::SameLine();
            ImGui::TextDisabled("Rescans audio manifest");
            ImGui::Separator();

            //refresh each frame when add/removed
            audioList = AudioCache::GetAllAssets();

            // cats
            std::vector<std::string> musicList;
            std::vector<std::string> sfxList;
            std::vector<std::string> uiList;

            for (auto& [name, asset] : all)
            {
                if (asset.group == "Music")      musicList.push_back(name);
                else if (asset.group == "SFX")   sfxList.push_back(name);
                else if (asset.group == "UI")    uiList.push_back(name);
            }

            // ------------------------
            // MUSIC DROPDOWN
            // ------------------------
            if (ImGui::CollapsingHeader("Music", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (auto& name : musicList)
                {
                    const AudioAsset& asset = all[name];

                    ImGui::PushID(name.c_str()); //unique ID

                    // Title
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0.6f, 1));
                    ImGui::Text("[%s]", name.c_str());
                    ImGui::PopStyleColor();

                    ImGui::Indent();

                    ImGui::Text("Path: %s", asset.path.c_str());
                    ImGui::Text("Group: %s", asset.group.c_str());

                    // PLAY BUTTON
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1));
                    if (ImGui::Button("Play"))
                    {
                        StopAudioPreview();
                        FMOD::Sound* snd = AudioCache::GetSound(name);
                        if (snd)
                        {
                            SceneManager& sm = SceneManager::Get();
                            bool wasEdit = sm.inEditMode;
                            sm.inEditMode = false;
                            previewChannel = AudioSystem::PlaySoundGetChannel(snd, false);
                            sm.inEditMode = wasEdit;

                            // Disable looping for preview playback
                            if (previewChannel)
                            {
                                previewChannel->setMode(FMOD_LOOP_OFF);
                            }

                            sm.inEditMode = wasEdit;
                        }
                    }
                    ImGui::PopStyleColor();

                    ImGui::SameLine();

                    // STOP BUTTON
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 1));
                    if (ImGui::Button("Stop"))
                        StopAudioPreview();
                    ImGui::PopStyleColor();

                    ImGui::SameLine();

                    // REMOVE BUTTON
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.3f, 0.1f, 1));
                    bool remove = false;
                    if (ImGui::Button("Remove"))
                        remove = true;
                    ImGui::PopStyleColor();

                    ImGui::Unindent();
                    ImGui::Separator();
                    ImGui::PopID();

                    if (remove)
                    {
                        DeleteAudioAsset(name);
                        audioList = RefreshAudioAssets();
                        break;
                    }
                }
            }

            // ------------------------
            // SFX DROPDOWN
            // ------------------------
            if (ImGui::CollapsingHeader("SFX", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (auto& name : sfxList)
                {
                    const AudioAsset& asset = all[name];
                    ImGui::PushID(name.c_str());

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0.6f, 1));
                    ImGui::Text("[%s]", name.c_str());
                    ImGui::PopStyleColor();

                    ImGui::Indent();
                    ImGui::Text("Path: %s", asset.path.c_str());
                    ImGui::Text("Group: %s", asset.group.c_str());

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1));
                    if (ImGui::Button("Play"))
                    {
                        StopAudioPreview();
                        FMOD::Sound* snd = AudioCache::GetSound(name);
                        if (snd)
                        {
                            SceneManager& sm = SceneManager::Get();
                            bool wasEdit = sm.inEditMode;
                            sm.inEditMode = false;
                            previewChannel = AudioSystem::PlaySoundGetChannel(snd, false);
                            sm.inEditMode = wasEdit;

                            // Disable looping for preview playback
                            if (previewChannel)
                            {
                                previewChannel->setMode(FMOD_LOOP_OFF);
                            }

                            sm.inEditMode = wasEdit;
                        }
                    }
                    ImGui::PopStyleColor();

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 1));
                    if (ImGui::Button("Stop"))
                        StopAudioPreview();
                    ImGui::PopStyleColor();

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.3f, 0.1f, 1));
                    bool remove = false;
                    if (ImGui::Button("Remove"))
                        remove = true;
                    ImGui::PopStyleColor();

                    ImGui::Unindent();
                    ImGui::Separator();
                    ImGui::PopID();

                    if (remove)
                    {
                        DeleteAudioAsset(name);
                        audioList = RefreshAudioAssets();
                        break;
                    }
                }
            }

            // ------------------------
            // UI DROPDOWN
            // ------------------------
            if (ImGui::CollapsingHeader("UI", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (auto& name : uiList)
                {
                    const AudioAsset& asset = all[name];
                    ImGui::PushID(name.c_str());

                    // Title
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0.6f, 1));
                    ImGui::Text("[%s]", name.c_str());
                    ImGui::PopStyleColor();

                    ImGui::Indent();
                    ImGui::Text("Path: %s", asset.path.c_str());
                    ImGui::Text("Group: %s", asset.group.c_str());

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1));
                    if (ImGui::Button("Play"))
                    {
                        StopAudioPreview();
                        FMOD::Sound* snd = AudioCache::GetSound(name);
                        if (snd)
                        {
                            SceneManager& sm = SceneManager::Get();
                            bool wasEdit = sm.inEditMode;
                            sm.inEditMode = false;
                            previewChannel = AudioSystem::PlaySoundGetChannel(snd, false);
                            sm.inEditMode = wasEdit;

                            // Disable looping for preview playback
                            if (previewChannel)
                            {
                                previewChannel->setMode(FMOD_LOOP_OFF);
                            }

                            sm.inEditMode = wasEdit;
                        }
                    }
                    ImGui::PopStyleColor();

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 1));
                    if (ImGui::Button("Stop"))
                        StopAudioPreview();
                    ImGui::PopStyleColor();

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.3f, 0.1f, 1));
                    bool remove = false;
                    if (ImGui::Button("Remove"))
                        remove = true;
                    ImGui::PopStyleColor();

                    ImGui::Unindent();
                    ImGui::Separator();
                    ImGui::PopID();

                    if (remove)
                    {
                        DeleteAudioAsset(name);
                        audioList = RefreshAudioAssets();
                        break;
                    }
                }
            }


            ImGui::EndTabItem();
        }

        // ====================================================
        // VIDEO TAB
        // ====================================================
        if (ImGui::BeginTabItem("Video"))
        {
            static bool videoInitialized = false;

            auto refreshVideoList = [&]()
                {
                    g_videoNames.clear();
                    const std::string assetFolder = "../Assets/";
                    if (!std::filesystem::exists(assetFolder)) return;

                    auto options = std::filesystem::directory_options::skip_permission_denied;
                    try {
                        for (auto& entry : std::filesystem::recursive_directory_iterator(assetFolder, options))
                        {
                            if (!entry.is_regular_file()) continue;

                            std::string ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(),
                                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                            if (ext == ".mp4")
                            {
                                g_videoNames.push_back(entry.path().filename().string());
                            }
                        }
                        std::sort(g_videoNames.begin(), g_videoNames.end());
                    }
                    catch (const std::exception& e) {
                        std::cerr << "[AssetBrowser] Video scan error: " << e.what() << "\n";
                    }
                    videoInitialized = true;
                };

            if (!videoInitialized)
            {
                refreshVideoList();
            }

            if (ImGui::Button("Refresh Video List"))
            {
                refreshVideoList();
            }
            ImGui::SameLine();
            ImGui::TextDisabled("Refresh for .mp4 files");

            ImGui::Separator();

            ImGui::BeginChild("VideoScroll", ImVec2(0, 0), true);

            if (g_videoNames.empty())
            {
                ImGui::TextDisabled("No .mp4 files found in Assets folder.");
            }
            else
            {
                for (size_t i = 0; i < g_videoNames.size(); ++i)
                {
                    ImGui::PushID(static_cast<int>(i));

                    ImGui::Bullet();
                    ImGui::Selectable(g_videoNames[i].c_str());

                    // Drag and drop source
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        const std::string fullPath = "../Assets/" + g_videoNames[i];
                        ImGui::SetDragDropPayload("ASSET_VIDEO", fullPath.c_str(), fullPath.size() + 1);
                        ImGui::Text("Drag %s", g_videoNames[i].c_str());
                        ImGui::EndDragDropSource();
                    }

                    // Right-click context menu for delete
                    if (ImGui::BeginPopupContextItem())
                    {
                        ImGui::Text("%s", g_videoNames[i].c_str());
                        ImGui::Separator();
                        if (ImGui::MenuItem("Delete Video"))
                        {
                            g_pendingDeleteVideoName = g_videoNames[i];
                            g_showDeleteVideoPopup = true;
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::PopID();
                }
            }

            ImGui::EndChild();

            // Delete video confirmation modal
            if (g_showDeleteVideoPopup)
            {
                ImGui::OpenPopup("Delete Video Confirmation");
                g_showDeleteVideoPopup = false;
            }

            if (ImGui::BeginPopupModal("Delete Video Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Are you sure you want to delete:\n%s ?", g_pendingDeleteVideoName.c_str());
                ImGui::Separator();

                if (ImGui::Button("Yes, Delete", ImVec2(120, 0)))
                {
                    namespace fs = std::filesystem;
                    const std::string assetFolder = "../Assets/";
                    bool deleted = false;

                    for (auto& entry : fs::recursive_directory_iterator(assetFolder))
                    {
                        if (entry.is_regular_file() && entry.path().filename().string() == g_pendingDeleteVideoName)
                        {
                            try
                            {
                                fs::remove(entry.path());
                                std::cout << "[AssetBrowser] Deleted video: " << entry.path() << "\n";
                                deleted = true;
                            }
                            catch (const std::exception& e)
                            {
                                std::cerr << "[AssetBrowser] Failed to delete video: " << e.what() << "\n";
                            }
                            break;
                        }
                    }

                    if (deleted)
                    {
                        // Remove from video list so it disappears immediately
                        for (size_t j = 0; j < g_videoNames.size(); ++j)
                        {
                            if (g_videoNames[j] == g_pendingDeleteVideoName)
                            {
                                g_videoNames.erase(g_videoNames.begin() + j);
                                break;
                            }
                        }
                    }

                    g_pendingDeleteVideoName.clear();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    g_pendingDeleteVideoName.clear();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::EndTabItem();
        }

        // ====================================================
        // FONTS & OTHER FILES TAB
        // ====================================================
        if (ImGui::BeginTabItem("Fonts & Other"))
        {
            static std::vector<std::string> s_ttfFiles;
            static std::vector<std::string> s_jsonFiles;
            static std::vector<std::string> s_fragFiles;
            static std::vector<std::string> s_vertFiles;
            static bool s_fontsInitialised = false;
            static bool deferredRefresh = false; // Flag to prevent mid-loop clearing

            auto rebuildFontFileLists = [&]()
                {
                    s_ttfFiles.clear();
                    s_jsonFiles.clear();
                    s_fragFiles.clear();
                    s_vertFiles.clear();

                    const std::string assetFolder = "../Assets/";
                    if (!std::filesystem::exists(assetFolder)) return;

                    // Use directory_options to skip permission errors
                    auto options = std::filesystem::directory_options::skip_permission_denied;

                    try {
                        for (auto& entry : std::filesystem::recursive_directory_iterator(assetFolder, options))
                        {
                            if (!entry.is_regular_file()) continue;

                            std::string ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(),
                                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                            std::string relPath = entry.path().generic_string();

                            if (ext == ".ttf")  s_ttfFiles.push_back(relPath);
                            else if (ext == ".json") s_jsonFiles.push_back(relPath);
                            else if (ext == ".frag") s_fragFiles.push_back(relPath);
                            else if (ext == ".vert") s_vertFiles.push_back(relPath);
                        }
                        std::sort(s_ttfFiles.begin(), s_ttfFiles.end());
                        std::sort(s_jsonFiles.begin(), s_jsonFiles.end());
                        std::sort(s_fragFiles.begin(), s_fragFiles.end());
                        std::sort(s_vertFiles.begin(), s_vertFiles.end());
                    }
                    catch (const std::exception& e) {
                        std::cerr << "FS Error: " << e.what() << "\n";
                    }

                    s_fontsInitialised = true;
                    deferredRefresh = false;
                };

            if (!s_fontsInitialised || deferredRefresh) rebuildFontFileLists();

            // Trigger the flag instead of calling the function directly
            if (ImGui::Button("Refresh Assets"))
            {
                deferredRefresh = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("Rescans Assets/ folder");

            ImGui::Separator();

            static std::string s_pendingDeleteOtherFile;
            static bool s_showDeleteOtherPopup = false;

            auto drawFileSection = [&](const char* title, std::vector<std::string>& files)
                {
                    ImGui::PushID(title);
                    char headerBuf[128];
                    std::snprintf(headerBuf, sizeof(headerBuf), "%s (%d)", title, (int)files.size());

                    if (ImGui::CollapsingHeader(headerBuf, ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (files.empty()) {
                            ImGui::TextDisabled("  None found.");
                        }

                        for (int idx = 0; idx < (int)files.size(); ++idx)
                        {
                            ImGui::PushID(idx);
                            std::string filename = std::filesystem::path(files[idx]).filename().string();

                            ImGui::Bullet();
                            ImGui::Selectable(filename.c_str());

                            if (ImGui::BeginPopupContextItem()) {
                                // FAILSAFE CHECK
                                bool isProtected = PROTECTED_ASSETS.count(filename);

                                if (isProtected) {
                                    ImGui::BeginDisabled();
                                    ImGui::MenuItem("Delete File (System Protected)");
                                    ImGui::EndDisabled();
                                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                                        ImGui::SetTooltip("This file is required by the engine and cannot be deleted.");
                                    }
                                }
                                else {
                                    if (ImGui::MenuItem("Delete File")) {
                                        s_pendingDeleteOtherFile = files[idx];
                                        s_showDeleteOtherPopup = true;
                                    }
                                }
                                ImGui::EndPopup();
                            }
                            ImGui::PopID();
                        }
                    }
                    ImGui::PopID();
                };

            drawFileSection("Fonts (.ttf)", s_ttfFiles);
            drawFileSection("JSON configs", s_jsonFiles);
            drawFileSection("Fragment shaders", s_fragFiles);
            drawFileSection("Vertex shaders", s_vertFiles);

            // -----------------------------------------------------------------
            // Delete confirmation modal
            // -----------------------------------------------------------------
            if (s_showDeleteOtherPopup)
            {
                ImGui::OpenPopup("Delete File Confirmation");
                s_showDeleteOtherPopup = false;
            }

            if (ImGui::BeginPopupModal("Delete File Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Are you sure you want to delete:\n%s ?", s_pendingDeleteOtherFile.c_str());
                ImGui::Separator();

                if (ImGui::Button("Yes, Delete", ImVec2(120, 0)))
                {
                    try
                    {
                        if (std::filesystem::exists(s_pendingDeleteOtherFile))
                        {
                            std::filesystem::remove(s_pendingDeleteOtherFile);
                            std::cout << "[AssetBrowser] Deleted file: " << s_pendingDeleteOtherFile << "\n";
                        }
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "[AssetBrowser] Failed to delete: " << e.what() << "\n";
                    }

                    s_pendingDeleteOtherFile.clear();
                    // Rebuild lists so the deleted file disappears immediately
                    rebuildFontFileLists();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    s_pendingDeleteOtherFile.clear();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::EndTabItem();
        }
        // ====================================================
       // RENDER LAYERS TAB
       // ====================================================
        if (ImGui::BeginTabItem("Render Layers"))
        {
            
            // s_layerEnabled[i] maps 1-to-1 onto the RenderLayer enum:
            //   0 = LAYER_BACKGROUND
            //   1 = LAYER_GAMEOBJECT
            //   2 = LAYER_FOREGROUND
            //   3 = LAYER_UI
            bool* layerEnabled = RenderSystem::GetLayerEnabledArray();

            // Count entities per layer so the editor can show them
            int counts[4] = { 0, 0, 0, 0 };
            Registry& reg = Registry::Get();
            for (auto& [entID, rc] : reg.Get_ComponentMap<RenderComponent>())
            {
                int idx = static_cast<int>(rc.GetRenderLayer());
                if (idx >= 0 && idx < 4) counts[idx]++;
            }
            // GuiSystem UI lives outside RenderComponent add it into the UI count
            {
                auto& gui = GuiSystem::Get();
                counts[LAYER_UI] += (int)gui.Buttons().size()
                    + (int)gui.TextBoxes().size()
                    + (int)gui.Dialogues().size()
                    + (int)gui.Displays().size();
            }
            static const char* layerNames[4] = {
                "Background",
                "GameObject",
                "Foreground",
                "UI"
            };


            static const ImVec4 layerColours[4] = {
                { 0.3f, 0.5f, 0.8f, 1.0f },   // blue-ish
                { 0.3f, 0.7f, 0.3f, 1.0f },   // reen-ish
                { 0.8f, 0.6f, 0.2f, 1.0f },   // orange-ish
                { 0.7f, 0.3f, 0.7f, 1.0f }    // purple-ish
            };

            ImGui::SeparatorText("Layer Visibility");
            ImGui::Spacing();

            float panelW = ImGui::GetContentRegionAvail().x;

            for (int i = 0; i < 4; ++i)
            {
                ImGui::PushID(i);

                // Coloured label strip on the left
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled( pos, ImVec2(pos.x + 6.0f, pos.y + ImGui::GetFrameHeight()),ImGui::ColorConvertFloat4ToU32(layerColours[i]));
                ImGui::SetCursorScreenPos(ImVec2(pos.x + 10.0f, pos.y));

                // Checkbox
                ImGui::Checkbox("##enable", &layerEnabled[i]);
                ImGui::SameLine();

                // Layer name
                if (layerEnabled[i])
                    ImGui::TextColored(layerColours[i], "%s", layerNames[i]);
                else
                    ImGui::TextDisabled("%s  (disabled)", layerNames[i]);

                // Entity count on the right side
                char countBuf[32];
                snprintf(countBuf, sizeof(countBuf), "%d entit%s", counts[i], counts[i] == 1 ? "y" : "ies");
                float countW = ImGui::CalcTextSize(countBuf).x;
                ImGui::SameLine(panelW - countW - 4.0f);
                ImGui::TextDisabled("%s", countBuf);

                ImGui::PopID();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // enable / disable all
            if (ImGui::Button("Enable All"))
            {
                for (int i = 0; i < 4; ++i) layerEnabled[i] = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Disable All"))
            {
                for (int i = 0; i < 4; ++i) layerEnabled[i] = false;
            }

            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
        //other than .wav
        if (g_showAudioErrorPopup)
        {
            ImGui::OpenPopup("Invalid Audio File");
            //reset so dont ope every frame
            g_showAudioErrorPopup = false;
        }

        if (ImGui::BeginPopupModal("Invalid Audio File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            //err msg
            ImGui::TextWrapped("%s", g_audioErrorMessage.c_str());
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        // Failsafe Warning Popup
        if (g_showProtectedFileError) {
            ImGui::OpenPopup("Protected File Warning");
            g_showProtectedFileError = false;
        }

        if (ImGui::BeginPopupModal("Protected File Warning", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "CRITICAL ASSET PROTECTION");
            ImGui::Separator();
            ImGui::Text("The file '%s' is required for the engine to run.", g_protectedFileName.c_str());
            ImGui::Text("Deleting shader/vertex files will cause the engine to crash on next load.");
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

    }

    // ------------------------------
    // Audio Import Popup (drag n drop)
    // ------------------------------
    if (g_showAudioImportPopup)
    {
        ImGui::OpenPopup("Import Audio File");
        g_showAudioImportPopup = false;
    }

    if (ImGui::BeginPopupModal("Import Audio File", nullptr,ImGuiWindowFlags_AlwaysAutoResize))
    {
        namespace fs = std::filesystem;
        //path of the audio file being imported
        ImGui::TextWrapped("Import audio file:\n%s", g_pendingAudioSourcePath.c_str());
        ImGui::Separator();

        //catergories dropdown
        const char* categories[] = { "Music", "SFX", "UI" };
        ImGui::Text("Import into category:");
        ImGui::Combo("##AudioCategory", &g_pendingAudioCategory, categories, IM_ARRAYSIZE(categories));

        //manifest key
        static char nameBuf[64] = "";
        // prefill name from filename if empty
        if (nameBuf[0] == '\0' && !g_pendingAudioSourcePath.empty())
        {
            std::string key = MakeAudioKeyFromFilename(g_pendingAudioSourcePath);
            strncpy(nameBuf, key.c_str(), sizeof(nameBuf));
            nameBuf[sizeof(nameBuf) - 1] = '\0';
        }

        ImGui::InputText("Manifest Name", nameBuf, IM_ARRAYSIZE(nameBuf));
        ImGui::TextDisabled("This is the name used in audio_manifest.json");

        ImGui::Separator();


        if (ImGui::Button("Import", ImVec2(120, 0)))
        {
            std::string assetKey = nameBuf;
            // prevent empty asset names
            if (assetKey.empty())
            {
                std::cout << "[AudioEditor] Name cannot be empty.\n";
            }
            else
            {
                try
                {
                    //folders
                    std::string catKey;
                    std::string groupStr;
                    std::string subdir;

                    switch (g_pendingAudioCategory)
                    {
                    default:
                    case 0: {
                        catKey = "music";
                        groupStr = "Music";
                        subdir = "music";
                        break;
                    }
                    case 1: {
                        catKey = "sfx";
                        groupStr = "SFX";
                        subdir = "sfx";
                        break;
                    }
                    case 2: {
                        catKey = "ui";
                        groupStr = "UI";
                        subdir = "ui";
                        break;
                        }
                    }
                    //copy file into folder
                    std::string destDir = "../Assets/audio/" + subdir + "/";
                    if (!fs::exists(destDir))
                        fs::create_directories(destDir);

                    //filename only like image
                    std::string destPath = destDir + fs::path(g_pendingAudioSourcePath).filename().string();

                    fs::copy_file(g_pendingAudioSourcePath, destPath, fs::copy_options::overwrite_existing);

                    std::cout << "[AudioEditor] Copied audio to: " << destPath << "\n";

                    // update manifest
                    nlohmann::json manifest;
                    if (!LoadAudioManifest(manifest))
                        //if dh make new
                        manifest = nlohmann::json::object();

                    if (!manifest.contains(catKey))
                        manifest[catKey] = nlohmann::json::object();

                    nlohmann::json& filecats = manifest[catKey];

                    filecats[assetKey] = {
                        { "path",  destPath },
                        { "loop",  false },
                        { "group", groupStr }
                    };

                    if (SaveAudioManifest(manifest))
                    {
                        // refresh cache so Audio tab see immediately
                        AudioCache::Initialize(AUDIO_MANIFEST_PATH);
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "[AudioEditor] Import failed: "  << e.what() << "\n";
                }

                // reset state
                g_pendingAudioSourcePath.clear();
                nameBuf[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            g_pendingAudioSourcePath.clear();
            nameBuf[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ImGuiSystem::ShowEntityCamera(ImVec2 viewportPos, ImVec2 viewportSize)
{
    SceneManager& sm = SceneManager::Get();
    bool isPlaying = /**/ !sm.inEditMode;
    Registry& reg = Registry::Get();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Mouse position relative to the viewport
    ImVec2 mousePos = ImGui::GetMousePos();
    Vec2 mouse(mousePos.x - viewportPos.x, mousePos.y - viewportPos.y);

    // --- Define your full world dimensions ---
    //const float worldWidth = 1920.f;
    //const float worldHeight = 1080.f;
    const float worldWidth = sceneMax.x - sceneMin.x;
    const float worldHeight = sceneMax.y - sceneMin.y;

    // --- Fixed mini-scale factor (smaller = more zoomed out) ---
    //const float scale = 0.2f;  // 20% of the real world size (mini-map view)

    float scale = (worldWidth != 0) ? viewportSize.x / worldWidth : 0.f;
    ImVec2 displaySize(worldWidth * scale, worldHeight * scale);

    // --- Center world inside the viewport ---
    Vec2 offset{ 0.0f,0.0f };

    // --- Draw background rectangle for visual clarity ---
    ImGui::BeginChild("MinimapRegion", displaySize);

    dl->AddRectFilled(
        { viewportPos.x + offset.x, viewportPos.y + offset.y },
        { viewportPos.x + offset.x + displaySize.x, viewportPos.y + offset.y + displaySize.y },
        IM_COL32(40, 40, 40, 255)
    );
    dl->AddRect(
        { viewportPos.x + offset.x, viewportPos.y + offset.y },
        { viewportPos.x + offset.x + displaySize.x, viewportPos.y + offset.y + displaySize.y },
        IM_COL32(100, 100, 100, 255)
    );
    ImGui::EndChild();


    // --- Mouse state ---
    bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

    static Vec2 dragOffset{ 0.f, 0.f };
    static bool isDragging = false;

    // --- Draw Camera zone ---
    Camera& cam = Camera::GetInstance();
    bool camHover = false;
    Vec2 camOffset = cam.GetPosition() - sceneMin;
    if (sm.GetSceneType() == ST_GAME)
    {
        {
            float camZoom = cam.GetZoom();
            camZoom = (camZoom != 0) ? 1 / camZoom : 0.f;
            camZoom *= scale;

            Vec2 camMin = cam.GetWorldMin();

            Vec2 camMax = cam.GetWorldMax();

            //ImVec2 min = { viewportPos.x + (camOffset.x * scale) - (worldWidth * camZoom) / 2,
            //               viewportPos.y + (worldHeight * scale) - (camOffset.y * scale) - (worldHeight * camZoom) / 2 };

            //ImVec2 max = { viewportPos.x + (camOffset.x * scale) + (worldWidth * camZoom) / 2,
            //               viewportPos.y + (worldHeight * scale) - (camOffset.y * scale) + (worldHeight * camZoom) / 2 };

            ImVec2 min = { viewportPos.x + (camOffset.x * scale) - (GLApp::VIRTUAL_W * camZoom) / 2,
                           viewportPos.y + (GLApp::VIRTUAL_H * scale) - (camOffset.y * scale) - (GLApp::VIRTUAL_H * camZoom) / 2 };

            ImVec2 max = { viewportPos.x + (camOffset.x * scale) + (GLApp::VIRTUAL_W * camZoom) / 2,
                           viewportPos.y + (GLApp::VIRTUAL_H * scale) - (camOffset.y * scale) + (GLApp::VIRTUAL_H * camZoom) / 2 };

            bool inside = (mousePos.x >= min.x && mousePos.x <= max.x &&
                mousePos.y >= min.y && mousePos.y <= max.y);

            ImU32 color = inside ? IM_COL32(255, 192, 203, 255) : IM_COL32(255, 0, 255, 255);

            dl->AddRect(min, max, color);
            if (inside) { camHover = true; }
        }
    }
    else
    {
        cam.ResetToCenter();
    }


    // === Draw all entities ===
    for (auto& [entity, transform] : reg.Get_ComponentMap<TransformComponent>())
    {
        Vec2 entPos = transform.pos;

        // Convert world → viewport (flip Y to top-left origin)
        Vec2 screenPos;
        screenPos.x = offset.x + entPos.x * scale;
        screenPos.y = offset.y + (worldHeight - entPos.y) * scale;

        Vec2 half{ 25.f * scale, 25.f * scale };

        ImVec2 min = { viewportPos.x + screenPos.x - half.x, viewportPos.y + screenPos.y - half.y };
        ImVec2 max = { viewportPos.x + screenPos.x + half.x, viewportPos.y + screenPos.y + half.y };

        ImU32 color = (entity == selectedEntity) ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 255, 255, 100);

        dl->AddRect(min, max, color, 0.f, 0, 2.f);
    }

    // Drag camera around
    if (!isPlaying && sm.GetSceneType() == ST_GAME)
    {
        if (mouseClicked && camHover)
        {
            isDragging = true;
            Vec2 mouseWorld{
                (mouse.x - offset.x) / scale,
                worldHeight - (mouse.y - offset.y) / scale
            };
            dragOffset = camOffset - mouseWorld;
        }

        if (isDragging && mouseDown)
        {
            Vec2 mouseWorld{
                (mouse.x - offset.x) / scale,
                worldHeight - (mouse.y - offset.y) / scale
            };
            cam.SetPosition(mouseWorld + dragOffset);

            float zoom = cam.GetZoom();
            zoom += ImGui::GetIO().MouseWheel * 0.2f;
            cam.SetZoom(zoom);
        }

        if (mouseReleased)
            isDragging = false;
    }


    if (sm.GetSceneType() == ST_MENU) ImGui::BeginDisabled(); 
    if (ImGui::Button("Reset"))
    {
        cam.SetZoom(1.0f);
        cam.ResetToCenter();
        std::cout << "[Editor] Camera reset.\n";
    }
    if (sm.GetSceneType() == ST_MENU) ImGui::EndDisabled();
    //ImGui::SeparatorText("World settings");

    //static const char* sceneTypes[] = { "Menu", "Game" };
    //int currType = static_cast<int>(SceneManager::Get().GetSceneType());
    //if (ImGui::Combo("Scene Type", &currType, sceneTypes, IM_ARRAYSIZE(sceneTypes)))
    //{
    //    SceneManager::Get().SetSceneType(static_cast<SceneType>(currType));
    //}

    //if (currType == ST_MENU) { ImGui::BeginDisabled(); }
    //ImGui::InputFloat2("Size", &sceneSize.x);
    //if (ImGui::Button("Set size"))
    //{
    //    if (sceneSize.x != 0 && sceneSize.y != 0)
    //    {
    //        sceneMin.x = 990 - sceneSize.x / 2;
    //        sceneMax.x = 990 + sceneSize.x / 2;
    //        sceneMin.y = 540 - sceneSize.y / 2;
    //        sceneMax.y = 540 + sceneSize.y / 2;
    //    }
    //    cam.SetBounds(sceneMin, sceneMax);
    //}
    //if (currType == ST_MENU) { ImGui::EndDisabled(); }


    //ImGui::SeparatorText("Camera spawn settings");
    //// Spawn position
    //ImGui::InputFloat2("Position", &camSpawnPos.x);
    //// Zoom value
    //ImGui::InputFloat("Zoom", &camSpawnZoom);

    //if (ImGui::Button("Set spawn to current setting"))
    //{
    //    camSpawnZoom = cam.GetZoom();
    //    camSpawnPos = cam.GetPosition();
    //}

    //if (ImGui::Button("Reset", ImVec2(80, 0)))
    //{
    //    cam.SetZoom(1.0f);
    //    cam.ResetToCenter();
    //    std::cout << "[Editor] Camera reset.\n";
    //}

}




void ImGuiSystem::ShowPrefabControls()
{
    ImGui::Text("Prefab Management:");

    static std::string pendingPrefabName; //waiting coz popup
    static bool triggerPopup = false;
    static std::string pendingDeletePrefab;
    static bool DeletePopup = false;
    static bool updateInstancesPopup = false;

    if (selectedEntity != INVALID_ENTITY)
    {
        static char prefabNameBuf[64] = "";
        //update entity name in hierarchy
        if (ImGui::InputText("Prefab Name", prefabNameBuf, IM_ARRAYSIZE(prefabNameBuf)))
        {
            std::string newName = prefabNameBuf;
            if (!newName.empty())
                Registry::Get().Get_EntityNameMap()[selectedEntity] = newName;
        }

        ImGui::SameLine();

        if (ImGui::Button("Save As Prefab"))
        {
            std::string prefabName = prefabNameBuf;
            if (prefabName.empty())
            {
                std::cout << "[Editor] Prefab name cannot be empty.\n";
            }
            else
            {
                std::string outPath = FilePath::PREFABS_FOLDER + prefabName + ".json";

                //upper lower check
                bool prefabExists = false;
                std::string existingName;
                std::string lowerInput = prefabName;
                std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), 
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                for (auto& entry : std::filesystem::directory_iterator(FilePath::PREFABS_FOLDER))
                {
                    if (entry.path().extension() == ".json")
                    {
                        //same as scene
                        std::string existing = entry.path().stem().string();
                        std::string lowerExisting = existing;
                        std::transform(lowerExisting.begin(), lowerExisting.end(), lowerExisting.begin(),
                            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                        if (lowerExisting == lowerInput)
                        {
                            prefabExists = true;
                            existingName = existing;
                            break;
                        }
                    }
                }

                if (prefabExists)
                {
                    pendingPrefabName = prefabName;
                    triggerPopup = true;
                    std::cout << "[Editor] Prefab '" << existingName << "' already exists (case-insensitive match).\n";
                }
                else
                {
                    //else save 
                    if (JsonLoad::SaveEntity(outPath, selectedEntity))
                    {
                        Registry::Get().Get_EntityNameMap()[selectedEntity] = prefabName;
                        std::cout << "[Editor] Saved new prefab: " << outPath << "\n";
                    }
                }
            }
        }

        //popup for replacing
        if (triggerPopup)
        {
            pendingPrefabName = prefabNameBuf;
            ImGui::OpenPopup("Prefab Replace Confirmation");
            triggerPopup = false;
        }

        if (ImGui::BeginPopupModal("Prefab Replace Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Prefab '%s' already exists.\nReplace?", pendingPrefabName.c_str());
            ImGui::Separator();

            if (ImGui::Button("Yes", ImVec2(120, 0)))
            {
                std::string outpath = FilePath::PREFABS_FOLDER + pendingPrefabName + ".json";
                if (JsonLoad::SaveEntity(outpath, selectedEntity))
                {
                    Registry::Get().Get_EntityNameMap()[selectedEntity] = pendingPrefabName;
                    std::cout << "[Editor] Replaced prefab: " << outpath << "\n";
                    updateInstancesPopup = true;
                    ImGui::CloseCurrentPopup();
                }
               // pendingPrefabName.clear();
                //ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("No", ImVec2(120, 0)))
            {
                //pendingPrefabName.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        // exceptional part
        if (updateInstancesPopup)
        {
            ImGui::OpenPopup("Update Prefab Instances?");
            updateInstancesPopup = false;
        }

        if (ImGui::BeginPopupModal("Update Prefab Instances?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Prefab has been replaced.\nUpdate existing instances?");
            ImGui::Separator();

            if (ImGui::Button("Yes, Update All", ImVec2(150, 0)))
            {
                auto& registry = Registry::Get();
                auto& nameMap = registry.Get_EntityNameMap();

                std::string targetName = pendingPrefabName;
                std::string targetLower = targetName;
                std::transform(targetLower.begin(), targetLower.end(), targetLower.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                // Collect ALL entities that match the name
                std::vector<Entity> replaceList;
                for (auto& [entID, name] : nameMap)
                {
                    std::string n = name;
                    std::transform(n.begin(), n.end(), n.begin(),
                        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                    if (n == targetLower)
                    {
                        replaceList.push_back(entID);
                    }
                }



                std::string prefabPath = FilePath::PREFABS_FOLDER + targetName + ".json";

                // Replace old entities
                for (Entity oldEnt : replaceList)
                {

                    // Check if transform exists
                    if (!registry.Has_Component<TransformComponent>(oldEnt))
                    {
                     
                        continue;
                    }

                    // Only preserve POSITION and optionally ROTATION from old entity
                    TransformComponent oldTransform = registry.Get_Component<TransformComponent>(oldEnt);

                    // Delete the old entity
                    registry.Remove_Entity(oldEnt);

                    // Spawn new entity from prefab at OLD position
                    Entity newEnt = JsonLoad::LoadEntityFromFile(prefabPath, oldTransform.pos.x, oldTransform.pos.y);

                    if (newEnt == INVALID_ENTITY)
                    {
                        continue;
                    }


                    // Verify new entity has transform
                    if (!registry.Has_Component<TransformComponent>(newEnt))
                    {
                        continue;
                    }

                    // Get reference to new transform
                    TransformComponent& newTransform = registry.Get_Component<TransformComponent>(newEnt);
                    newTransform.rot = oldTransform.rot;


                    // Update collider with NEW prefab scale
                    if (registry.Has_Component<ColliderComponent>(newEnt))
                    {
                        auto& col = registry.Get_Component<ColliderComponent>(newEnt);
                        col.UpdateHitboxes(newTransform.pos, newTransform.scale);
                    }

                    // Set the name
                    registry.Get_EntityNameMap()[newEnt] = targetName;
                }

                std::cout << "[Update Instances] Finished updating " << replaceList.size() << " instances" << std::endl;

                ImGui::CloseCurrentPopup();
                pendingPrefabName.clear();
            }

            ImGui::SameLine();

            if (ImGui::Button("No", ImVec2(80, 0)))
            {
                ImGui::CloseCurrentPopup();
                pendingPrefabName.clear();

            }

            ImGui::EndPopup();
        }

    }
    else
    {
        ImGui::TextDisabled("Select an entity to save as prefab");
    }

    //list of prefabs
    ImGui::Separator();
    ImGui::Text("Spawn / Manage Prefabs:");

    static std::vector<std::string> prefabList;
    prefabList.clear();

    for (auto& entry : std::filesystem::directory_iterator(FilePath::PREFABS_FOLDER))
    {
        if (entry.path().extension() == ".json")
            prefabList.push_back(entry.path().stem().string());
    }

    if (prefabList.empty())
    {
        ImGui::TextDisabled("No prefabs found in %s", FilePath::PREFABS_FOLDER.c_str());
        return;
    }

    ImGui::BeginChild("PrefabList", ImVec2(0, 180), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& prefabName : prefabList)
    {
        bool selected = (pendingPrefabPath == FilePath::PREFABS_FOLDER + prefabName + ".json");
        ImGui::PushID(prefabName.c_str());

        // Regular selectable item
        if (ImGui::Selectable(prefabName.c_str(), selected))
        {
            pendingPrefabPath = FilePath::PREFABS_FOLDER + prefabName + ".json";
            isPlacingPrefab = true;
            std::cout << "[Editor] Prefab '" << prefabName << "' selected for placement mode.\n";
        }

        // Right-click context menu
        if (ImGui::BeginPopupContextItem("PrefabContext"))
        {
            if (ImGui::MenuItem("Delete Prefab"))
            {
                pendingDeletePrefab = prefabName;
                DeletePopup = true;
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();
    }

    ImGui::EndChild();

    //same delete thing 
    if (DeletePopup)
    {
        ImGui::OpenPopup("Delete Prefab Confirmation");
        DeletePopup = false;
    }

    if (ImGui::BeginPopupModal("Delete Prefab Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to delete prefab '%s'?\nThis action cannot be undone.", pendingDeletePrefab.c_str());
        ImGui::Separator();

        if (ImGui::Button("Yes, Delete", ImVec2(130, 0)))
        {
            std::string filePath = FilePath::PREFABS_FOLDER + pendingDeletePrefab + ".json";
            try
            {
                std::filesystem::remove(filePath);
                std::cout << "[Editor] Deleted prefab: " << filePath << "\n";
            }
            catch (const std::exception& e)
            {
                std::cerr << "[Editor] Failed to delete prefab: " << e.what() << "\n";
            }

            pendingDeletePrefab.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            pendingDeletePrefab.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (isPlacingPrefab)
    {
        ImGui::TextColored(ImVec4(0.2f, 1.f, 0.2f, 1.f), "Placement Mode Active: Click in Game View to spawn. (0 to cancel)");
    }
}


//play stop button
void ImGuiSystem::ShowSimulationControls(bool& isPlaying)
{
    ImGui::SeparatorText("Simulation Controls");

    // center horizontally in game view
    float panelWidth = ImGui::GetContentRegionAvail().x;
    float buttonWidth = 80.f;
    float spacing = 16.f;
    float totalWidth = buttonWidth * 2 + spacing;
    float startX = (panelWidth - totalWidth) * 0.5f;
    ImGui::SetCursorPosX(startX);
    SceneManager& sm = SceneManager::Get();
    bool inEdit = sm.inEditMode;
    // ───────────────────────────────
     // PLAY BUTTON
     // ───────────────────────────────
    if (!inEdit) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Play", ImVec2(buttonWidth, 0)))
    {
        selectedEntity = INVALID_ENTITY;
        // Set GameViewRect FIRST before mode switch
        GameViewRect rect;
        rect.x = 0.0f;
        rect.y = 0.0f;
        rect.width = static_cast<float>(GLApp::width);
        rect.height = static_cast<float>(GLApp::height);
        GuiSystem::Get().SetGameViewRect(rect);
        // 1) Switch to PLAY first so camera becomes gameplay camera
        sm.EnterPlayMode();

        Camera& cam = Camera::GetInstance();
        cam.SetBounds(sceneMin, sceneMax);
        cam.SetPosition(camSpawnPos);
        cam.SetZoom(camSpawnZoom);

        // 2) NOW camera is GAME camera — save it
        SavedScene::SavedCamera camSet;
        camSet.pos = cam.GetPosition();
        camSet.zoom = cam.GetZoom();
        camSet.worldMin = cam.GetWorldMin();
        camSet.worldMax = cam.GetWorldMax();

        sm.SaveScene(sm.GetTempScenePath(), camSet);
      
        std::cout << "[Editor] Gameplay started.\n";
        isPlaying = true;

    }


    if (!inEdit) { ImGui::EndDisabled(); }

    ImGui::SameLine(0, spacing);

    // ───────────────────────────────
    // STOP BUTTON
    // ─────────────────────────────── 
    if (inEdit) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Stop", ImVec2(buttonWidth, 0)))
    {
        // Set GameViewRect FIRST before mode switch
        GameViewRect rect;
        rect.x = 0.0f;
        rect.y = 0.0f;
        rect.width = static_cast<float>(GLApp::width);
        rect.height = static_cast<float>(GLApp::height);
        GuiSystem::Get().SetGameViewRect(rect);
        // 1) Exit play mode → go back to editor rule set
        sm.EnterEditMode();

        // 2) Reload gameplay entities from temp
        JsonLoad::ClearAllEntities();
        JsonLoad::LoadScene(sm.GetTempScenePath());

        // 3) Restore EDITOR camera only (we do NOT use temp camera)
        Camera& cam = Camera::GetInstance();
        camSpawnPos = cam.GetPosition();
        camSpawnZoom = cam.GetZoom();
        sceneMin = cam.GetWorldMin();
        sceneMax = cam.GetWorldMax();
        sceneSize = sceneMax - sceneMin;

        cam.SetMode(Camera::Mode::EDITOR);
        cam.ResetToCenter();

        ImGuiSystem::selectedEntity = INVALID_ENTITY;

        std::cout << "[Editor] Gameplay stopped.\n";
        isPlaying = false;
    }
    if (inEdit) { ImGui::EndDisabled(); }

    // Status text
    ImGui::SameLine(spacing * 2);
    ImGui::TextColored(
        isPlaying ? ImVec4(0.2f, 0.8f, 0.2f, 1.f) : ImVec4(0.8f, 0.3f, 0.3f, 1.f),
        isPlaying ? "PLAYING" : "EDITING"
    );
}

void ImGuiSystem::ShowLayerCollisionMatrix()
{
    auto& clm = CollisionLayerManager::Get();
    const auto& nameMap = clm.GetLayerNameMap();

    if (nameMap.empty()) {
        ImGui::TextDisabled("No collision layers registered.");
        return;
    }

    // Collect + sort layers alphabetically
    //collision layer represented as a single bit inside a 32-bit mask
    struct Layer { uint32_t bit; std::string name; };
    std::vector<Layer> layers;

    for (auto& [bit, name] : nameMap)
        layers.push_back({ bit, name });
    // Sort alphabetically
    std::sort(layers.begin(), layers.end(), [](auto& a, auto& b) { return a.name < b.name; });

    ImGui::Separator();
    ImGui::SeparatorText("Collision Layer Matrix");
    ImGui::Separator();


    float minColSpacing = 35.0f;  // minimum so boxes not too cramped
    float colSpacing = minColSpacing;

    for (const auto& l : layers)
    {
        float w = ImGui::CalcTextSize(l.name.c_str()).x + 10.0f; // padding
        if (w > colSpacing)
            colSpacing = w;
    }

    float rowIndent = 120.0f; // space for row labels on the left

    //top header
    ImVec2 headerStart = ImGui::GetCursorScreenPos();
    float xStart = headerStart.x + rowIndent;
    float headerHeight = ImGui::GetTextLineHeightWithSpacing();

    ImDrawList* dl = ImGui::GetWindowDrawList();

    for (int i = 0; i < (int)layers.size(); i++)
    {
        float x = xStart + i * colSpacing;

        // invisible button reserves space so layout remains stable
        ImGui::SetCursorScreenPos(ImVec2(x, headerStart.y));
        ImGui::InvisibleButton(("##col" + std::to_string(i)).c_str(), ImVec2(colSpacing, headerHeight));
        
        
        ImVec2 cellMin = ImGui::GetItemRectMin();
        ImVec2 cellMax = ImGui::GetItemRectMax();

        // center horizontal text inside this cell
        ImVec2 textSize = ImGui::CalcTextSize(layers[i].name.c_str());
        float centerX = cellMin.x + (cellMax.x - cellMin.x) * 0.5f - textSize.x * 0.5f;
        float centerY = cellMin.y + (cellMax.y - cellMin.y) * 0.5f - textSize.y * 0.5f;

        //label
        dl->AddText(
            ImVec2(centerX, centerY),
            ImGui::GetColorU32(ImGuiCol_Text),
            layers[i].name.c_str()
        );
    }

    // push cursor down below header
    ImGui::Dummy(ImVec2(0, headerHeight + 5.0f));
    ImGui::Separator();

    //the triangle thing
    float rowHeight = ImGui::GetFrameHeightWithSpacing();

    for (int row = 0; row < (int)layers.size(); ++row)
    {
        // one bit representing row layer
        uint32_t rowBit = layers[row].bit;

        // start position for this row
        ImVec2 rowStart = ImGui::GetCursorScreenPos();

        // row label on the left
        ImGui::Text("%s", layers[row].name.c_str());

        //checkboxes in this row
        for (int col = 0; col <= row; ++col)
        {
            //one bit representing column layer
            uint32_t colBit = layers[col].bit;

            // whether row-layer currently collides with col-layer
            //from colili manager
            //returns 32-bit mask representing all layers this layer collides
            bool checked = (clm.GetLayerMask(rowBit) & colBit) != 0;

            float x = rowStart.x + rowIndent + col * colSpacing;
            float y = rowStart.y;

            ImGui::SetCursorScreenPos(ImVec2(x, y));
            ImGui::PushID(row * 100 + col);

            //checkbox toggling
            if (ImGui::Checkbox("##box", &checked))
            {
                // symetry update
                uint32_t maskA = clm.GetLayerMask(rowBit);
                uint32_t maskB = clm.GetLayerMask(colBit);

                if (checked) {
                    maskA |= colBit;
                    maskB |= rowBit;
                }
                else {
                    maskA &= ~colBit;
                    maskB &= ~rowBit;
                }

                clm.SetLayerMask(rowBit, maskA);
                clm.SetLayerMask(colBit, maskB);
            }

            ImGui::PopID();
        }

        // cursor to next row
        ImGui::SetCursorScreenPos(ImVec2(rowStart.x, rowStart.y + rowHeight));
    }
}

void ImGuiSystem::UpdateSpawnCamera() {
    Camera& cam = Camera::GetInstance();
    camSpawnPos = cam.GetPosition();
    camSpawnZoom = cam.GetZoom();
    sceneMin = cam.GetWorldMin();
    sceneMax = cam.GetWorldMax();
    sceneSize = sceneMax - sceneMin;
}


void ImGuiSystem::Shutdown() {
    ShutdownImGui();
}

void ImGuiSystem::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

#endif