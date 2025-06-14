#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 vTexCoords;
out vec3 vPosition;
out vec3 vNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uModelTranspose;

void main()
{
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    vTexCoords = aTexCoord;
    vPosition = (uModel * vec4(aPos, 1.0)).xyz;
    vNormal = (uModelTranspose * vec4(aNormal, 0.0)).xyz;
}