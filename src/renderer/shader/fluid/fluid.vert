#version 430 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in float _aDensity;

out vec3 aPosViewSpace;
out float aDensity;

uniform mat4 view;
uniform mat4 projection;

uniform float POINT_SIZE;
// scaler should be a function relative to camera's aspect and fovy and screen's size,
// cause I do not change this parameters and I am lazy,
// so it's fine to make it a constant
const float POINT_SIZE_SCALER = 2000;

void main() {
    gl_Position = projection * view * vec4(aPos.xyz, 1.0);
    aPosViewSpace = (view * vec4(aPos.xyz, 1.0)).xyz;
    gl_PointSize = POINT_SIZE_SCALER * POINT_SIZE / (-aPosViewSpace.z);
    aDensity = _aDensity;
}
