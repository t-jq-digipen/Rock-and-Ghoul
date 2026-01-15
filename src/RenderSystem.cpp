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
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "RenderSystem.h"
#include "Render.h"
#include "Animator.h"
#include <Camera.h>
#include "LogicSystem.h"
#include "CollisionManager.h"
#include "CoreEngine.h"
#include "RenderComponent.h"
#include "SceneManager.h"
#include "SimpleAtlas.h"
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
static std::vector<std::string> s_backgroundTexturePaths;

// Pixels to units conversion, assigned values on initialization of rendersystem
float RenderSystem::pixelsPerUnit;
float RenderSystem::unitsPerPixel;

void RenderSystem::AddBackgroundTexture(GLuint tex, const std::string& path) {
    s_backgroundTextures.push_back(tex);
    s_backgroundTexturePaths.push_back(path);
}

std::vector<std::string> RenderSystem::GetBackgroundTexturePaths() {
    return s_backgroundTexturePaths;
}

void RenderSystem::ClearBackgroundTextures() {
    s_backgroundTextures.clear();
    s_backgroundTexturePaths.clear();
}

//extern Camera mainCamera;
//Camera& mainCamera = Camera::GetInstance();

// NEW RENDERCOMPONENT draw overloads
void DrawItem(const Sprite::SpriteComponent& spriteComp, const TransformComponent& transComp) {
    const Image& img = spriteComp.Get_Image();

    Vec2 worldSize = { img.widthPixels * RenderSystem::unitsPerPixel, img.heightPixels * RenderSystem::unitsPerPixel };

    Vec2 scale = worldSize * spriteComp.Get_Scale() * transComp.scale;
    //Vec2 scaledSize = { img.widthPixels * spriteComp.Get_Scale().x, img.heightPixels * spriteComp.Get_Scale().y };

    glm::vec4 uv = { 0.f, 1.f, 1.f, 0.f };

    if (spriteComp.GetFlipX()) { std::swap(uv.x, uv.z); }
    if (spriteComp.GetFlipY()) { std::swap(uv.y, uv.w); }

    renderInstance.SubmitSpriteBatchUV(
        img,
        transComp.pos,
        scale,
        uv,
        transComp.rot + spriteComp.Get_Rotation()
    );

    //renderInstance.SubmitSpriteBatch(
    //    img,
    //    transComp.pos,
    //    scale,
    //    transComp.rot + spriteComp.Get_Rotation()
    //);
}

void DrawItem(const Sprite::SpriteSheet& spriteSheet, const TransformComponent& transComp) {
    const Image& img = spriteSheet.Sprite().Get_Image();

    Vec2 frameSize = { spriteSheet.u1 - spriteSheet.u0, spriteSheet.v1 - spriteSheet.v0 };
    Vec2 worldSize = frameSize * RenderSystem::unitsPerPixel;

    Vec2 scale = worldSize * spriteSheet.Sprite().Get_Scale() * transComp.scale;

    glm::vec4 uv(spriteSheet.u0 / img.widthPixels,
        spriteSheet.v1 / img.heightPixels,
        spriteSheet.u1 / img.widthPixels,
        spriteSheet.v0 / img.heightPixels
    );

    if (spriteSheet.Sprite().GetFlipX()) { std::swap(uv.x, uv.z); }
    if (spriteSheet.Sprite().GetFlipY()) { std::swap(uv.y, uv.w); }

    renderInstance.SubmitSpriteBatchUV(
        img,
        transComp.pos,
        scale,
        uv,
        transComp.rot + spriteSheet.Sprite().Get_Rotation()
    );

}

