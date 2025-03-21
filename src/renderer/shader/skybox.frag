#version 430 core
in vec3 TexCoords;

out vec4 FragColor;

uniform samplerCube skyboxTexture;

void main() {
    FragColor = texture(skyboxTexture, TexCoords);
}
