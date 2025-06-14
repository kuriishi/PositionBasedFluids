#version 430 core

in vec3 vCenterPosViewSpace;
in float vDensity;

layout(location = 0) out float fDepth;
layout(location = 1) out int fValid;

uniform mat4 uProjection;
uniform float uPointSize;
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
    vec4 fragPosClipSpace = uProjection * vec4(fragPosViewSpace, 1.0);

    gl_FragDepth = (fragPosClipSpace.z / fragPosClipSpace.w) * 0.5 + 0.5;

    fDepth = gl_FragDepth;
    fValid = 1;
}
