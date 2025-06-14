#version 430 core

in vec2 vTexCoords;
in vec3 vPosition;
in vec3 vNormal;

layout(location = 0) out vec4 fColor;
layout(location = 1) out int fValid;
layout(location = 2) out vec4 fPosition;
layout(location = 3) out vec4 fNormal;

uniform sampler2D uFloorTexture;

struct Light {
    vec3 position;
    vec3 intensity;
};
uniform Light uLight;

void main()
{
    vec3 lightDirection = normalize(uLight.position - vPosition);
    float diffuse = max(dot(vNormal, lightDirection), 0.0);
    vec3 diffuseColor = uLight.intensity * diffuse;

    fColor = texture(uFloorTexture, vTexCoords);
    // fColor = texture(uFloorTexture, vTexCoords) * vec4(diffuseColor, 1.0);
    fValid = 1;
    fPosition = vec4(vPosition, 1.0);
    fNormal = vec4(vNormal, 1.0);
}

