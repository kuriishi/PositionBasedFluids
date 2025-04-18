#version 430 core

in vec3 aPosViewSpace;
in float aDensity;

out vec4 FragColor;

uniform vec2 screenSize;
uniform float POINT_SIZE;
uniform float MINIMUN_DENSITY;

uniform sampler2D fluidThicknessTexture;

vec2 POINT_CENTER = vec2(0.5, 0.5);
uniform vec3 fluidColor;

void main() {
    if (aDensity < MINIMUN_DENSITY) {
        discard;
    }
    vec2 pointCoord = (gl_PointCoord - POINT_CENTER) * vec2(2.0, -2.0);
    if (dot(pointCoord, pointCoord) > 1.0) {
        discard;
    }
    vec2 texCoord = gl_FragCoord.xy / screenSize;

    float thickness = texture(fluidThicknessTexture, texCoord).r;
    vec3 result = thickness * fluidColor;

    FragColor = vec4(result, 1.0);
}