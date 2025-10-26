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
bool ImGuiSystem::guiEnabled = false;
static FMOD::Channel* previewChannel = nullptr;
#include "DataDefs.h"

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
static void MakeWidget(Editor::Widget_Dropdown& w, const std::string& label) {
    if (!w.ref || w.options.empty()) return;
    std::vector<const char*> items;
    for (auto& opt : w.options) items.push_back(opt.c_str());
    int current = static_cast<int>(*w.ref);
    if (ImGui::Combo(label.c_str(), &current, items.data(), (int)items.size())) {
        *w.ref = static_cast<unsigned int>(current);
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
        if (ImGui::TreeNode(componentData.GetComponentName().c_str()))
        {
            for (auto& [name, widgetVariant] : componentData.GetDataMap())
            {
                // create a unique ID scope using the field name
                ImGui::PushID(name.c_str());

                std::visit([&](auto& widget) {
                    MakeWidget(widget, name);
                    }, widgetVariant);

                ImGui::PopID();
            }

            ImGui::TreePop();
        }
    }


    //ImGui::Separator();
    //ImGui::Text("Component Management");
    //ImGui::Separator();

    //Registry& reg = Registry::Get();

    //// current components of selected entity
    ////std::vector<Editor::ComponentData> compDataVec = reg.Get_EntityData(selectedEntity);
    //std::unordered_set<std::string> currentNames;
    //for (auto& c : compDataVec)
    //    currentNames.insert(c.GetComponentName());

    //// auto-discover all registered components dynamically
    //std::unordered_set<std::string> allComponentNames;

    //// scan every entity for component types (covers all registered managers)
    //for (auto& [e, _] : reg.Get_EntityNameMap())
    //{
    //    if (!EntityManager::get().isAlive(e))
    //        continue;

    //    auto data = reg.Get_EntityData(e);
    //    for (auto& c : data)
    //        allComponentNames.insert(c.GetComponentName());
    //}

    // build a list of components that this entity doesn't yet have
    //std::vector<std::string> addable;
    //for (auto& name : allComponentNames)
    //{
    //    if (!currentNames.contains(name))
    //        addable.push_back(name);
    //}

    //// show "Add Component" dropdown
    //if (!addable.empty())
    //{
    //    static int addIndex = 0;
    //    ImGui::Combo("Add Component", &addIndex,
    //        [](void* data, int idx, const char** out_text)
    //        {
    //            const auto& v = *reinterpret_cast<const std::vector<std::string>*>(data);
    //            *out_text = v[idx].c_str();
    //            return true;
    //        },
    //        (void*)&addable, (int)addable.size());

    //    if (ImGui::Button("Add Selected Component"))
    //    {
    //        Registry& reg = Registry::Get();
    //        std::string compToAdd = addable[addIndex];

    //        // Step 1: Save the selected entity
    //        SavedEntity saved = reg.Save_Entity(selectedEntity);

    //        // Step 2: Create a temporary entity
    //        Entity temp = reg.Create_Entity();
    //        reg.Set_EntityName(temp, "TempEntityForAdd");

    //        // Step 3: Try to add the component using serialization system
    //        std::string defaultData = "{}";
    //        try
    //        {
    //            // Build a SavedEntity with only this new component
    //            SavedEntity tempSave;
    //            tempSave.name = "TempEntityForAdd";
    //            tempSave.componentData[compToAdd] = "{}";  // empty data is OK if DeSerialize() is safe

    //            // Load to get proper default internal state
    //            Entity temp2 = reg.Load_Entity(tempSave);

    //            // Serialize that component back out to get valid default JSON
    //            defaultData = reg.Save_Entity(temp2).componentData[compToAdd];

    //            // cleanup
    //            reg.Remove_Entity(temp2);
    //        }
    //        catch (...)
    //        {
    //            std::cout << "[AddComponent Warning] Default data generation failed for "
    //                << compToAdd << ", using empty {}\n";
    //        }

    //        // Step 4: Merge the new component into the entity
    //        nlohmann::json j;
    //        j["name"] = saved.name;
    //        for (auto& [key, val] : saved.componentData)
    //            j["componentData"][key] = val;

    //        j["componentData"][compToAdd] = defaultData;

    //        // Step 5: Rebuild SavedEntity manually (no private Registry calls)
    //        SavedEntity modified;
    //        modified.name = j["name"];
    //        for (auto& [key, val] : j["componentData"].items())
    //            modified.componentData[key] = val.dump();

    //        // Step 6: Replace the entity safely
    //        reg.Remove_Entity(selectedEntity);
    //        selectedEntity = reg.Load_Entity(modified);

    //        std::cout << "[ImGui] Added component: " << compToAdd
    //            << " to entity " << selectedEntity << "\n";
    //    }

    //}
    //else
    //{
    //    ImGui::TextDisabled("(All registered components already added)");
    //}

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
                        bool removeThis = false;

                        if (ImGui::Button("Remove")) {
                            removeThis = true;
                        }

                        ImGui::Separator();
                        ImGui::PopID();

                        if (removeThis) {
                            assets.erase(name);
                            SaveAudioManifest(manifestPath);
                            break;
                        }
                    }
                }
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
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