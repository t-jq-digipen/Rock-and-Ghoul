#include "Transform.h"
#include <cmath> //idk if can use this or need use remath lib

// ----------------- public API -----------------
Transform2D& Transform2DSystem::Create(Entity e) {
    Transform2D& t = mStore[e]; // default-construct if missing
    t.dirty = true;
    return t;
}

Transform2D* Transform2DSystem::Get(Entity e) {
    auto it = mStore.find(e);
    return (it == mStore.end()) ? nullptr : &it->second;
}

bool Transform2DSystem::Has(Entity e) const {
    return mStore.find(e) != mStore.end();
}

void Transform2DSystem::SetParent(Entity child, Entity parent) {
    Transform2D& c = mStore[child];          // creates if missing
    if (c.parent == parent) return;

    // Remove from old parent's children (no <algorithm>)
    if (c.parent != kInvalidEntity) {
        auto& kids = mStore[c.parent].children;
        for (size_t i = 0; i < kids.size(); ++i) {
            if (kids[i] == child) { kids.erase(kids.begin() + static_cast<long>(i)); break; }
        }
    }

    c.parent = parent;
    if (parent != kInvalidEntity) {
        mStore[parent].children.push_back(child);
    }
    c.dirty = true;
}

void Transform2DSystem::Update() {
    // Update all roots; recursion handles subtrees
    for (auto& kv : mStore) {
        if (kv.second.parent == kInvalidEntity) {
            UpdateRecursive(kv.first);
        }
    }
}

glm::mat4 Transform2DSystem::ToMat4(const glm::mat3& m) {
    // Expand 2D affine (column-major) to 4x4 for typical sprite shaders
    glm::mat4 M(1.0f);
    M[0] = glm::vec4(m[0].x, m[0].y, 0.0f, 0.0f); // a, c, 0, 0
    M[1] = glm::vec4(m[1].x, m[1].y, 0.0f, 0.0f); // b, d, 0, 0
    M[2] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f); // z-axis unchanged
    M[3] = glm::vec4(m[2].x, m[2].y, 0.0f, 1.0f); // tx, ty, 0, 1
    return M;
}

glm::mat4 Transform2DSystem::BuildModelNoHierarchy(const Transform2D& t) {
    // OK to call BuildLocal here since we're inside the same class
    return ToMat4(BuildLocal(t));  // local == world when there’s no parent
}

// ----------------- private helpers -----------------
glm::mat3 Transform2DSystem::BuildLocal(const Transform2D& t) {
    const float c = std::cos(t.rotationRad);
    const float s = std::sin(t.rotationRad);

    // Rotation+Scale in column-major 2D affine
    // [ a b 0 ]
    // [ c d 0 ]  with columns (a,c,tx), (b,d,ty), (0,0,1)
    // [tx ty 1 ]
    const float a = c * t.scale.x;
    const float b = -s * t.scale.y;
    const float c2 = s * t.scale.x;
    const float d = c * t.scale.y;

    glm::mat3 RS(1.0f);
    RS[0] = glm::vec3(a, c2, 0.0f);
    RS[1] = glm::vec3(b, d, 0.0f);
    RS[2] = glm::vec3(0, 0, 1.0f);

    // Rotate/scale about pivot: T(pivot) * RS * T(-pivot)
    glm::mat3 Tp(1.0f); Tp[2] = glm::vec3(t.pivot.x, t.pivot.y, 1.0f);
    glm::mat3 Tm(1.0f); Tm[2] = glm::vec3(-t.pivot.x, -t.pivot.y, 1.0f);
    glm::mat3 L = Tp * RS * Tm;

    // Finally translate to position: T(position) * L
    glm::mat3 T(1.0f);  T[2] = glm::vec3(t.position.x, t.position.y, 1.0f);
    return T * L;
}

void Transform2DSystem::UpdateRecursive(Entity e) {
    Transform2D& t = mStore[e];

    if (t.dirty) {
        t.local = BuildLocal(t);
        if (t.parent == kInvalidEntity) t.world = t.local;
        else                             t.world = mStore[t.parent].world * t.local;
        t.dirty = false;
    }

    // Recurse children
    const auto& kids = t.children;
    for (size_t i = 0; i < kids.size(); ++i) {
        UpdateRecursive(kids[i]);
    }
}