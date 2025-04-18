#version 430 core

in vec3 aPosViewSpace;
in float aDensity;

out vec4 FragColor;

vec2 POINT_CENTER = vec2(0.5, 0.5);
uniform mat4 projection;
uniform mat4 viewMatrixTranspose;

uniform float POINT_SIZE;
uniform float MINIMUN_DENSITY;

struct Light {
    vec3 direction;
    vec3 color;
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};
uniform Light light;
uniform vec3 viewPos;

uniform vec3 fluidColor;

void main() {
    if (aDensity < MINIMUN_DENSITY) {
        discard;
    }
    vec2 pointCoord = (gl_PointCoord - POINT_CENTER) * vec2(2.0, -2.0);
    float xySquaredLength = dot(pointCoord, pointCoord);
    if (xySquaredLength > 1.0) {
        discard;
    }
    vec3 normalViewSpace = vec3(pointCoord, sqrt(1.0 - xySquaredLength));
    vec3 normalWorldSpace = (viewMatrixTranspose * vec4(normalViewSpace, 0.0)).xyz;
    vec3 fragPosViewSpace = aPosViewSpace + normalViewSpace * POINT_SIZE;
    vec4 fragPosClipSpace = projection * vec4(fragPosViewSpace, 1.0);

    float fragDepth = (fragPosClipSpace.z / fragPosClipSpace.w) * 0.5 + 0.5;
    gl_FragDepth = fragDepth;


    vec3 lightDir = normalize(light.direction);
    vec3 viewDir = normalize(-fragPosViewSpace);
    vec3 halfDir = normalize(viewDir + lightDir);

    vec3 ambient = light.ambient * light.color;

    float diff = max(dot(normalViewSpace, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * light.color;

    float spec = pow(max(dot(normalViewSpace, halfDir), 0.0), light.shininess);
    vec3 specular = light.specular * spec * light.color;

    vec3 result = (ambient + diffuse + specular) * fluidColor;

    FragColor = vec4(result, 1.0);
}
