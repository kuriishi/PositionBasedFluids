#version 430 core

in vec2 vTexCoords;

layout(location = 0) out vec4 fTerminatePosition;
layout(location = 1) out int fValid;
layout(location = 2) out vec4 fRedTerminatePosition;
layout(location = 3) out vec4 fGreenTerminatePosition;
layout(location = 4) out vec4 fBlueTerminatePosition;

// uniforms
uniform mat4 uView;
uniform mat4 uProjection;
// (width, height)
uniform ivec2 uScreenSize;
// eta, inward refraction / outward refraction
uniform float uRefractionRatio;
uniform float uRedRefractionRatio;
uniform float uGreenRefractionRatio;
uniform float uBlueRefractionRatio;
uniform vec3 uLightPosition;
// textures
uniform sampler2D uSceneDepthTexture;
uniform sampler2D uScenePositionTexture;
uniform isampler2D uSceneValidTexture;
uniform sampler2D uFluidDepthTexture;
uniform sampler2D uFluidNormalTexture;
uniform sampler2D uFluidPositionTexture;
uniform isampler2D uFluidValidTexture;

const float FLOAT_MAX = 1e8;

// get image coord from world position
vec2 getTexCoords(vec3 pos) {
    vec4 clipSpace = uProjection * uView * vec4(pos, 1.0);
    vec4 ndc = clipSpace / clipSpace.w;
    return (ndc.xy + 1.0) * 0.5;
}

// get z-coordinate from texture
float zFromTexture(vec3 pos) {
    vec2 texCoords = getTexCoords(pos);
    if (texCoords.x < 0.0 || texCoords.x > 1.0 || texCoords.y < 0.0 || texCoords.y > 1.0) {
        return -1e8f;
    }
    vec3 scenePosition = texture(uScenePositionTexture, texCoords).xyz;
    vec3 scenePositionView = (uView * vec4(scenePosition, 1.0)).xyz;
    return scenePositionView.z;
}

// get z-coordinate from world position
float zFromPosition(vec3 pos) {
    vec3 posView = (uView * vec4(pos, 1.0)).xyz;
    return posView.z;
}

int MAX_MARCHING_STEPS = 256;
float STEP_SCALER = 0.05;
int rayMarching(vec3 pos, vec3 dir, out vec3 hitPos) {
    int steps = 0;
    dir = normalize(dir);
    vec3 delta = dir * STEP_SCALER;
    while (steps < MAX_MARCHING_STEPS) {
        float z_current = zFromPosition(pos);
        float z_texture = zFromTexture(pos);
        // greater z-coordinate means closer to the camera, as right-handed coordinate system
        if (z_current < z_texture) {
            vec2 texCoords = getTexCoords(pos);
            hitPos = texture(uScenePositionTexture, texCoords).xyz;
            return 1;
        }
        pos += delta;
        steps++;
    }

    hitPos = vec3(FLOAT_MAX);
    return 0;
}

const int FOO = 1;

void main() {
    fValid = 0;
    // fTerminatePosition = vec4(0.0, 0.0, 0.0, 0.0);
    // fRedTerminatePosition = vec4(0.0, 0.0, 0.0, 0.0);
    // fGreenTerminatePosition = vec4(0.0, 0.0, 0.0, 0.0);
    // fBlueTerminatePosition = vec4(0.0, 0.0, 0.0, 0.0);
    fTerminatePosition = vec4(vec3(FLOAT_MAX), 0.0);
    fRedTerminatePosition = vec4(vec3(FLOAT_MAX), 0.0);
    fGreenTerminatePosition = vec4(vec3(FLOAT_MAX), 0.0);
    fBlueTerminatePosition = vec4(vec3(FLOAT_MAX), 0.0);

    float sceneDepth = texture(uSceneDepthTexture, vTexCoords).r;
    gl_FragDepth = sceneDepth;
    vec3 scenePosition = texture(uScenePositionTexture, vTexCoords).xyz;

    // only photon refract from fluid
    int fluidValid = texture(uFluidValidTexture, vTexCoords).r;
    if (fluidValid == 1) {
        vec3 pos = texture(uFluidPositionTexture, vTexCoords).xyz;
        vec3 normal = texture(uFluidNormalTexture, vTexCoords).xyz;
        float depth = texture(uFluidDepthTexture, vTexCoords).r;
        if (depth < sceneDepth) {
            gl_FragDepth = depth;
        }
        else {
            return;
        }

        // inward direction
        vec3 I = normalize(pos - uLightPosition);

        // outward direction
        vec3 R = refract(I, normal, uRefractionRatio);
        vec3 R_red = refract(I, normal, uRedRefractionRatio);
        vec3 R_green = refract(I, normal, uGreenRefractionRatio);
        vec3 R_blue = refract(I, normal, uBlueRefractionRatio);

        vec3 hitPos = vec3(FLOAT_MAX);
        vec3 hitPos_red = vec3(FLOAT_MAX);
        vec3 hitPos_green = vec3(FLOAT_MAX);
        vec3 hitPos_blue = vec3(FLOAT_MAX);
        int valid = rayMarching(pos, R, hitPos);
        int valid_red = rayMarching(pos, R_red, hitPos_red);
        int valid_green = rayMarching(pos, R_green, hitPos_green);
        int valid_blue = rayMarching(pos, R_blue, hitPos_blue);

        if (valid == 1) {
            fTerminatePosition = vec4(hitPos, 1.0);
        }
        else {
            fTerminatePosition = vec4(scenePosition, 1.0);
        }
        if (valid_red == 1) {
            fRedTerminatePosition = vec4(hitPos_red, 1.0);
        }
        if (valid_green == 1) {
            fGreenTerminatePosition = vec4(hitPos_green, 1.0);
        }
        if (valid_blue == 1) {
            fBlueTerminatePosition = vec4(hitPos_blue, 1.0);
        }

        valid += valid_red + valid_green + valid_blue;
        if (valid == 4) {
            fValid = 1;
        }
    }
    else if (FOO == 1) {
        int sceneValid = texture(uSceneValidTexture, vTexCoords).r;
        if (sceneValid == 1) {
            fTerminatePosition = vec4(scenePosition, 1.0);
        }
    }
}