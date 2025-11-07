/*!
@file       batch.vert
@author   kervinguancheng.ong@digipen.edu
@date    	  05/11/2025

This file implements the vertex shader required for batch rendering

*//*__________________________________________________________________________*/

#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aColor;
layout(location = 3) in float aTexIndex;

uniform mat4 uProjection;

out vec2 vTex;
out vec4 vColor;
flat out int vTexIndex;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    vTex = aTex;
    vColor = aColor;
    vTexIndex = int(aTexIndex);
}