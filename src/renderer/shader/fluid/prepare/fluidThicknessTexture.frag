#version 430 core

in vec3 aPosViewSpace;
in float aDensity;

out vec4 fragThickness;

uniform float POINT_SIZE;
uniform float THICKNESS_SCALER;
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

    fragThickness = vec4(THICKNESS_SCALER * normalViewSpace.z, 0.0, 0.0, 1.0);
}
