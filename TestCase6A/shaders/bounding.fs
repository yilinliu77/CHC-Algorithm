#version 330 core
out vec4 FragColor;

uniform bool isInstanced;
uniform bool isLeft;

in vec4 ClipPos;

void main()
{    
    FragColor = vec4(1.0,1.0,1.0, 1.0);

    vec4 NDC = ClipPos / ClipPos.w;
    if (isLeft)
    {
        if (NDC.x > 0)
        {
            discard;
        }
    }
    else
    {
        if (NDC.x < 0)
        {
            discard;
        }
    } 

}