
#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
uniform mat4 uProjection;
uniform vec2 uPos;
uniform vec2 uScale;
uniform float uRotation;
uniform vec4 uUVRect;
uniform bool uUseSubImage;
uniform bool uUseCustomVerts;
out vec2 TexCoord;
void main() {
    vec2 world;
    if (uUseCustomVerts) {
        world = aPos;
    } else {
        float c = cos(uRotation); float s = sin(uRotation);
        mat2 rot = mat2(c, -s, s, c);
        world = rot * aPos * uScale + uPos;
    }
    gl_Position = uProjection * vec4(world, 0.0, 1.0);
    TexCoord = uUseSubImage ? mix(uUVRect.xy, uUVRect.zw, aTex) : aTex;
}
