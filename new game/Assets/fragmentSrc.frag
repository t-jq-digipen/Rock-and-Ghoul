
#version 450 core
out vec4 FragColor;
uniform sampler2D uTexture;
uniform bool uUseTexture;
uniform vec3 uColor;
uniform bool uUseText;
uniform vec3 uTextColor;
uniform float uAlpha;
uniform float uBrightness;
in vec2 TexCoord;
void main() {
    if (uUseText) {
        float alpha = texture(uTexture, TexCoord).r;
        FragColor = vec4(uTextColor, alpha);
    } else if (uUseTexture) {
        vec4 t = texture(uTexture, TexCoord);
        vec3 tinted = mix(t.rgb * (uColor * 0.5 + 0.5), vec3(1.0), uBrightness);
        FragColor = vec4(tinted, t.a);
    } else {
        FragColor = vec4(uColor, uAlpha);
    }
}
