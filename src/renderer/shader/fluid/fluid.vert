#version 430 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in float aDensity;

out vec3 vCenterPosViewSpace;
out float vDensity;

uniform mat4 uView;
uniform mat4 uProjection;
uniform float uPointSize;

// scaler should be a function relative to camera's aspect and fovy and screen's size,
// cause I do not change these parameters and I am lazy,
// so it's fine to make it a constant
const float POINT_SIZE_SCALER = 2000;

void main() {
    gl_Position = uProjection * uView * vec4(aPos.xyz, 1.0);
    vCenterPosViewSpace = (uView * vec4(aPos.xyz, 1.0)).xyz;
    gl_PointSize = POINT_SIZE_SCALER * uPointSize / (-vCenterPosViewSpace.z);
    vDensity = aDensity;
}
