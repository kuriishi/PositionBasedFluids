#version 430 core

in vec3 aPosViewSpace;
in float aDensity;

layout(location = 0) out float fragDepth;
layout(location = 1) out int fragSampledFlag;

uniform mat4 projection;
uniform float POINT_SIZE;
uniform float MINIMUN_DENSITY;


vec2 POINT_CENTER = vec2(0.5, 0.5);

void main() {
    if (aDensity < MINIMUN_DENSITY) {
        discard;
    }
    vec2 pointCoord = (gl_PointCoord - POINT_CENTER) * vec2(2.0, -2.0);
    float xySquaredLength = dot(pointCoord, pointCoord);
    if (xySquaredLength > 1.0) {
        discard;
    }
    vec3 normalViewSpace = vec3(pointCoord, sqrt(1.0 - xySquaredLength));

    vec3 fragPosViewSpace = aPosViewSpace + normalViewSpace * POINT_SIZE;
    vec4 fragPosClipSpace = projection * vec4(fragPosViewSpace, 1.0);

    gl_FragDepth = (fragPosClipSpace.z / fragPosClipSpace.w) * 0.5 + 0.5;

    fragDepth = gl_FragDepth;
    fragSampledFlag = 1;
}
