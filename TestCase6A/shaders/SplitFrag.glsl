#version 450 

layout (location = 0) out vec4 LeftColor;
layout (location = 1) out vec4 RightColor;

in vec2 Tex;

uniform sampler2D uTex;

void main()
{
	LeftColor = texture(uTex, vec2(float(Tex.x/2.0), Tex.y));
	RightColor = texture(uTex, vec2(float((Tex.x + 1.0) / 2.0),  Tex.y));
}