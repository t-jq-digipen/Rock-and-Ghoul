#version 330 core
layout (location = 0) in vec2 aPos;   // base quad vertex pos
layout (location = 1) in vec2 aTex;   // base quad UV

// Per-instance attributes
layout (location = 2) in vec2 iPos;
layout (location = 3) in vec2 iScale;
layout (location = 4) in vec4 iUVRect;
layout (location = 5) in float iRotation;

uniform mat4 uProjection;

out vec2 TexCoord;

void main() {
    // Build rotation matrix
    float c = cos(iRotation);
    float s = sin(iRotation);
    mat2 rot = mat2(c, -s,
                    s,  c);

    // Center quad around (0,0) before scaling
    vec2 centered = aPos - 0.5;

    // Apply transform
    vec2 transformed = rot * (centered * iScale) + iPos;

    gl_Position = uProjection * vec4(transformed, 0.0, 1.0);

    // Apply per-instance UV rect
    TexCoord = mix(iUVRect.xy, iUVRect.zw, aTex);
}
