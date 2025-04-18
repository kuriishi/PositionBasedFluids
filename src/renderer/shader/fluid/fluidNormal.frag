#version 430 core

in vec3 aPosViewSpace;
in float aDensity;

out vec4 FragColor;

uniform mat4 viewMatrixTranspose;

uniform vec2 screenSize;
uniform float POINT_SIZE;
uniform float MINIMUN_DENSITY;

uniform sampler2D fluidNormalViewSpaceTexture;

vec2 POINT_CENTER = vec2(0.5, 0.5);

void main() {
    if (aDensity < MINIMUN_DENSITY) {
        discard;
    }
    vec2 pointCoord = (gl_PointCoord - POINT_CENTER) * vec2(2.0, -2.0);
    if (dot(pointCoord, pointCoord) > 1.0) {
        discard;
    }
    vec2 texCoord = gl_FragCoord.xy / screenSize;

    vec3 normalViewSpace = texture(fluidNormalViewSpaceTexture, texCoord).xyz;
    vec3 normalWorldSpace = (viewMatrixTranspose * vec4(normalViewSpace, 0.0)).xyz;

    FragColor = vec4(normalWorldSpace, 1.0);
}

