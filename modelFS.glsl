#version 460 core

#define MAX_LIGHTS 16

struct Light {
    int type;       // e.g. 0=none, 1=directional, 2=point, 3=spot
    vec3 position;  // for point/spot
    vec3 direction; // for directional/spot
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant; // for point/spot
    float linear;   // for point/spot
    float quadratic;    // for point/spot
    float cutOff;       // for spot
    float outerCutOff;  // for spot
};

layout(std140, binding = 0) uniform LightsBlock {
    int numLights;
    //int padding[3];
    Light lights[MAX_LIGHTS];
};

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
    int hasTexture;
};

in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragTexCoords;

uniform Material material;
uniform sampler2D texture_diffuse1;

uniform vec3 viewPos;   // camera position in world
uniform float ambientOcclusion;

out vec4 FragColor;

vec4 CalcLightContribution(Light light, vec3 norm, vec3 viewDir, vec4 diffuseColor) {
    vec4 result = vec4(0.0);
    if(light.type == 1) {
        // directional light
        vec3 lightDir = normalize(-light.direction); // direction from fragment to light source
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);    // Blinn-Phong specular
        float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
        result = light.ambient * material.ambient * ambientOcclusion +
                 light.diffuse * diff * diffuseColor +
                 light.specular * spec * material.specular;
    } else if(light.type == 2) {
        // point light
        vec3 lightDir = normalize(light.position - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        result = (light.ambient * material.ambient +
                  light.diffuse * diff * diffuseColor +
                  light.specular * spec * material.specular) * attenuation;
    } else if(light.type == 3) {
        // spotlight
        vec3 lightDir = normalize(light.position - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        result = (light.ambient * material.ambient +
                  light.diffuse * diff * diffuseColor +
                  light.specular * spec * material.specular) * attenuation * intensity;
    }
    return result;
}

void main() {
    vec4 diffuseColor = (material.hasTexture == 1) ? texture(texture_diffuse1, fragTexCoords) : material.diffuse;
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec4 finalColor = vec4(0.0);
    for(int i = 0; i < numLights; i++) {
        finalColor += CalcLightContribution(lights[i], norm, viewDir, diffuseColor);
    }

    FragColor = finalColor;
}