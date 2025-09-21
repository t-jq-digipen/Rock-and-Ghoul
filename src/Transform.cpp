#include "Transform.h"
#include <cmath> //idk if can use this or need use remath lib

// Constants (can change when math library done?)
const float DEG_TO_RAD = 0.017453292519943295f;
const float RAD_TO_DEG = 57.29577951308232f;

// Constructors
Transform2D::Transform2D(glm::vec2 pos) : position(pos) {}

Transform2D::Transform2D(glm::vec2 pos, float rotation, glm::vec2 scl)
    : position(pos), rotationRad(rotation), scale(scl) {
}

// Setters
void Transform2D::SetPosition(glm::vec2 pos) {
    position = pos;
}

void Transform2D::SetRotationDeg(float degrees) {
    rotationRad = degrees * DEG_TO_RAD;
}

void Transform2D::SetRotationRad(float radians) {
    rotationRad = radians;
}

void Transform2D::SetScale(glm::vec2 scl) {
    scale = scl;
}

void Transform2D::SetScale(float uniformScale) {
    scale = glm::vec2(uniformScale, uniformScale);
}

void Transform2D::SetPivot(glm::vec2 piv) {
    pivot = piv;
}

// Getters
float Transform2D::GetRotationDeg() const {
    return rotationRad * RAD_TO_DEG;
}

glm::vec2 Transform2D::GetPosition() const {
    return position;
}

glm::vec2 Transform2D::GetScale() const {
    return scale;
}

glm::vec2 Transform2D::GetPivot() const {
    return pivot;
}

// Transform operations
void Transform2D::Translate(glm::vec2 offset) {
    position += offset;
}

void Transform2D::Rotate(float radians) {
    rotationRad += radians;
}

void Transform2D::RotateDeg(float degrees) {
    rotationRad += degrees * DEG_TO_RAD;
}

void Transform2D::ScaleBy(glm::vec2 factor) {
    scale *= factor;
}

void Transform2D::ScaleBy(float factor) {
    scale *= factor;
}

// Matrix generation
glm::mat4 Transform2D::GetMatrix() const {
    const float c = std::cos(rotationRad);
    const float s = std::sin(rotationRad);

    // Build 2D transformation matrix components
    const float a = c * scale.x;  // rotation + scale X
    const float b = -s * scale.y; // rotation + scale Y  
    const float c2 = s * scale.x; // rotation + scale X
    const float d = c * scale.y;  // rotation + scale Y

    // Calculate final translation including pivot offset
    // This applies: T(position) * T(pivot) * R * S * T(-pivot)
    const float tx = position.x + pivot.x * (1.0f - a) - pivot.y * b;
    const float ty = position.y - pivot.x * c2 + pivot.y * (1.0f - d);

    // Create 4x4 matrix (column-major order follow da OpenGL)
    glm::mat4 matrix(1.0f);
    matrix[0] = glm::vec4(a, c2, 0.0f, 0.0f); // first column
    matrix[1] = glm::vec4(b, d, 0.0f, 0.0f); // second column  
    matrix[2] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f); // third column (z-axis)
    matrix[3] = glm::vec4(tx, ty, 0.0f, 1.0f); // fourth column (translation + w)

    return matrix;
}

glm::mat3 Transform2D::GetMatrix3() const {
    const float c = std::cos(rotationRad);
    const float s = std::sin(rotationRad);

    // Build 2D transformation matrix components
    const float a = c * scale.x;
    const float b = -s * scale.y;
    const float c2 = s * scale.x;
    const float d = c * scale.y;

    // Calculate final translation including pivot offset
    const float tx = position.x + pivot.x * (1.0f - a) - pivot.y * b;
    const float ty = position.y - pivot.x * c2 + pivot.y * (1.0f - d);

    // Create 3x3 matrix (column-major order)
    glm::mat3 matrix(1.0f);
    matrix[0] = glm::vec3(a, c2, 0.0f); // first column
    matrix[1] = glm::vec3(b, d, 0.0f); // second column
    matrix[2] = glm::vec3(tx, ty, 1.0f); // third column (translation + homogeneous)

    return matrix;
}