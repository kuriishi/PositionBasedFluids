#version 430 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D backgroundTexture;

void main() {
    FragColor = texture(backgroundTexture, TexCoords);
}
