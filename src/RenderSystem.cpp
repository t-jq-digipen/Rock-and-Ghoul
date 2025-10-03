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
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "RenderSystem.h"
#include "Render.h"
#include "Animator.h"

Render& renderInstance = Render::GetInstance();
std::vector<InstanceData> batch;

void RenderSystem::Initialize() {
    // ? 
}

void RenderSystem::Update(float dt) {

    for (auto& [entID, spriteComp] : CM_Sprite().getComponents())
    {
        if (spriteComp.IsAnimated())
        {
            // draw animated logic
            TransformComponent& transComp = Get_TransformComponent(entID);
            Animator::AnimatorComponent& animComp = Get_AnimatorComponent(entID);

            animComp.UpdateFrame(dt);

            renderInstance.DrawImageSub(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, animComp.u0, animComp.v0, animComp.u1, animComp.v1, transComp.rot + spriteComp.Get_Rotation());
        }
        else
        {
            TransformComponent& transComp = Get_TransformComponent(entID);
            renderInstance.DrawImageScaled(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, transComp.rot + spriteComp.Get_Rotation());
        }
    }

    // Loop through all ShapeComponents in the ECS
    for (auto& [entID, shapeComp] : CM_Shape().getComponents())
    {   
        // Skip entities without a transform (need position/rotation to draw)
        if (!Has_Component(entID, CT_Transform)) continue;
        TransformComponent& transComp = Get_TransformComponent(entID);
        Colour3 color = { shapeComp.color.x, shapeComp.color.y, shapeComp.color.z };
        if (Has_Component(entID, CT_Tag)) {
            TagComponent& tag = Get_TagComponent(entID);

            if (shapeComp.shape == SHAPE_RECTANGLE && tag.tag == T_INSTANCING) {
                InstanceData inst;
                inst.position = transComp.pos;
                inst.scale = transComp.scale;
                inst.rotation = transComp.rot;
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
                32,
                transComp.rot
            );
        }

        // ------------------------------------------------
        // Debug drawing section (Velocity, AABBs, Colliders)
        // ------------------------------------------------
        if (ImGuiSystem::showColliders || ImGuiSystem::showVelocity) {
            for (auto& [entID, physics] : CM_Physics().getComponents()) {
                if (!Has_Component(entID, CT_Transform)) continue;
                auto& t = Get_TransformComponent(entID);

                // Velocity debug (only for players)
                if (ImGuiSystem::showVelocity && Has_Component(entID, CT_Tag)) {
                    auto& tag = Get_TagComponent(entID);
                    if (tag.tag == T_PLAYER) {
                        DebugDraw::DrawVelocity(entID, t, renderInstance, dt);
                    }
                }

                // Collider debug
                if (ImGuiSystem::showColliders) {
                    DebugDraw::DrawAABB(physics.Get_AABB(), renderInstance);

                    if (Has_Component(entID, CT_Shape)) {
                        auto& shape = Get_ShapeComponent(entID);
                        if (shape.shape == SHAPE_CIRCLE) {
                            DebugDraw::DrawCircleCollider(physics.Get_Circle(), renderInstance);
                        }
                    }

                    DebugDraw::DrawHotspot(t.pos, renderInstance);
                }
            }
        }

    }

    if (!batch.empty()) {
        renderInstance.DrawRectInstanced(batch, { 0.2f, 0.8f, 1.0f });
        batch.clear();
    }
    
}

void RenderSystem::Shutdown() {
    // clear all image cache here? seems like a good place, if we only store not a lot of images
    Render::GetInstance().ClearAllTextures();
}