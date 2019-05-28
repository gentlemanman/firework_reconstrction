#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 pointColor;

uniform sampler2D texture_diffuse1;
//uniform vec3 pointColor; 

void main()
{    
    // FragColor = texture(texture_diffuse1, TexCoords);
	vec3 mColor = vec3(1.0f, 0.5f, 0.31f);
	FragColor = vec4(pointColor, 1.0);
	// FragColor = vec4(mColor, 1.0);
}