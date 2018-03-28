#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec3 FragPos;  
out vec2 TexCoords;
out flat int ID;
out vec4 ClipPos;

uniform mat4 VPMatrix[4];
uniform mat4 uHMDMatrix;

void main()
{
	vec4 Vert = VPMatrix[2 + gl_InstanceID]  * VPMatrix[gl_InstanceID] * uHMDMatrix * vec4(aPos, 1.0);
	vec4 VertNDC = Vert / Vert.w;

	if (gl_InstanceID == 0)
	{
		VertNDC.x = (VertNDC.x - 1.0) / 2.0;
	}
	else
	{
		VertNDC.x = (VertNDC.x + 1.0) / 2.0;
	}

	ClipPos = VertNDC * Vert.w;
	ID = gl_InstanceID;

	gl_Position = ClipPos;
	FragPos = vec3(uHMDMatrix * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(uHMDMatrix))) * aNormal;

}