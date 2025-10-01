/* ========================================================
 * @file    ImguiSystem.cpp
 * @author   w.shiyuregine@digipen.edu
 * @date    01/10/2025
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
 * Copyright © 2025 DigiPen, All rights reserved.
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

bool ImGuiSystem::showColliders = false;
bool ImGuiSystem::showVelocity = false;
Entity ImGuiSystem::selectedEntity = INVALID_ENTITY;
bool ImGuiSystem::guiEnabled = false;
static Panel activePanel = Panel::None;


namespace {
    // small helper to name an entity for the Hierarchy list
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
        if (Has_Component(e, CT_Shape)) {
            auto& s = Get_ShapeComponent(e);
            if (s.shape == SHAPE_RECTANGLE) {
                name += "  [Square]";
            }
            else if (s.shape == SHAPE_CIRCLE) {
                name += "  [Circle]";
            }
        }
        else if (Has_Component(e, CT_Sprite)) {
            std::string entName = GetEntityName(e);
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
    if (!guiEnabled) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Game Editor");

    if (ImGui::Button("Engine Info")) activePanel = Panel::Engine;
    if (ImGui::Button("Collision + Scene Management")) activePanel = Panel::Collision;
    if (ImGui::Button("Editor")) activePanel = activePanel = Panel::HierarchyInspector;

    ImGui::Separator();
    switch (activePanel) {
    case Panel::Engine:
        ShowEngineDebugWindow();
        break;

    case Panel::Collision:
        ShowCollisionDebugWindow();
        break;

    case Panel::HierarchyInspector:
        //ImGui own 2D vector struct
        ImGui::BeginChild("LeftPane", ImVec2(ImGui::GetWindowWidth() * 0.4f, 0), true);
        ShowHierarchyWindow();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightPane", ImVec2(0, 0), true);
        ShowInspectorWindow();
        ImGui::EndChild();
        break;

    default:
        ImGui::Text("Select a panel above.");
        break;
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

    for (auto& [entity, physics] : CM_Physics().getComponents()) {
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
        for (auto& [entity, tag] : CM_Tag().getComponents()) {
            if (Has_Component(entity, CT_Transform)) {
                TransformComponent& transform = Get_TransformComponent(entity);
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
}


void ImGuiSystem::ShowHierarchyWindow() {

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

    // JQ COMMENT OUT HAHAHA
    ImGui::Separator();
    ImGui::Text("Create:");
    if (ImGui::Button("Add Player")) {
        selectedEntity = Create_Player(200.f, 200.f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Rectangle")) {
        selectedEntity = Create_Shape(400.f, 200.f, SHAPE_RECTANGLE, CS_IJKL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Circle")) {
        selectedEntity = Create_Shape(600.f, 200.f, SHAPE_CIRCLE, CS_ARROWS);
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
            std::string texPath = s.Get_TexturePath();
            ImGui::Text("Texture: %s", texPath.c_str());

            //scale/dimensions
            Vec2 scale = s.Get_Scale();
            if (ImGui::InputFloat2("Scale", &scale.x)) {
                s.Set_Size(scale);
            }
            ImGui::TreePop();
        }
    }
    if (Has_Component(selectedEntity, CT_Shape)) {
        auto& s = Get_ShapeComponent(selectedEntity);
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
    if (Has_Component(selectedEntity, CT_Physics)) {
        auto& p = Get_PhysicsComponent(selectedEntity);
        if (ImGui::TreeNode("Physics")) {
            auto& aabb = p.Get_AABB();
            auto& circ = p.Get_Circle();
            ImGui::Checkbox("Collision Active", const_cast<bool*>(&p.CollisionActive()));
            Vec2 aabbSize = aabb.dim;
            if (ImGui::DragFloat2("AABB Size", &aabbSize.x, 1.0f, 10.0f, 1000.0f)) {
                auto& trans = Get_TransformComponent(selectedEntity);
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
        Entity clone = Create_Entity();

        // Copy Transform
        if (Has_Component(selectedEntity, CT_Transform)) {
            auto& t = Get_TransformComponent(selectedEntity);
            TransformComponent newT = t;
            newT.pos.x += 50.f;  // so it's not exactly on top
            Add_Component(clone, newT);
        }

        // Copy Physics
        if (Has_Component(selectedEntity, CT_Physics)) {
            auto& p = Get_PhysicsComponent(selectedEntity);
            auto& aabb = p.Get_AABB();
            auto& circle = p.Get_Circle();
            Add_Component(clone, p);
        }

        // Copy Shape
        if (Has_Component(selectedEntity, CT_Shape)) {
            auto& s = Get_ShapeComponent(selectedEntity);
            Add_Component(clone, ShapeComponent{ s.shape, s.dimensions, s.color });
        }

        // Copy Sprite
        if (Has_Component(selectedEntity, CT_Sprite)) {
            auto& s = Get_SpriteComponent(selectedEntity);
            Add_Component(clone, s);
        }

        if (Has_Component(selectedEntity, CT_Animator)) {
            auto& anim = Get_AnimatorComponent(selectedEntity);
            Add_Component(clone, anim);
        }

        // Copy Tag
        if (Has_Component(selectedEntity, CT_Tag)) {
            auto& tag = Get_TagComponent(selectedEntity);
            Add_Component(clone, tag);
        }

        // Copy Input
        if (Has_Component(selectedEntity, CT_Input)) {
            auto& inp = Get_InputComponent(selectedEntity);
            Add_Component(clone, inp);
        }

        //// Copy Health
        //if (Has_Component(selectedEntity, CT_Health)) {
        //    auto& h = Get_HealthComponent(selectedEntity);
        //    Add_Component(clone, h);
        //}

        selectedEntity = clone;  // select new clone
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
        if (selectedEntity <= 3) {
            ImGui::OpenPopup("Protected Entity");
        }
        else {
            Remove_Entity(selectedEntity);
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

void ImGuiSystem::Shutdown() {
    ShutdownImGui();
}

void ImGuiSystem::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}