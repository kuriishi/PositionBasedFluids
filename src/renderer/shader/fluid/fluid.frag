#version 430 core

in vec3 vCenterPosViewSpace;
in float vDensity;

layout(location = 0) out vec4 fColor;
layout(location = 1) out vec4 fPosition;
layout(location = 2) out vec4 fNormal;

uniform mat4 uViewInverse;
uniform mat4 uViewTranspose;
// (width, height)
uniform ivec2 uScreenSize;
uniform vec3 uFluidColor;
uniform float uPointSize;
uniform float uMinimumDensity;
// textures
uniform sampler2D uNormalViewSpaceTexture;
uniform sampler2D uThicknessTexture;
uniform sampler2D uSceneColorTexture;
uniform sampler2D uSmoothedDepthTexture;
uniform samplerCube uSkyboxTexture;

const vec2 POINT_CENTER = vec2(0.5, 0.5);

void main() {
    if (vDensity < uMinimumDensity) {
        discard;
    }
    vec2 pointCoord = (gl_PointCoord - POINT_CENTER) * vec2(2.0, -2.0);
    if (dot(pointCoord, pointCoord) > 1.0) {
        discard;
    }
    vec2 texCoords = gl_FragCoord.xy / vec2(uScreenSize);

    vec3 normalViewSpace = texture(uNormalViewSpaceTexture, texCoords).xyz;
    vec3 normalWorldSpace = normalize((uViewTranspose * vec4(normalViewSpace, 0.0)).xyz);
    vec3 posViewSpace = vCenterPosViewSpace + normalViewSpace * uPointSize;
    vec3 posWorldSpace = (uViewInverse * vec4(posViewSpace, 1.0)).xyz;

    vec3 I = normalize(posViewSpace);
    vec3 N = normalWorldSpace;

    // refract
    float thickness = texture(uThicknessTexture, texCoords).r;
    vec3 attenuationFactor = exp(-(vec3(1.0) - uFluidColor) * thickness);
    float ratio = 1.0 / 1.33;
    vec3 R = refract(I, N, ratio);
    float refractScaler = R.y * 0.025;
    vec2 texCoordRefract = texCoords + normalViewSpace.xy * refractScaler;
    vec3 colorRefract = texture(uSceneColorTexture, texCoordRefract).rgb * attenuationFactor;

    // reflect
    vec3 reflectDir = normalize((uViewTranspose * vec4(reflect(I, N), 0.0)).xyz);
    vec3 colorReflect = texture(uSkyboxTexture, reflectDir).rgb;

    // vec3 result = colorRefract;
    // vec3 result = colorReflect;
    vec3 result = colorRefract + colorReflect * 0.75;

    fColor = vec4(result, 1.0);
    fPosition = vec4(posWorldSpace, 1.0);
    fNormal = vec4(normalWorldSpace, 1.0);
    gl_FragDepth = texture(uSmoothedDepthTexture, texCoords).r;
}
