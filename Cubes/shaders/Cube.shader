#shader vertex
#version 330 core

// Phong shading

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

out vec3 vNormal;
out vec3 vFragPos;
out vec2 TexCoords;

void main()
{
	gl_Position = matProjection * matView * matModel * vec4(vPos, 1.0f);

	vFragPos = vec3(matModel * vec4(vPos, 1.0f));				// Fragment pos in view space
	vNormal = mat3(transpose(inverse(matModel))) * vNorm;
	TexCoords = aTexCoords;
}

#shader fragment
#version 330 core

struct Material
{
	sampler2D diffuse;
	sampler2D specular;

	float fShininess;
};

struct Light
{
	vec3 vPosition;
	//vec3 vDirection;

	vec3 vLightColor;
	vec3 vAmbient;
	vec3 vDiffuse;
	vec3 vSpecular;
};

uniform vec3 vViewPos;
uniform Material material;
uniform Light light;

in vec3 vNormal;
in vec3 vFragPos;
in vec2 TexCoords;

out vec4 FragColor;

void main()
{
	vec3 vObjectColor = vec3(1.0f, 0.5f, 0.31f);

	// Ambient lighting

	vec3 vAmbient = light.vAmbient * texture(material.diffuse, TexCoords).rgb;
	//vec3 vAmbient = light.vAmbient * vObjectColor * 0.2f;

	// Diffuse lighting
	vec3 vNorm = normalize(vNormal);
	vec3 vLightDir = normalize(light.vPosition - vFragPos);										// Vector pointing from fragment to the light source
	//vec3 vLightDir = normalize(-light.vDirection);
	float fDiff = max(dot(vNorm, vLightDir), 0.0f);

	vec3 vDiffuse = light.vDiffuse * fDiff * texture(material.diffuse, TexCoords).rgb;
	//vec3 vDiffuse = light.vDiffuse * fDiff * vObjectColor;

	// Specular lighting
	vec3 vViewDir = normalize(vViewPos - vFragPos);												// Vector pointing from fragment to the camera (camera at [0, 0, 0])
	vec3 vReflectDir = reflect(-vLightDir, vNorm);												// Reflection vector (negate the vLightDir as it's pointing the other way around)
	float fSpec = pow(max(dot(vViewDir, vReflectDir), 0.0f), material.fShininess);				// Dot of reflection vector and view vector
	
	vec3 vSpecular = light.vSpecular * fSpec * texture(material.specular, TexCoords).rgb;
	//vec3 vSpecular = light.vSpecular * fSpec * vObjectColor;

	vec3 vResult = vAmbient + vDiffuse + vSpecular;			
	FragColor = vec4(vResult, 1.0f);
}