void DrawItem(const ShapeComponent& shapeComp, const TransformComponent& transComp) {
    Colour3 color = { shapeComp.color.x, shapeComp.color.y, shapeComp.color.z };
    switch (shapeComp.shape)
    {
    case SHAPE_RECTANGLE:
    {
        renderInstance.DrawRect(
            transComp.pos,
            shapeComp.dimensions.x * transComp.scale.x,
            shapeComp.dimensions.y * transComp.scale.y,
            color,
            transComp.rot
        );
    }
    break; // end SHAPE_RECTANGLE

    case SHAPE_CIRCLE:
    {
        float radius = (shapeComp.dimensions.x + shapeComp.dimensions.y) / 4.0f;
        renderInstance.DrawCircle(
            transComp.pos,
            radius * transComp.scale.x,
            color,
            32
        );
    }
    break; // end SHAPE_CIRCLE

    default:
        break;
    }
}
// NEW RENDERCOMPONENT draw overloads end

void RenderSystem::Initialize() {
    // ? 
    std::cout << "[RenderSystem] Initializing..." << std::endl;

    Render& renderer = Render::GetInstance();
    renderer.Init();        // quad VAO/VBO + sprite shader
    //renderer.InitBatch();
    // Initialize your offscreen framebuffer for Game View
    renderer.InitSceneFramebuffer(GLApp::width, GLApp::height);

    std::cout << "[RenderSystem] Scene framebuffer created ("
        << GLApp::width << "x" << GLApp::height << ")\n";

    BehaviourFCT camFollow;
    camFollow.Init = [](Entity e) { CamFollowScript::Start(e); };
    camFollow.Update = [](Entity e, float dt) { CamFollowScript::Update(e, dt); };
    camFollow.End = [](Entity e) {CamFollowScript::End(e); };
    LogicSystem::Get().RegisterScript("CamTarget", camFollow, ScriptID::CAMERA_TRACK);

    pixelsPerUnit = 100.f;
    unitsPerPixel = 1.f / pixelsPerUnit;

    //atlas testing below 
    /*SimpleAtlas atlas;
    SimpleAtlasBuilder builder;

    builder.addImage("rock", "../Assets/Rock_2048px.png");
    builder.addImage("rock_fight", "../Assets/Rock_FightingPose_2048px.png");
    builder.addImage("ghoul", "../Assets/Ghoul_2048px.png");
    builder.build(atlas, 4000);

    std::cout << "\n=== ATLAS BUILD RESULT ===\n";
    std::cout << "Atlas Texture ID: " << atlas.textureID << "\n";
    std::cout << "Atlas Size: " << atlas.width << " x " << atlas.height << "\n";

    for (auto& [key, sp] : atlas.sprites)
    {
        std::cout << "Sprite: " << key << "\n";
        std::cout << "  UV: (" << sp.u0 << "," << sp.v0
            << ") -> (" << sp.u1 << "," << sp.v1 << ")\n";
        std::cout << "  Size: " << sp.width << "x" << sp.height << "\n";
    }*/

}

