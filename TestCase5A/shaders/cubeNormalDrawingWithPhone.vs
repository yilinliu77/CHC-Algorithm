#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec3 FragPos;  
out vec2 TexCoords;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uHMDMatrix;

void main()
{
	gl_Position = uProjectionMatrix * uViewMatrix * uHMDMatrix * vec4(aPos, 1.0f);
	FragPos = vec3(uHMDMatrix * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(uHMDMatrix))) * aNormal;

}