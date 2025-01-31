#version 460 core
in vec3 fragNormal;
in vec2 fragTexCoords;

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float reflectivity;
    int hasTexture;
};

uniform Material material;
uniform sampler2D texture_diffuse1;

out vec4 FragColor;

void main() {
    vec3 norm = normalize(fragNormal);

    // basic directional light from above
    vec3 lightDir = normalize(vec3(0.0, 1.0, 1.0));
    float diff = max(dot(norm, lightDir), 0.0);

    vec4 diffuseColor = (material.hasTexture == 1) ? texture(texture_diffuse1, fragTexCoords) : material.diffuse;
    vec4 color = material.ambient + diffuseColor * diff;

    FragColor = color;
}