void RenderSystem::Update(float dt) {
    
    //Camera::GetInstance().UpdateMatrices();

    Camera::GetInstance().UpdateCamera();

    Render& renderer = Render::GetInstance();
    
    bool camcheck = renderer.useCamera;

#ifndef NO_IMGUI
    if (ImGuiSystem::guiEnabled) {
        renderer.BeginSceneCapture(); 
        renderer.ClearBackground({ 0.1f, 0.1f, 0.1f, 1.0f });
    }
    else 
#endif
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, GLApp::width, GLApp::height);
        //renderer.ApplyLetterboxViewport();
        renderer.ClearBackground({ 0.0f, 0.0f, 0.0f, 1.0f }); // clear main framebuffer
    }

    static bool wasPaused = false;

    // New pause logic driven by SceneManager
    SceneManager& sm = SceneManager::Get();

    // Simulation is paused whenever we are NOT in play mode
    bool simPaused = !sm.IsPlaying();

    if (simPaused)
    {
        wasPaused = true;
    }
    else if (wasPaused)
    {
        // Just unpaused → reset velocity lines so they snap correctly
        DebugDraw::ClearVelocityCache();
        wasPaused = false;
    }

    Render::GetInstance().UpdateProjection();
   // renderer.ClearBackground({ 0.05f, 0.0f, 0.1f, 1.0f });
    // ===== DRAW BACKGROUND IF SET =====

    /*for (GLuint tex : s_backgroundTextures)
        renderer.DrawFullscreenTexture(tex, true);*/

        // ===== DRAW BACKGROUND IF SET =====

    // figure out current game state 
   // GameState currState = GSM_CurrentState();  


    //if (SceneManager::Get().GetCurrentSceneName() == "MainMenu")
    //{
    //    // Letterboxed menu background
    //    renderer.ClearBackground({ 0.0f, 0.0f, 0.0f, 1.0f });

    //    for (GLuint tex : s_backgroundTextures)
    //    {
    //        renderer.DrawLetterboxedTexture(tex);
    //    }
    //}
    //else
    //{
    //    // Gameplay or editor: fullscreen world background
    //    for (GLuint tex : s_backgroundTextures)
    //    {
    //        renderer.DrawFullscreenTexture(tex, true);
    //    }
    //}

    if (SceneManager::Get().GetSceneType() == ST_MENU)
    {
        // Letterboxed menu background
        renderer.ClearBackground({ 0.0f, 0.0f, 0.0f, 1.0f });

        for (GLuint tex : s_backgroundTextures)
        {
            renderer.DrawLetterboxedTexture(tex);
        }
    }
    else // ST_GAME
    {
        // Gameplay or editor: fullscreen world background
        for (GLuint tex : s_backgroundTextures)
        {
            renderer.DrawFullscreenTexture(tex, true);
        }
    }

   

    //if (!SceneManager::Get().IsPlaying()) {
    //    // main menu / editor mode
    //    for (GLuint tex : s_backgroundTextures)
    //        renderer.DrawLetterboxedTexture(tex); // OR letterbox version
    //}
    //else {
    //    // gameplay
    //    for (GLuint tex : s_backgroundTextures)
    //        renderer.DrawLetterboxedTexture(tex);
    //}

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
    // renderInstance.BeginBatch();

    //struct RenderItem { //iidentify sprite or shape
    //    Entity id;
    //    bool isSprite;
    //};

    ////containers that contain entities for each layer
    //std::vector<RenderItem> backgroundEntities;
    //std::vector<RenderItem> gameEntities;
    //std::vector<RenderItem> foregroundEntities; //for sepcial effects? particles?
    //std::vector<RenderItem> uiEntities;


    ////Categorize entities by layer
    //for (auto& [entID, spriteComp] : m_registry.Get_ComponentMap<Sprite::SpriteComponent>())
    //{
    //    RenderLayer layer = LAYER_GAMEOBJECT; // default, if not specified 

    //    if (m_registry.Has_Component<RenderLayerComponent>(entID)) {
    //        layer = m_registry.Get_Component<RenderLayerComponent>(entID).layer;
    //    }

    //    RenderItem item{ entID, true };

    //    switch (layer)
    //    {
    //    case LAYER_BACKGROUND:
    //        backgroundEntities.push_back(item);
    //        break;
    //    case LAYER_GAMEOBJECT:
    //        gameEntities.push_back(item);
    //        break;
    //    case LAYER_FOREGROUND:
    //        foregroundEntities.push_back(item);
    //        break;
    //    case LAYER_UI:
    //        uiEntities.push_back(item);
    //        break;
    //    }
    //}

    ////Handle the shape components
    //for (auto& [entID, shapeComp] : m_registry.Get_ComponentMap<ShapeComponent>()) {

    //    RenderLayer layer = LAYER_GAMEOBJECT; //default give them layer_gameobject, if want change need explicitly add layer comp to entitycreation.cpp

    //    if (m_registry.Has_Component<RenderLayerComponent>(entID)) {
    //        layer = m_registry.Get_Component<RenderLayerComponent>(entID).layer;
    //    }

    //    RenderItem item{ entID, false };

    //    switch (layer)
    //    {
    //    case LAYER_BACKGROUND:
    //        backgroundEntities.push_back(item);
    //        break;
    //    case LAYER_GAMEOBJECT:
    //        gameEntities.push_back(item);
    //        break;
    //    case LAYER_FOREGROUND:
    //        foregroundEntities.push_back(item);
    //        break;
    //    case LAYER_UI:
    //        uiEntities.push_back(item);
    //        break;
    //    }
    //}

    // new rendercomp
