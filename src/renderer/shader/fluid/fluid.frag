#version 430 core

in vec3 aPosViewSpace;
in float aDensity;

out vec4 FragColor;

uniform vec2 screenSize;
uniform mat4 viewMatrixInverse;
uniform mat4 viewMatrixTranspose;
uniform float POINT_SIZE;
uniform float MINIMUN_DENSITY;

uniform sampler2D fluidNormalViewSpaceTexture;
uniform sampler2D fluidThicknessTexture;
uniform sampler2D backgroundTexture;

uniform samplerCube skyboxTexture;

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

    vec3 normalViewSpace = texture(fluidNormalViewSpaceTexture, texCoord).xyz;
    vec3 normalWorldSpace = normalize((viewMatrixTranspose * vec4(normalViewSpace, 0.0)).xyz);
    vec3 posViewSpace = aPosViewSpace + normalViewSpace * POINT_SIZE;
    vec3 posWorldSpace = (viewMatrixInverse * vec4(posViewSpace, 1.0)).xyz;

    vec3 I = normalize(posViewSpace);
    vec3 N = normalWorldSpace;

    // refract
    float thickness = texture(fluidThicknessTexture, texCoord).r;
    vec3 attenuationFactor = exp(-(vec3(1.0) - fluidColor) * thickness);
    float ratio = 1.0 / 1.33;
    vec3 R = refract(I, N, ratio);
    float refractScaler = R.y * 0.025;
    vec2 texCoordRefract = texCoord + normalViewSpace.xy * refractScaler;
    vec3 colorRefract = texture(backgroundTexture, texCoordRefract).rgb * attenuationFactor;

    // reflect
    vec3 reflectDir = normalize((viewMatrixTranspose * vec4(reflect(I, N), 0.0)).xyz);
    vec3 colorReflect = texture(skyboxTexture, reflectDir).rgb;

    // vec3 result = colorRefract;
    // vec3 result = colorReflect;
    vec3 result = colorRefract + colorReflect * 0.75;

    FragColor = vec4(result, 1.0);
}
