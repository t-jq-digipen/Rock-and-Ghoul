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

bool ImGuiSystem::showColliders = false;
bool ImGuiSystem::showVelocity = false;
Entity ImGuiSystem::selectedEntity = INVALID_ENTITY;
bool ImGuiSystem::guiEnabled = false;
static FMOD::Channel* previewChannel = nullptr;

static std::unordered_map<ComponentTypeID, std::vector<ComponentTypeID>> dependencies = {
    { CT_Animator, { CT_Sprite } },      // Animator needs Sprite
    { CT_Physics,  { CT_Transform } },   // Physics needs Transform
    { CT_Shape,    { CT_Transform } },   // Shape needs Transform
    { CT_Sprite,   { CT_Transform } },   // Sprite needs Transform
    { CT_Audio,    {} },                 // Audio independent
    { CT_Tag,      {} },                 // Tag independent
    { CT_Input,    {} }                  // Input independent
};

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
        if (reg.Has_Component<TagComponent>(e)) {
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
}

//init hear so is easier
void ImGuiSystem::Initialize() {
    InitializeImGui();  // Do it here instead of main
    
    //CM_Tag();
    //CM_Transform();
    //CM_Sprite();
    //CM_Physics();
    //CM_Shape();
    //CM_Input();
    //CM_Animator();
    //CM_Audio();
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
            ImGui::Columns(2);
            ShowHierarchyWindow();
            ImGui::NextColumn();
            ShowInspectorWindow();
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
    // ==============================
// Image loader (runtime only)
// ==============================
    ImGui::Separator();
    ImGui::Text("Image Loader:");

    static char imgNameBuf[128] = "";
    static std::vector<std::string> loadedImages;


    ImGui::InputText("Image Filename", imgNameBuf, IM_ARRAYSIZE(imgNameBuf));
    ImGui::SameLine();

    if (ImGui::Button("Add Image")) {
        std::string filename = imgNameBuf;


        std::string fullPath = "../Assets/" + filename;
        Image img = CreateImage(fullPath); // uses your Render.cpp helper
        if (img.IsValid()) {
            loadedImages.push_back(filename);

        }

    }

    ImGui::Separator();
    ImGui::Text("Loaded Images:");
    if (loadedImages.empty()) {
        ImGui::TextDisabled("(none yet)");
    }
    else {
        for (auto const& name : loadedImages) {
            ImGui::BulletText("%s", name.c_str());
        }
    }


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
    // show all known entities (even blank)
    auto& nameMap = m_registry.Get_EntityNameMap();

    for (auto& [entity, name] : nameMap)
    {
        if (!EntityManager::get().isAlive(entity))
            continue;

        bool selected = (entity == selectedEntity);
        if (ImGui::Selectable(name.c_str(), selected))
            selectedEntity = entity;
    }


}

namespace {
    template<typename T>
    void DrawAddComponentButton(
        const char* label,
        Entity entity,
        Registry& registry,
        bool canAdd = true)
    {
        if (registry.Has_Component<T>(entity))
        {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s (already added)", label);
            return;
        }

        if (!canAdd)
            ImGui::BeginDisabled();

        if (ImGui::Button(label, ImVec2(140, 0)))
        {
            if (canAdd)
            {
                registry.Add_Component<T>(entity, T{});
                std::cout << "[ImGui] Added component " << label << " to Entity " << entity << std::endl;
            }
        }

        if (!canAdd)
        {
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::TextDisabled("(requires dependency)");
        }
    }


    template<typename T>
    void CloneComponent(Entity src, Entity dst, Registry& registry)
    {
        if (registry.Has_Component<T>(src))
        {
            const T& comp = registry.Get_Component<T>(src);
            registry.Add_Component<T>(dst, comp);
        }
    }

