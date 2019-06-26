#version 330 core
out vec4 FragColor;

// Material
in vec4 Ambient;
in vec4 Diffuse;
in vec4 Specular;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 color;

void main()
{    
	vec3 ambient = Diffuse.rgb * 0.5;
	
	vec3 result = ambient;
	FragColor = vec4(result, 1.0);
    //FragColor = texture(texture_diffuse1, TexCoords);
	//vec3 red = vec3(0.2, 0.3, 0.0);
	//FragColor = vec4(red, 1.0);
}