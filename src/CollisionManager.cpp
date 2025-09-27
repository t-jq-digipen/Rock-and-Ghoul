#include "CollisionManager.h"

void CollisionManager::Clear() {
    events.clear();
}

void CollisionManager::AddEvent(Entity a, Entity b) {
    events.push_back({ a, b });
}

void CollisionManager::ProcessEvents() {
    //loops through each stored collision event from this frame.
    for (auto& e : events) {
        //if both entities have a tag
        //tags tell us what kind of entity
        if (Has_Component(e.a, CT_Tag) && Has_Component(e.b, CT_Tag)) {
            auto& tagA = Get_TagComponent(e.a);
            auto& tagB = Get_TagComponent(e.b);

            std::cout << "Collision detected between " << TagToString(tagA.tag) << " (Entity " << e.a << ")" << " and "  << TagToString(tagB.tag) << " (Entity " << e.b << ")" << std::endl;
        }
        else {
            std::cout << "Collision detected between Entity " << e.a << " and Entity " << e.b<< " (no tags)" << std::endl;
        }
    }
    Clear();
}

void CollisionManager::CheckCollisions() {
    // check every pair of entities with PhysicsComponent
    auto& physicsComps = CM_Physics().getComponents();

    //loop over each entity with a PhysicsComponent
    for (auto itA = physicsComps.begin(); itA != physicsComps.end(); ++itA) {
        // a = entity ID, compA = PhysicsComponent
        Entity a = itA->first;
        Physics::PhysicsComp& compA = itA->second;

        // skip if not active or entity has no Transform
        if (!compA.CollisionActive() || !Has_Component(a, CT_Transform)) continue;

        // get transform of entity A
        auto& transA = Get_TransformComponent(a);

        // build AABB for entity A using transform pos + size
        // OLD REGCODE
        /*AABB boxA{
            { transA.pos.x - compA.width * 0.5f, transA.pos.y - compA.height * 0.5f },
            { transA.pos.x + compA.width * 0.5f, transA.pos.y + compA.height * 0.5f }
        };*/
        // NEW JQCODE
        compA.UpdateHitboxes(transA.pos);

        //check collisions with all entities B
        for (auto itB = std::next(itA); itB != physicsComps.end(); ++itB) {
            Entity b = itB->first;
            Physics::PhysicsComp& compB = itB->second;

            // skip if not active or no Transform
            if (!compB.CollisionActive() || !Has_Component(b, CT_Transform)) continue;

            auto& transB = Get_TransformComponent(b);

            // OLD REGCODE
            /*AABB boxB{
                { transB.pos.x - compB.width * 0.5f, transB.pos.y - compB.height * 0.5f },
                { transB.pos.x + compB.width * 0.5f, transB.pos.y + compB.height * 0.5f }
            };*/
            // NEW JQCODE
            compB.UpdateHitboxes(transB.pos);

            // AABB vs AABB
            if (CollisionIntersection_RectRect_Static(compA.Get_AABB(), compB.Get_AABB())) {
                // store event for processing later
                AddEvent(a, b);
            }
        }
    }
}

