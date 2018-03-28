#version 330 core

uniform mat4 matrix;

layout (location = 0) in vec3 aPos;
 
void main()
{
	gl_Position =matrix * vec4(aPos, 1.0f);
}