#version 430 core

in vec2 vTexCoords;

out vec4 fColor;

uniform sampler2D uOtherSceneColorTexture;
uniform sampler2D uSkyboxColorTexture;
uniform isampler2D uValidTexture;

void main() {
    int valid = texture(uValidTexture, vTexCoords).r;
    if (valid == 1) {
        fColor = texture(uOtherSceneColorTexture, vTexCoords);
    } else {
        fColor = texture(uSkyboxColorTexture, vTexCoords);
    }
}
