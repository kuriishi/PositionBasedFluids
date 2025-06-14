#version 430 core

in vec2 vTexCoords;

out vec4 fColor;

// uniforms
// textures
// scene
uniform sampler2D uSceneColorTexture;
uniform sampler2D uSceneDepthTexture;
// fluid
uniform sampler2D uFluidColorTexture;
uniform sampler2D uFluidDepthTexture;
uniform isampler2D uFluidValidTexture;

void main() {
    // default is sample from scene
    fColor = texture(uSceneColorTexture, vTexCoords);
    gl_FragDepth = texture(uSceneDepthTexture, vTexCoords).r;

    // but if fluid is valid
    int fluidValid = texture(uFluidValidTexture, vTexCoords).r;
    if (fluidValid == 1) {
        // and fluid's depth is less than scene's
        float fluidDepth = texture(uFluidDepthTexture, vTexCoords).r;
        if (fluidDepth <= gl_FragDepth) {
            // then sample from fluid
            fColor = texture(uFluidColorTexture, vTexCoords);
            gl_FragDepth = fluidDepth;
        }
    }
}


