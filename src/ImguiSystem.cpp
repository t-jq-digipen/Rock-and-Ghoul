/* ========================================================
 * @file        ImguiSystem.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 * 
 * Implements the ImGuiSystem, which provides a in-game editor UI. The system toggles with F1 and
 * displays a single "Game Editor" window containing multiple panels:
 * 
 * - Engine Info:           Framerate and system timings
 * - Collision:             Displays collider stats and scene management
 * - Hierarchy + Editor:    Entity list and property editor
 *
 * Panels are switched using buttons inside the editor window.
 * This design avoids multiple pop-up windows by embedding
 * panel content inline within one parent window.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "ImGuiSystem.h"
#include "Registry.h"
#include "glapp.h"
#include "CoreEngine.h"
#include "EntityCreation.h"
#include "Input.h"
#include "JsonLoad.h"
#include "FilePath.h"
#include "Audio.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_set> 
bool ImGuiSystem::showColliders = false;
bool ImGuiSystem::showVelocity = false;
Entity ImGuiSystem::selectedEntity = INVALID_ENTITY;
static bool isDraggingEntity = false;
static glm::vec2 dragOffset = { 0.f, 0.f };

bool ImGuiSystem::guiEnabled = false;
static FMOD::Channel* previewChannel = nullptr;
#include "DataDefs.h"
static std::vector<std::string> droppedFiles;
// global (file-scope) thumbnail cache for runtime updates
static std::vector<Image> g_loadedThumbnails;
static std::vector<std::string> g_imageNames;
bool ImGuiSystem::dragEditMode = false;

static std::string pendingImportPath;
static std::string pendingDestPath;
static bool showReplacePopup = false;
extern GSM gsm;

// Checkbox
static void MakeWidget(Editor::Widget_Checkmark& w, const std::string& label) {
    if (!w.ref) return;
    ImGui::Checkbox(label.c_str(), w.ref);
}

// String
static void MakeWidget(Editor::Widget_String& w, const std::string& label) {
    if (!w.ref) return;
    char buf[128];
    strncpy(buf, w.ref->c_str(), sizeof(buf));
    if (ImGui::InputText(label.c_str(), buf, IM_ARRAYSIZE(buf))) {
        *w.ref = buf;
    }
}

// Dropdown
static void MakeWidget(Editor::Widget_Dropdown& w, const std::string& label)
{
    if (!w.ref || w.options.empty()) return;
    std::vector<const char*> items;
    items.reserve(w.options.size());
    for (auto& opt : w.options) items.push_back(opt.c_str());
    // find index of current value
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


// Int
static void MakeWidget(Editor::Widget_Int& w, const std::string& label) {
    if (!w.ref) return;
    if (w.slider && w.min != w.max)
        ImGui::SliderInt(label.c_str(), w.ref, w.min, w.max);
    else
        ImGui::InputInt(label.c_str(), w.ref);
}

// Float
static void MakeWidget(Editor::Widget_Float& w, const std::string& label) {
    if (!w.ref) return;
    if (w.slider && w.min != w.max)
        ImGui::SliderFloat(label.c_str(), w.ref, w.min, w.max);
    else
        ImGui::InputFloat(label.c_str(), w.ref);
}

void ImGuiSystem::OnFileDropped(const std::string& path)
{
    namespace fs = std::filesystem;

    if (!fs::exists(path)) {
        std::cerr << "[ImGui Drop] File not found: " << path << std::endl;
        return;
    }

    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    std::string filename = fs::path(path).filename().string();
    droppedFiles.push_back(filename);

    std::cout << "[ImGui Drop] File dropped: " << filename << " (" << path << ")\n";

    // --- Image loading --- 
    //Image img = CreateImage(path);
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
    {
        std::string src = path;
        std::string destDir = "../Assets/";
        std::string destPath = destDir + fs::path(src).filename().string();
        // make sure the folder exists
        if (!fs::exists(destDir))
            fs::create_directories(destDir);


        // --- check for same base name even if different extension ---
        std::string baseName = fs::path(src).stem().string(); // e.g., "MyTexture"
        bool foundExistingVariant = false;
        std::string existingVariantPath;

        // loop through asset directory to find same basename
        for (auto& entry : fs::directory_iterator(destDir))
        {
            if (!entry.is_regular_file()) continue;

            std::string existingBase = entry.path().stem().string();
            if (existingBase == baseName)
            {
                foundExistingVariant = true;
                existingVariantPath = entry.path().string();
                break;
            }
        }

        if (foundExistingVariant)
        {
            pendingImportPath = src;
            pendingDestPath = existingVariantPath;
            showReplacePopup = true;
            std::cout << "[ImGui Drop] Found variant of same asset ("
                << existingVariantPath << "), prompting replace...\n";
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

    // --- Prefab loading ---
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

    // --- Audio preview ---
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")
    {
        FMOD::Sound* sound = AudioSystem::LoadSound(path, false);
        if (sound)
        {
            if (previewChannel) previewChannel->stop();
            previewChannel = AudioSystem::PlaySoundGetChannel(sound, false);
            previewChannel->setVolume(0.8f);
            std::cout << "[ImGui Drop] Playing audio preview.\n";
        }
        else
        {
            std::cerr << "[ImGui Drop] Failed to load audio.\n";
        }
        return;
    }

    std::cout << "[ImGui Drop] Unsupported file type: " << ext << std::endl;
}

using json = nlohmann::json;


namespace {
    // small helper to name an entity for the Hierarchy list
    std::string EntityDisplayName(Entity e) {
        Registry& reg = Registry::Get();
        std::string name = "Entity " + std::to_string(e);

        if (reg.Has_Component<TagComponent>(e)) {
            auto& tag = reg.Get_Component<TagComponent>(e);
            const char* tagName = (tag.tag == T_PLAYER) ? "Player"
                : (tag.tag == T_ENEMY) ? "Enemy"
                : (tag.tag == T_PROJECTILE) ? "Projectile"
                : "Unknown";
            name = std::string(tagName) + " " + std::to_string(e);
        }
        if (reg.Has_Component<ShapeComponent>(e)) {
            auto& s = reg.Get_Component<ShapeComponent>(e);
            if (s.shape == SHAPE_RECTANGLE) {
                name += "  [Square]";
            }
            else if (s.shape == SHAPE_CIRCLE) {
                name += "  [Circle]";
            }
        }
        else if (reg.Has_Component<Sprite::SpriteComponent>(e)) {
            std::string entName = reg.Get_EntityName(e);
            name += "  [Textured]";
        
        }
        return name;
    }

    //selectedEntity stays valid
    void EnsureSelectionValid() {
        if (ImGuiSystem::selectedEntity != INVALID_ENTITY && !EntityManager::get().isAlive(ImGuiSystem::selectedEntity)) {
            ImGuiSystem::selectedEntity = 0;
        }
    }

    //another helpers
    static nlohmann::json audioManifest;
    static bool audioManifestLoaded = false;

    static bool LoadAudioManifest(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cout << "[ImGui AudioManifest] File not found, creating new.\n";
            audioManifest = json::object();
            audioManifest["music"] = json::object();
            audioManifest["ui"] = json::object();
            return false;
        }
        try {
            file >> audioManifest;
            file.close();
            audioManifestLoaded = true;
            return true;
        }
        catch (...) {
            std::cout << "[ImGui AudioManifest] Failed to parse JSON.\n";
            return false;
        }
    }

    static void SaveAudioManifest(const std::string& path)
    {
        std::ofstream out(path);
        out << std::setw(4) << audioManifest;
        out.close();
        std::cout << "[ImGui AudioManifest] Saved.\n";
    }
}

void ImGuiSystem::InitializeImGui() {
    IMGUI_CHECKVERSION(); //checks version compatibility
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); //imgui i/o settings
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; //enable keyboard nav in imgui

    ImGui::StyleColorsDark(); //dark themed lol
    ImGui_ImplGlfw_InitForOpenGL(GLApp::ptr_window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); //init opengl3 backend


    static bool dropCallbackSet = false;
    if (!dropCallbackSet && GLApp::ptr_window)
    {
        glfwSetDropCallback(GLApp::ptr_window, [](GLFWwindow*, int count, const char** paths) {
            for (int i = 0; i < count; ++i)
            {
                std::string path = paths[i];
                ImGuiSystem::OnFileDropped(path);
            }
            });
        dropCallbackSet = true;
        std::cout << "[ImGuiSystem] File drop callback registered.\n";
    }
}

//init hear so is easier
void ImGuiSystem::Initialize() {
    InitializeImGui();  // Do it here instead of main
   
}

void ImGuiSystem::Update(float dt) {
    (void)dt;
    if (Input::IsKeyPressed(GLFW_KEY_F1)) {
        guiEnabled = !guiEnabled; // toggle on/off
    }
   

    if (!guiEnabled) return;


    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame(); 
    ImGui::NewFrame();

    HandleEntityDragging();
    //// Choose where to place it
    //ImVec2 cornerPos(ImGui::GetIO().DisplaySize.x - 420, 10);   // top-right corner, 10 offset
    //ImGui::SetNextWindowPos(cornerPos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(400, 225));                 // 16:9

    //ImGui::SetNextWindowBgAlpha(0.8f);

   
    //if (ImGui::Begin("Scene Viewport",
    //    nullptr,
    //    ImGuiWindowFlags_NoResize |
    //    ImGuiWindowFlags_NoMove |
    //    ImGuiWindowFlags_NoCollapse |
    //    ImGuiWindowFlags_NoScrollbar |
    //    ImGuiWindowFlags_NoScrollWithMouse |
    //    ImGuiWindowFlags_NoTitleBar))
    //{
    //    ImVec2 cursor = ImGui::GetCursorScreenPos();
    //    ImVec2 size = ImGui::GetContentRegionAvail();

    //    // Draw your overlay (entity boxes etc.)
    //    ShowEntityInteractionOverlay(cursor, size);

    //    ImGui::Dummy(size); // keeps layout from collapsing
    //}
    //ImGui::End();


    ImGui::Begin("Game Editor");

    if (ImGui::BeginTabBar("EditorTabs")) {
        if (ImGui::BeginTabItem("Engine Info")) {
            ShowEngineDebugWindow();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Collision + Prefabs")) {
            ShowCollisionDebugWindow();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Entity Editor")) {
            //splits the into two columns
            ImGui::Columns(2);
            ShowHierarchyWindow();

            ImGui::NextColumn();
            ShowInspectorWindow();
            //resets back to a single column layout for the rest of the UI
            ImGui::Columns(1);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Assets")) {
            ShowAssetPreviewWindow();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
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
    static bool isPlaying = false;
    // --- Editor Control Section ---
    ImGui::Separator();
    ImGui::Text("Simulation Controls:");

    if (!isPlaying)
    {
        if (ImGui::Button("Play"))
        {
            // Save the current edited scene before starting simulation
            JsonLoad::SaveCurrentScene(FilePath::TEMP_SCENE, "TempScene");
            std::cout << "[Editor] Temp scene saved before Play.\n";

            gsm.isPaused = false;
            gsm.inEditMode = false;
            isPlaying = true;

            std::cout << "[Editor] Gameplay started.\n";
        }
    }
    else
    {
        if (ImGui::Button("Stop"))
        {
            // Stop simulation
            gsm.isPaused = true;
            gsm.inEditMode = true;
            isPlaying = false;

            std::cout << "[Editor] Gameplay stopped. Restoring temp scene...\n";

            // clear the current entities
            JsonLoad::ClearAllEntities();

            // reload the scene you were editing before pressing Play
            if (JsonLoad::LoadScene(FilePath::TEMP_SCENE))
            {
                std::cout << "[Editor] Restored scene from temp file.\n";
            }
            else
            {
                std::cout << "[Editor] Failed to load temp scene! Falling back to Level1.\n";
                JsonLoad::LoadScene(FilePath::LEVEL1_SCENE);
            }

            ImGuiSystem::selectedEntity = INVALID_ENTITY;
        }
    }

    ImGui::SameLine();
    ImGui::TextColored(
        isPlaying ? ImVec4(0.2f, 0.8f, 0.2f, 1.f) : ImVec4(0.8f, 0.3f, 0.3f, 1.f),
        isPlaying ? "PLAYING" : "EDITING"
    );
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
        if (physics.CollisionActive()) { // Use the physics component's collision active check
            activeColliders++;
        }
    }

    ImGui::Text("AABB Colliders: %d / %d active", activeColliders, totalColliders);
    ImGui::Checkbox("Show Colliders", &showColliders);
    ImGui::Checkbox("Show Velocity", &showVelocity);
    //enityt
    if (ImGui::CollapsingHeader("Entity Positions")) {
        for (auto& [entity, tag] : m_registry.Get_ComponentMap<TagComponent>()) {
            if (m_registry.Has_Component<TransformComponent>(entity)) {
                TransformComponent& transform = m_registry.Get_Component<TransformComponent>(entity);
                const char* tagName = (tag.tag == T_PLAYER) ? "Player" :(tag.tag == T_ENEMY) ? "Enemy" :(tag.tag == T_PROJECTILE) ? "Projectile" : "Unknown";
                ImGui::Text("%s %d: (%.1f, %.1f)", tagName, entity, transform.pos.x, transform.pos.y);
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("Scene Management:");

    if (ImGui::Button("Save Scene (F5)")) {
        JsonLoad::SaveCurrentScene(FilePath::LEVEL1_SCENE, "Level1");
        std::cout << "Scene saved from ImGui!" << std::endl;
    }
    ImGui::SameLine();

    if (ImGui::Button("Load Scene (F9)")) {
        JsonLoad::ClearAllEntities();
        if (JsonLoad::LoadScene(FilePath::LEVEL1_SCENE)) {
            std::cout << "Scene loaded from ImGui!" << std::endl;
        }
        else {
            std::cout << "Failed to load scene!" << std::endl;
        }
    }

    static int selectedPrefab = -1; //currently selected index cuz imgui dropdown dont take a string
    //int index into an array of items
    static std::vector<std::string> prefabNames;

    //empties the vector each frame
    prefabNames.clear();
    //from json file path
    for (auto& entry : std::filesystem::directory_iterator(FilePath::PREFABS_FOLDER)) {
        if (entry.path().extension() == ".json") { //.extension() just the file extension (with the dot)
            //.stem() the filename without the extension
            //.string() converts the path object into a std::string
            prefabNames.push_back(entry.path().stem().string()); // no ".json"
        }
    }

    ImGui::Separator();
    ImGui::Text("Spawn Prefab:");

    //show dropdown if we have prefabs
    if (!prefabNames.empty()) {
        //array of const char* for ImGui::Combo
        std::vector<const char*> items;
        //iterates over each string in that vector.
        for (auto& name : prefabNames) items.push_back(name.c_str());

        if (ImGui::Combo("##PrefabCombo", &selectedPrefab, items.data(), (int)items.size())) {
            //spawn
            std::string filepath = FilePath::PREFABS_FOLDER + prefabNames[selectedPrefab] + ".json";
            selectedEntity = JsonLoad::LoadEntityFromFile(filepath, 200.f, 200.f);
            std::cout << "Spawned prefab: " << prefabNames[selectedPrefab] << std::endl;
        }
    }
    else {
        ImGui::TextDisabled("No prefabs available in %s", FilePath::PREFABS_FOLDER.c_str());
    }
    //// ==============================
    //// Image loader (runtime only)
    //// ==============================
    //ImGui::Separator();
    //ImGui::Text("Image Loader:");

    //static char imgNameBuf[128] = "";
    //static std::vector<std::string> loadedImages;


    //ImGui::InputText("Image Filename", imgNameBuf, IM_ARRAYSIZE(imgNameBuf));
    //ImGui::SameLine();

    //if (ImGui::Button("Add Image")) {
    //    std::string filename = imgNameBuf;


    //    std::string fullPath = "../Assets/" + filename;
    //    Image img = CreateImage(fullPath); // uses your Render.cpp helper
    //    if (img.IsValid()) {
    //        loadedImages.push_back(filename);

    //    }

    //}

    //ImGui::Separator();
    //ImGui::Text("Loaded Images:");
    //if (loadedImages.empty()) {
    //    ImGui::TextDisabled("(none yet)");
    //}
    //else {
    //    for (auto const& name : loadedImages) {
    //        ImGui::BulletText("%s", name.c_str());
    //    }
    //}


}


void ImGuiSystem::ShowHierarchyWindow() {

    EnsureSelectionValid();

    if (ImGui::Button("Add Entity"))
    {
        Entity newEntity = m_registry.Create_Entity();
        selectedEntity = newEntity;

        // Store editor-only name (not actual TagComponent)
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
                std::cout << "[Clone Warning] Skipping empty component: " << it->first << "\n";
                it = src.componentData.erase(it);
            }
            else
                ++it;
        }

        Entity clone = reg.Load_Entity(src);
    }



    ImGui::SameLine();
    if (ImGui::Button("Delete Entity") && selectedEntity != INVALID_ENTITY)
    {
        m_registry.Remove_Entity(selectedEntity);
        selectedEntity = INVALID_ENTITY;
    }
    ImGui::Separator();


    // show all known entities (even blank)
    auto& nameMap = m_registry.Get_EntityNameMap();

    for (auto& [entity, name] : nameMap)
    {
        if (!EntityManager::get().isAlive(entity))
            continue;

        bool selected = (entity == selectedEntity);
        //Cant select old entity after clone
        //Use ImGui::PushID(entity) so diff imguiID
        ImGui::PushID((int)entity);
        if (ImGui::Selectable(name.c_str(), selected))
            selectedEntity = entity;
        ImGui::PopID();
    }


}

void ImGuiSystem::ShowInspectorWindow() {

    EnsureSelectionValid();
    //if no entity is selected, just show a message and return
    if (selectedEntity == INVALID_ENTITY) {
        ImGui::Text("No entity selected");
        return;
    }
    // show the entity ID at top
    ImGui::Text("Entity %d", selectedEntity);
    ImGui::Separator();

    std::vector<Editor::ComponentData> compDataVec = m_registry.Get_EntityData(selectedEntity);
    for (auto& componentData : compDataVec)
    {
        const std::string& compName = componentData.GetComponentName();

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


            ImGui::TreePop();
        }
    }

}

void ImGuiSystem::ShowAssetPreviewWindow()
{
    ImGui::Text( "Asset Preview Browser");
    ImGui::Separator();

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
                for (auto& entry : std::filesystem::recursive_directory_iterator(assetFolder))
                {
                    if (!entry.is_regular_file()) continue;
                    std::string path = entry.path().string();
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                    {

                        try {
                            Image img = CreateImage(path);
                            if (img.IsValid()) {
                                g_loadedThumbnails.push_back(img);
                                g_imageNames.push_back(entry.path().filename().string());
                            }
                        }
                        catch (...) {
                            std::cout << "Failed to load " << path << std::endl;
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
                if (columns < 1) columns = 1;

                ImGui::BeginChild("ImageScroll", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

                int count = 0;
                for (size_t i = 0; i < g_loadedThumbnails.size(); ++i)
                {
                    ImGui::BeginGroup();
                    GLuint texID = g_loadedThumbnails[i].textureID;

                    // show image (flipped UVs)
                    ImGui::Image((ImTextureID)(intptr_t)texID,
                        ImVec2(thumbSize, thumbSize),
                        ImVec2(0, 1), ImVec2(1, 0));

                    // filename
                    ImGui::TextWrapped("%s", g_imageNames[i].c_str());
                    ImGui::EndGroup();

                    if (++count % columns != 0)
                        ImGui::SameLine();
                }

                ImGui::EndChild();
            }
            // --- replacement confirmation popup ---
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
                        const std::string newName = fs::path(pendingImportPath).filename().string(); // e.g., Luke2023.jpg
                        const std::string newStem = fs::path(pendingImportPath).stem().string();     // Luke2023
                        const std::string destNewPath = assetFolder + newName;

                        // 1) Remove all existing variants (by stem) from disk
                        for (auto& entry : fs::directory_iterator(assetFolder))
                        {
                            if (!entry.is_regular_file()) continue;
                            if (entry.path().stem().string() == newStem)
                            {
                                fs::remove(entry.path());
                                std::cout << "[ImGui Replace] Deleted old variant: "
                                    << entry.path().filename().string() << "\n";
                            }
                        }

                        // 2) Copy the new file using its own filename (so extension is preserved)
                        fs::copy_file(pendingImportPath, destNewPath, fs::copy_options::overwrite_existing);
                        std::cout << "[ImGui Replace] Installed: " << destNewPath << "\n";

                        // 3) Load thumbnail for the new file
                        if (Image newImg = CreateImage(destNewPath); newImg.IsValid())
                        {
                            g_loadedThumbnails.push_back(newImg);
                            g_imageNames.push_back(fs::path(destNewPath).filename().string());
                        }

                        // 4) Optionally, full refresh (keeps grid consistent if there are many files)
                        g_loadedThumbnails.clear();
                        g_imageNames.clear();
                        for (auto& entry : fs::recursive_directory_iterator(assetFolder))
                        {
                            if (!entry.is_regular_file()) continue;
                            auto ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                            {
                                if (Image img = CreateImage(entry.path().string()); img.IsValid())
                                {
                                    g_loadedThumbnails.push_back(img);
                                    g_imageNames.push_back(entry.path().filename().string());
                                }
                            }
                        }

                        std::cout << "[ImGui Replace] Asset preview refreshed.\n";
                    }
                    catch (const std::filesystem::filesystem_error& e)
                    {
                        std::cerr << "[ImGui Replace] Failed: " << e.what() << std::endl;
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

            // --- Load or recreate audio manifest safely ---
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

            // --- Defensive check before using it ---
            if (!audioManifest.is_object() || audioManifest.empty())
            {
                ImGui::TextDisabled("No audio manifest data available.");
                ImGui::EndTabItem();
                return;
            }

            ImGui::Text("Audio Manifest Editor");
            ImGui::Separator();

            // --- ADD NEW AUDIO ENTRY ---
            static char categoryBuf[32] = "music";
            static char nameBuf[64] = "";
            static char pathBuf[128] = "";
            static int idBuf = 0;
            static bool loopBuf = false;
            static char groupBuf[32] = "Music";

            ImGui::InputText("Category (music/ui)", categoryBuf, IM_ARRAYSIZE(categoryBuf));
            ImGui::InputText("Asset Name", nameBuf, IM_ARRAYSIZE(nameBuf));
            ImGui::InputText("Path", pathBuf, IM_ARRAYSIZE(pathBuf));
            ImGui::InputInt("ID", &idBuf);
            ImGui::Checkbox("Loop", &loopBuf);
            ImGui::InputText("Group", groupBuf, IM_ARRAYSIZE(groupBuf));

            if (ImGui::Button("Add Audio Asset"))
            {
                std::string category = categoryBuf;
                std::string name = nameBuf;
                std::string path = pathBuf;

                if (name.empty() || path.empty())
                {
                    std::cout << "[ImGui AudioManifest] Missing name or path!\n";
                }
                else
                {
                    // Ensure category exists
                    if (!audioManifest.contains(category))
                        audioManifest[category] = json::object();

                    // Add entry safely
                    audioManifest[category][name] = {
                        { "id", idBuf },
                        { "path", path },
                        { "loop", loopBuf },
                        { "group", groupBuf }
                    };

                    SaveAudioManifest(manifestPath);
                    std::cout << "[ImGui AudioManifest] Added " << name << " to " << category << "\n";
                }
            }

            ImGui::Separator();

            // --- DISPLAY EXISTING ENTRIES SAFELY ---
            try
            {
                for (auto& [category, assets] : audioManifest.items())
                {
                    if (!assets.is_object())
                        continue; // skip malformed entries

                    if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        for (auto& [name, data] : assets.items())
                        {
                            if (!data.is_object())
                                continue;

                            ImGui::PushID(name.c_str());

                            // Safely retrieve values
                            std::string path = data.contains("path") ? data["path"].get<std::string>() : "(none)";
                            int id = data.contains("id") ? data["id"].get<int>() : -1;
                            bool loop = data.contains("loop") ? data["loop"].get<bool>() : false;
                            std::string group = data.contains("group") ? data["group"].get<std::string>() : "(none)";

                            ImGui::Text("Name: %s", name.c_str());
                            ImGui::Text("Path: %s", path.c_str());
                            ImGui::Text("ID: %d", id);
                            ImGui::Text("Loop: %s", loop ? "true" : "false");
                            ImGui::Text("Group: %s", group.c_str());

                            // --- Preview controls ---
                            if (ImGui::Button("Play"))
                            {

                                    FMOD::Sound* snd = AudioSystem::LoadSound(path, loop);
                                    if (snd)
                                    {
                                        if (previewChannel) previewChannel->stop();
                                        previewChannel = AudioSystem::PlaySoundGetChannel(snd, false);
                                        if (previewChannel) previewChannel->setVolume(0.8f);
                                    }
   
                            }

                            ImGui::SameLine();
                            if (ImGui::Button("Stop"))
                            {
                                if (previewChannel) previewChannel->stop();
                                previewChannel = nullptr;
                            }

                            ImGui::SameLine();
                            bool removeThis = false;
                            if (ImGui::Button("Remove"))
                                removeThis = true;

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
                std::cerr << "[ImGui AudioManifest] Exception while reading: " << e.what() << std::endl;
            }

            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }
}

//void ImGuiSystem::ShowEntityInteractionOverlay(ImVec2 viewportPos, ImVec2 viewportSize)
//{
//    Registry& reg = Registry::Get();
//    ImDrawList* dl = ImGui::GetWindowDrawList();
//
//    // Mouse position relative to the viewport
//    ImVec2 mousePos = ImGui::GetMousePos();
//    Vec2 mouse(mousePos.x - viewportPos.x, mousePos.y - viewportPos.y);
//
//    // --- Define your full world dimensions ---
//    const float worldWidth = 1920.f;
//    const float worldHeight = 1080.f;
//
//    // --- Fixed mini-scale factor (smaller = more zoomed out) ---
//    const float scale = 0.2f;  // 20% of the real world size (mini-map view)
//
//    // --- Center world inside the viewport ---
//    Vec2 offset{0.0f,0.0f};
//
//    // --- Draw background rectangle for visual clarity ---
//    dl->AddRectFilled(
//        { viewportPos.x + offset.x, viewportPos.y + offset.y },
//        { viewportPos.x + offset.x + worldWidth * scale, viewportPos.y + offset.y + worldHeight * scale },
//        IM_COL32(40, 40, 40, 255)
//    );
//    dl->AddRect(
//        { viewportPos.x + offset.x, viewportPos.y + offset.y },
//        { viewportPos.x + offset.x + worldWidth * scale, viewportPos.y + offset.y + worldHeight * scale },
//        IM_COL32(100, 100, 100, 255)
//    );
//
//    // --- Mouse state ---
//    bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
//    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
//    bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
//
//    static Entity draggedEntity = INVALID_ENTITY;
//    static Vec2 dragOffset{ 0.f, 0.f };
//    static bool isDragging = false;
//
//    Entity hovered = INVALID_ENTITY;
//
//    // === Draw all entities ===
//    for (auto& [entity, transform] : reg.Get_ComponentMap<TransformComponent>())
//    {
//        Vec2 pos = transform.pos;
//
//        // Convert world → viewport (flip Y to top-left origin)
//        Vec2 screenPos;
//        screenPos.x = offset.x + pos.x * scale;
//        screenPos.y = offset.y + (worldHeight - pos.y) * scale;
//
//        Vec2 half{ 25.f * scale, 25.f * scale };
//
//        ImVec2 min = { viewportPos.x + screenPos.x - half.x, viewportPos.y + screenPos.y - half.y };
//        ImVec2 max = { viewportPos.x + screenPos.x + half.x, viewportPos.y + screenPos.y + half.y };
//
//        bool inside = (mousePos.x >= min.x && mousePos.x <= max.x &&
//            mousePos.y >= min.y && mousePos.y <= max.y);
//
//        ImU32 color =
//            (entity == selectedEntity) ? IM_COL32(0, 255, 0, 255) :
//            inside ? IM_COL32(255, 255, 0, 200) :
//            IM_COL32(255, 255, 255, 100);
//
//        dl->AddRect(min, max, color, 0.f, 0, 2.f);
//
//        if (inside)
//            hovered = entity;
//    }
//
//    // === Selection & dragging ===
//    if (mouseClicked && hovered != INVALID_ENTITY)
//    {
//        selectedEntity = hovered;
//        draggedEntity = hovered;
//        isDragging = true;
//
//        auto& t = reg.Get_Component<TransformComponent>(hovered);
//        Vec2 mouseWorld{
//            (mouse.x - offset.x) / scale,
//            worldHeight - (mouse.y - offset.y) / scale
//        };
//        dragOffset = t.pos - mouseWorld;
//        std::cout << "[Editor] Selected entity " << hovered << std::endl;
//    }
//
//    if (isDragging && draggedEntity != INVALID_ENTITY && mouseDown)
//    {
//        auto& t = reg.Get_Component<TransformComponent>(draggedEntity);
//        Vec2 mouseWorld{
//            (mouse.x - offset.x) / scale,
//            worldHeight - (mouse.y - offset.y) / scale
//        };
//        t.pos = mouseWorld + dragOffset;
//
//        // Draw a blue outline for the dragged entity
//        Vec2 pos = t.pos;
//        Vec2 screenPos;
//        screenPos.x = offset.x + pos.x * scale;
//        screenPos.y = offset.y + (worldHeight - pos.y) * scale;
//
//        ImVec2 min = { viewportPos.x + screenPos.x - 25 * scale, viewportPos.y + screenPos.y - 25 * scale };
//        ImVec2 max = { viewportPos.x + screenPos.x + 25 * scale, viewportPos.y + screenPos.y + 25 * scale };
//        dl->AddRect(min, max, IM_COL32(0, 128, 255, 255), 0.f, 0, 2.5f);
//    }
//
//    if (mouseReleased)
//    {
//        isDragging = false;
//        draggedEntity = INVALID_ENTITY;
//    }
////}
//

void ImGuiSystem::HandleEntityDragging()
{
    if (!guiEnabled || !dragEditMode)
        return;

    Registry& reg = Registry::Get();
    ImDrawList* dl = ImGui::GetForegroundDrawList(); // draw directly on top of scene

    const float worldHeight = static_cast<float>(GLApp::height);

    // --- Mouse state ---
    Vec2 mouse = Input::GetMousePosition();
    mouse.y = worldHeight - mouse.y;

    bool mouseDown = Input::IsMouseHeld(GLFW_MOUSE_BUTTON_LEFT);
    bool mouseClicked = Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT);
    bool mouseReleased = Input::IsMouseReleased(GLFW_MOUSE_BUTTON_LEFT);

    static Entity draggedEntity = INVALID_ENTITY;
    static Vec2 dragOffset{ 0.f, 0.f };
    static bool isDragging = false;
    Entity hovered = INVALID_ENTITY;

    // --- constants ---
    const float handleRadius = 10.f;

    // === draw handles + hover detection ===
    for (auto& [entity, transform] : reg.Get_ComponentMap<TransformComponent>())
    {
        if (!EntityManager::get().isAlive(entity))
            continue;

        Vec2 pos = transform.pos; // world position in screen space
        ImVec2 handleCenter(pos.x, worldHeight - pos.y); // convert to top-left ImGui space

        // distance to mouse
        float dx = mouse.x - pos.x;
        float dy = mouse.y - pos.y;
        float dist2 = dx * dx + dy * dy;
        bool inside = dist2 <= (handleRadius * handleRadius);

        // highlight color if hovered or selected
        ImU32 color =
            (entity == draggedEntity) ? IM_COL32(0, 180, 255, 255) :
            inside ? IM_COL32(255, 255, 0, 255) :
            IM_COL32(255, 255, 255, 180);

        dl->AddCircleFilled(handleCenter, handleRadius, color, 24);
        dl->AddCircle(handleCenter, handleRadius, IM_COL32(0, 0, 0, 200), 24, 1.5f);

        if (inside)
            hovered = entity;
    }

    // === click to start drag ===
    if (mouseClicked && hovered != INVALID_ENTITY)
    {
        selectedEntity = hovered;
        draggedEntity = hovered;
        isDragging = true;

        auto& t = reg.Get_Component<TransformComponent>(hovered);
        dragOffset = t.pos - mouse;

        std::cout << "[Editor] Drag handle start entity " << hovered << "\n";
    }

    // === drag entity ===
    if (isDragging && draggedEntity != INVALID_ENTITY && mouseDown)
    {
        auto& t = reg.Get_Component<TransformComponent>(draggedEntity);
        t.pos = mouse + dragOffset;
    }

    // === release ===
    if (mouseReleased)
    {
        isDragging = false;
        draggedEntity = INVALID_ENTITY;
    }
}


void ImGuiSystem::Shutdown() {
    ShutdownImGui();
}

void ImGuiSystem::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}