#pragma region new rendercomp

    std::vector<std::pair<Entity,RenderComponent>> bg_list;
    std::vector<std::pair<Entity,RenderComponent>> go_list;
    std::vector<std::pair<Entity,RenderComponent>> fg_list; //for sepcial effects? particles?
    std::vector<std::pair<Entity,RenderComponent>> ui_list;

    // NEW RENDERCOMPONENT
    for (auto& [entID, renderComp] : m_registry.Get_ComponentMap<RenderComponent>()) {
        renderComp.UpdateData(dt);

        switch (renderComp.GetRenderLayer())
        {
        case LAYER_BACKGROUND:
            bg_list.push_back({ entID,renderComp });
            break;
        case LAYER_GAMEOBJECT:
            go_list.push_back({ entID,renderComp });
            break;
        case LAYER_FOREGROUND:
            fg_list.push_back({ entID,renderComp });
            break;
        case LAYER_UI:
            ui_list.push_back({ entID,renderComp });
            break;
        }
    }

    //Sort the entities with GAMEOBJECT layer by Y-position
    //std::sort(gameEntities.begin(), gameEntities.end(),
    //    [&](const RenderItem& a, const RenderItem& b) {
    //        const auto& ta = m_registry.Get_Component<TransformComponent>(a.id);
    //        const auto& tb = m_registry.Get_Component<TransformComponent>(b.id);
    //        return ta.pos.y > tb.pos.y; // Higher Y = further back (drawn first)
    //    });

    // SORT ENTITIES FOR NEW RENDERCOMPONENT
    std::sort(go_list.begin(), go_list.end(),
        [&](const std::pair<Entity, RenderComponent>& a, const std::pair<Entity, RenderComponent>& b) {
            const auto& ta = m_registry.Get_Component<TransformComponent>(a.first);
            const auto& tb = m_registry.Get_Component<TransformComponent>(b.first);
            return ta.pos.y > tb.pos.y; // Higher Y = further back (drawn first)
        });

    auto drawLayer = [&](const std::vector<std::pair<Entity, RenderComponent>>& renderList)
        {
            for (const auto& [entID, renderComp] : renderList)
            {
                if (!m_registry.Has_Component<TransformComponent>(entID)) { continue; }
                TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);

                // update rendercomponent
                switch (renderComp.GetRenderType())
                {
                case RT_SHAPE:
                {
                    DrawItem(renderComp.GetShape(), transComp);
                }
                break; // end RT_SHAPE

                case RT_IMAGE:
                {
                    DrawItem(renderComp.GetSprite(), transComp);
                }
                break; // end RT_IMAGE

                case RT_SPRITESHEET:
                {
                    DrawItem(renderComp.GetSpriteSheet(), transComp);
                }
                break; // end RT_SPRITESHEET

                default:
                    break;
                }

                // draw de thing
                /*std::visit([&](auto& renderdata)
                    { DrawItem(renderdata, transComp); }, renderComp.m_renderData);*/
            }
        };
