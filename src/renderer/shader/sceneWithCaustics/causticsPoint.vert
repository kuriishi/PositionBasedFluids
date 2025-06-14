#version 430 core

layout(location = 0) in vec4 aPosition;

out vec4 vCenterPositionWorldSpace;

// uniforms
// MVP transforms
uniform mat4 uView;
uniform mat4 uProjection;
// other parameters
uniform float uPhotonSize;

// constants
const float POINT_SIZE_SCALER = 512;

void main() {
    vec4 posViewSpace = uView * vec4(aPosition.xyz, 1.0);
    gl_Position = uProjection * posViewSpace;
    gl_PointSize = POINT_SIZE_SCALER * uPhotonSize / abs(posViewSpace.z);
    // gl_PointSize = 8;
    vCenterPositionWorldSpace = aPosition;
}
