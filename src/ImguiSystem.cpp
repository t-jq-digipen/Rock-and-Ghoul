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


#define IMGUI_DEFINE_MATH_OPERATORS

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
#include "Render.h" 
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


// ========================================================
// File-Local Static Variables (Editor State)
// ========================================================

static bool isPlacingPrefab = false; //the mouse prefab spawner
static bool isDraggingEntity = false; //drag handles
static bool showReplacePopup = false; //replace same image

static std::string pendingPrefabPath; //prefab mouse thing
static std::string pendingImportPath; //holds new image replace
static std::string pendingDestPath; //store dest path of existing img

// runtime thumbnail cache
static std::vector<Image> g_loadedThumbnails;
static std::vector<std::string> g_imageNames;

//g_loadedThumbnails[0] = Image(texture for "player.png")
//g_imageNames[0] = "player.png"

static FMOD::Channel* previewChannel = nullptr; //audio preview channel and tracking

// ========================================================
// Static Member Definitions
// ========================================================

bool   ImGuiSystem::showColliders = false; //rendering system aabb boxes
bool   ImGuiSystem::showVelocity = false;
bool   ImGuiSystem::showSpatialGrid = false;
bool   ImGuiSystem::dragEditMode = false; //drag handle
bool   ImGuiSystem::guiEnabled = false; //f1 
Entity ImGuiSystem::selectedEntity = INVALID_ENTITY;
extern GSM gsm; //play/stop game state

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

    // ---- Audio manifest helpers ----
    nlohmann::json audioManifest; //settings per audio file
    bool audioManifestLoaded = false; //load once

    bool showAudioImportPopup = false; //can choose da files
    std::vector<std::string> pendingAudioImports; //Q of audio file paths 
    float globalVolume = 0.8f;
    std::unordered_map<std::string, FMOD::Channel*> activeAudioChannels;


    bool LoadAudioManifest(const std::string& path)
    {
        namespace fs = std::filesystem;
        if (!fs::exists(path)) {
            audioManifest = { {"music", {}}, {"ui", {}}, {"sfx", {}} };
            audioManifestLoaded = true;
            return false;
        }

        try {
            std::ifstream ifs(path);
            if (ifs.is_open()) {
                ifs >> audioManifest;
                ifs.close();
                audioManifestLoaded = true;
                return true;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[AudioManifest] Load failed: " << e.what() << "\n";
        }
        return false;
    }

    void SaveAudioManifest(const std::string& path)
    {
        try {
            std::ofstream out(path);
            out << std::setw(4) << audioManifest; //formatting
            std::cout << "[AudioManifest] Saved to " << path << "\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[AudioManifest] Save failed: " << e.what() << "\n";
        }
    }
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
    if (ext == ".wav" || ext == ".mp3")
    {
        //popup for storing files
        pendingAudioImports.push_back(path);
        showAudioImportPopup = true;
        std::cout << "[ImGui Drop] Queued audio import: " << path << std::endl;
        return;
    }

    std::cout << "[ImGui Drop] Unsupported file type: " << ext << std::endl;

   
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
    gsm.inEditMode = false;  // not in editor mode
    gsm.isPaused = false;    // unpause game logic / physics
}


