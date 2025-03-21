#version 430 core

layout (location = 0) in vec4 aPos;

out vec3 viewSpacePos;
out vec3 aColor;

uniform mat4 view;
uniform mat4 projection;

uniform float POINT_SIZE;
// scaler should be a function relative to camera's aspect and fovy and screen's size,
// cause I do not change this parameters and I am lazy,
// so it's fine to make it a constant
float POINT_SIZE_SCALER = 2000;

uniform uint HALF_PARTICLE_COUNT;
// rgb(234, 67, 54)
vec3 RED = vec3(234.0 / 255.0, 67.0 / 255.0, 54.0 / 255.0);
// rgb(66, 132, 244)
vec3 BLUE = vec3(66.0 / 255.0, 132.0 / 255.0, 244.0 / 255.0);

void main() {
    gl_Position = projection * view * vec4(aPos.xyz, 1.0);

    viewSpacePos = (view * vec4(aPos.xyz, 1.0)).xyz;
    gl_PointSize = POINT_SIZE_SCALER * POINT_SIZE / (-viewSpacePos.z);

    if (gl_VertexID < HALF_PARTICLE_COUNT) {
        aColor = RED;
    }
    else {
        aColor = BLUE;
    }
}
