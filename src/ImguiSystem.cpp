/* ========================================================
 * @file        ImguiSystem.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        06 NOV 2025
 *
 * @brief
 * Implements the ImGuiSystem, which provides the in-game editor
 * and debugging interface for the engine.
 *
 *
 * - Engine Info: Displays Framerate and system timings
 * - Collision + Scene: Shows collider stats, spatial grid stats and scene management
 * - Entity Editor + Prefabs: Allows editing, adding, and saving entities.
 * - Assets: Browser for textures and audio files via drag-and-drop import.
 * - Game View: Renders the current scene with play/stop simulation controls.
 * - Editor Camera: Provides overview of the level camera.
 *
 * It also supports prefab placement, entity dragging, file drag-and-drop
 * imports, and scene.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
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
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

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
        // Reload AudioCache fully
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
        catch (const std::exception& e)
        {
            std::cerr << e.what();
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
            out << manifest.dump(4);
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what();
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
        const char* categories[] = { "music", "sfx", "ui", "ambience" };
        for (const char* cat : categories)
        {
            if (manifest.contains(cat) &&
                manifest[cat].is_object() &&
                manifest[cat].contains(assetName))
            {
                filePath = manifest[cat][assetName].value("path", "");
                manifest[cat].erase(assetName);
                std::cout << "[AudioEditor] Removed '" << assetName
                    << "' from category '" << cat << "'\n";
                break;
            }
        }

        // remove from banks
        if (manifest.contains("banks") && manifest["banks"].is_object())
        {
            for (auto& [bankName, list] : manifest["banks"].items())
            {
                if (!list.is_array())
                    continue;
                auto& arr = list;
                for (size_t i = 0; i < arr.size();)
                {
                    if (arr[i].is_string() && arr[i].get<std::string>() == assetName)
                        arr.erase(arr.begin() + static_cast<long long>(i));
                    else
                        ++i;
                }
            }
        }

        if (!SaveAudioManifest(manifest))
            return false;

        // delete audio file (if exists)
        if (!filePath.empty())
        {
            try
            {
                if (std::filesystem::exists(filePath))
                {
                    std::filesystem::remove(filePath);
                    std::cout << "[AudioEditor] Deleted file: "
                        << filePath << "\n";
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
        entry["spatial"] = true; // optional field

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
    catch (const fs::filesystem_error& e)
    {
        std::cerr << e.what();
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

    // From Render Component
    if (reg.Has_Component<RenderComponent>(e))
    {
        auto& r = reg.Get_Component<RenderComponent>(e);
        switch (r.GetRenderType())
        {
        case RT_IMAGE:
        {
            Sprite::SpriteComponent& sprite = r.GetSprite();
            const Image& img = sprite.Get_Image();

            Vec2 worldSize = { img.widthPixels * RenderSystem::unitsPerPixel, img.heightPixels * RenderSystem::unitsPerPixel };
            Vec2 size = worldSize * r.GetSprite().Get_Scale() * t.scale;
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
    // Physics fallback
    else if (reg.Has_Component<ColliderComponent>(e))
    {
        auto& col = reg.Get_Component<ColliderComponent>(e);
        AABB a = col.Get_AABB();
        minW = a.min;
        maxW = a.max;
    }
    else return;

    // Convert world to panel
    ImVec2 tl = Render::GetInstance().WorldToPanel(minW, cursor, { w, h });
    ImVec2 br = Render::GetInstance().WorldToPanel(maxW, cursor, { w, h });

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Outer glow
    dl->AddRect(tl, br, IM_COL32(0, 255, 255, 60), 0.0f, 0, 6.0f);
    // Main outline
    dl->AddRect(tl, br, IM_COL32(0, 255, 255, 255), 0.0f, 0, 2.0f);

}
Entity PickEntityAtWorldPos(const Vec2& world)
{
    Registry& reg = Registry::Get();
    Entity best = INVALID_ENTITY;
    float bestDist = FLT_MAX;

    for (auto& [e, t] : reg.Get_ComponentMap<TransformComponent>())
    {
        if (!EntityManager::get().isAlive(e))
            continue;

        bool hit = false;

        if (reg.Has_Component<RenderComponent>(e))
        {
            auto& r = reg.Get_Component<RenderComponent>(e);
            RenderType rt = r.GetRenderType();
            Vec2 min, max;

            if (rt == RT_IMAGE)
            {
                Sprite::SpriteComponent& sprite = r.GetSprite();
                const Image& img = sprite.Get_Image();

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
                //const Image& img = sprite.Get_Image();

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

            hit = (world.x >= min.x && world.x <= max.x &&
                world.y >= min.y && world.y <= max.y);
        }

        else if (reg.Has_Component<ColliderComponent>(e))
        {
            auto& col = reg.Get_Component<ColliderComponent>(e);
            AABB a = col.Get_AABB();
            hit = (world.x >= a.min.x && world.x <= a.max.x &&
                world.y >= a.min.y && world.y <= a.max.y);
        }

        if (hit)
        {
            float dx = world.x - t.pos.x;
            float dy = world.y - t.pos.y;
            float d = dx * dx + dy * dy;

            if (d < bestDist)
            {
                bestDist = d;
                best = e;
            }
        }
    }

    return best;
}

inline float AngleToMouse(const Vec2& posWorld, const Vec2& mouseWorld)
{
    return glm::degrees(atan2(mouseWorld.y - posWorld.y,
        mouseWorld.x - posWorld.x));
}

inline float SmoothDampAngle(float current, float target, float factor)
{
    float diff = glm::mod(target - current + 540.0f, 360.0f) - 180.0f;
    return current + diff * factor;
}

// Widget templates for component data
// ========================================================
#pragma region WIDGETS

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


// Int      | For int lol
static void MakeWidget(Editor::Widget_Int& w, const std::string& label) {
    if (!w.ref) return;
    if (w.slider && w.min != w.max)
        ImGui::SliderInt(label.c_str(), w.ref, w.min, w.max);
    else
        ImGui::InputInt(label.c_str(), w.ref);
}

// Float    | For float lol
static void MakeWidget(Editor::Widget_Float& w, const std::string& label) {
    if (!w.ref) return;
    if (w.slider && w.min != w.max)
        ImGui::SliderFloat(label.c_str(), w.ref, w.min, w.max);
    else
        ImGui::InputFloat(label.c_str(), w.ref);
}

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


void ImGuiSystem::OnFileDropped(const std::string& path)
{
    namespace fs = std::filesystem;

    if (!fs::exists(path)) {
        std::cerr << "[ImGui Drop] File not found: " << path << std::endl;
        return;
    }

    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
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
            selectedEntity = ent;
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
        g_pendingAudioCategory = 0;         // default to Music
        g_showAudioImportPopup = true;      // popup handled in Assets window
        return;
    }

    if (ext == ".mp3" || ext == ".ogg" || ext == ".aac" || ext == ".flac")
    {
        g_audioErrorMessage = "Unsupported audio format: " + filename +
            "\n\nOnly .wav files are allowed for import.";
        g_showAudioErrorPopup = true;
        return;
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
    Render::GetInstance().EndSceneCapture();
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

    if (ImGui::Begin("UI Editor", nullptr)) {
        if (/* */!sm.inEditMode) ImGui::BeginDisabled();

        auto& gui = GuiSystem::Get();

        // UI Element Vectors
        auto& buttons = gui.Buttons();
        auto& textboxes = gui.TextBoxes();
        auto& dialogues = gui.Dialogues();

        // Next ID Helper
        auto nextID = [&]() {
            int mx = -1;
            for (auto& b : buttons)   mx = std::max(mx, b.ID);
            for (auto& t : textboxes) mx = std::max(mx, t.ID);
            for (auto& d : dialogues) mx = std::max(mx, d.ID);
            return mx + 1;
            };

        // Selection
        static SelectedType uiSelectedType = SelectedType::None;
        static int uiSelectedIndex = -1;

        // UI element counts
        int numBtn = (int)buttons.size();
        int numTxt = (int)textboxes.size();
        int numDlg = (int)dialogues.size();
        int total = numBtn + numTxt + numDlg;

        auto totalCount = [&]()
            {
                return (int)buttons.size() + (int)textboxes.size() + (int)dialogues.size();
            };

        auto globalToLocal = [&](int gIdx, SelectedType& outType, int& outLocal)
            {
                const int numBtn = (int)buttons.size();
                const int numTxt = (int)textboxes.size();
                if (gIdx < 0) { outType = SelectedType::None; outLocal = -1; return; }
                if (gIdx < numBtn) { outType = SelectedType::Button;  outLocal = gIdx; return; }
                if (gIdx < numBtn + numTxt) { outType = SelectedType::TextBox; outLocal = gIdx - numBtn; return; }
                outType = SelectedType::Dialogue; outLocal = gIdx - numBtn - numTxt;
            };

        auto localToGlobal = [&](SelectedType type, int lIdx)
            {
                const int numBtn = (int)buttons.size();
                const int numTxt = (int)textboxes.size();
                if (type == SelectedType::Button)   return lIdx;
                if (type == SelectedType::TextBox)  return numBtn + lIdx;
                if (type == SelectedType::Dialogue) return numBtn + numTxt + lIdx;
                return -1;
            };

        static bool openAddPopup = false;
        static int addTypeIdx = 0;
        const char* addTypeNames[] = { "Button", "TextBox", "Dialogue" };

        if (ImGui::Button("Add UI"))
        {
            openAddPopup = true;
            ImGui::OpenPopup("Add UI Type");
        }

        // Additional popup to choose which UI element type to add
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
                    b.action = ButtonAction::None;
                    b.ID = nextID();
                    gui.BindButtonAction(b);
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
                else // Dialogue
                {
                    // Prepare default text box
                    DialogueConfig d;
                    d.textBox.text = "";
                    d.textBox.basePos = { 510, 300 };
                    d.textBox.baseScale = 0.9f;
                    d.textBox.pos = d.textBox.basePos;
                    d.textBox.scale = d.textBox.baseScale;
                    d.textBox.color = { 0, 0, 0 };
                    d.textBox.centered = false;
                    d.isVisible = true;
                    d.jsonPath = FilePath::DIALOGUE;
                    d.sceneID = "0";
                    d.ID = nextID();
                    d.owned = std::make_shared<TextSystem::TextComponent>();
                    if (d.owned) 
                    {
                        TextSystem::LoadDialogueFromJson(d.jsonPath, *d.owned);
                        d.textComp = nullptr;
                    }
                    dialogues.push_back(std::move(d));

                    uiSelectedType = SelectedType::Dialogue;
                    uiSelectedIndex = localToGlobal(SelectedType::Dialogue, (int)dialogues.size() - 1);
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

                const TextSystem::TextComponent* sc = src.textComp ? src.textComp : src.owned.get();
                if (sc)
                {
                    dst.owned = std::make_shared<TextSystem::TextComponent>();
                    dst.owned->SetLines(sc->GetLines());
                    dst.owned->SetCurrentLineIndex(sc->GetCurrentLineIndex());
                    dst.textComp = dst.owned.get();
                }
                else
                {
                    dst.textComp = nullptr;
                }

                dialogues.push_back(std::move(dst));
                uiSelectedType = SelectedType::Dialogue;
                uiSelectedIndex = localToGlobal(SelectedType::Dialogue, (int)dialogues.size() - 1);
            }
        }

        ImGui::SameLine();
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
                    dialogues.erase(dialogues.begin() + lIdx);
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

        // UI HIERARCHY
        ImGui::SeparatorText("UI Elements");

        // Build flat list sorted by ID
        struct Row { SelectedType type; int local; int id; };
        std::vector<Row> flat;
        flat.reserve(buttons.size() + textboxes.size() + dialogues.size());
        for (int i = 0; i < (int)buttons.size(); ++i) flat.push_back({ SelectedType::Button,   i, buttons[i].ID });
        for (int i = 0; i < (int)textboxes.size(); ++i) flat.push_back({ SelectedType::TextBox,  i, textboxes[i].ID });
        for (int i = 0; i < (int)dialogues.size(); ++i) flat.push_back({ SelectedType::Dialogue, i, dialogues[i].ID });

        std::sort(flat.begin(), flat.end(), [](const Row& a, const Row& b) { return a.id < b.id; });

        // Resolve current selection to (type, local)
        SelectedType curSelType = uiSelectedType;
        int          curSelLocal = -1;
        if (uiSelectedIndex >= 0) {
            globalToLocal(uiSelectedIndex, curSelType, curSelLocal);
        }

        ImGui::BeginChild("List", ImVec2(0, 220), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (int i = 0; i < (int)flat.size(); ++i) 
        {
            const Row& r = flat[i];

            std::string label;
            if (r.type == SelectedType::Button) 
            {
                const auto& b = buttons[r.local];
                label = "Button: " + b.label + "  [ID " + std::to_string(b.ID) + "]";
            }
            else if (r.type == SelectedType::TextBox) 
            {
                const auto& tb = textboxes[r.local];
                label = "Text: " + tb.text + "  [ID " + std::to_string(tb.ID) + "]";
            }
            else 
            {
                const auto& d = dialogues[r.local];
                auto* comp = d.textComp ? d.textComp : d.owned.get();
                label = "Dialogue";
                if (comp && !comp->GetLines().empty()) 
                {
                    const auto& ln = comp->GetLines()[comp->GetCurrentLineIndex()];
                    label += ": " + (ln.size() > 24 ? (ln.substr(0, 24) + "...") : ln);
                }
                label += "  [ID " + std::to_string(d.ID) + "]";
            }

            bool selected = (r.type == curSelType && r.local == curSelLocal);
            if (ImGui::Selectable(label.c_str(), selected)) 
            {
                uiSelectedType = r.type;
                uiSelectedIndex = localToGlobal(r.type, r.local);

                // Re-arm dialogue typing on selection (keeps your previous behavior)
                if (r.type == SelectedType::Dialogue) 
                {
                    auto& d = dialogues[r.local];
                    auto* tc = d.textComp ? d.textComp : d.owned.get();
                    if (tc) GuiSystem::Get().SetDialogueLine(d, tc->GetCurrentLineIndex(), true);
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
            if (A.type == SelectedType::Button)      idA = &buttons[A.local].ID;
            else if (A.type == SelectedType::TextBox) idA = &textboxes[A.local].ID;
            else                                       idA = &dialogues[A.local].ID;

            if (B.type == SelectedType::Button)      idB = &buttons[B.local].ID;
            else if (B.type == SelectedType::TextBox) idB = &textboxes[B.local].ID;
            else                                       idB = &dialogues[B.local].ID;

            std::swap(*idA, *idB);
            };

        // pointer getter for an element's ID (works across all types)
        auto idPtrOf = [&](const Row& r) -> int* {
            if (r.type == SelectedType::Button)   return &buttons[r.local].ID;
            if (r.type == SelectedType::TextBox)  return &textboxes[r.local].ID;
            /* r.type == SelectedType::Dialogue */ return &dialogues[r.local].ID;
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

        // UI PROPER
        ImGui::SeparatorText("UI Properties");
        ImGui::BeginChild("Properties", ImVec2(0, 0), true);

        if (uiSelectedIndex < 0 || uiSelectedType == SelectedType::None)
        {
            ImGui::TextDisabled("No UI element selected.");
        }
        else
        {
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
                    // Action
                    {
                        const char* actionNames[] =
                        {
                            "None",
                            "Load Level 1",
                            "Load Level 2",
                            "Load Main Menu",
                            "Quit",
                            "Open State Window",
                            "Toggle All Buttons",
                            "Toggle All Textboxes",
                            "Toggle All Dialogues"
                        };
                        int actionCount = (int)(sizeof(actionNames) / sizeof(actionNames[0]));
                        int curr = (int)b.action;
                        curr = (curr >= 0 && curr < actionCount) ? curr : 0;
                        if (ImGui::Combo("Action", &curr, actionNames, actionCount))
                        {
                            b.action = (ButtonAction)curr;
                            GuiSystem::Get().BindButtonAction(b);
                        }
                    }
                    ImGui::Separator();
                    // Action new
                    {


                        std::vector<const char*> actionNames;
                        int selected = -1;
                        auto& map = GuiSystem::Get().GetActionMap();

                        actionNames.reserve(map.size());
                        for (auto& [name, func] : map)
                        {
                            actionNames.push_back(name.c_str());
                        }
                        for (size_t i = 0; i < actionNames.size(); i++)
                        {
                            if (b.actionName == actionNames[i]) {
                                selected = i;
                                break;
                            }
                        }

                        if (ImGui::Combo("ClickAction", &selected, actionNames.data(), actionNames.size()))
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
                            { sceneFilePtr.push_back(file.c_str()); }
                            // check scene index for selected scene
                            std::string currScene = b.actionParams["scene"];
                            for (size_t i = 0; i < sceneFilePtr.size(); i++)
                            {
                                if (currScene == sceneFilePtr[i])
                                {
                                    selectedScene = i;
                                    break;
                                }
                            }

                            if (ImGui::Combo("Go scene", &selectedScene, sceneFilePtr.data(), sceneFilePtr.size()))
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
                    // Render data
                    {
                        const char* typeNames[] = { "None", "Shape", "CustomQuad", "Image", "ImageSheet" };
                        int currType = (int)b.m_renderData.GetRenderType();
                        if (ImGui::Combo("RenderType", &currType, typeNames, IM_ARRAYSIZE(typeNames)))
                        {
                            b.m_renderData.SetRenderType((UIRenderType)currType);
                        }

                        switch (b.m_renderData.GetRenderType())
                        {
                        case UIRenderType::Shape:
                        {
                            auto& shape = b.m_renderData.GetShapeData();

                            const char* shapeNames[] = { "Rectangle", "Rounded", "Circle" };
                            int currShape = (int)shape.shape;
                            if (ImGui::Combo("Shape", &currShape, shapeNames, IM_ARRAYSIZE(shapeNames)))
                            {
                                shape.shape = (ButtonShape)currShape;
                            }

                            float col[3] = { shape.color.r, shape.color.g, shape.color.b};
                            if (ImGui::ColorEdit3("Color", col))
                            {
                                shape.color.r = col[0]; shape.color.g = col[1]; shape.color.b = col[2];
                            }
                        }
                        break; //end case SHAPE

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

                        case UIRenderType::Image:
                        {
                            auto& imgData = b.m_renderData.GetImageData();
                            Image& img = imgData.m_image;

                            // dropdown.enumMap = Render::GetInstance().Get_TextureCache();
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
                                    selected = i;
                                    break;
                                }
                            }


                            if (ImGui::Combo("textureIDnew", &selected, imgOptions.data(), imgOptions.size()))
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
                                scale *= 0.9f;

                                ImVec2 displaySize(texW * scale, texH * scale);

                                ImGui::Image((ImTextureID)(intptr_t)texID, displaySize, ImVec2(0, 1), ImVec2(1, 0));

                            }
                            
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

                        case UIRenderType::ImageSheet:
                        {
                            auto& imgsheet = b.m_renderData.GetImageSheetData();

                            Image& img = imgsheet.m_image;

                            // dropdown.enumMap = Render::GetInstance().Get_TextureCache();
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
                                    selected = i;
                                    break;
                                }
                            }

                            if (ImGui::Combo("textureIDnew", &selected, imgOptions.data(), imgOptions.size()))
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
                                scale *= 0.9f;

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

                            ImGui::InputInt("Rows", (int*)&imgsheet.total_rows);
                            if (imgsheet.total_rows < 1) { imgsheet.total_rows = 1; }
                            ImGui::InputInt("Columns", (int*)&imgsheet.total_cols);
                            if (imgsheet.total_cols < 1) { imgsheet.total_cols = 1; }

                            ImGui::Separator();

                            ImGui::SliderInt("Current Row", (int*)&imgsheet.frame_index.curr_row, 0, (int)imgsheet.total_rows-1);
                            ImGui::SliderInt("Current Col", (int*)&imgsheet.frame_index.curr_col, 0, (int)imgsheet.total_cols-1);


                            imgsheet.total_cols = std::max(1u, imgsheet.total_cols);
                            imgsheet.total_rows = std::max(1u, imgsheet.total_rows);

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
                        default:
                            break;
                        }
                    }
                    ImGui::Separator();
                    //// Shape
                    //{
                    //    const char* shapeNames[] = { "Rectangle", "Rounded", "Circle" };
                    //    int currShape = (int)b.shape;
                    //    if (ImGui::Combo("ButtonShape", &currShape, shapeNames, IM_ARRAYSIZE(shapeNames)))
                    //    {
                    //        b.shape = (ButtonShape)currShape;
                    //    }
                    //}
                    ImGui::Separator();
                    // Position / Size / Text scale
                    ImGui::Separator();
                    ImGui::Text("Transform");
                    ImGui::InputFloat("posX", &b.basePos.x, 0.1f);
                    ImGui::InputFloat("posY", &b.basePos.y, 0.1f);
                    ImGui::InputFloat("sizeX", &b.baseSize.x, 0.1f);
                    ImGui::InputFloat("sizeY", &b.baseSize.y, 0.1f);
                    ImGui::InputFloat("textScale", &b.baseTextScale, 0.1f);

                    //// Color
                    //ImGui::Separator();
                    //ImGui::Text("Color");
                    //float col[3] = { b.color.r, b.color.g, b.color.b };
                    //if (ImGui::ColorEdit3("Button Color", col))
                    //{
                    //    b.color.r = col[0]; b.color.g = col[1]; b.color.b = col[2];
                    //}

                    ImGui::Separator();
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

                    // Transform
                    ImGui::Separator();
                    ImGui::Text("Transform");
                    ImGui::SliderFloat("posX", &t.basePos.x, 0.f, 1920.f);
                    ImGui::SliderFloat("posY", &t.basePos.y, 0.f, 1080.f);
                    ImGui::SliderFloat("scale", &t.baseScale, 0.1f, 100.f);

                    // Color
                    ImGui::Separator();
                    ImGui::Text("Color");
                    float col[3] = { t.color.r, t.color.g, t.color.b };
                    if (ImGui::ColorEdit3("Text Color", col)) {
                        t.color.r = col[0]; t.color.g = col[1]; t.color.b = col[2];
                    }

                    ImGui::Separator();
                    if (ImGui::Button("Delete This Text")) {
                        textboxes.erase(textboxes.begin() + selectedLocal);
                        uiSelectedIndex = -1;
                        uiSelectedType = SelectedType::None;
                        clampSelectionAfterErase();
                        goto _UI_PROPS_END_;
                    }
                }
            }
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

                    static json dlgDoc;
                    static bool dlgLoaded = false;
                    static std::vector<std::string> sceneIDs;
                    if (!dlgLoaded)
                    {
                        try
                        {
                            std::ifstream ifs(FilePath::DIALOGUE);
                            if (ifs)
                            {
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
                            }
                            if (dlgDoc.contains("scenes") && dlgDoc["scenes"].is_array())
                            {
                                sceneIDs.clear();
                                for (auto& sc : dlgDoc["scenes"])
                                {
                                    sceneIDs.push_back(sc.value("sceneID", std::string("0")));
                                }
                                dlgLoaded = true;
                            }
                        }
                        catch (...)
                        {
                            dlgLoaded = true;
                        }
                        if (sceneIDs.empty()) sceneIDs.push_back("0");
                    }

                    static std::unordered_map<int, int> perDlgSceneIdx;
                    if (d.sceneID.empty()) d.sceneID = "0";
                    if (!perDlgSceneIdx.count(selectedLocal))
                    {
                        int initIdx = 0;
                        for (int i = 0; i < (int)sceneIDs.size(); ++i)
                            if (sceneIDs[i] == d.sceneID) { initIdx = i; break; }
                        perDlgSceneIdx[selectedLocal] = initIdx;
                    }

                    int& activeSceneIdx = perDlgSceneIdx[selectedLocal];
                    if (!sceneIDs.empty())
                    {
                        if (activeSceneIdx < 0) activeSceneIdx = 0;
                        if (activeSceneIdx >= (int)sceneIDs.size()) activeSceneIdx = (int)sceneIDs.size() - 1;
                    }
                    else
                    {
                        activeSceneIdx = 0;
                    }

                    auto applySceneToComp = [&](int sceneIdx)
                        {
                            if (!dlgDoc.contains("scenes") || !dlgDoc["scenes"].is_array()) return;
                            if (sceneIdx < 0 || sceneIdx >= (int)dlgDoc["scenes"].size()) return;

                            const auto& sc = dlgDoc["scenes"][sceneIdx];

                            std::vector<std::string> lines;
                            if (sc.contains("script") && sc["script"].is_array())
                            {
                                for (const auto& node : sc["script"])
                                {
                                    std::string txt = node.value("line", std::string(""));
                                    lines.push_back(txt);
                                }
                            }
                            if (lines.empty()) lines.push_back("");

                            comp->SetLines(lines);
                            comp->SetCurrentLineIndex(0);
                            comp->SetIsTyping(false);

                            comp->SetTextSpeed(sc.value("textSpeed", 30.0f));
                            comp->SetFontPath(sc.value("fontPath", std::string("../Assets/fonts/Marker.ttf")));
                            comp->SetFontSize(sc.value("fontSize", 1.0f));

                            if (sc.contains("color") && sc["color"].is_array() && sc["color"].size() == 3)
                            {
                                Vec3 c{ sc["color"][0].get<float>(), sc["color"][1].get<float>(), sc["color"][2].get<float>() };
                                comp->SetColor(c);
                                d.textBox.color = { c.x, c.y, c.z };
                            }

                            comp->StartTyping();
                        };

                    if (comp->GetLines().empty())
                    {
                        int wantIdx = 0;
                        for (int i = 0; i < (int)sceneIDs.size(); ++i)
                            if (sceneIDs[i] == d.sceneID) { wantIdx = i; break; }
                        activeSceneIdx = wantIdx;
                        if (activeSceneIdx < 0) activeSceneIdx = 0;
                        if (activeSceneIdx >= (int)sceneIDs.size()) activeSceneIdx = (int)sceneIDs.size() - 1;
                        applySceneToComp(activeSceneIdx);
                    }

                    ImGui::SeparatorText("Dialogue Text Box");
                    ImGui::SliderFloat("posX", &d.textBox.basePos.x, 0.f, 1920.f, "%.3f");
                    ImGui::SliderFloat("posY", &d.textBox.basePos.y, 0.f, 1080.f, "%.3f");
                    ImGui::SliderFloat("scale", &d.textBox.baseScale, 0.1f, 10.f, "%.3f");

                    float col[3] = { d.textBox.color.r, d.textBox.color.g, d.textBox.color.b };
                    if (ImGui::ColorEdit3("Text Color", col))
                    {
                        d.textBox.color = { col[0], col[1], col[2] };
                        comp->SetColor({ col[0], col[1], col[2] });
                    }

                    ImGui::SeparatorText("Scene");
                    {
                        std::vector<const char*> sceneItems;
                        sceneItems.reserve(sceneIDs.size());
                        for (auto& s : sceneIDs) sceneItems.push_back(s.c_str());

                        int prevScene = activeSceneIdx;
                        if (activeSceneIdx < 0) activeSceneIdx = 0;
                        if (!sceneItems.empty() && activeSceneIdx >= (int)sceneItems.size())
                            activeSceneIdx = (int)sceneItems.size() - 1;

                        ImGui::Combo("Scene ID", &activeSceneIdx, sceneItems.data(), (int)sceneItems.size());

                        if (!sceneItems.empty())
                        {
                            if (activeSceneIdx < 0) activeSceneIdx = 0;
                            if (activeSceneIdx >= (int)sceneItems.size()) activeSceneIdx = (int)sceneItems.size() - 1;
                        }
                        else
                        {
                            activeSceneIdx = 0;
                        }

                        if (activeSceneIdx != prevScene)
                        {
                            applySceneToComp(activeSceneIdx);
                            if (!sceneIDs.empty()) d.sceneID = sceneIDs[activeSceneIdx];
                            else d.sceneID = "0";
                        }
                    }

                    ImGui::SeparatorText("Lines");
                    if (comp)
                    {
                        const auto& lines = comp->GetLines();
                        int idx = comp->GetCurrentLineIndex();
                        int lineCount = (int)lines.size();

                        ImGui::Text("Current Line: %d / %d", lineCount ? (idx + 1) : 0, lineCount);

                        if (lineCount > 0)
                        {
                            static std::vector<std::string> lbls;
                            lbls.clear();
                            lbls.reserve(lines.size());
                            for (const auto& s : lines)
                            {
                                if (s.size() > 96) lbls.push_back(s.substr(0, 96) + "...");
                                else               lbls.push_back(s);
                            }
                            std::vector<const char*> citems;
                            citems.reserve(lbls.size());
                            for (auto& s : lbls) citems.push_back(s.c_str());

                            int prevIdx = idx;
                            ImGui::Combo("Current Line", &idx, citems.data(), (int)citems.size());
                            if (idx != prevIdx)
                            {
                                if (idx < 0) idx = 0;
                                if (idx >= lineCount) idx = lineCount - 1;
                                comp->SetCurrentLineIndex(idx);
                                comp->StartTyping();
                            }
                        }
                        else
                        {
                            ImGui::TextDisabled("No lines in this scene!");
                        }
                    }
                    else
                    {
                        ImGui::TextDisabled("No TextComponent attached!");
                    }

                    ImGui::Separator();
                    if (ImGui::Button("Delete This Dialogue"))
                    {
                        dialogues.erase(dialogues.begin() + selectedLocal);
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

        // Refresh UI element counts
        numBtn = (int)buttons.size();
        numTxt = (int)textboxes.size();
        numDlg = (int)dialogues.size();
        total = numBtn + numTxt + numDlg;
        {
            // Build a flat list of all UI elements for the dropdown
            static int comboIndex = -1;
            const int totalcnt = totalCount();

            static std::vector<std::string> labels;
            labels.clear();
            labels.reserve(totalcnt);

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
            else if (totalcnt == 0)
                comboIndex = -1;

            if (ImGui::Combo("UI Elements", &comboIndex,
                items.data(), (int)items.size()))
            {
                if (comboIndex >= 0 && comboIndex < totalcnt)
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
        total = numBtn + numTxt + numDlg;

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

            // -------------------------------------------------------
            // 1. Compose model matrix
            // -------------------------------------------------------
            glm::mat4 model = glm::translate(glm::mat4(1.0f),
                glm::vec3(t.pos.x, t.pos.y, 0.0f));

            model = glm::rotate(model, glm::radians(t.rot), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(t.scale.x, t.scale.y, 1.0f));

            // -------------------------------------------------------
            // 2. Camera matrices
            // -------------------------------------------------------
            Camera& cam = Camera::GetInstance();
            glm::mat4 view = cam.GetViewMatrix();
            glm::mat4 proj = cam.GetProjMatrix();

            ImGuizmo::SetOrthographic(true);

            // -------------------------------------------------------
            // 3. Gizmo Manipulate
            // -------------------------------------------------------
            ImGuizmo::Manipulate(
                glm::value_ptr(view),
                glm::value_ptr(proj),
                gizmoOperation,
                gizmoMode,
                glm::value_ptr(model)
            );

            bool isUsingNow = ImGuizmo::IsUsing();

            // -------------------------------------------------------
            // 4. UNDO: BeginModify (first frame gizmo starts)
            // -------------------------------------------------------
            if (isUsingNow && !wasGizmoUsing)
            {
                UndoSystem::Get().BeginModify(selectedEntity, t);
                lastScale = glm::vec3(1.0f);
            }

            // -------------------------------------------------------
            // 5. Apply transform while gizmo active
            // -------------------------------------------------------
            if (isUsingNow)
            {
                glm::vec3 translation, rotation3, scaleAbs;
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(model),
                    glm::value_ptr(translation),
                    glm::value_ptr(rotation3),
                    glm::value_ptr(scaleAbs)
                );

                // Position
                t.pos.x = translation.x;
                t.pos.y = translation.y;

                // Rotation (follow mouse)
                if (gizmoOperation == ImGuizmo::ROTATE)
                {
                    ImVec2 mouseScreen = ImGui::GetMousePos();

                    float u = (mouseScreen.x - cursor.x) / newWidth;
                    float v = (mouseScreen.y - cursor.y) / newHeight;

                    float ndcX = u * 2.0f - 1.0f;
                    float ndcY = (1.0f - v) * 2.0f - 1.0f;

                    Vec2 mouseWorld = cam.NDCToWorld({ ndcX, ndcY });
                    float targetRot = AngleToMouse(t.pos, mouseWorld);
                    t.rot = SmoothDampAngle(t.rot, targetRot, 0.18f);
                }

                //// Scale (incremental)
                //glm::vec3 scaleDelta = scaleAbs / lastScale;
                //lastScale = scaleAbs;

                // scaleAbs = { t.scale.x,t.scale.y,1.f };
                //if (reg.Has_Component<RenderComponent>(selectedEntity))
                //{
                //    auto& r = reg.Get_Component<RenderComponent>(selectedEntity);

                //    if (r.GetRenderType() == RT_IMAGE)
                //    {
                //        auto& sprite = r.GetSprite();
                //        Vec2 cur = sprite.Get_Scale();
                //        sprite.Set_Size({ cur.x * scaleDelta.x, cur.y * scaleDelta.y });
                //    }
                //    else if (r.GetRenderType() == RT_SPRITESHEET)
                //    {
                //        auto& sprite = r.GetSpriteSheet().Sprite();
                //        Vec2 cur = sprite.Get_Scale();
                //        sprite.Set_Size({ cur.x * scaleDelta.x, cur.y * scaleDelta.y });
                //    }
                //    else if (r.GetRenderType() == RT_SHAPE)
                //    {
                //        auto& shape = r.GetShape();
                //        shape.dimensions.x *= scaleDelta.x;
                //        shape.dimensions.y *= scaleDelta.y;
                //    }
                //}

                t.scale.x = scaleAbs.x;
                t.scale.y = scaleAbs.y;

                // Collider sync
                if (reg.Has_Component<ColliderComponent>(selectedEntity))
                {
                    auto& col = reg.Get_Component<ColliderComponent>(selectedEntity);
                    col.UpdateHitboxes(t.pos, t.scale);
                }
            }

            // -------------------------------------------------------
            // 6. UNDO: EndModify (first frame gizmo stops)
            // -------------------------------------------------------
            if (!isUsingNow && wasGizmoUsing)
            {
                UndoSystem::Get().EndModify(selectedEntity, t);
            }

            wasGizmoUsing = isUsingNow;
        }



        if (!isPlaying && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (!ImGuizmo::IsUsing())
            {
                // Convert mouse position to world position
                ImVec2 mousePos = ImGui::GetMousePos();

                ImVec2 imgMin = cursor;
                ImVec2 imgMax = ImVec2(cursor.x + newWidth, cursor.y + newHeight);

                if (mousePos.x >= imgMin.x && mousePos.x <= imgMax.x &&
                    mousePos.y >= imgMin.y && mousePos.y <= imgMax.y)
                {
                    float u = (mousePos.x - imgMin.x) / (imgMax.x - imgMin.x);
                    float v = (mousePos.y - imgMin.y) / (imgMax.y - imgMin.y);

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
                    sprite.Set_Size(Vec2((float)img.widthPixels, (float)img.heightPixels));
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

                // Physics (AABB sized to sprite; SetStatic(false))
                ColliderComponent col;

                // Build an AABB around current transform using sprite size
                Vec2 size = sprite.Get_Scale();         // your Sprite API's "size"
                Vec2 half = size * 0.5f;
                AABB aabb;
                aabb.min = t.pos - half;
                aabb.max = t.pos + half;

                // Build AABB
                col.Set_AABB(aabb);
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

    //new scene
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
    ImGui::SeparatorText("World settings");

    static const char* sceneTypes[] = { "Menu", "Game" };
    int currType = static_cast<int>(SceneManager::Get().GetSceneType());
    if (ImGui::Combo("Scene Type", &currType, sceneTypes, IM_ARRAYSIZE(sceneTypes)))
    {
        SceneManager::Get().SetSceneType(static_cast<SceneType>(currType));
    }

    if (currType == ST_MENU) { ImGui::BeginDisabled(); }
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
    // ------------------------------------------------------------

    // CAMERA SETTINGS
    // - Camera spawn position and zoom
    // - Reset to cente
    // ------------------------------------------------------------
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

    if (currType == ST_MENU) { ImGui::EndDisabled(); }
    // ------------------------------------------------------------

    // BACKGROUND MUSIC
    // - Music bound
    // - Fade out timing
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
            { selectedTrack = (int)musicTrackList.size()-1; }
        }
    }

    if (ImGui::Combo("BGM Track", &selectedTrack, musicTrackList.data(), musicTrackList.size()))
    {
        //std::string trackName = AudioCache::GetAssetName(selectedTrack);
        std::string trackName = musicTrackList[selectedTrack];
        sm.SetCurrentSceneBGM(trackName);
    }
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
        ImGui::Text("No entity selected");
        ImGui::Columns(1); //reset
        return;
    }
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
                    Editor::Widget_Int widgetcolstart(&temp.col_start, true, 0, spritesheet.GetTotalCols());
                    Editor::Widget_Int widgetcolend(&temp.col_end, true, temp.col_start, spritesheet.GetTotalCols());
                    Editor::Widget_Float widgetduration(&temp.duration);
                    //ImGui::PushItemWidth(displaySize.x/4);
                    MakeWidget(widgetrow, "row");
                    //ImGui::SameLine();
                    MakeWidget(widgetcolstart, "first col");
                    //ImGui::SameLine();
                    MakeWidget(widgetcolend, "last col");
                    //ImGui::PopItemWidth();
                    MakeWidget(widgetduration, "duration");

                    ImGui::Separator();
                    if (ImGui::SmallButton("Add State"))
                    {
                        spritesheet.AddState(static_cast<int>(spritesheet.GetStateFrames().size()), { 0,0,0 });
                    }

                    spritesheet.EditFrames(temp.row, temp.col_start, temp.col_end, temp.duration);

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


void ImGuiSystem::ShowAssetPreviewWindow()
{
    ImGui::Text("Asset Preview Browser");
    ImGui::Separator();
    if (ImGui::Button("Refresh"))
    {
        RefreshImageAssets();

    

    }
    ImGui::SameLine();
    ImGui::TextDisabled("Reloads asset folder");

    if (ImGui::BeginTabBar("AssetsTabs"))
    {
        // ====================================================
        // IMAGES TAB
        // ====================================================
        if (ImGui::BeginTabItem("Images"))
        {
            static bool initialized = false;

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
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

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
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
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
            static std::unordered_map<std::string, AudioAsset> audioList;
            auto& all = audioList;


            audioList = AudioCache::GetAllAssets();

            // categories
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

                    ImGui::PushID(name.c_str());

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




        ImGui::EndTabBar();
        if (g_showAudioErrorPopup)
        {
            ImGui::OpenPopup("Invalid Audio File");
            g_showAudioErrorPopup = false;
        }

        if (ImGui::BeginPopupModal("Invalid Audio File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextWrapped("%s", g_audioErrorMessage.c_str());
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
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

    if (ImGui::BeginPopupModal("Import Audio File", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        namespace fs = std::filesystem;

        ImGui::TextWrapped("Import audio file:\n%s",
            g_pendingAudioSourcePath.c_str());
        ImGui::Separator();
        const char* categories[] = { "Music", "SFX", "UI" };
        ImGui::Text("Import into category:");
        ImGui::Combo("##AudioCategory", &g_pendingAudioCategory,
            categories, IM_ARRAYSIZE(categories));

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
            if (assetKey.empty())
            {
                std::cout << "[AudioEditor] Name cannot be empty.\n";
            }
            else
            {
                try
                {
                    // determine folders
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

                    std::string destDir = "../Assets/audio/" + subdir + "/";
                    if (!fs::exists(destDir))
                        fs::create_directories(destDir);

                    std::string destPath = destDir + fs::path(g_pendingAudioSourcePath).filename().string();

                    fs::copy_file(g_pendingAudioSourcePath, destPath, fs::copy_options::overwrite_existing);

                    std::cout << "[AudioEditor] Copied audio to: " << destPath << "\n";

                    // update manifest
                    nlohmann::json manifest;
                    if (!LoadAudioManifest(manifest))
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
                        // refresh cache so Audio tab sees it immediately
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
                std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);

                for (auto& entry : std::filesystem::directory_iterator(FilePath::PREFABS_FOLDER))
                {
                    if (entry.path().extension() == ".json")
                    {
                        //same as scene
                        std::string existing = entry.path().stem().string();
                        std::string lowerExisting = existing;
                        std::transform(lowerExisting.begin(), lowerExisting.end(), lowerExisting.begin(), ::tolower);

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
                }
                pendingPrefabName.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("No", ImVec2(120, 0)))
            {
                pendingPrefabName.clear();
                ImGui::CloseCurrentPopup();
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
    struct Layer { uint32_t bit; std::string name; };
    std::vector<Layer> layers;

    for (auto& [bit, name] : nameMap)
        layers.push_back({ bit, name });

    std::sort(layers.begin(), layers.end(),
        [](auto& a, auto& b) { return a.name < b.name; });

    //// Buttons
    //if (ImGui::Button("Reload Collision Settings"))
    //    clm.LoadCollisionSettings();
    //ImGui::SameLine();
    //if (ImGui::Button("Save Collision Settings"))
    //    clm.SaveCollisionSettings();

    ImGui::Separator();
    ImGui::SeparatorText("Collision Layer Matrix");
    ImGui::Separator();

    // ------------------------------------------------
    // 1) Compute column width from max label width
    // ------------------------------------------------
    float minColSpacing = 35.0f;  // minimum so boxes not too cramped
    float colSpacing = minColSpacing;

    for (const auto& l : layers)
    {
        float w = ImGui::CalcTextSize(l.name.c_str()).x + 10.0f; // + padding
        if (w > colSpacing)
            colSpacing = w;
    }

    float rowIndent = 120.0f; // space for row labels on the left

    // ------------------------------------------------
    // 2) Draw header row (top labels)
    // ------------------------------------------------
    ImVec2 headerStart = ImGui::GetCursorScreenPos();
    float xStart = headerStart.x + rowIndent;
    float headerHeight = ImGui::GetTextLineHeightWithSpacing();

    ImDrawList* dl = ImGui::GetWindowDrawList();

    for (int i = 0; i < (int)layers.size(); i++)
    {
        float x = xStart + i * colSpacing;

        // Reserve column cell so layout below doesn't collide
        ImGui::SetCursorScreenPos(ImVec2(x, headerStart.y));
        ImGui::InvisibleButton(("##col" + std::to_string(i)).c_str(),
            ImVec2(colSpacing, headerHeight));

        ImVec2 cellMin = ImGui::GetItemRectMin();
        ImVec2 cellMax = ImGui::GetItemRectMax();

        // Center horizontal text inside this cell
        ImVec2 textSize = ImGui::CalcTextSize(layers[i].name.c_str());
        float centerX = cellMin.x + (cellMax.x - cellMin.x) * 0.5f - textSize.x * 0.5f;
        float centerY = cellMin.y + (cellMax.y - cellMin.y) * 0.5f - textSize.y * 0.5f;

        dl->AddText(
            ImVec2(centerX, centerY),
            ImGui::GetColorU32(ImGuiCol_Text),
            layers[i].name.c_str()
        );
    }

    // Push cursor down below header
    ImGui::Dummy(ImVec2(0, headerHeight + 5.0f));
    ImGui::Separator();

    // ------------------------------------------------
    // 3) Draw lower-triangle checkbox matrix
    // ------------------------------------------------
    float rowHeight = ImGui::GetFrameHeightWithSpacing();

    for (int row = 0; row < (int)layers.size(); ++row)
    {
        uint32_t rowBit = layers[row].bit;

        // Starting position for this row
        ImVec2 rowStart = ImGui::GetCursorScreenPos();

        // Row label on the left
        ImGui::Text("%s", layers[row].name.c_str());

        // Now manually place checkboxes in this row
        for (int col = 0; col <= row; ++col)
        {
            uint32_t colBit = layers[col].bit;

            bool checked = (clm.GetLayerMask(rowBit) & colBit) != 0;

            float x = rowStart.x + rowIndent + col * colSpacing;
            float y = rowStart.y;

            ImGui::SetCursorScreenPos(ImVec2(x, y));
            ImGui::PushID(row * 100 + col);

            if (ImGui::Checkbox("##box", &checked))
            {
                // Symmetric update (A with B == B with A)
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

        // Move cursor to next row manually
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