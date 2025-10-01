/* ========================================================
 * @file    RenderSystem.cpp
 * @author  w.shiyuregine@digipen.edu
 * @date    01/10/2025
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

            renderInstance.DrawImageSub(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, animComp.u0, animComp.v0, animComp.u1, animComp.v1, transComp.rot);
        }
        else
        {
            TransformComponent& transComp = Get_TransformComponent(entID);
            renderInstance.DrawImageScaled(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, transComp.rot);
        }
    }

    // Loop through all ShapeComponents in the ECS
    for (auto& [entID, shapeComp] : CM_Shape().getComponents())
    {   
        // Skip entities without a transform (need position/rotation to draw)
        if (!Has_Component(entID, CT_Transform)) continue;
        TransformComponent& transComp = Get_TransformComponent(entID);
        Colour3 color = { shapeComp.color.x, shapeComp.color.y, shapeComp.color.z };

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
        auto& em = EntityManager::get();
        for (Entity e = 0; e < em.maxEntities(); ++e) {
            if (!em.isAlive(e)) continue;
            if (!Has_Component(e, CT_Transform)) continue;
            auto& t = Get_TransformComponent(e);

            // If velocity debug is enabled in ImGui and entity is a player with physics,
            // draw the velocity vector as a line/arrow
            if (ImGuiSystem::showVelocity && Has_Component(e, CT_Physics) && Has_Component(e, CT_Tag)) {
                auto& tag = Get_TagComponent(e);
                if (tag.tag == T_PLAYER) {
                    DebugDraw::DrawVelocity(e, t, renderInstance, dt);
                }
            }
            // If collider debug is enabled in ImGui, draw collision boundaries
            if (ImGuiSystem::showColliders && Has_Component(e, CT_Physics)) {
                auto& p = Get_PhysicsComponent(e);
                DebugDraw::DrawAABB(p.Get_AABB(), renderInstance);

                // Only draw circle collider debug for circle shapes
                if (Has_Component(e, CT_Shape)) {
                    auto& shape = Get_ShapeComponent(e);
                    if (shape.shape == SHAPE_CIRCLE) {
                        DebugDraw::DrawCircleCollider(p.Get_Circle(), renderInstance);
                    }
                }
                // Draw the hotspot (center point) of the transform
                DebugDraw::DrawHotspot(t.pos, renderInstance);
            }
        }

    }

}

void RenderSystem::Shutdown() {
    // clear all image cache here? seems like a good place, if we only store not a lot of images
    Render::GetInstance().ClearAllTextures();
}