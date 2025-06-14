#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 vTexCoords;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vTexCoords = aPos;
    vec4 pos = uProjection * uView * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}