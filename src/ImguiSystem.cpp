#include "ImGuiSystem.h"
#include "Registry.h"
#include "glapp.h"
#include "CoreEngine.h"
#include "EntityCreation.h"

bool ImGuiSystem::showColliders = false;
bool ImGuiSystem::showVelocity = false;

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
    //start a frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //all windows
    ShowEngineDebugWindow(); //engine fps n shii
    ShowCollisionDebugWindow(); //collision n entity loc
    ShowEntityDebugWindow(); //entity debug pos
}
// called every frame after Update() to actually render ImGui to screen
//  MUSTTT be called after NewFrame() and before buffer swap
void ImGuiSystem::Render() {
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

//entity positions n edit
void ImGuiSystem::ShowEntityDebugWindow() {
    ImGui::Begin("Entity");

    int playerCount = 0, squareCount = 0, circleCount = 0, totalEntities = 0;
    for (auto& [entity, tag] : CM_Tag().getComponents()) {
        totalEntities++;
        if (Has_Component(entity, CT_Sprite)) {
            auto& sprite = Get_SpriteComponent(entity);
            if (sprite.textureID == 0) squareCount++;
            else if (sprite.textureID == 1) circleCount++;
            else playerCount++;
        }
        //if (tag.tag == T_PLAYER) playerCount++;
        //if (tag.tag == T_ENEMY) enemyCount++;
    }

    ImGui::Text("Total Entities: %d", totalEntities);
    ImGui::Text("Players: %d", playerCount);
    //ImGui::Text("Enemies: %d", enemyCount);

    // Show health info future????????????
    //if (ImGui::CollapsingHeader("Health Status")) {
    //    for (auto& [entity, health] : CM_Health().getComponents()) {
    //        if (Has_Component(entity, CT_Tag)) {
    //            TagComponent& tag = Get_TagComponent(entity);
    //            const char* tagName = (tag.tag == T_PLAYER) ? "Player" : (tag.tag == T_ENEMY) ? "Enemy" : "Unknown";
    //            ImGui::Text("%s %d: %.1f / %.1f HP", tagName, entity, health.currhp, health.maxhp);
    //        }
    //    }
    //}

    if (ImGui::CollapsingHeader("Entity Edit")) {
        std::vector<Entity> toDelete;
        std::vector<std::function<void()>> toAdd;
        for (auto& [entity, tag] : CM_Tag().getComponents()) { //loop through enity tag
            if (Has_Component(entity, CT_Transform)) { //only entities w xform
                //need get ref to change xform
                TransformComponent& transform = Get_TransformComponent(entity);
                //same to readable shii
                const char* tagName = (tag.tag == T_PLAYER) ? "Player" : (tag.tag == T_ENEMY) ? "Enemy" : "Unknown";

                //creates unique ID for each entity sliders
                ImGui::PushID(entity);

                
                ImGui::Text("%s %d:", tagName, entity);
                ImGui::SameLine();
                ImGui::SliderFloat("X", &transform.pos.x, 0.0f, 1920.0f);
                ImGui::SliderFloat("Y", &transform.pos.y, 0.0f, 1080.0f);

                //typing values
                // ImGui::InputFloat("X", &transform.pos.x);
                // ImGui::InputFloat("Y", &transform.pos.y);                
               
                if (ImGui::Button("Clone")) {
                    if (Has_Component(entity, CT_Sprite)) {
                        auto& sprite = Get_SpriteComponent(entity);
                        // Here you could use tag.tag OR texture ID to decide
                        if (sprite.textureID == 0) {
                            toAdd.push_back([] { Create_Square(400.f, 200.f); });
                        }
                        else if (sprite.textureID == 1) {
                            toAdd.push_back([] { Create_Circle(600.f, 200.f); });
                        }
                        else {
                            toAdd.push_back([] { Create_Player(200.f, 200.f); });
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete Entity")) {
                    // allow deleting players only if more than 1 remain
                    if (Has_Component(entity, CT_Sprite)) {
                        auto& sprite = Get_SpriteComponent(entity);
                        if (sprite.textureID == 0) { // Square
                            if (squareCount > 1) {
                                toDelete.push_back(entity);
                            }
                        }
                        else if (sprite.textureID == 1) { // Circle
                            if (circleCount > 1) {
                                toDelete.push_back(entity);
                            }
                        }
                        else { // Player
                            if (playerCount > 1) {
                                toDelete.push_back(entity);
                            }
                        }
                    }
                }

                ImGui::PopID(); // end unique ID scope
                ImGui::Separator(); // visual separator between entities
            }

        }
        for (Entity e : toDelete) {
            Remove_Entity(e);
        }
        for (auto& fn : toAdd) {
            fn();
        }
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