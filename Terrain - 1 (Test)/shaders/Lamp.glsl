#ifdef SHADER_VERTEX

layout (location = 0) in vec3 vPos;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

void main()
{
	gl_Position = matProjection * matView * matModel * vec4(vPos, 1.0f);
}
#endif

#ifdef SHADER_FRAGMENT

uniform vec3 vLampColor;
out vec4 FragColor;

void main()
{
	FragColor = vec4(vLampColor, 1.0f);
}
#endif