#version 430 core

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

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
uniform vec3 objectColor;

void main() {
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.direction);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfDir = normalize(viewDir + lightDir);

    vec3 ambient = light.ambient * light.color;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * light.color;

    float spec = pow(max(dot(normal, halfDir), 0.0), light.shininess);
    vec3 specular = light.specular * spec * light.color;

    vec3 result = (ambient + diffuse + specular) * objectColor;

    FragColor = vec4(result, 1.0f);

    // FragColor = vec4(normal, 1.0f);
}
