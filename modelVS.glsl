#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoords;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
	fragPos = worldPos.xyz;
	fragNormal = mat3(transpose(inverse(model))) * aNormal;
	fragTexCoords = aTex;

	gl_Position = projection * view * worldPos;
}