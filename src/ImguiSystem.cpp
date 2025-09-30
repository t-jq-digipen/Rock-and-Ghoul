#include "ImGuiSystem.h"
#include "Registry.h"
#include "glapp.h"
#include "CoreEngine.h"
#include "EntityCreation.h"
#include "Input.h"

bool ImGuiSystem::showColliders = false;
bool ImGuiSystem::showVelocity = false;
Entity ImGuiSystem::selectedEntity = INVALID_ENTITY;
bool ImGuiSystem::guiEnabled = false;

namespace {
    // A small helper to name an entity for the Hierarchy list
    std::string EntityDisplayName(Entity e) {
        std::string name = "Entity " + std::to_string(e);

        if (Has_Component(e, CT_Tag)) {
            auto& tag = Get_TagComponent(e);
            const char* tagName = (tag.tag == T_PLAYER) ? "Player"
                : (tag.tag == T_ENEMY) ? "Enemy"
                : (tag.tag == T_PROJECTILE) ? "Projectile"
                : "Unknown";
            name = std::string(tagName) + " " + std::to_string(e);
        }

        // If it has a sprite, append a hint (textured / square / circle)
        if (Has_Component(e, CT_Sprite)) {
            auto& s = Get_SpriteComponent(e);
            if (s.textureID == 0)      name += "  [Square]";
            else if (s.textureID == 1) name += "  [Circle]";
            else                       name += "  [Textured]";
        }

        return name;
    }

    //selectedEntity stays valid
    void EnsureSelectionValid() {
        if (ImGuiSystem::selectedEntity != INVALID_ENTITY && !EntityManager::get().isAlive(ImGuiSystem::selectedEntity)) {
            ImGuiSystem::selectedEntity = 0;
        }
    }
}

void ImGuiSystem::InitializeImGui(GLFWwindow* window) {
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
    InitializeImGui(GLApp::ptr_window);  // Do it here instead of main
}

void ImGuiSystem::Update(float dt) {
    if (Input::IsKeyPressed(GLFW_KEY_F1)) {
        guiEnabled = !guiEnabled; // toggle on/off
    }
    ////start a frame
    //ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplGlfw_NewFrame();
    //ImGui::NewFrame();

    ////all windows
    //ShowEngineDebugWindow(); //engine fps n shii
    //ShowCollisionDebugWindow(); //collision n entity loc
    //ShowHierarchyWindow();   // NEW
    //ShowInspectorWindow();

    if (guiEnabled) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ShowEngineDebugWindow();
        ShowCollisionDebugWindow();
        ShowHierarchyWindow();
        ShowInspectorWindow();
    }
}
// called every frame after Update() to actually render ImGui to screen
//  MUSTTT be called after NewFrame() and before buffer swap
void ImGuiSystem::Render() {
    if (!guiEnabled) return; //skip if GUI is off
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); //draw to screen
}

void ImGuiSystem::ShowEngineDebugWindow() {
    ImGui::Begin("Engine Info");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate); //ms
    CoreEngine& engine = CoreEngine::GetInstance(); // however you access it
    auto& timings = engine.GetTimings();

    // Calculate total frame time
    double total = 0.0;
    for (auto& [name, t] : timings) total += t.ms;

    ImGui::Separator();
    ImGui::Text("System Timings:");
    // Loop through each system and display its timing
    for (auto& [name, t] : timings) { //gets reference to the real map
        // Calculate percentage of total frame time this system used
        double percent = (total > 0.0) ? (t.ms / total) * 100.0 : 0.0;
        // Show system name, elapsed time in ms, and percentage of total frame
        ImGui::Text("%s: %.3f ms (%.1f%%)", name.c_str(), t.ms, percent);

        //visual bar
        ImGui::ProgressBar((float)(percent / 100.0f), ImVec2(-1, 0), (std::to_string((int)percent) + "%").c_str());
    }
    ImGui::End();
}

//collili
void ImGuiSystem::ShowCollisionDebugWindow() {
    ImGui::Begin("Collision Debug");

    // count active colliders
    int activeColliders = 0;
    int totalColliders = 0;

    for (auto& [entity, physics] : CM_Physics().getComponents()) {
        totalColliders++;
        if (physics.CollisionActive()) { // Use the physics component's collision active check
            activeColliders++;
        }
    }

    ImGui::Text("AABB Colliders: %d / %d active", activeColliders, totalColliders);

    // colili
    //if (ImGui::CollapsingHeader("Physics Components")) {
    //    for (auto& [entity, physics] : CM_Physics().getComponents()) {
    //        if (Has_Component(entity, CT_Transform) && Has_Component(entity, CT_Tag)) {
    //            // oni process entities that have Transform and Tag components
    //            TransformComponent& transform = Get_TransformComponent(entity);
    //            TagComponent& tag = Get_TagComponent(entity);
    //            // udate collision shapes (AABB and Circle) to match current position
    //            physics.UpdateHitboxes(transform.pos);

    //            const char* tagName = (tag.tag == T_PLAYER) ? "Player" :(tag.tag == T_ENEMY) ? "Enemy" :(tag.tag == T_PROJECTILE) ? "Projectile" : "Unknown";

    //            AABB aabb = physics.Get_AABB();
    //            Circle circle = physics.Get_Circle();

    //            ImGui::Text("%s %d:", tagName, entity);
    //            ImGui::Text("  Position: (%.1f, %.1f)", transform.pos.x, transform.pos.y);
    //            ImGui::Text("  AABB: (%.1f, %.1f) to (%.1f, %.1f)", aabb.min.x, aabb.min.y, aabb.max.x, aabb.max.y);
    //            ImGui::Text("  Circle: center(%.1f, %.1f) radius %.1f", circle.pos.x, circle.pos.y, circle.rad);
    //            ImGui::Text("  Collision Active: %s", physics.CollisionActive() ? "Yes" : "No");
    //            ImGui::Separator();
    //        }
    //    }
    //}
    ImGui::Checkbox("Show Colliders", &showColliders);
    ImGui::Checkbox("Show Velocity", &showVelocity);
    //enityt
    if (ImGui::CollapsingHeader("Entity Positions")) {
        for (auto& [entity, tag] : CM_Tag().getComponents()) {
            if (Has_Component(entity, CT_Transform)) {
                TransformComponent& transform = Get_TransformComponent(entity);
                const char* tagName = (tag.tag == T_PLAYER) ? "Player" :(tag.tag == T_ENEMY) ? "Enemy" :(tag.tag == T_PROJECTILE) ? "Projectile" : "Unknown";
                ImGui::Text("%s %d: (%.1f, %.1f)", tagName, entity, transform.pos.x, transform.pos.y);
            }
        }
    }

    ImGui::End();
}

