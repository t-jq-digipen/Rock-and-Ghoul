/*!
@file       fragmentSrc.frag
@author   kervinguancheng.ong@digipen.edu
@date    	  28/11/2025

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

//faded
uniform float uAlpha;

//brightness (use 0 for normal, 1 for full white)
uniform float uBrightness;

in vec2 TexCoord;

void main() {
    if (uUseText) {
        // Sample single-channel font atlas
        float alpha = texture(uTexture, TexCoord).r;
        //FragColor = vec4(uTextColor, 1.0) * vec4(1.0, 1.0, 1.0, alpha);
	FragColor = vec4(uTextColor, alpha); 
    }
    else if (uUseTexture) {
    	vec4 texColor = texture(uTexture, TexCoord);

	//tinting
	vec3 tinted = texColor.rgb * (uColor * 0.5 + 0.5);
	
	//gonna blend towards white through brightness
	tinted = mix(tinted, vec3(1.0), uBrightness);

	//final color
    	FragColor = vec4(tinted, texColor.a);
	
    }
    else{
        FragColor = vec4(uColor, uAlpha);
    }
}