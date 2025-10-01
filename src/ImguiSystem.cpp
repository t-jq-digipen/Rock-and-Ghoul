/* ========================================================
 * @file	ImguiSystem.cpp
 * @author	WONG SHI YU REGINE
 * @date	1 OCTOBER 2025
 *
 * @brief
 *
 *
 *
 *
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

        // JQ COMMENT OUT HAHAHA
        // If it has a sprite, append a hint (textured / square / circle)
        /*if (Has_Component(e, CT_Sprite)) {
            auto& s = Get_SpriteComponent(e);
            if (s.textureID == 0)      name += "  [Square]";
            else if (s.textureID == 1) name += "  [Circle]";
            else                       name += "  [Textured]";
        }*/
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
            // Try to identify sprite type by name or other properties
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

    // JQ COMMENT OUT HAHAHA
    // --- Sprite ---
    /*if (Has_Component(selectedEntity, CT_Sprite)) {
        auto& s = Get_SpriteComponent(selectedEntity);
        if (ImGui::TreeNode("Sprite")) {
            ImGui::Text("Texture ID: %u", s.textureID);
            ImGui::SliderFloat("Width", &s.size.x, 10.f, 2000.f);
            ImGui::SliderFloat("Height", &s.size.y, 10.f, 2000.f);
            ImGui::ColorEdit3("Tint", &s.color.x);
            ImGui::TreePop();
        }
    }*/
    // --- Sprite ---
    if (Has_Component(selectedEntity, CT_Sprite)) {
        auto& s = Get_SpriteComponent(selectedEntity);
        if (ImGui::TreeNode("Sprite")) {
            // Display texture path (read-only)
            std::string texPath = s.Get_TexturePath();
            ImGui::Text("Texture: %s", texPath.c_str());

            //scale/dimensions
            Vec2 scale = s.Get_Scale();
            if (ImGui::DragFloat2("Scale", &scale.x, 0.01f, 0.01f, 10.0f)) {
                s.Set_Size(scale);
            }

            // if animated
            bool animated = s.IsAnimated();
            ImGui::Checkbox("Animated", &animated);
            s.Set_Animated(animated);

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
            // Show AABB & Circle data (read-only snapshot)
            auto aabb = p.Get_AABB();
            auto circ = p.Get_Circle();
            ImGui::Checkbox("Collision Active", const_cast<bool*>(&p.CollisionActive())); // if your API allows toggling, change to setter
            //Edit AABB size
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

    // JQ COMMENT OUT HAHAHA
    /*if (ImGui::Button("Clone")) {
        if (Has_Component(selectedEntity, CT_Sprite)) {
            auto& s = Get_SpriteComponent(selectedEntity);
            if (s.shape == SHAPE_RECT)      selectedEntity = Create_Square(400.f, 200.f);
            else if (s.shape == SHAPE_CIRCLE) selectedEntity = Create_Circle(600.f, 200.f);
            else                       selectedEntity = Create_Player(200.f, 200.f);
        }
        else {
            selectedEntity = Create_Player(200.f, 200.f);
        }
    }*/

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
            auto aabb = p.Get_AABB();
            auto circle = p.Get_Circle();
            Physics::PhysicsComponent newP(0.015f, p.CollisionActive(), aabb, circle);
            Add_Component(clone, newP);
        }

        // Copy Shape
        if (Has_Component(selectedEntity, CT_Shape)) {
            auto& s = Get_ShapeComponent(selectedEntity);
            Add_Component(clone, ShapeComponent{ s.shape, s.dimensions, s.color });
        }

        // Copy Sprite
        if (Has_Component(selectedEntity, CT_Sprite)) {
            auto& s = Get_SpriteComponent(selectedEntity);
            Add_Component(clone, s);  // Copy entire sprite component
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

        selectedEntity = clone;  // Select the new clone
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