void ImGuiSystem::Update(float dt)
{
    (void)dt;
    if (Input::IsKeyPressed(GLFW_KEY_F1))
        guiEnabled = !guiEnabled;

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
        ImGuiID dock_left_top = ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Up, 0.4f, nullptr, &dock_left);
        //wtv left
        ImGuiID dock_left_bottom = dock_left;


        ImGui::DockBuilderDockWindow("Editor Camera", dock_left_top);
        ImGui::DockBuilderDockWindow("Engine Info", dock_left_bottom);
        ImGui::DockBuilderDockWindow("Entity Editor + Prefabs", dock_right);
        ImGui::DockBuilderDockWindow("Collision + Scene", dock_bottom);
        ImGui::DockBuilderDockWindow("Assets", dock_bottom);
        ImGui::DockBuilderDockWindow("Game View", dockspaceID);
        ImGui::DockBuilderFinish(dockspaceID);
    }
    

    if (ImGui::Begin("Editor Camera")) {
        if (!gsm.isPaused && !gsm.inEditMode) ImGui::BeginDisabled();
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
        if (!gsm.isPaused && !gsm.inEditMode) ImGui::EndDisabled();
    }
    ImGui::End();



    if (ImGui::Begin("Engine Info")) {
        if (!gsm.isPaused && !gsm.inEditMode) ImGui::BeginDisabled();
        ShowEngineDebugWindow();
        if (!gsm.isPaused && !gsm.inEditMode) ImGui::EndDisabled();
    }
    ImGui::End();



    if (ImGui::Begin("Collision + Scene")) {
        if (!gsm.isPaused && !gsm.inEditMode) ImGui::BeginDisabled();
        ShowCollisionDebugWindow();
        if (!gsm.isPaused && !gsm.inEditMode) ImGui::EndDisabled();
    }
    ImGui::End();

    
    if (ImGui::Begin("Entity Editor + Prefabs")) {
        if (!gsm.isPaused && !gsm.inEditMode) ImGui::BeginDisabled();
        // MakeNeaterNextTime
        if (ImGui::Button("Add Entity"))
        {
            Entity newEntity = m_registry.Create_Entity();
            selectedEntity = newEntity;

            // Store editor name
            m_registry.Get_EntityNameMap()[newEntity] = "New Entity " + std::to_string(newEntity);

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

            reg.Load_Entity(src);
        }

        ImGui::SameLine();
        if (ImGui::Button("Delete Entity") && selectedEntity != INVALID_ENTITY)
        {
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

        if (!gsm.isPaused && !gsm.inEditMode) ImGui::EndDisabled();
        
    }
    ImGui::End();


    if (ImGui::Begin("Assets")) {
        if (!gsm.isPaused && !gsm.inEditMode) ImGui::BeginDisabled();
        ShowAssetPreviewWindow();
        if (!gsm.isPaused && !gsm.inEditMode) ImGui::EndDisabled();
       
    }
    ImGui::End();
    

    // game view panel
    if (ImGui::Begin("Game View")) {
        bool isPlaying = !gsm.isPaused && !gsm.inEditMode;
        ShowSimulationControls(isPlaying);
        isPlaying = !gsm.isPaused && !gsm.inEditMode;
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
        ImGui::Image( (ImTextureID)(intptr_t)texID, ImVec2(newWidth, newHeight), ImVec2(0, 1), ImVec2(1, 0) );

        //GUISYSTEM STUFF SCALE FOR MOUSE
        GameViewRect rect;
        rect.x = cursor.x;
        rect.y = cursor.y;
        rect.width = newWidth;
        rect.height = newHeight;

        GuiSystem::Get().SetGameViewRect(rect);

        if (dragEditMode) {
            DrawEntityDragHandles(cursor, ImVec2(newWidth, newHeight));
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
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate); //ms
    CoreEngine& engine = CoreEngine::GetInstance(); //in coreengine.h
    auto& timings = engine.GetTimings();

    // total frame time
    double total = 0.0;
    for (auto& [name, t] : timings) total += t.ms;

    ImGui::Separator();
    ImGui::Text("System Timings:");
    //each system and display its timing
    for (auto& [name, t] : timings) { //gets reference to the real map
        // percentage of total frame time this system used
        double percent = (total > 0.0) ? (t.ms / total) * 100.0 : 0.0;
        //system name, elapsed time in ms, and percentage of total frame
        ImGui::Text("%s: %.3f ms (%.1f%%)", name.c_str(), t.ms, percent);

        //visual bar
        ImGui::ProgressBar((float)(percent / 100.0f), ImVec2(-1, 0), (std::to_string((int)percent) + "%").c_str());
    }

    ImGui::Separator();
    ImGui::Checkbox("Enable Drag Handles", &dragEditMode);

}

//collili
void ImGuiSystem::ShowCollisionDebugWindow() {
    // count active colliders
    int activeColliders = 0;
    int totalColliders = 0;

    for (auto& [entity, physics] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>()) {
        totalColliders++;
        if (physics.CollisionActive()) { // Use the physics comp collision active check
            activeColliders++;
        }
    }

    if (!gsm.isPaused && !gsm.inEditMode) ImGui::EndDisabled();

    ImGui::Text("AABB Colliders: %d / %d active", activeColliders, totalColliders);
    ImGui::Checkbox("Show Colliders", &showColliders);
    ImGui::Checkbox("Show Velocity", &showVelocity);
    ImGui::Checkbox("Show Spatial Grid", &showSpatialGrid);

    // Spatial Grid Statistics
    //FILLTHISSHITYASM
    ImGui::Separator();
    ImGui::Text("Spatial Grid (Broad Phase):");
    SpatialGrid* grid = CollisionManager::GetSpatialGrid();
    if (grid) {
        auto stats = grid->GetStats();
        size_t collisionCellCount = grid->GetCollisionCellCount();

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

    if (!gsm.isPaused && !gsm.inEditMode) ImGui::BeginDisabled();
    ImGui::Text("Scene Management:");
    ShowSceneBrowser();

}

void ImGuiSystem::ShowSceneBrowser()
{
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

    ImGui::Separator();

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
        ImGui::Text("Are you sure you want to delete scene '%s'?",pendingDeleteScene.c_str());
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
        std::string sceneName = sceneFiles[selectedScene];
        std::string path = sceneFolder + sceneName + ".json";

        JsonLoad::ClearAllEntities();

        if (JsonLoad::LoadScene(path))
        {
            std::cout << "[Editor] Loaded scene: " << path << "\n";


            if (sceneName == "Level1" || sceneName == "level1")
            {
                GSM_SetNextState(GameState::GS_LEVEL1);
                std::cout << "[Editor] Switching to LEVEL 1 state.\n";
            }
            else if (sceneName == "Level2" || sceneName == "level2")
            {
                GSM_SetNextState(GameState::GS_LEVEL2);
                std::cout << "[Editor] Switching to LEVEL 2 state.\n";
            }
            else
            {
                std::cout << "[Editor] Loaded custom scene (no state switch).\n";
            }
        }
        else
        {
            std::cerr << "[Editor] Failed to load scene: " << path << "\n";
        }
    }


    ImGui::SameLine();
    if (ImGui::Button("Save Current Scene") && selectedScene >= 0)
    {
        std::string path = sceneFolder + sceneFiles[selectedScene] + ".json";
        JsonLoad::SaveCurrentScene(path, sceneFiles[selectedScene]);
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
            }
        }
    }
}

