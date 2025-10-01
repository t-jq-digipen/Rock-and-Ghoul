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


    for (auto& [entID, shapeComp] : CM_Shape().getComponents())
    {
        // shape drawing tings (REGINE PUT YO SHIT HERE)
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
        auto& em = EntityManager::get();
        for (Entity e = 0; e < em.maxEntities(); ++e) {
            if (!em.isAlive(e)) continue;
            if (!Has_Component(e, CT_Transform)) continue;
            auto& t = Get_TransformComponent(e);

            if (ImGuiSystem::showVelocity && Has_Component(e, CT_Physics) && Has_Component(e, CT_Tag)) {
                auto& tag = Get_TagComponent(e);
                if (tag.tag == T_PLAYER) {
                    DebugDraw::DrawVelocity(e, t, renderInstance, dt);
                }
            }
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

                DebugDraw::DrawHotspot(t.pos, renderInstance);
            }
        }

    }

    //auto& em = EntityManager::get();
    //static bool showDebug = false;
    //if (Input::IsKeyPressed(GLFW_KEY_5)) {
    //    showDebug = !showDebug;
    //}
    //for (Entity e = 0; e < em.maxEntities(); ++e) {
    //    if (!em.isAlive(e)) continue;
    //    if (!Has_Component(e, CT_Transform) || !Has_Component(e, CT_Sprite))
    //        continue;

    //    auto& t = Get_TransformComponent(e);
    //    auto& s = Get_SpriteComponent(e);

    //    if (s.textureID == 0) {
    //        // Render as colored rectangle (no texture)
    //        Colour3 color = { s.color.x, s.color.y, s.color.z };
    //        renderer.DrawRect(
    //            t.pos,
    //            s.size.x,
    //            s.size.y,
    //            color, t.rot
    //        );
    //    }
    //    else if (s.textureID == 1) {
    //        // Render as colored circle
    //        Colour3 color = { s.color.x, s.color.y, s.color.z };
    //        float radius = (s.size.x + s.size.y) / 4.0f; // Average of width and height, divided by 2
    //        renderer.DrawCircle(
    //            { t.pos.x, t.pos.y },
    //            radius,
    //            color, 32, t.rot
    //        );
    //    }
    //    else {
    //        // Render as textured quad
    //        renderer.DrawTexturedQuad(
    //            t.pos,
    //            s.size,
    //            s.textureID,
    //            t.rot
    //        );
    //    }

    //    if (ImGuiSystem::showVelocity && Has_Component(e, CT_Physics) && Has_Component(e, CT_Tag)) {
    //        auto& tag = Get_TagComponent(e);
    //        if (tag.tag == T_PLAYER) {
    //            DebugDraw::DrawVelocity(e, t, renderer, dt);
    //        }
    //    }
    //    // Debug drawing (AABB + hotspot)

    //    if (ImGuiSystem::showColliders && Has_Component(e, CT_Physics)) {
    //        auto& p = Get_PhysicsComponent(e);

    //        DebugDraw::DrawAABB(p.Get_AABB(), renderer);
    //        DebugDraw::DrawHotspot(t.pos, renderer);
    //    }

    //}

}

void RenderSystem::Shutdown() {
    // clear all image cache here? seems like a good place, if we only store not a lot of images
    Render::GetInstance().ClearAllTextures();
}