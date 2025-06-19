#version 430 core

in vec2 vTexCoords;

out vec4 fColor;

uniform isampler2D uFoamTexture;

void main() {
    int foam = texture(uFoamTexture, vTexCoords).r;
    if (foam == 0) {
        discard;
    }
    else {
        fColor = vec4(1.0);
    }
}