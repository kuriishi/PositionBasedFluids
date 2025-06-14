#version 430 core

in vec2 vTexCoords;

layout(location = 0) out vec4 fColor;
layout(location = 1) out int fValid;
layout(location = 2) out vec4 fPosition;
layout(location = 3) out vec4 fNormal;

// uniforms
// textures
// caustics
uniform sampler2D uCausticsTexture;
uniform sampler2D uSceneColorTexture;
uniform isampler2D uSceneValidTexture;
uniform sampler2D uScenePositionTexture;
uniform sampler2D uSceneNormalTexture;
uniform sampler2D uSceneDepthTexture;
// shadow
uniform sampler2D uCausticsDepthTexture;

// other uniforms
uniform ivec2 uCausticsResolution;
uniform vec3 uLightPosition;
uniform mat4 uCausticsView;
uniform mat4 uCausticsProjection;
uniform float uEpsilon;

// constants
/*
Seems familiar? The PCSS relative code comes from GAMES202.
*/
const int NUM_SAMPLES = 64;
const int BLOCKER_SEARCH_NUM_SAMPLES = NUM_SAMPLES;
const int PCF_NUM_SAMPLES = NUM_SAMPLES;
const float PI = 3.141592653589793;
const float PI2 = 6.283185307179586;
const int NUM_RINGS = 10;
const float LIGHT_GRID_WIDTH = 4.0;

vec2 poissonDisk[NUM_SAMPLES];

highp float rand_1to1(highp float x) { 
    return fract(sin(x) * 10000.0);
}

highp float rand_2to1(vec2 uv) { 
    const highp float a = 12.9898, b = 78.233, c = 43758.5453;
    highp float dt = dot(uv.xy, vec2(a, b)), sn = mod(dt, PI);
    return fract(sin(sn) * c);
}

void poissonDiskSamples(const in vec2 randomSeed) {
    float ANGLE_STEP = PI2 * float(NUM_RINGS) / float(NUM_SAMPLES);
    float INV_NUM_SAMPLES = 1.0 / float(NUM_SAMPLES);

    float angle = rand_2to1(randomSeed) * PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for(int i = 0; i < NUM_SAMPLES; i++) {
        poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
        radius += radiusStep;
        angle += ANGLE_STEP;
    }
}

vec2 getTexCoordsFromWorldPosition(vec3 posWorld) {
    vec3 posView = (uCausticsView * vec4(posWorld, 1.0)).xyz;
    vec4 posClip = uCausticsProjection * vec4(posView, 1.0);
    vec2 posNDC = posClip.xy / posClip.w;
    vec2 posTex = 0.5 * posNDC + 0.5;
    return posTex;
}

float getDepthFromWorldPosition(vec3 posWorld) {
    vec3 posView = (uCausticsView * vec4(posWorld, 1.0)).xyz;
    vec4 posClip = uCausticsProjection * vec4(posView, 1.0);
    return (posClip.z / posClip.w) * 0.5 + 0.5;
}

bool isPositionInCausticsRange(vec3 posWorld) {
    vec2 texCoords = getTexCoordsFromWorldPosition(posWorld);
    return texCoords.x >= 0.0 && texCoords.x <= 1.0 && 
           texCoords.y >= 0.0 && texCoords.y <= 1.0;
}

float findBlocker(vec2 uv, float zReceiver) {
    float scaler = LIGHT_GRID_WIDTH * (1.0 - zReceiver);
    float sumDepth = 0.0;
    float numBlockers = 0.0;

    for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; i++) {
        vec2 coords = uv + poissonDisk[i] * scaler;
        
        if (coords.x >= 0.0 && coords.x <= 1.0 && coords.y >= 0.0 && coords.y <= 1.0) {
            float depth = texture(uCausticsDepthTexture, coords).r;
            if (zReceiver + uEpsilon > depth) {
                sumDepth += depth;
                numBlockers += 1.0;
            }
        }
    }

    if (numBlockers == 0.0) {
        return zReceiver;
    }
    return sumDepth / numBlockers;
}

float PCF(vec2 uv, float zReceiver, float filterSize) {
    float visibility = 0.0;
    float validSamples = 0.0;
    
    for (int i = 0; i < PCF_NUM_SAMPLES; i++) {
        vec2 coords = uv + poissonDisk[i] * filterSize;
        if (coords.x >= 0.0 && coords.x <= 1.0 && coords.y >= 0.0 && coords.y <= 1.0) {
            float depth = texture(uCausticsDepthTexture, coords).r;
            if (zReceiver - uEpsilon < depth) {
                visibility += 1.0;
            }
            validSamples += 1.0;
        }
    }
    
    if (validSamples == 0.0) {
        return 1.0;
    }
    
    return visibility / validSamples;
}

float PCSS(vec3 posWorld) {
    vec2 texCoords = getTexCoordsFromWorldPosition(posWorld);
    float zReceiver = getDepthFromWorldPosition(posWorld);
    float avgBlockerDepth = findBlocker(texCoords, zReceiver);
    
    float penumbraSize = (zReceiver - avgBlockerDepth) / avgBlockerDepth * LIGHT_GRID_WIDTH;

    return PCF(texCoords, zReceiver, penumbraSize);
}

void main() {
    vec3 scene = texture(uSceneColorTexture, vTexCoords).rgb;
    vec3 posWorld = texture(uScenePositionTexture, vTexCoords).xyz;
    int valid = texture(uSceneValidTexture, vTexCoords).r;

    vec3 color = scene;
    if (valid == 1 && isPositionInCausticsRange(posWorld)) {
        poissonDiskSamples(vTexCoords);
        float visibility = PCSS(posWorld);
        color = 0.5 * scene * visibility + 0.5 * scene;
    }

    vec4 caustics = texture(uCausticsTexture, vTexCoords);
    color = color + scene * caustics.rgb;

    fColor = vec4(color, 1.0);
    // fColor = vec4(caustics.rgb, 1.0);
    fValid = valid;
    fPosition = texture(uScenePositionTexture, vTexCoords);
    fNormal = texture(uSceneNormalTexture, vTexCoords);
    gl_FragDepth = texture(uSceneDepthTexture, vTexCoords).r;
}
