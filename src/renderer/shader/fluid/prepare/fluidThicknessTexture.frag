#version 430 core

in vec3 vCenterPosViewSpace;
in float vDensity;

out vec4 fThickness;

uniform float uPointSize;
uniform float uThicknessScaler;
uniform float uMinimumDensity;

const vec2 POINT_CENTER = vec2(0.5, 0.5);

void main() {
    if (vDensity < uMinimumDensity) {
        discard;
    }
    vec2 pointCoord = (gl_PointCoord - POINT_CENTER) * vec2(2.0, -2.0);
    float xySquaredLength = dot(pointCoord, pointCoord);
    if (xySquaredLength > 1.0) {
        discard;
    }
    vec3 normalViewSpace = vec3(pointCoord, sqrt(1.0 - xySquaredLength));

    vec3 fragPosViewSpace = vCenterPosViewSpace + normalViewSpace * uPointSize;

    fThickness = vec4(uThicknessScaler * normalViewSpace.z, 0.0, 0.0, 1.0);
}
