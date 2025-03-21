#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

layout (std430, binding = 0) buffer ParticlePosition {
    vec4 particlePosition[];
};

out vec3 Normal;
out vec3 FragPos;
out vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

uniform uint HALF_PARTICLE_COUNT;
// rgb(234, 67, 54)
vec3 RED = vec3(234.0 / 255.0, 67.0 / 255.0, 54.0 / 255.0);
// rgb(66, 132, 244)
vec3 BLUE = vec3(66.0 / 255.0, 132.0 / 255.0, 244.0 / 255.0);

void main() {
    vec3 offset = particlePosition[gl_InstanceID].xyz;
    vec3 pos = vec3(model * vec4(aPos, 1.0)); 
    Normal = normalize(normalMatrix * aNormal);
    FragPos = pos;
    gl_Position = projection * view * vec4(pos + offset, 1.0);

    if (gl_InstanceID < HALF_PARTICLE_COUNT) {
        aColor = RED;
    } else {
        aColor = BLUE;
    }
}