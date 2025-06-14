#version 430 core

in vec2 vTexCoords;

out vec4 fColor;

uniform sampler2D uCausticsTerminatePositionTexture;

void main() {
    vec3 position = texture(uCausticsTerminatePositionTexture, vTexCoords).xyz;
    fColor = vec4(position, 1.0f);
}