void ImGuiSystem::ShowHierarchyWindow() {

    EnsureSelectionValid();

    //if (ImGui::Button("Add Entity"))
    //{
    //    Entity newEntity = m_registry.Create_Entity();
    //    selectedEntity = newEntity;

    //    // Store editor name
    //    m_registry.Get_EntityNameMap()[newEntity] = "New Entity " + std::to_string(newEntity);

    //}

    //ImGui::SameLine();
    //if (ImGui::Button("Clone Entity") && selectedEntity != INVALID_ENTITY)
    //{
    //    Registry& reg = Registry::Get();
    //    SavedEntity src = reg.Save_Entity(selectedEntity);

    //    //filter out empty json data
    //    for (auto it = src.componentData.begin(); it != src.componentData.end();)
    //    {
    //        if (it->second.empty() || it->second == "{}" || it->second == "null")
    //        {
    //            std::cout << "[Clone] Skipping empty component: " << it->first << "\n";
    //            it = src.componentData.erase(it);
    //        }
    //        else
    //        ++it;
    //    }

    //   reg.Load_Entity(src);
    //}

    //ImGui::SameLine();
    //if (ImGui::Button("Delete Entity") && selectedEntity != INVALID_ENTITY)
    //{
    //    m_registry.Remove_Entity(selectedEntity);
    //    selectedEntity = INVALID_ENTITY;
    //}
    //
    //ImGui::Separator();


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

        // Custom data fields for animator component
        if (compName == "Animator")
        {
            Animator::AnimatorComponent& animcomp = m_registry.Get_Component<Animator::AnimatorComponent>(selectedEntity);
            if (ImGui::TreeNode(compName.c_str()))
            {
                // --- show component fields ---
                for (auto& [name, widgetVariant] : componentData.GetDataMap())
                {
                    ImGui::PushID(name.c_str());
                    std::visit([&](auto& widget) {
                        MakeWidget(widget, name);
                        }, widgetVariant);
                    ImGui::PopID();
                }

                // need to figure out how to add state...
                animcomp.UpdateUV();
                Animator::ActiveFrames temp = animcomp.GetFrames();

                Editor::Widget_Int widgetrow(&temp.row, true, 0, animcomp.GetTotalRows());
                Editor::Widget_Int widgetcolstart(&temp.col_start, true, 0, animcomp.GetTotalCols());
                Editor::Widget_Int widgetcolend(&temp.col_end, true, temp.col_start, animcomp.GetTotalCols());
                Editor::Widget_Float widgetduration(&temp.duration);
                ImGui::PushItemWidth(50);
                MakeWidget(widgetrow, "row");
                ImGui::SameLine();
                MakeWidget(widgetcolstart, "first col");
                ImGui::SameLine();
                MakeWidget(widgetcolend, "last col");
                ImGui::PopItemWidth();
                MakeWidget(widgetduration, "duration");

                ImGui::Separator();
                if (ImGui::SmallButton("Add State"))
                {
                    animcomp.AddState(animcomp.GetStateFrames().size(), { 0,0,0 });
                }

                animcomp.EditFrames(temp.row, temp.col_start, temp.col_end, temp.duration);
                

                ImGui::TreePop();
            }

        }
        // Custom data fields for behaviour component
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
            m_registry.Has_Component<Physics::PhysicsComponent>(selectedEntity) &&
            m_registry.Has_Component<TransformComponent>(selectedEntity))
        {
            auto& p = m_registry.Get_Component<Physics::PhysicsComponent>(selectedEntity);
            auto& t = m_registry.Get_Component<TransformComponent>(selectedEntity);
            p.UpdateHitboxes(t.pos);
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
                if (compName == "Animator" &&
                    m_registry.Has_Component<Sprite::SpriteComponent>(eID))
                {
                    auto& sprite = m_registry.Get_Component<Sprite::SpriteComponent>(eID);
                    sprite.Set_Animated(false);
                }

                m_registry.Remove_Component(eID, typeID);
                std::cout << "[Editor] Removed component from entity "<< eID << "\n";
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
    ImGui::Text( "Asset Preview Browser");
    ImGui::Separator();
    if (ImGui::Button("Refresh"))
    {
        RefreshImageAssets();

        const std::string manifestPath = "../Assets/audio_manifest.json";
        audioManifest.clear();
        audioManifestLoaded = false;
        if (!LoadAudioManifest(manifestPath))
        {
            std::cout << "[ImGui Audio] Manifest missing or invalid. Creating new.\n";
            audioManifest = json::object();
            audioManifest["music"] = json::object();
            audioManifest["ui"] = json::object();
            audioManifest["sfx"] = json::object();
        }

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
            static bool manifestLoaded = false;
            const std::string manifestPath = "../Assets/audio_manifest.json";

            if (!manifestLoaded)
            {
                if (!LoadAudioManifest(manifestPath))
                {
                    std::cout << "[ImGui Audio] Manifest missing or invalid. Creating new.\n";
                    audioManifest = json::object();
                    audioManifest["music"] = json::object();
                    audioManifest["ui"] = json::object();
                }
                manifestLoaded = true;
            }

            //safety check
            if (!audioManifest.is_object() || audioManifest.empty())
            {
                ImGui::TextDisabled("No audio manifest data available.");
                ImGui::EndTabItem();
                return;
            }
            ImGui::Separator();


            //existing
            try
            {
                for (auto& [category, assets] : audioManifest.items())
                {
                    if (!assets.is_object()) continue;

                    if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        for (auto& [name, data] : assets.items())
                        {
                            if (!data.is_object()) continue;

                            ImGui::PushID(name.c_str());

                            //get val
                            std::string path = data.value("path", "(none)");                           
                            std::string group = data.value("group", "(none)");                        

                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.6f, 1.0f));
                            ImGui::Text("%s", name.c_str());
                            ImGui::PopStyleColor();

                            ImGui::Indent();
                            ImGui::Text("Path: %s", path.c_str());                           
                            ImGui::Text("Group: %s", group.c_str());

                            //buttons
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                            if (ImGui::Button("Play"))
                            {
                                FMOD::Sound* snd = AudioSystem::LoadSound(path, false);
                                if (snd)
                                {
                                    if (previewChannel) previewChannel->stop();
                                    previewChannel = AudioSystem::PlaySoundGetChannel(snd, false);

                                }
                            }
                            ImGui::PopStyleColor();

                            ImGui::SameLine();
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
                            if (ImGui::Button("Stop"))
                            {
                                if (previewChannel) previewChannel->stop();
                                previewChannel = nullptr;
                            }
                            ImGui::PopStyleColor();


                            ImGui::SameLine();
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.3f, 0.1f, 1.0f));
                            bool removeThis = false;
                            if (ImGui::Button("Remove"))
                                removeThis = true;
                            ImGui::PopStyleColor();

                            ImGui::Unindent();
                            ImGui::Separator();
                            ImGui::PopID();

                            if (removeThis)
                            {
                                assets.erase(name);
                                SaveAudioManifest(manifestPath);
                                break;
                            }
                        }
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "[ImGui AudioManifest] Exception: " << e.what() << std::endl;
            }

            ImGui::EndTabItem();
        }

        // ========================================================
        // Audio Import Popup
        // ========================================================
        if (showAudioImportPopup)
            ImGui::OpenPopup("Import Audio Files");

        if (ImGui::BeginPopupModal("Import Audio Files", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Select category and confirm import:");

            static int selectedCategory = 0;
            const char* categories[] = { "music", "ui", "sfx" };
            ImGui::Combo("Category", &selectedCategory, categories, IM_ARRAYSIZE(categories));

           
            ImGui::Separator();

            if (pendingAudioImports.empty())
                ImGui::TextDisabled("(No audio files pending)");
            else
                for (auto& file : pendingAudioImports)
                    ImGui::BulletText("%s", file.c_str());

            if (ImGui::Button("Import", ImVec2(120, 0)))
            {
                namespace fs = std::filesystem;
                const std::string manifestPath = "../Assets/audio_manifest.json";
                if (!audioManifestLoaded) LoadAudioManifest(manifestPath);

                for (auto& path : pendingAudioImports)
                {
                    std::string destDir = "../Assets/Audio/";
                    if (!fs::exists(destDir)) fs::create_directories(destDir);
                    std::string destPath = destDir + fs::path(path).filename().string();

                    try { fs::copy_file(path, destPath, fs::copy_options::overwrite_existing); }
                    catch (...) {}

                    std::string baseName = fs::path(destPath).stem().string();
                    audioManifest[categories[selectedCategory]][baseName] = {
                         { "path", destPath },
                         { "group", categories[selectedCategory] },
                         { "loop", false },
                         { "spatial", false },
                         { "minDistance", 1.0f },
                         { "maxDistance", 100.0f }
                    };

                    FMOD::Sound* snd = AudioSystem::LoadSound(destPath, false);
                    if (snd)
                    {
                        FMOD::Channel* ch = AudioSystem::PlaySoundGetChannel(snd, false);
                        if (ch) {
                            ch->setVolume(globalVolume);
                            activeAudioChannels[baseName] = ch;
                        }
                    }

                    std::cout << "[ImGui AudioImport] Imported and playing: " << baseName << "\n";
                }

                SaveAudioManifest(manifestPath);
                pendingAudioImports.clear();
                showAudioImportPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                pendingAudioImports.clear();
                showAudioImportPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        ImGui::SeparatorText("Currently Playing");

        if (activeAudioChannels.empty())
            ImGui::TextDisabled("(none)");
        else {
            for (auto& [name, ch] : activeAudioChannels)
            {
                bool playing = false;
                if (ch) ch->isPlaying(&playing);
                ImGui::Text("%s %s", name.c_str(), playing ? "(Playing)" : "(Stopped)");
                ImGui::SameLine();
                if (ImGui::Button(("Stop##" + name).c_str()))
                {
                    if (ch) ch->stop();
                    activeAudioChannels.erase(name);
                    break;
                }
            }
        }

        ImGui::EndTabBar();
    }
}

//FILLTHISSHITJQ
void ImGuiSystem::ShowEntityCamera(ImVec2 viewportPos, ImVec2 viewportSize)
{
    (void)viewportSize;
    bool isPlaying = !gsm.isPaused && !gsm.inEditMode;
    Registry& reg = Registry::Get();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Mouse position relative to the viewport
    ImVec2 mousePos = ImGui::GetMousePos();
    Vec2 mouse(mousePos.x - viewportPos.x, mousePos.y - viewportPos.y);

    // --- Define your full world dimensions ---
    const float worldWidth = 1920.f;
    const float worldHeight = 1080.f;

    // --- Fixed mini-scale factor (smaller = more zoomed out) ---
    const float scale = 0.2f;  // 20% of the real world size (mini-map view)

    // --- Center world inside the viewport ---
    Vec2 offset{0.0f,0.0f};

    // --- Draw background rectangle for visual clarity ---
    dl->AddRectFilled(
        { viewportPos.x + offset.x, viewportPos.y + offset.y },
        { viewportPos.x + offset.x + worldWidth * scale, viewportPos.y + offset.y + worldHeight * scale },
        IM_COL32(40, 40, 40, 255)
    );
    dl->AddRect(
        { viewportPos.x + offset.x, viewportPos.y + offset.y },
        { viewportPos.x + offset.x + worldWidth * scale, viewportPos.y + offset.y + worldHeight * scale },
        IM_COL32(100, 100, 100, 255)
    );

    // --- Draw Camera zone ---

    Camera& cam = Camera::GetInstance();
    bool camHover = false;
    Vec2 camOffset = cam.GetPosition();
    {
        float camZoom = cam.GetZoom();
        if (camZoom != 0.f)
        { camZoom = 1/camZoom; }
        else
        { camZoom = 0; }
        camZoom *= scale;
        

        ImVec2 min = { viewportPos.x + (camOffset.x * scale) - (worldWidth * camZoom) / 2,
                       viewportPos.y + (worldHeight * scale) - (camOffset.y * scale) - (worldHeight * camZoom) / 2 };

        ImVec2 max = { viewportPos.x + (camOffset.x * scale) + (worldWidth * camZoom) / 2,
                       viewportPos.y + (worldHeight * scale) - (camOffset.y * scale) + (worldHeight * camZoom) / 2 };

        bool inside = (mousePos.x >= min.x && mousePos.x <= max.x &&
            mousePos.y >= min.y && mousePos.y <= max.y);

        ImU32 color = inside ? IM_COL32(255, 192, 203, 255) : IM_COL32(255, 0, 255, 255);

        dl->AddRect( min, max, color );
        if (inside) { camHover = true; }
    }

    // --- Mouse state ---
    bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

    static Vec2 dragOffset{ 0.f, 0.f };
    static bool isDragging = false;


    // === Draw all entities ===
    for (auto& [entity, transform] : reg.Get_ComponentMap<TransformComponent>())
    {
        Vec2 pos = transform.pos;

        // Convert world → viewport (flip Y to top-left origin)
        Vec2 screenPos;
        screenPos.x = offset.x + pos.x * scale;
        screenPos.y = offset.y + (worldHeight - pos.y) * scale;

        Vec2 half{ 25.f * scale, 25.f * scale };

        ImVec2 min = { viewportPos.x + screenPos.x - half.x, viewportPos.y + screenPos.y - half.y };
        ImVec2 max = { viewportPos.x + screenPos.x + half.x, viewportPos.y + screenPos.y + half.y };

        ImU32 color = (entity == selectedEntity) ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 255, 255, 100);

        dl->AddRect(min, max, color, 0.f, 0, 2.f);
    }

    if (!isPlaying)
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


    ImGui::Dummy(ImVec2(10.0f, 220.0f));
    if (ImGui::Button("Reset", ImVec2(80, 0)))
    {
        cam.SetZoom(1.0f);
        cam.ResetToCenter();
        std::cout << "[Editor] Camera reset.\n";
    }

}