void ImGuiSystem::ShowHierarchyWindow() {
    ImGui::Begin("Hierarchy");

    EnsureSelectionValid();

    // all entities that have a Tag
    for (auto& [entity, tag] : CM_Tag().getComponents()) {
        bool selected = (entity == selectedEntity);
        std::string label = EntityDisplayName(entity);
        //highlight cn click
        if (ImGui::Selectable(label.c_str(), selected)) {
            selectedEntity = entity;
        }
    }

    ImGui::Separator();
    ImGui::Text("Create:");
    if (ImGui::Button("Add Player")) {
        selectedEntity = Create_Player(200.f, 200.f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Square")) {
        selectedEntity = Create_Square(400.f, 200.f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Circle")) {
        selectedEntity = Create_Circle(600.f, 200.f);
    }

    ImGui::End();
}

void ImGuiSystem::ShowInspectorWindow() {
    ImGui::Begin("Inspector");

    EnsureSelectionValid();
    //if no entity is selected, just show a message and return
    if (selectedEntity == INVALID_ENTITY) {
        ImGui::Text("No entity selected");
        ImGui::End();
        return;
    }
    // show the entity ID at top
    ImGui::Text("Entity %d", selectedEntity);
    ImGui::Separator();

    // --- Transform ---
    if (Has_Component(selectedEntity, CT_Transform)) {
        auto& t = Get_TransformComponent(selectedEntity);
        // collapsible tree node for Transform data
        if (ImGui::TreeNode("Transform")) {
            ImGui::SliderFloat("X", &t.pos.x, 0.f, 1920.f);
            ImGui::SliderFloat("Y", &t.pos.y, 0.f, 1080.f);
            ImGui::SliderAngle("Rotation", &t.rot, -360.0f, 360.0f);
            ImGui::TreePop();
        }
    }

    // --- Sprite ---
    if (Has_Component(selectedEntity, CT_Sprite)) {
        auto& s = Get_SpriteComponent(selectedEntity);
        if (ImGui::TreeNode("Sprite")) {
            ImGui::Text("Texture ID: %u", s.textureID);
            ImGui::SliderFloat("Width", &s.size.x, 10.f, 2000.f);
            ImGui::SliderFloat("Height", &s.size.y, 10.f, 2000.f);
            ImGui::ColorEdit3("Tint", &s.color.x);
            ImGui::TreePop();
        }
    }

    // --- Physics ---
    if (Has_Component(selectedEntity, CT_Physics)) {
        auto& p = Get_PhysicsComponent(selectedEntity);
        if (ImGui::TreeNode("Physics")) {
            // Show AABB & Circle data (read-only snapshot)
            auto aabb = p.Get_AABB();
            auto circ = p.Get_Circle();
            ImGui::Checkbox("Collision Active", const_cast<bool*>(&p.CollisionActive())); // if your API allows toggling, change to setter
            ImGui::Text("AABB: (%.1f, %.1f) -> (%.1f, %.1f)", aabb.min.x, aabb.min.y, aabb.max.x, aabb.max.y);
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
    if (Has_Component(selectedEntity, CT_Tag)) {
        auto& tg = Get_TagComponent(selectedEntity);
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

    if (ImGui::Button("Clone")) {
        if (Has_Component(selectedEntity, CT_Sprite)) {
            auto& s = Get_SpriteComponent(selectedEntity);
            if (s.textureID == 0)      selectedEntity = Create_Square(400.f, 200.f);
            else if (s.textureID == 1) selectedEntity = Create_Circle(600.f, 200.f);
            else                       selectedEntity = Create_Player(200.f, 200.f);
        }
        else {
            selectedEntity = Create_Player(200.f, 200.f);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
        Remove_Entity(selectedEntity);
        selectedEntity = INVALID_ENTITY;
    }

    ImGui::End();
}

void ImGuiSystem::Shutdown() {
    // ImGui cleanup is handled by static ShutdownImGui() in main
}

void ImGuiSystem::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}