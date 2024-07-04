#shader vertex
#version 330 core

layout (location = 0) in vec3 vPos;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

void main()
{
	gl_Position = matProjection * matView * matModel * vec4(vPos, 1.0f);
}

#shader fragment
#version 330 core

uniform vec3 vLampColor;
out vec4 FragColor;

void main()
{
	FragColor = vec4(vLampColor, 1.0f);
}