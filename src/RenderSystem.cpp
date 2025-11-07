/* ========================================================
 * @file        RenderSystem.cpp
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @co-author   Kervin Ong Guan Cheng | kervinguancheng.ong@digipen.edu
 * @date        05 NOV 2025
 *
 * @brief
 *
 * Implements the RenderSystem, which is responsible for
 * drawing all visual components in the ECS framework.
 *
 * Responsibilities:
 * - Renders sprite components (static and animated).
 * - Renders shape components (rectangles and circles).
 * - Integrates with Animator for updating animated frames.
 * - Uses the central Render instance to issue draw calls.
 * - Supports debug drawing for physics (velocity vectors,
 *   AABBs, circle colliders, hotspots) controlled by ImGui.
 *
 * On shutdown, clears the texture cache to free GPU resources.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "RenderSystem.h"
#include "Render.h"
#include "Animator.h"
#include <Camera.h>
#include "LogicSystem.h"
#include "CollisionManager.h"
#include "CoreEngine.h"
 // ---------------------------------------------
 // @brief Temporary struct for per-frame draw queue
 // ---------------------------------------------
/*struct RenderEntry {
    Entity id;
    const Sprite::SpriteComponent* sprite;
    const TransformComponent* transform;
    float sortY;
};*/


Render& renderInstance = Render::GetInstance();
std::vector<InstanceData> batch; //added for instancing
static std::vector<GLuint> s_backgroundTextures;

void RenderSystem::AddBackgroundTexture(GLuint tex) {
    s_backgroundTextures.push_back(tex);
}

void RenderSystem::ClearBackgroundTextures() {
    s_backgroundTextures.clear();
}
//extern Camera mainCamera;
//Camera& mainCamera = Camera::GetInstance();


void RenderSystem::Initialize() {
    // ? 
    std::cout << "[RenderSystem] Initializing..." << std::endl;

    Render& renderer = Render::GetInstance();

    // Initialize your offscreen framebuffer for Game View
    renderer.InitSceneFramebuffer(GLApp::width, GLApp::height);

    std::cout << "[RenderSystem] Scene framebuffer created ("
        << GLApp::width << "x" << GLApp::height << ")\n";

    BehaviourFCT camFollow;
    camFollow.Init = [](Entity e) { CamFollowScript::Start(e); };
    camFollow.Update = [](Entity e, float dt) { CamFollowScript::Update(e, dt); };
    camFollow.End = [](Entity e) {CamFollowScript::End(e); };
    LogicSystem::Get().RegisterScript("CamTarget", camFollow, ScriptID::CAMERA_TRACK);

}

void RenderSystem::Update(float dt) {
    

    Camera::GetInstance().UpdateCamera();
    Render& renderer = Render::GetInstance();

    bool camcheck = renderer.useCamera;
    if (ImGuiSystem::guiEnabled) {
        renderer.BeginSceneCapture(); 
        renderer.ClearBackground({ 0.1f, 0.1f, 0.1f, 1.0f });
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, GLApp::width, GLApp::height);
        renderer.ClearBackground({ 0.0f, 0.0f, 0.0f, 1.0f }); // clear main framebuffer
    }

    static bool wasPaused = false;

    if (GSM_IsPaused()) {
        wasPaused = true;
    }
    else if (wasPaused) {
        // just unpaused — reset velocity cache so lines reappear immediately
        DebugDraw::ClearVelocityCache();
        wasPaused = false;
    }

    Render::GetInstance().UpdateProjection();
   // renderer.ClearBackground({ 0.05f, 0.0f, 0.1f, 1.0f });
    // ===== DRAW BACKGROUND IF SET =====

    for (GLuint tex : s_backgroundTextures)
        renderer.DrawFullscreenTexture(tex, true);
   


    // Use the camera for all subsequent draw calls (ok why this not workinGGGGGGGG AJBFEEUFBUJFBFDJSBJF)
    //mainCamera.UpdateMatrices();
    // Camera::GetInstance().UpdateMatrices();


    //glm::mat4 viewProj = mainCamera.GetViewProjection();
    glm::mat4 viewProj = Camera::GetInstance().GetViewProjection();

    renderInstance.SetUniformMat4(renderInstance.GetSpriteShader(), "uProjection", viewProj);
    


    /*glm::mat4 viewProj = mainCamera.GetViewProjection();
    GLuint shader = renderInstance.GetSpriteShader();
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjection"), 1, GL_FALSE, &viewProj[0][0]);*/

    //glUseProgram(renderInstance.GetSpriteShader());
    //glUniformMatrix4fv(glGetUniformLocation(renderInstance.GetSpriteShader(), "uProjection"), 1, GL_FALSE, &viewProj[0][0]);



