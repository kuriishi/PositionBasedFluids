#version 430 core

in vec2 textureCoord;
in vec3 normal;

out vec4 FragColor;

uniform sampler2D floorTexture;

void main()
{
    FragColor = texture(floorTexture, textureCoord);
}

