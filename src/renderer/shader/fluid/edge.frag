#version 430 core

in vec2 vTexCoords;

out vec4 fColor;

uniform isampler2D uEdgeTexture;

void main() {
    int edge = texture(uEdgeTexture, vTexCoords).r;
    if (edge == 0) {
        discard;
    }
    else {
        fColor = vec4(vec3(0.0), 1.0);
    }
}