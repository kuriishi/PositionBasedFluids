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
// fluid
uniform vec3 uFluidColor;
uniform float uPointSize;
uniform float uMinimumDensity;
uniform vec3 uCameraPosition;
// textures
uniform sampler2D uNormalViewSpaceTexture;
uniform sampler2D uThicknessTexture;
uniform sampler2D uSceneColorTexture;
uniform sampler2D uSmoothedDepthTexture;
uniform samplerCube uSkyboxTexture;
uniform isampler2D uValidTexture;
// cartoon
uniform float uBrightThreshold;
uniform float uBrightFactor;
uniform float uDarkThreshold;
uniform float uDarkFactor;
uniform float uRefractThreshold;
uniform float uRefractMax;
uniform float uReflectThreshold;
uniform float uReflectMax;
uniform int uEdgeKernelRadius;

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

    vec2 delta = 1.0 / vec2(uScreenSize);
    int validCount = 0;
    for (int i = -uEdgeKernelRadius; i <= uEdgeKernelRadius; i++) {
        for (int j = -uEdgeKernelRadius; j <= uEdgeKernelRadius; j++) {
            if (texture(uValidTexture, texCoords + vec2(i, j) * delta).r == 1) {
                validCount++;
            }
        }
    }
    if (validCount < (uEdgeKernelRadius * 2 + 1) * (uEdgeKernelRadius * 2 + 1)) {
        fColor = vec4(vec3(0.0), 1.0);
        return;
    }

    vec3 normalViewSpace = texture(uNormalViewSpaceTexture, texCoords).xyz;
    vec3 normalWorldSpace = normalize((uViewTranspose * vec4(normalViewSpace, 0.0)).xyz);
    vec3 posViewSpace = vCenterPosViewSpace + normalViewSpace * uPointSize;
    vec3 posWorldSpace = (uViewInverse * vec4(posViewSpace, 1.0)).xyz;

    vec3 I = normalize(uCameraPosition - posWorldSpace);
    vec3 N = normalWorldSpace;

    vec3 quantizedColor = uFluidColor;
    float cosine = dot(I, N);
    if (cosine > uBrightThreshold) {
        quantizedColor *= uBrightFactor;
    }
    else if (cosine < uDarkThreshold) {
        quantizedColor *= uDarkFactor;
    }
    
    float refractFactor;
    if (cosine > uRefractThreshold) {
        refractFactor = uRefractMax;
    }
    else if (cosine < uReflectThreshold) {
        refractFactor = 0.0;
    }
    else {
        refractFactor = mix(0.0, uRefractMax, (cosine - uReflectThreshold) / (uRefractThreshold - uReflectThreshold));
    }

    float reflectFactor;
    if (cosine < uReflectThreshold) {
        reflectFactor = uReflectMax;
    }
    else if (cosine > uRefractThreshold) {
        reflectFactor = 0.0;
    }
    else {
        reflectFactor = mix(uReflectMax, 0.0, (cosine - uReflectThreshold) / (uRefractThreshold - uReflectThreshold));
    }

    // refract
    float thickness = texture(uThicknessTexture, texCoords).r;
    float ratio = 1.0 / 1.33;
    vec3 R = refract(I, N, ratio);
    float refractScaler = R.y * 0.025;
    // float refractScaler = thickness * 0.025;
    vec2 texCoordRefract = texCoords + normalViewSpace.xy * refractScaler;
    vec3 refractColor = texture(uSceneColorTexture, texCoordRefract).rgb;

    vec3 finalColor = mix(quantizedColor, refractColor, refractFactor * exp(-thickness));

    // reflect
    vec3 reflectDir = normalize((uViewTranspose * vec4(reflect(I, N), 0.0)).xyz);
    vec3 reflectColor = texture(uSkyboxTexture, reflectDir).rgb;

    finalColor = finalColor + reflectColor * reflectFactor;

    fColor = vec4(finalColor, 1.0);
    fPosition = vec4(posWorldSpace, 1.0);
    fNormal = vec4(normalWorldSpace, 1.0);
    gl_FragDepth = texture(uSmoothedDepthTexture, texCoords).r;
}
