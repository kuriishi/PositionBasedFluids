#version 430 core

in vec3 vCenterPosViewSpace;
in float vDensity;

layout(location = 0) out int fFoam;

uniform float uMinimumDensity;
uniform float uFoamDensity;

const vec2 POINT_CENTER = vec2(0.5, 0.5);

void main() {
    vec2 pointCoord = (gl_PointCoord - POINT_CENTER) * vec2(2.0, -2.0);
    float xySquaredLength = dot(pointCoord, pointCoord);
    if (xySquaredLength > 1.0) {
        discard;
    }

    if (vDensity < uMinimumDensity || vDensity > uFoamDensity) {
        fFoam = 0;
    }
    else {
        fFoam = 1;
    }
}