#pragma endregion new rendercomp

    //Draw a list of entities
    //auto DrawLayer = [&](const std::vector<RenderItem>& entities)
    //    {
    //        for (const RenderItem& item : entities)
    //        {
    //            Entity entID = item.id;

    //            if (item.isSprite) { //if is sprite..

    //                auto& spriteComp = m_registry.Get_Component<Sprite::SpriteComponent>(entID);
    //                auto& transComp = m_registry.Get_Component<TransformComponent>(entID);

    //                spriteComp.Update_ImageSize();

    //                bool isBatched = false;
    //                if (m_registry.Has_Component<TagComponent>(entID))
    //                {
    //                    const auto& tag = m_registry.Get_Component<TagComponent>(entID);
    //                    if (tag.tag == T_BATCH) {
    //                        isBatched = true;
    //                    }
    //                }

    //                if (spriteComp.IsAnimated())
    //                {
    //                    auto& animComp = m_registry.Get_Component<Animator::AnimatorComponent>(entID);
    //                    animComp.UpdateFrame(dt);

    //                    Image img = spriteComp.Get_Image();
    //                    glm::vec4 uv(animComp.u0 / img.widthPixels,
    //                        animComp.v1 / img.heightPixels,
    //                        animComp.u1 / img.widthPixels,
    //                        animComp.v0 / img.heightPixels);

    //                    if (isBatched) {
    //                        renderInstance.SubmitSpriteBatchUV(img, transComp.pos, spriteComp.Get_Scale(), uv, transComp.rot + spriteComp.Get_Rotation());
    //                    }
    //                    else {
    //                        renderInstance.DrawImageSub(img,
    //                            transComp.pos.x, transComp.pos.y,
    //                            spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
    //                            animComp.u0, animComp.v0, animComp.u1, animComp.v1,
    //                            glm::radians(transComp.rot + spriteComp.Get_Rotation()));
    //                    }
    //                }
    //                else
    //                {
    //                    Image img = spriteComp.Get_Image();
    //                    Vec2 scaledSize = { img.widthPixels * spriteComp.Get_Scale().x, img.heightPixels * spriteComp.Get_Scale().y };

    //                    if (isBatched) {
    //                        renderInstance.SubmitSpriteBatch(img, transComp.pos, scaledSize, transComp.rot + spriteComp.Get_Rotation());
    //                    }
    //                    else {
    //                        renderInstance.DrawImageScaled(img,
    //                            transComp.pos.x, transComp.pos.y,
    //                            spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
    //                            glm::radians(transComp.rot + spriteComp.Get_Rotation()));
    //                    }
    //                }
    //            }

    //            //shape stuff here (if is shape...)
    //            else {

    //                // Skip entities without a transform (need position/rotation to draw)
    //                if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
    //                TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
    //                ShapeComponent& shapeComp = m_registry.Get_Component<ShapeComponent>(entID);
    //                Colour3 color = { shapeComp.color.x, shapeComp.color.y, shapeComp.color.z };

    //                if (m_registry.Has_Component<TagComponent>(entID)) {
    //                    TagComponent& tag = m_registry.Get_Component<TagComponent>(entID);

    //                    if (shapeComp.shape == SHAPE_RECTANGLE && tag.tag == T_INSTANCING) { //check entity is using instancing
    //                        //Store matching entities inside InstanceData vector
    //                        InstanceData inst;
    //                        inst.position = transComp.pos;
    //                        inst.scale = transComp.scale;
    //                        inst.rotation = glm::radians(transComp.rot);
    //                        inst.uvRect = { 0,0,1,1 };
    //                        batch.push_back(inst);
    //                        continue;
    //                    }


    //                }

    //                if (shapeComp.shape == SHAPE_RECTANGLE) {
    //                    renderInstance.DrawRect(
    //                        transComp.pos,
    //                        shapeComp.dimensions.x,
    //                        shapeComp.dimensions.y,
    //                        color,
    //                        transComp.rot
    //                    );
    //                }
    //                else if (shapeComp.shape == SHAPE_CIRCLE) {
    //                    float radius = (shapeComp.dimensions.x + shapeComp.dimensions.y) / 4.0f;
    //                    renderInstance.DrawCircle(
    //                        transComp.pos,
    //                        radius,
    //                        color,
    //                        32
    //                    );
    //                }
    //            }
    //        }
    //    };

    // Background (rendered behind game objects)
    renderInstance.BeginBatch();
    // DrawLayer(backgroundEntities);
    drawLayer(bg_list);
    renderInstance.EndBatch();
    renderInstance.FlushBatch();

    // Game objects
    renderInstance.BeginBatch();
    // DrawLayer(gameEntities);
    drawLayer(go_list);
    renderInstance.EndBatch();
    renderInstance.FlushBatch();

    // Foreground (rendered above game objects)
    renderInstance.BeginBatch();
    // DrawLayer(foregroundEntities);
    drawLayer(fg_list);
    renderInstance.EndBatch();
    renderInstance.FlushBatch();

    // UI (rendered last, most in front)

    renderInstance.SetProjectionType(ProjectionType::UI);
    //renderer.useCamera = false;
    renderInstance.BeginBatch();
    // DrawLayer(uiEntities);
    drawLayer(ui_list);
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


