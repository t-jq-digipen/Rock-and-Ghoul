/*!
@file       vertexSrc.vert
This file implements the vertex shader.

*//*__________________________________________________________________________*/


#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

uniform mat4 uProjection;
uniform vec2 uPos;
uniform vec2 uScale;

out vec2 TexCoord;

void main() {
    vec2 scaled = aPos * uScale + uPos;
    gl_Position = uProjection * vec4(scaled, 0.0, 1.0);
    TexCoord = aTex;
}