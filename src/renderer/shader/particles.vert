#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aOffset;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main() {
    vec3 pos = vec3(model * vec4(aPos, 1.0)); 
    Normal = normalize(normalMatrix * aNormal);
    FragPos = pos;
    gl_Position = projection * view * vec4(pos + aOffset, 1.0);
}