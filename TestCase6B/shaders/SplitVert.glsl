#version 450 

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec2 TexCoord;

out vec2 Tex;

void main()
{
	gl_Position = vec4 (Pos, 1.0);
	Tex = TexCoord;
}