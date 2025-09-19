#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

using Entity = uint32_t;
static constexpr Entity kInvalidEntity = 0;

// -------------------------------
// Component: Transform2D
// -------------------------------
struct Transform2D {

    // Authoring data (local space)
    glm::vec2 position{ 0.0f, 0.0f };  // x, y translation
    float     rotationRad{ 0.0f };     // rotation about +Z in radians
    glm::vec2 scale{ 1.0f, 1.0f };     // non-uniform XY scale
    glm::vec2 pivot{ 0.0f, 0.0f };     // rotation/scale origin (e.g., sprite center)

    // Hierarchy
    Entity parent{ kInvalidEntity };
    std::vector<Entity> children;

    // Cached transforms (column-major, GLM style)
    glm::mat3 local{ 1.0f };           // local 2D affine
    glm::mat3 world{ 1.0f };           // world 2D affine
    bool dirty{ true };                 // mark true when TRS changes

    // Convenience setters (mark dirty)
    inline void SetPosition(glm::vec2 p) { position = p; dirty = true; }
    inline void SetRotationRad(float r) { rotationRad = r; dirty = true; }
    inline void SetRotationDeg(float d) { rotationRad = d * 0.017453292519943295f; dirty = true; }
    inline void SetScale(glm::vec2 s) { scale = s; dirty = true; }
    inline void SetPivot(glm::vec2 p) { pivot = p; dirty = true; }
};

// -------------------------------
// System: Transform2DSystem
// -------------------------------
class Transform2DSystem {
public:

    Transform2D& Create(Entity e);
    Transform2D* Get(Entity e);
    bool         Has(Entity e) const;

    void SetParent(Entity child, Entity parent);
    void Update();                                // recompute world matrices from roots

    // If your shader expects mat4, expand the 2D affine:
    static glm::mat4 ToMat4(const glm::mat3& m);

    static glm::mat4 BuildModelNoHierarchy(const Transform2D& t);

private:
    std::unordered_map<Entity, Transform2D> mStore;

    static glm::mat3 BuildLocal(const Transform2D& t); // T(pos)*T(pivot)*R*S*T(-pivot)
    void UpdateRecursive(Entity e);
};