/*!
@file       fragmentSrc.frag
This file implements the fragment shader.

*//*__________________________________________________________________________*/


#version 450 core
out vec4 FragColor;

uniform sampler2D uTexture;

//for sprites
uniform bool uUseTexture;
uniform vec3 uColor;

//for text
uniform bool uUseText;       // true = font rendering
uniform vec3 uTextColor;    // text color

in vec2 TexCoord;

void main() {
    if (uUseText) {
        // Sample single-channel font atlas
        float alpha = texture(uTexture, TexCoord).r;
        //FragColor = vec4(uTextColor, 1.0) * vec4(1.0, 1.0, 1.0, alpha);
	FragColor = vec4(uTextColor, alpha); 
    }
    else if (uUseTexture){
        FragColor = texture(uTexture, TexCoord);
    }
    else{
        FragColor = vec4(uColor, 1.0);
    }
}