#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;
layout (location = 7) in vec3 aInstanceColor;

out vec2 TexCoords;
out vec3 pointColor;

uniform mat4 view;
uniform mat4 projection;


void main()
{
    TexCoords = aTexCoords;  
	pointColor = aInstanceColor; 
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
}
