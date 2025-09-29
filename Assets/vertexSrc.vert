#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

uniform mat4 uProjection;
uniform vec2 uPos;
uniform vec2 uScale;
uniform float uRotation;

// sub-image UV rectangle (u0, v0, u1, v1)
uniform vec4 uUVRect;
uniform bool uUseSubImage;

out vec2 TexCoord;

void main() {
    // Create rotation matrix
    float c = cos(uRotation);
    float s = sin(uRotation);
    mat2 rot = mat2(c, -s,
                    s,  c);

    // Rotate first, then scale
    vec2 rotated = rot * aPos;
    vec2 scaled  = rotated * uScale;

    // Translate to world position
    vec2 world = scaled + uPos;

    gl_Position = uProjection * vec4(world, 0.0, 1.0);

    // Choose texture coordinates
    if (uUseSubImage) {
        TexCoord = mix(uUVRect.xy, uUVRect.zw, aTex);
    } else {
        TexCoord = aTex;
    }
}
