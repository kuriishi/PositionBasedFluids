#version 430 core

in vec3 vTexCoords;

out vec4 FragColor;

uniform samplerCube uSkyboxTexture;

void main() {
    FragColor = texture(uSkyboxTexture, vTexCoords);
}