#ifndef NO_IMGUI
    renderInstance.SetProjectionType(ProjectionType::WORLD);

    // Make absolutely sure we're rendering to the scene FBO

    //// ===== TEST: Draw using DrawRect instead of DrawLine =====
    //{
    //    // Try drawing a rectangle which we KNOW works
    //    Vec2 testPos{ 960.f, 540.f };  // Center of screen
    //    Colour3 brightRed{ 1.f, 0.f, 0.f };

    //    renderInstance.DrawRect(testPos, 200.f, 200.f, brightRed);
    //    std::cout << "[RenderSystem] Drew test RECTANGLE at center (960,540)\n";
    //}

    //// Now try the line
    //{
    //    Vec2 testStart{ 0.f, 540.f };
    //    Vec2 testEnd{ 1920.f, 540.f };
    //    Colour3 brightMagenta{ 1.f, 0.f, 1.f };

    //    glLineWidth(10.0f);
    //    renderInstance.DrawLine(testStart, testEnd, brightMagenta);
    //    glLineWidth(1.0f);

    //    std::cout << "[RenderSystem] Drew test LINE\n";
    //}
    //{
    //    Vec2 dummyPos{ -9999.f, -9999.f };  // Way off-screen
    //    Colour3 transparent{ 0.f, 0.f, 0.f };
    //    renderInstance.DrawRect(dummyPos, 1.f, 1.f, transparent, 0.0f, 0.0f); // Fully transparent
    //}
    // Debug drawing for spatial grid
    {
        SpatialGrid* grid = CollisionManager::GetSpatialGrid();
        if (ImGuiSystem::showSpatialGrid) {
            if (grid) {
                DebugDraw::DrawSpatialGrid(grid, renderInstance);
            }
        }
    }

    // Debug drawing for colliders
    if (ImGuiSystem::showColliders) {
        for (auto& [entID, collider] : m_registry.Get_ComponentMap<ColliderComponent>()) {
            if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
            auto& t = m_registry.Get_Component<TransformComponent>(entID);
            std::cout << "[RenderSystem] Drawing AABB for entity " << entID << "\n";
            DebugDraw::DrawAABB(collider.Get_AABB(), renderInstance);
            DebugDraw::DrawHotspot(t.pos, renderInstance);
        }
    }

    // Debug drawing for velocities
    if (ImGuiSystem::showVelocity) {
        for (auto& [entID, physics] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>()) {
            if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
            auto& t = m_registry.Get_Component<TransformComponent>(entID);

            DebugDraw::DrawVelocity(entID, t, renderInstance, dt);
        }
    }

    if (!batch.empty()) {
        //then draw them all at once
        renderInstance.DrawRectInstanced(batch, { 0.2f, 0.8f, 1.0f });
        batch.clear();
    }
    if (ImGuiSystem::guiEnabled) {
        // renderer.EndSceneCapture();
    }
#endif
    //renderer.EndSceneCapture();
    //renderer.PresentScene();
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