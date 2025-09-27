#include "ImGuiSystem.h"
#include "Registry.h"
#include "glapp.h"
#include "CoreEngine.h"
#include "EntityCreation.h"


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
    int activeAABBs = 0;
    int totalAABBs = 0;
    //for (auto& [entity, aabb] : CM_AABB().getComponents()) { //loop all aabb
    //    totalAABBs++; 
    //    if (aabb.active) activeAABBs++;
    //}

    ImGui::Text("AABB Colliders: %d / %d active", activeAABBs, totalAABBs);

    // show entity positions
    if (ImGui::CollapsingHeader("Entity Positions")) {
        for (auto& [entity, tag] : CM_Tag().getComponents()) {
            if (Has_Component(entity, CT_Transform)) {
                TransformComponent& transform = Get_TransformComponent(entity);
                //tag enum to readable string
                const char* tagName = (tag.tag == T_PLAYER) ? "Player" : (tag.tag == T_ENEMY) ? "Enemy" : "Unknown";
                //entity type, ID, and current position
                ImGui::Text("%s %d: (%.1f, %.1f)", tagName, entity,transform.pos.x, transform.pos.y);
            }
        }
    }

    ImGui::End();
}

//entity positions n edit
void ImGuiSystem::ShowEntityDebugWindow() {
    ImGui::Begin("Entity");

    int playerCount = 0, enemyCount = 0, totalEntities = 0;
    for (auto& [entity, tag] : CM_Tag().getComponents()) {
        totalEntities++;
        if (tag.tag == T_PLAYER) playerCount++;
        if (tag.tag == T_ENEMY) enemyCount++;
    }

    ImGui::Text("Total Entities: %d", totalEntities);
    ImGui::Text("Players: %d", playerCount);
    ImGui::Text("Enemies: %d", enemyCount);

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
    if (ImGui::Button("Add New Player")) {
        Entity newPlayer = Create_Player(100.f, 100.f);
    }


    if (ImGui::CollapsingHeader("Entity Edit")) {
        std::vector<Entity> toDelete;
        for (auto& [entity, tag] : CM_Tag().getComponents()) { //loop through enity tag
            if (Has_Component(entity, CT_Transform)) { //only entities w xform
                //need get ref to change xform
                TransformComponent& transform = Get_TransformComponent(entity);
                //same to readable shii
                const char* tagName = (tag.tag == T_PLAYER) ? "Player" : (tag.tag == T_ENEMY) ? "Enemy" : "Unknown";

                //creates unique ID for each entity sliders
                ImGui::PushID(entity);

                
                ImGui::Text("%s %d:", tagName, entity);
                ImGui::SliderFloat("X", &transform.pos.x, 0.0f, 1920.0f);
                ImGui::SliderFloat("Y", &transform.pos.y, 0.0f, 1080.0f);

                //typing values
                // ImGui::InputFloat("X", &transform.pos.x);
                // ImGui::InputFloat("Y", &transform.pos.y);
                if (ImGui::Button("Delete Entity")) {
                    // allow deleting players only if more than 1 remain
                    if (tag.tag == T_PLAYER && playerCount > 1) {
                        toDelete.push_back(entity);
                    }
                }

                ImGui::PopID(); // end unique ID scope
                ImGui::Separator(); // visual separator between entities
            }

        }
        for (Entity e : toDelete) {
            Remove_Entity(e);
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