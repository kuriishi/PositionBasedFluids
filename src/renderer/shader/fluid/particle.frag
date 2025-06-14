#version 430 core

in vec3 vCenterPosViewSpace;
in float vDensity;

layout(location = 0) out vec4 fColor;
layout(location = 1) out vec4 fPosition;
layout(location = 2) out vec4 fNormal;

uniform mat4 uProjection;
uniform mat4 uViewInverse;
uniform mat4 uViewTranspose;
// (width, height)
uniform ivec2 uScreenSize;
uniform vec3 uFluidColor;
uniform float uPointSize;
uniform float uMinimumDensity;

struct Light {
    vec3 direction;
    vec3 color;
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};
uniform Light uLight;

const vec2 POINT_CENTER = vec2(0.5, 0.5);

void main() {
    if (vDensity < uMinimumDensity) {
        discard;
    }
    vec2 pointCoord = (gl_PointCoord - POINT_CENTER) * vec2(2.0, -2.0);
    float xySquaredLength = dot(pointCoord, pointCoord);
    if (xySquaredLength > 1.0) {
        discard;
    }
    vec3 normalViewSpace = normalize(vec3(pointCoord, sqrt(1.0 - xySquaredLength)));
    vec3 normalWorldSpace = normalize((uViewTranspose * vec4(normalViewSpace, 0.0)).xyz);
    vec3 posViewSpace = vCenterPosViewSpace + normalViewSpace * uPointSize;
    vec3 posWorldSpace = (uViewInverse * vec4(posViewSpace, 1.0)).xyz;
    vec4 posClipSpace = uProjection * vec4(posViewSpace, 1.0);

    float fragDepth = (posClipSpace.z / posClipSpace.w) * 0.5 + 0.5;

    vec3 lightDir = normalize(uLight.direction);
    vec3 viewDir = normalize(-posViewSpace);
    vec3 halfDir = normalize(viewDir + lightDir);

    vec3 ambient = uLight.ambient * uLight.color;

    float diff = max(dot(normalViewSpace, lightDir), 0.0);
    vec3 diffuse = uLight.diffuse * diff * uLight.color;

    float spec = pow(max(dot(normalViewSpace, halfDir), 0.0), uLight.shininess);
    vec3 specular = uLight.specular * spec * uLight.color;

    vec3 result = (ambient + diffuse + specular) * uFluidColor;

    fColor = vec4(result, 1.0);
    fPosition = vec4(posWorldSpace, 1.0);
    fNormal = vec4(normalWorldSpace, 1.0);
    gl_FragDepth = fragDepth;
}
