#version 430 core
out vec4 FragColor;

struct Material {
    vec3 diffuse0;
    vec3 specular0;
    float shininess0;
}; 

struct DirLight {
    vec3 direction;
    vec3 color;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 ClipPos;
in flat int ID;

uniform vec3 viewPos;
uniform Material material;
uniform sampler2D texture_diffuse1;

DirLight dirLight;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{    

    vec4 NDC = ClipPos / ClipPos.w;

	if (ID == 0)
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

    dirLight.direction=vec3(-1,-1,-1);
    dirLight.color=vec3(1.0,1.0,1.0);
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);  
    
    FragColor = vec4(result, 1.0);
    //FragColor = vec4(0.0,1.0,0.0, 1.0);

}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess0);
    // combine results
    vec3 ambient = vec3(0.1,0.1,0.1);
    vec3 diffuse = light.color * diff * material.diffuse0;
    vec3 specular = light.color * spec * material.specular0;
    //return (ambient + diffuse + specular);
    return (ambient + diffuse);    
}