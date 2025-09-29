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
uniform float uRotation;

//sub-image UV rectangle (u0, v0, u1, v1)
uniform vec4 uUVRect;
uniform bool uUseSubImage;

out vec2 TexCoord;

void main() {

    // Apply scaling first
    vec2 scaled = aPos * uScale;
    
    // Create rotation matrix
    float cosR = cos(uRotation);
    float sinR = sin(uRotation);
    mat2 rotationMatrix = mat2(cosR, -sinR, sinR,  cosR);
    
    // Apply rotation around center (0,0)
    vec2 rotated = rotationMatrix * scaled;
    
    // Apply translation
    vec2 transformed = rotated + uPos;
    
    gl_Position = uProjection * vec4(transformed, 0.0, 1.0);
    
    
    if (uUseSubImage) {
    	TexCoord = mix(uUVRect.xy, uUVRect.zw, aTex);
    } else {
        TexCoord = aTex;
    }
    //TexCoord = aTex;
    //TexCoord = mix(uUVRect.xy, uUVRect.zw, aTex);
}