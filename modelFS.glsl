#version 460 core
in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragTexCoords;

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
    int hasTexture;
};

struct DirLight {
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

uniform Material material;
uniform DirLight dirLight;
uniform sampler2D texture_diffuse1;

uniform vec3 viewPos;   // camera position in world
uniform float ambientOcclusion;

out vec4 FragColor;

vec4 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec4 diffuseColor) {
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(-light.direction); // direction from fragment to light source

    float diff = max(dot(norm, lightDir), 0.0);
    
    // Blinn-Phong specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);

    vec4 ambient = light.ambient * material.ambient * ambientOcclusion;
    vec4 diffuse = light.diffuse  * diff * diffuseColor;
    vec4 specular = light.specular * spec * material.specular;
    return ambient + diffuse + specular;
}

void main() {
    vec4 diffuseColor = (material.hasTexture == 1) ? texture(texture_diffuse1, fragTexCoords) : material.diffuse;
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos); // Replace with actual fragment world position

    vec4 result = calcDirLight(dirLight, norm, viewDir, diffuseColor);
    FragColor = result;
}