// ======================================================
// trying batch-based rendering with layer and y-axis sorting
// my thought process is: 
// added a RenderLayerComponent for each entity (defaulted to gameobject layer unless specified inside entitycreation or json)
// group entities by layers, push them into each of their respective specified containers
// sort the game object layer entities based on their y coords from 
// reusable function drawlayer will be used to draw each layer
// ======================================================
    renderInstance.BeginBatch();

    struct RenderItem { //iidentify sprite or shape
        Entity id;
        bool isSprite;
    };

    //containers that contain entities for each layer
    std::vector<RenderItem> backgroundEntities;
    std::vector<RenderItem> gameEntities;
    std::vector<RenderItem> foregroundEntities; //for sepcial effects? particles?
    std::vector<RenderItem> uiEntities;

    //Categorize entities by layer
    for (auto& [entID, spriteComp] : m_registry.Get_ComponentMap<Sprite::SpriteComponent>())
    {
        RenderLayer layer = LAYER_GAMEOBJECT; // default, if not specified 

        if (m_registry.Has_Component<RenderLayerComponent>(entID)) {
            layer = m_registry.Get_Component<RenderLayerComponent>(entID).layer;
        }

        RenderItem item{ entID, true };

        switch (layer)
        {
        case LAYER_BACKGROUND:
            backgroundEntities.push_back(item);
            break;
        case LAYER_GAMEOBJECT:
            gameEntities.push_back(item);
            break;
        case LAYER_FOREGROUND:
            foregroundEntities.push_back(item);
            break;
        case LAYER_UI:
            uiEntities.push_back(item);
            break;
        }
    }

    //Handle the shape components
    for (auto& [entID, shapeComp] : m_registry.Get_ComponentMap<ShapeComponent>()) {

        RenderLayer layer = LAYER_GAMEOBJECT; //default give them layer_gameobject, if want change need explicitly add layer comp to entitycreation.cpp

        if (m_registry.Has_Component<RenderLayerComponent>(entID)) {
            layer = m_registry.Get_Component<RenderLayerComponent>(entID).layer;
        }

        RenderItem item{ entID, false };

        switch (layer)
        {
        case LAYER_BACKGROUND:
            backgroundEntities.push_back(item);
            break;
        case LAYER_GAMEOBJECT:
            gameEntities.push_back(item);
            break;
        case LAYER_FOREGROUND:
            foregroundEntities.push_back(item);
            break;
        case LAYER_UI:
            uiEntities.push_back(item);
            break;
        }
    }

    //Sort the entities with GAMEOBJECT layer by Y-position
    std::sort(gameEntities.begin(), gameEntities.end(),
        [&](const RenderItem& a, const RenderItem& b) {
            const auto& ta = m_registry.Get_Component<TransformComponent>(a.id);
            const auto& tb = m_registry.Get_Component<TransformComponent>(b.id);
            return ta.pos.y > tb.pos.y; // Higher Y = further back (drawn first)
        });

    //Draw a list of entities
    auto DrawLayer = [&](const std::vector<RenderItem>& entities)
        {
            for (const RenderItem& item : entities)
            {
                Entity entID = item.id;

                if (item.isSprite) { //if is sprite..

                    auto& spriteComp = m_registry.Get_Component<Sprite::SpriteComponent>(entID);
                    auto& transComp = m_registry.Get_Component<TransformComponent>(entID);

                    spriteComp.Update_ImageSize();

                    bool isBatched = false;
                    if (m_registry.Has_Component<TagComponent>(entID))
                    {
                        const auto& tag = m_registry.Get_Component<TagComponent>(entID);
                        if (tag.tag == T_BATCH) {
                            isBatched = true;
                        }
                    }

                    if (spriteComp.IsAnimated())
                    {
                        auto& animComp = m_registry.Get_Component<Animator::AnimatorComponent>(entID);
                        animComp.UpdateFrame(dt);

                        Image img = spriteComp.Get_Image();
                        glm::vec4 uv(animComp.u0 / img.widthPixels,
                            animComp.v1 / img.heightPixels,
                            animComp.u1 / img.widthPixels,
                            animComp.v0 / img.heightPixels);

                        if (isBatched) {
                            renderInstance.SubmitSpriteBatchUV(img, transComp.pos, spriteComp.Get_Scale(), uv, transComp.rot + spriteComp.Get_Rotation());
                        }
                        else {
                            renderInstance.DrawImageSub(img,
                                transComp.pos.x, transComp.pos.y,
                                spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
                                animComp.u0, animComp.v0, animComp.u1, animComp.v1,
                                glm::radians(transComp.rot + spriteComp.Get_Rotation()));
                        }
                    }
                    else
                    {
                        Image img = spriteComp.Get_Image();
                        Vec2 scaledSize = { img.widthPixels * spriteComp.Get_Scale().x, img.heightPixels * spriteComp.Get_Scale().y };

                        if (isBatched) {
                            renderInstance.SubmitSpriteBatch(img, transComp.pos, scaledSize, transComp.rot + spriteComp.Get_Rotation());
                        }
                        else {
                            renderInstance.DrawImageScaled(img,
                                transComp.pos.x, transComp.pos.y,
                                spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
                                glm::radians(transComp.rot + spriteComp.Get_Rotation()));
                        }
                    }
                }

                //shape stuff here (if is shape...)
                else {

                    // Skip entities without a transform (need position/rotation to draw)
                    if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
                    TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
                    ShapeComponent& shapeComp = m_registry.Get_Component<ShapeComponent>(entID);
                    Colour3 color = { shapeComp.color.x, shapeComp.color.y, shapeComp.color.z };

                    if (m_registry.Has_Component<TagComponent>(entID)) {
                        TagComponent& tag = m_registry.Get_Component<TagComponent>(entID);

                        if (shapeComp.shape == SHAPE_RECTANGLE && tag.tag == T_INSTANCING) { //check entity is using instancing
                            //Store matching entities inside InstanceData vector
                            InstanceData inst;
                            inst.position = transComp.pos;
                            inst.scale = transComp.scale;
                            inst.rotation = glm::radians(transComp.rot);
                            inst.uvRect = { 0,0,1,1 };
                            batch.push_back(inst);
                            continue;
                        }


                    }

                    if (shapeComp.shape == SHAPE_RECTANGLE) {
                        renderInstance.DrawRect(
                            transComp.pos,
                            shapeComp.dimensions.x,
                            shapeComp.dimensions.y,
                            color,
                            transComp.rot
                        );
                    }
                    else if (shapeComp.shape == SHAPE_CIRCLE) {
                        float radius = (shapeComp.dimensions.x + shapeComp.dimensions.y) / 4.0f;
                        renderInstance.DrawCircle(
                            transComp.pos,
                            radius,
                            color,
                            32
                        );
                    }
                }
            }
        };

    // Background (rendered behind game objects)
    renderInstance.BeginBatch();
    DrawLayer(backgroundEntities);
    renderInstance.EndBatch();
    renderInstance.FlushBatch();

    // Game objects
    renderInstance.BeginBatch();
    DrawLayer(gameEntities);
    renderInstance.EndBatch();
    renderInstance.FlushBatch();

    // Foreground (rendered above game objects)
    renderInstance.BeginBatch();
    DrawLayer(foregroundEntities);
    renderInstance.EndBatch();
    renderInstance.FlushBatch();

    // UI (rendered last, most in front)
    renderer.useCamera = false;
    renderInstance.BeginBatch();
    DrawLayer(uiEntities);
    renderInstance.EndBatch();
    renderInstance.FlushBatch();
    renderer.useCamera = camcheck;

    //Draw in the correct order
    /*DrawLayer(backgroundEntities);
      DrawLayer(gameEntities);
      DrawLayer(foregroundEntities);
      DrawLayer(uiEntities);

      renderInstance.EndBatch();
      renderInstance.FlushBatch();*/


    /*//try batching version (old version keep in case need ref)
    renderInstance.BeginBatch(); //added for batch

    for (auto& [entID, spriteComp] : m_registry.Get_ComponentMap<Sprite::SpriteComponent>())
    {
        // Update image size (need to link to event manager probably)
        spriteComp.Update_ImageSize();

        // Check if entity has the batching tag
        bool isBatched = false;
        if (m_registry.Has_Component<TagComponent>(entID))
        {
            const auto& tag = m_registry.Get_Component<TagComponent>(entID);
            if (tag.tag == T_BATCH)
                isBatched = true;
        }

        if (spriteComp.IsAnimated())
        {

            if (!m_registry.Has_Component<Animator::AnimatorComponent>(entID))
            {
                std::cout << "Sprite comp is animated but you tried to get AnimatorComponent! Setting sprite comp anim to false\n";
                spriteComp.Set_Animated(false);
                continue;
            }
            // draw animated logic
            TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
            if (!m_registry.Has_Component<Animator::AnimatorComponent>(entID))
            {
                std::cout << "RENDERSYSTEM: Sprite comp is animated but you tried to get AnimatorComponent! Setting sprite comp anim to false\n";
                spriteComp.Set_Animated(false);
                continue;
            }
            Animator::AnimatorComponent& animComp = m_registry.Get_Component<Animator::AnimatorComponent>(entID);

            animComp.UpdateFrame(dt);

            //added for batch
            Image img = spriteComp.Get_Image();
            glm::vec4 uv(animComp.u0 / img.widthPixels,
                (animComp.v1 / img.heightPixels),
                animComp.u1 / img.widthPixels,
                (animComp.v0 / img.heightPixels));

            if (isBatched) {
                renderInstance.SubmitSpriteBatchUV(img, transComp.pos, spriteComp.Get_Scale(), uv, transComp.rot + spriteComp.Get_Rotation());
            }
            //renderInstance.SubmitSpriteBatchUV(img, transComp.pos, spriteComp.Get_Scale(), uv, transComp.rot + spriteComp.Get_Rotation());
            //end of added for batch
            
            else {
                renderInstance.DrawImageSub(
                    img,
                    transComp.pos.x, transComp.pos.y,
                    spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
                    animComp.u0, animComp.v0, animComp.u1, animComp.v1,
                    glm::radians(transComp.rot + spriteComp.Get_Rotation()));
            }
            //renderInstance.DrawImageSub(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, animComp.u0, animComp.v0, animComp.u1, animComp.v1, transComp.rot + spriteComp.Get_Rotation());
        }
        else
        {
            TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);

            Image img = spriteComp.Get_Image();
            Vec2 scaledSize = {
                img.widthPixels * spriteComp.Get_Scale().x,
                img.heightPixels * spriteComp.Get_Scale().y
            };

            if (isBatched) {
                renderInstance.SubmitSpriteBatch(
                    img,
                    transComp.pos,
                    scaledSize,
                    transComp.rot + spriteComp.Get_Rotation());
            }
            /*renderInstance.SubmitSpriteBatch(
                img,
                transComp.pos,
                scaledSize,
                transComp.rot + spriteComp.Get_Rotation());*/

            /*else {
                renderInstance.DrawImageScaled(
                    img,
                    transComp.pos.x, transComp.pos.y,
                    spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
                    glm::radians(transComp.rot + spriteComp.Get_Rotation()));
            }
            //renderInstance.DrawImageScaled(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, transComp.rot + spriteComp.Get_Rotation());
        }
    }



    renderInstance.EndBatch();
    renderInstance.FlushBatch();*/


    // Debug drawing for spatial grid
    {
    SpatialGrid* grid = CollisionManager::GetSpatialGrid();
    if (ImGuiSystem::showSpatialGrid) {
        if (grid) {
            DebugDraw::DrawSpatialGrid(grid, renderInstance);
        }
    }
    grid->ClearCollisionMarkers();
    }

    // Debug drawing for physics colliders / velocities
    if (ImGuiSystem::showColliders || ImGuiSystem::showVelocity) {
        for (auto& [entID, physics] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>()) {

            if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
            auto& t = m_registry.Get_Component<TransformComponent>(entID);

            if (ImGuiSystem::showVelocity) {
                DebugDraw::DrawVelocity(entID, t, renderInstance, dt);
            }

            if (ImGuiSystem::showColliders) {
                DebugDraw::DrawAABB(physics.Get_AABB(), renderInstance);

                if (m_registry.Has_Component<ShapeComponent>(entID)) {
                    auto& shape = m_registry.Get_Component<ShapeComponent>(entID);
                    if (shape.shape == SHAPE_CIRCLE)
                        DebugDraw::DrawCircleCollider(physics.Get_Circle(), renderInstance);
                }

                DebugDraw::DrawHotspot(t.pos, renderInstance);
            }
        }
    }

    if (!batch.empty()) {
        //then draw them all at once
        renderInstance.DrawRectInstanced(batch, { 0.2f, 0.8f, 1.0f });
        batch.clear();
    }
    if (ImGuiSystem::guiEnabled) {
        renderer.EndSceneCapture();
    }
}

void RenderSystem::Shutdown() {
    // clear all image cache here? seems like a good place, if we only store not a lot of images
    Render::GetInstance().ClearAllTextures();
}

namespace CamFollowScript {
    inline void Start(Entity eID) {
        (void)eID;
    }

    inline void Update(Entity eID, float dt) {
        if (!Registry::Get().Has_Component<TransformComponent>(eID)) { return; }
        auto& transComp = Registry::Get().Get_Component<TransformComponent>(eID);
        Camera::GetInstance().FollowTarget(transComp.pos, dt);
    }

    inline void End(Entity eID) {
        (void)eID;
    }
}