/*!
@file       fragmentSrc.frag
This file implements the fragment shader.

*//*__________________________________________________________________________*/


#version 450 core
out vec4 FragColor;

uniform vec3 uColor;
uniform sampler2D uTexture;
uniform bool uUseTexture;

in vec2 TexCoord;

void main() {
    if (uUseTexture)
        FragColor = texture(uTexture, TexCoord);
    else
        FragColor = vec4(uColor, 1.0);
	
}