void ImGuiSystem::DrawEntityDragHandles(ImVec2 panelPos, ImVec2 panelSize)
{
    Registry& reg = Registry::Get();
    ImDrawList* dl = ImGui::GetWindowDrawList();
   // Camera& cam = Camera::GetInstance();
    /* w (x'/w, y' / w) omg comp graph*/
    //moved into cam
    //glm::mat4 VP = cam.GetViewProjection();
    //glm::mat4 invVP = glm::inverse(VP);

    // mouse n panel state
    ImVec2 mouseScreen = ImGui::GetMousePos();
    bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

    //drag only works if mouse in viewable area
    bool mouseInPanel =
        (mouseScreen.x >= panelPos.x && mouseScreen.x <= panelPos.x + panelSize.x) &&
        (mouseScreen.y >= panelPos.y && mouseScreen.y <= panelPos.y + panelSize.y);

    static Entity draggedEntity = INVALID_ENTITY;
    static Vec2 dragOffset{ 0.f, 0.f };
    static bool isDragging = false;
    Entity hovered = INVALID_ENTITY;

    const float handleRadius = 10.f; //circle rad

    //draw the circles
    for (auto& [entity, transform] : reg.Get_ComponentMap<TransformComponent>())
    {
        if (!EntityManager::get().isAlive(entity))
            continue;

        ImVec2 handleCenter = Render::WorldToPanel(transform.pos, panelPos, panelSize);//world pos to pixel coords

        float dx = mouseScreen.x - handleCenter.x;
        float dy = mouseScreen.y - handleCenter.y;
        bool inside = mouseInPanel && (dx * dx + dy * dy <= handleRadius * handleRadius);

        ImU32 color = (entity == draggedEntity) ? IM_COL32(0, 180, 255, 255) : //drag cyan
                      (entity == selectedEntity) ? IM_COL32(0, 255, 0, 255) : //select green
                       inside ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 255, 255, 180); //hover yellow else white

        dl->AddCircleFilled(handleCenter, handleRadius, color, 24);
        dl->AddCircle(handleCenter, handleRadius, IM_COL32(0, 0, 0, 200), 24, 1.5f); //black outline

        if (inside)
            hovered = entity;
    }

    //mouse clicked
    if (mouseClicked && mouseInPanel && hovered != INVALID_ENTITY)
    {
        selectedEntity = hovered;
        draggedEntity = hovered;
        isDragging = true;

        auto& t = reg.Get_Component<TransformComponent>(hovered);
        dragOffset = t.pos - Render::PanelToWorld(mouseScreen, panelPos, panelSize);

        std::cout << "[Editor] Started dragging entity " << hovered << "\n";
    }

    //mouse held
    if (isDragging && draggedEntity != INVALID_ENTITY && mouseDown && mouseInPanel)
    {
        auto& t = reg.Get_Component<TransformComponent>(draggedEntity);
        t.pos = Render::PanelToWorld(mouseScreen, panelPos, panelSize) + dragOffset;

        //need update aabb cause editor physic dt = 0
        if (reg.Has_Component<Physics::PhysicsComponent>(draggedEntity)) {
            auto& p = reg.Get_Component<Physics::PhysicsComponent>(draggedEntity);
            p.UpdateHitboxes(t.pos);
        }
    }

    //mouse release
    if (mouseReleased)
    {
        if (isDragging && draggedEntity != INVALID_ENTITY)
            std::cout << "[Editor] Released entity " << draggedEntity << "\n";
        isDragging = false;
        draggedEntity = INVALID_ENTITY;
    }
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
        ImGui::Text("Are you sure you want to delete prefab '%s'?\nThis action cannot be undone.",pendingDeletePrefab.c_str());
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

    bool inEdit = gsm.InEditMode();
    //play
    if (!inEdit) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Play", ImVec2(buttonWidth, 0)))
    {
        //FILLTHISSHITYASM
        // Save the current edited scene before starting simulation
        if (gsm.GetCurrentState() == GameState::GS_LEVEL1)
        {
            JsonLoad::SaveCurrentScene(FilePath::LEVEL1_TEMP_SCENE, "level1_temp.json");
              
        }
        else if (gsm.GetCurrentState() == GameState::GS_LEVEL2)
        {
            JsonLoad::SaveCurrentScene(FilePath::LEVEL2_TEMP_SCENE, "level2_temp.json");
        }

        std::cout << "[Editor] Temp scene saved before Play.\n";

        gsm.isPaused = false;
        gsm.inEditMode = false;
        dragEditMode = false;
        Camera& cam = Camera::GetInstance();
        cam.SetMode(Camera::Mode::GAME);
        cam.SetZoom(1.0f);
        cam.ResetToCenter();

        LogicSystem::Get().Initialize();

        std::cout << "[Camera] Switched to GAME mode\n";
        isPlaying = true;
        std::cout << "[Editor] Gameplay started.\n";
    }
    if (!inEdit) { ImGui::EndDisabled(); }

    ImGui::SameLine(0, spacing);

    //STOP
    if (inEdit) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Stop", ImVec2(buttonWidth, 0)))
    {
        gsm.isPaused = true;
        gsm.inEditMode = true;

        Camera& cam = Camera::GetInstance();
        cam.SetMode(Camera::Mode::EDITOR);
        cam.ResetToCenter();


        std::cout << "[Camera] Switched to EDITOR mode\n";
        isPlaying = false;
        //FILLTHISSHITYASM
        if (gsm.GetCurrentState() == GameState::GS_LEVEL1)
        {
               
            JsonLoad::ClearAllEntities();

            if (JsonLoad::LoadScene(FilePath::LEVEL1_TEMP_SCENE))
            {
                std::cout << "[Editor] Saved level scene, restored temp edits.\n";
            }
            else
            {
                std::cout << "[Editor] Failed to load temp scene! Falling back to Level1.\n";
                JsonLoad::LoadScene(FilePath::LEVEL1_SCENE);
            }
        }
        else if (gsm.GetCurrentState() == GameState::GS_LEVEL2)
        {

            JsonLoad::ClearAllEntities();

            if (JsonLoad::LoadScene(FilePath::LEVEL2_TEMP_SCENE))
            {
                std::cout << "[Editor] Saved level scene, restored temp edits.\n";
            }
            else
            {
                std::cout << "[Editor] Failed to load temp scene! Falling back to Level2.\n";
                JsonLoad::LoadScene(FilePath::LEVEL2_SCENE);
            }
        }

        ImGuiSystem::selectedEntity = INVALID_ENTITY;
    }
    if (inEdit) { ImGui::EndDisabled(); }

    ImGui::SameLine(spacing * 2);
    //COLORS
    ImGui::TextColored( isPlaying ? ImVec4(0.2f, 0.8f, 0.2f, 1.f) : ImVec4(0.8f, 0.3f, 0.3f, 1.f), isPlaying ? "PLAYING" : "EDITING");
}

void ImGuiSystem::Shutdown() {
    ShutdownImGui();
}

void ImGuiSystem::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


