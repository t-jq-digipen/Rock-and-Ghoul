
#version 450 core
in vec2 vTex;
in vec4 vColor;
flat in int vTexIndex;
out vec4 FragColor;
uniform sampler2D uTextures[16];
void main() {
    FragColor = texture(uTextures[vTexIndex], vTex) * vColor;
}
