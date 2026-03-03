/*!
@file       batch.frag
@author   kervinguancheng.ong@digipen.edu
@date    	  05/11/2025

This file implements the batch fragment shader.

*//*__________________________________________________________________________*/

#version 450 core
in vec2 vTex;
in vec4 vColor;
flat in int vTexIndex;

out vec4 FragColor;
uniform sampler2D uTextures[16]; // must match kMaxTexSlots

void main() {
    FragColor = texture(uTextures[vTexIndex], vTex) * vColor;
}