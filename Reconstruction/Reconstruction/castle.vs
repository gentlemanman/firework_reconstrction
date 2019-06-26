#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (location = 8) in vec3 aAmbient;
layout (location = 9) in vec3 aDiffuse;
layout (location = 10) in vec3 aSpecular;

/*
uniform Mat{
	vec4 aAmbient;
	vec4 aDiffuse;
	vec4 aSpecular;
};
*/

out vec3 FragPos;
out vec3 Normal;

out vec4 Ambient;
out vec4 Diffuse;
out vec4 Specular;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
	Ambient = vec4(aAmbient, 1.0);
	Diffuse = vec4(aDiffuse, 1.0);
	Specular = vec4(aSpecular, 1.0);
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}