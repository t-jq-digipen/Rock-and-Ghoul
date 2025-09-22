#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

class Transform2D {
public:
    // Transform properties
    glm::vec2 position{ 0.0f, 0.0f };
    float rotationRad{ 0.0f };
    glm::vec2 scale{ 1.0f, 1.0f };
    glm::vec2 pivot{ 0.0f, 0.0f };

    // Constructors
    Transform2D() = default;
    Transform2D(glm::vec2 pos);
    Transform2D(glm::vec2 pos, float rotation, glm::vec2 scale = { 1.0f, 1.0f });

    //setters
    void SetPosition(glm::vec2 pos);
    void SetRotationDeg(float degrees);
    void SetRotationRad(float radians);
    void SetScale(glm::vec2 scale);
    void SetScale(float uniformScale);
    void SetPivot(glm::vec2 pivot);

    // getters
    float GetRotationDeg() const;
    glm::vec2 GetPosition() const;
    glm::vec2 GetScale() const;
    glm::vec2 GetPivot() const;

    // Transform operations
    void Translate(glm::vec2 offset);
    void Rotate(float radians);
    void RotateDeg(float degrees);
    void ScaleBy(glm::vec2 factor);
    void ScaleBy(float factor);

    // Matrix generation
    glm::mat4 GetMatrix() const;
    glm::mat3 GetMatrix3() const;  // If 3x3 matrices
};