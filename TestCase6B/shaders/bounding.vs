#version 330 core

uniform mat4 matrix;
uniform bool isLeft;

layout (location = 0) in vec3 aPos;
out vec4 ClipPos;

void main()
{
	gl_Position =matrix * vec4(aPos, 1.0f);
	ClipPos=vec4(1,1,1,1);

	vec4 VertNDC = gl_Position / gl_Position.w;
	if (isLeft)
	{
		VertNDC.x = (VertNDC.x - 1.0) / 2.0;
	} 
	else
	{
		VertNDC.x = (VertNDC.x + 1.0) / 2.0;
	}
	ClipPos = VertNDC * gl_Position.w;
	gl_Position = ClipPos;
}