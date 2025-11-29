/*!
@file       vertexSrc.vert
@author   kervinguancheng.ong@digipen.edu
@date    	  02/10/2025

This file implements the vertex shader required for regular rendering

*//*__________________________________________________________________________*/

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

//use custom verts for quads
uniform bool uUseCustomVerts;

out vec2 TexCoord;

void main() {

    // Translate to world position
    vec2 world;
    if (uUseCustomVerts){
	world = aPos;
    } else {
	// Create rotation matrix
    	float c = cos(uRotation);
    	float s = sin(uRotation);
    	mat2 rot = mat2(c, -s,
                    s,  c);

    	// Rotate first, then scale
    	vec2 rotated = rot * aPos;
    	vec2 scaled  = rotated * uScale;
    	world = scaled + uPos;
    }

    gl_Position = uProjection * vec4(world, 0.0, 1.0);

    // Choose texture coordinates
    if (uUseSubImage) {
        TexCoord = mix(uUVRect.xy, uUVRect.zw, aTex);
    } else {
        TexCoord = aTex;
    }
}
