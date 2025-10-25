/* ========================================================
 * @file        RenderSystem.cpp
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @date        01 OCT 2025
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

Render& renderInstance = Render::GetInstance();
std::vector<InstanceData> batch; //added for instancing

void RenderSystem::Initialize() {
    // ? 
}

void RenderSystem::Update(float dt) {

    for (auto& [entID, spriteComp] : m_registry.Get_ComponentMap<Sprite::SpriteComponent>())
    {
        if (spriteComp.IsAnimated())
        {
            // draw animated logic
            TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
            Animator::AnimatorComponent& animComp = m_registry.Get_Component<Animator::AnimatorComponent>(entID);

            animComp.UpdateFrame(dt);

            renderInstance.DrawImageSub(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, animComp.u0, animComp.v0, animComp.u1, animComp.v1, transComp.rot + spriteComp.Get_Rotation());
        }
        else
        {
           // TransformComponent& transComp = Get_TransformComponent(entID);
            if (Input::IsKeyHeld(GLFW_KEY_SPACE))
                spriteComp.Set_Tint({ 1.0f, 0.0f, 0.0f });
            else
                spriteComp.Set_Tint({ 1.0f, 1.0f, 1.0f });
            renderInstance.SetTint(spriteComp.Get_Tint());
            TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
            renderInstance.DrawImageScaled(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, transComp.rot + spriteComp.Get_Rotation());
            renderInstance.ClearTint();

        }
    }

    // Loop through all ShapeComponents in the ECS
    for (auto& [entID, shapeComp] : m_registry.Get_ComponentMap<ShapeComponent>())
    {   
        // Skip entities without a transform (need position/rotation to draw)
        if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
        TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
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

    // Debug drawing for physics colliders / velocities
    if (ImGuiSystem::showColliders || ImGuiSystem::showVelocity) {
        for (auto& [entID, physics] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>()) {
            if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
            auto& t = m_registry.Get_Component<TransformComponent>(entID);

            if (ImGuiSystem::showVelocity && m_registry.Has_Component<TagComponent>(entID)) {
                auto& tag = m_registry.Get_Component<TagComponent>(entID);
                if (tag.tag == T_PLAYER)
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
    
}

void RenderSystem::Shutdown() {
    // clear all image cache here? seems like a good place, if we only store not a lot of images
    Render::GetInstance().ClearAllTextures();
}