    // Specialize for xform so we can offset the new position
    template<>
    void CloneComponent<TransformComponent>(Entity src, Entity dst, Registry& registry)
    {
        if (registry.Has_Component<TransformComponent>(src))
        {
            const auto& t = registry.Get_Component<TransformComponent>(src);
            TransformComponent newT = t;
            newT.pos.x += 50.f; // so it not exactly on top
            registry.Add_Component<TransformComponent>(dst, newT);
        }
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

    // --- Transform ---
    if (m_registry.Has_Component<TransformComponent>(selectedEntity)) {
        auto& t = m_registry.Get_Component<TransformComponent>(selectedEntity);
        // collapsible tree node for Transform data
        if (ImGui::TreeNode("Transform")) {
            ImGui::SliderFloat("X", &t.pos.x, 0.f, 1920.f);
            ImGui::SliderFloat("Y", &t.pos.y, 0.f, 1080.f);
            ImGui::SliderAngle("Rotation", &t.rot, -360.0f, 360.0f);
            ImGui::TreePop();
        }
    }

    // --- Sprite ---
    if (m_registry.Has_Component<Sprite::SpriteComponent>(selectedEntity)) {
        auto& s = m_registry.Get_Component<Sprite::SpriteComponent>(selectedEntity);
        if (ImGui::TreeNode("Sprite")) {
            std::string texPath = s.Get_TexturePath();
            ImGui::Text("Texture: %s", texPath.c_str());

            //scale/dimensions
            Vec2 scale = s.Get_Scale();
            if (ImGui::InputFloat2("Scale", &scale.x)) {
                s.Set_Size(scale);
            }

            float rot = s.Get_Rotation();
            if (ImGui::SliderAngle("Rotation", &rot, -360.0f, 360.0f)) {
                s.Set_Rotation(rot);
            }

            ImGui::TreePop();
        }
    }
    if (m_registry.Has_Component<ShapeComponent>(selectedEntity)) {
        auto& s = m_registry.Get_Component<ShapeComponent>(selectedEntity);
        if (ImGui::TreeNode("Shape")) {
            const char* shapeTypes[] = { "Rectangle", "Circle" };
            int currentShape = static_cast<int>(s.shape);
            if (ImGui::Combo("Shape Type", &currentShape, shapeTypes, 2)) {
                s.shape = static_cast<ShapeType>(currentShape);
            }
            ImGui::DragFloat2("Dimensions", &s.dimensions.x, 1.0f, 10.0f, 1000.0f);
            ImGui::ColorEdit3("Color", &s.color.x);
            ImGui::TreePop();
        }
    }

    // --- Physics ---
    if (m_registry.Has_Component<Physics::PhysicsComponent>(selectedEntity)) {
        auto& p = m_registry.Get_Component<Physics::PhysicsComponent>(selectedEntity);
        if (ImGui::TreeNode("Physics")) {
            auto& aabb = p.Get_AABB();
            auto& circ = p.Get_Circle();
            ImGui::Checkbox("Collision Active", const_cast<bool*>(&p.CollisionActive()));
            Vec2 aabbSize = aabb.dim;
            if (ImGui::DragFloat2("AABB Size", &aabbSize.x, 1.0f, 10.0f, 1000.0f)) {
                auto& trans = m_registry.Get_Component<TransformComponent>(selectedEntity);
                AABB newAABB = Set_AABB(trans.pos, aabbSize);
                p.Set_AABB(newAABB);
            }
            float circRad = circ.rad;
            if (ImGui::DragFloat("Circle Radius", &circRad, 1.0f, 10.0f, 500.0f)) {
                Circle newCircle = circ;
                newCircle.rad = circRad;
                p.Set_Circle(newCircle);
            }
            ImGui::Text("AABB: min: (%.1f, %.1f), max: (%.1f, %.1f)", aabb.min.x, aabb.min.y, aabb.max.x, aabb.max.y);
            ImGui::Text("Circle: center(%.1f, %.1f), r=%.1f", circ.pos.x, circ.pos.y, circ.rad);
            ImGui::TreePop();
        }
    }

    //// --- Health ????????????????/ ---
    //if (Has_Component(selectedEntity, CT_Health)) {
    //    auto& h = Get_HealthComponent(selectedEntity);
    //    if (ImGui::TreeNode("Health")) {
    //        ImGui::SliderFloat("HP", &h.currhp, 0.f, h.maxhp);
    //        ImGui::Text("Max HP: %.1f", h.maxhp);
    //        ImGui::TreePop();
    //    }
    //}

    // --- Tag ---
    if (m_registry.Has_Component<TagComponent>(selectedEntity)) {
        auto& tg = m_registry.Get_Component<TagComponent>(selectedEntity);
        if (ImGui::TreeNode("Tag")) {
            int tagVal = (int)tg.tag;
            const char* items[] = { "Player", "Enemy", "Projectile" };
            if (ImGui::Combo("Type", &tagVal, items, IM_ARRAYSIZE(items))) {
                tg.tag = (C_TagID)tagVal;
            }
            ImGui::TreePop();
        }
    }
    ImGui::Separator();
    ImGui::Text("Add Component:");
    auto& r = m_registry;

    DrawAddComponentButton<TransformComponent>("Transform", selectedEntity, r);
    DrawAddComponentButton<Sprite::SpriteComponent>("Sprite", selectedEntity, r, r.Has_Component<TransformComponent>(selectedEntity)); // needs Transform
    DrawAddComponentButton<Animator::AnimatorComponent>("Animator", selectedEntity, r,r.Has_Component<Sprite::SpriteComponent>(selectedEntity)); // needs Sprite
    DrawAddComponentButton<Physics::PhysicsComponent>("Physics", selectedEntity, r, r.Has_Component<TransformComponent>(selectedEntity)); // needs Transform
    DrawAddComponentButton<ShapeComponent>("Shape", selectedEntity, r, r.Has_Component<TransformComponent>(selectedEntity)); // needs Transform
    DrawAddComponentButton<AudioComponent>("Audio", selectedEntity, r);
    DrawAddComponentButton<InputComponent>("Input", selectedEntity, r);
    DrawAddComponentButton<TagComponent>("Tag", selectedEntity, r);

    ImGui::Separator();
    if (ImGui::Button("Clone")) {
        Entity clone = m_registry.Create_Entity();

        CloneComponent<TransformComponent>(selectedEntity, clone, m_registry);
        CloneComponent<Physics::PhysicsComponent>(selectedEntity, clone, m_registry);
        CloneComponent<ShapeComponent>(selectedEntity, clone, m_registry);
        CloneComponent<Sprite::SpriteComponent>(selectedEntity, clone, m_registry);
        CloneComponent<Animator::AnimatorComponent>(selectedEntity, clone, m_registry);
        CloneComponent<TagComponent>(selectedEntity, clone, m_registry);
        CloneComponent<InputComponent>(selectedEntity, clone, m_registry);

        //// Copy Health
        //if (Has_Component(selectedEntity, CT_Health)) {
        //    auto& h = Get_HealthComponent(selectedEntity);
        //    Add_Component(clone, h);
        //}

        selectedEntity = clone;  // select new clone
    }



    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
        if (selectedEntity <= 7) {
            ImGui::OpenPopup("Protected Entity");
        }
        else {
            m_registry.Remove_Entity(selectedEntity);
            selectedEntity = INVALID_ENTITY;
        }
    }
    if (ImGui::BeginPopup("Protected Entity")) {
        ImGui::Text("Cannot delete original Entities!");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
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
            static std::vector<Image> loadedThumbnails;
            static std::vector<std::string> imageNames;
            static bool initialized = false;

            if (!initialized)
            {
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
                                loadedThumbnails.push_back(img);
                                imageNames.push_back(entry.path().filename().string());
                            }
                        }
                        catch (...) {
                            std::cout << "Failed to load " << path << std::endl;
                        }
                    }
                }
                initialized = true;
            }

            if (loadedThumbnails.empty()) {
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
                for (size_t i = 0; i < loadedThumbnails.size(); ++i)
                {
                    ImGui::BeginGroup();
                    GLuint texID = loadedThumbnails[i].textureID;

                    // show image (flipped UVs)
                    ImGui::Image((ImTextureID)(intptr_t)texID,
                        ImVec2(thumbSize, thumbSize),
                        ImVec2(0, 1), ImVec2(1, 0));

                    // filename
                    ImGui::TextWrapped("%s", imageNames[i].c_str());
                    ImGui::EndGroup();

                    if (++count % columns != 0)
                        ImGui::SameLine();
                }

                ImGui::EndChild();
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
                LoadAudioManifest(manifestPath);
                manifestLoaded = true;
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

                    // Add entry
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

            // --- DISPLAY EXISTING ENTRIES ---
            for (auto& [category, assets] : audioManifest.items())
            {
                if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (auto& [name, data] : assets.items())
                    {
                        ImGui::PushID(name.c_str());
                        ImGui::Text("Name: %s", name.c_str());
                        ImGui::Text("Path: %s", data["path"].get<std::string>().c_str());
                        ImGui::Text("ID: %d", data["id"].get<int>());
                        ImGui::Text("Loop: %s", data["loop"].get<bool>() ? "true" : "false");
                        ImGui::Text("Group: %s", data["group"].get<std::string>().c_str());

                        // Preview controls
                        if (ImGui::Button("Play")) {
                            FMOD::Sound* snd = AudioSystem::LoadSound(data["path"], data["loop"]);
                            if (snd) {
                                if (previewChannel) previewChannel->stop();
                                previewChannel = AudioSystem::PlaySoundGetChannel(snd, false);
                                if (previewChannel) previewChannel->setVolume(0.8f);
                            }
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Stop")) {
                            if (previewChannel) previewChannel->stop();
                            previewChannel = nullptr;
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Remove")) {
                            assets.erase(name);
                            SaveAudioManifest(manifestPath);
                            ImGui::PopID();
                            break; // exit this loop safely since we modified
                        }

                        ImGui::Separator();
                        ImGui::PopID();
                    }
                }
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::Separator();
}



void ImGuiSystem::Shutdown() {
    ShutdownImGui();
}

void ImGuiSystem::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}