#ifdef SHADER_VERTEX

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

out vec3 vNormal;
out vec3 vFragPos;

void main()
{
	gl_Position = matProjection * matView * matModel * vec4(vPos, 1.0f);

	vFragPos = vec3(matModel * vec4(vPos, 1.0f));
	vNormal = mat3(transpose(inverse(matModel))) * vNorm;
}
#endif

#ifdef SHADER_FRAGMENT

struct Material
{
	vec3 vColor;

	float fShininess;
};

struct DirLight
{
	vec3 vDirection;
	vec3 vLightColor;

	vec3 vAmbient;
	vec3 vDiffuse;
};

struct PointLight
{
	vec3 vPosition;
	vec3 vLightColor;
	
	float fConstant;
	float fLinear;
	float fQuadratic;

	vec3 vAmbient;
	vec3 vDiffuse;
};

struct SpotLight
{
	vec3 vPosition;
	vec3 vDirection;
	vec3 vLightColor;

	vec3 vAmbient;
	vec3 vDiffuse;

	float fCutOff;
	float fOuterCutOff;

	float fConstant;
	float fLinear;
	float fQuadratic;
};

// Uniforms are indicated by the 'u_' prefix

#define	NR_POINT_LIGHTS 1
uniform DirLight u_dirLight;
uniform PointLight u_pointLights[NR_POINT_LIGHTS];
uniform SpotLight u_spotLight;

uniform vec3 u_vViewPos;
uniform Material u_material;

in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 vNormal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 vNormal, vec3 vFragPos, vec3 vViewDir);
vec3 CalcSpotLight(SpotLight light, vec3 vNormal, vec3 vFragPos, vec3 vViewDir);

void main()
{
	vec3 vNorm = normalize(vNormal);
	vec3 vViewDir = normalize(u_vViewPos - vFragPos);

	vec3 vResult = vec3(0.0f, 0.0f, 0.0f);

	// Directional lighting
	//vResult += CalcDirLight(u_dirLight, vNorm, vViewDir);

	// Point lights
	for(int i=0; i<NR_POINT_LIGHTS; i++)
	{
		vResult += CalcPointLight(u_pointLights[i], vNorm, vFragPos, vViewDir);
	}

	vResult += CalcSpotLight(u_spotLight, vNorm, vFragPos, vViewDir);

	FragColor = vec4(vResult, 1.0f);
}

vec3 CalcSpotLight(SpotLight light, vec3 vNormal, vec3 vFragPos, vec3 vViewDir)
{
	vec3 vLightDir = normalize(light.vPosition - vFragPos);

	// Ambient shading
	vec3 vAmbient = light.vAmbient * light.vLightColor * u_material.vColor;

	// Diffuse shading
	float fDiff = max(dot(vLightDir, vNormal), 0.0f);
	vec3 vDiffuse = light.vDiffuse * light.vLightColor * fDiff * u_material.vColor;

	// SpotLight
	float fTheta = dot(vLightDir, normalize(-light.vDirection));
	float fEpsilion = light.fCutOff - light.fOuterCutOff;
	float fIntensity = clamp((fTheta - light.fOuterCutOff) / fEpsilion, 0.0f, 1.0f);
	vDiffuse = vDiffuse * fIntensity;

	// Attenuation
	float fDistance = length(light.vPosition - vFragPos);
	float fAttenuation = 1.0f / (light.fConstant + light.fLinear * fDistance + light.fQuadratic * (fDistance * fDistance));
	
	vAmbient = vAmbient * fAttenuation;
	vDiffuse = vDiffuse * fAttenuation;

	return (vAmbient + vDiffuse);
}

vec3 CalcPointLight(PointLight light, vec3 vNormal, vec3 vFragPos, vec3 vViewDir)
{
	vec3 vLightDir = normalize(light.vPosition - vFragPos);

	// Ambient shading
	vec3 vAmbient = light.vAmbient * light.vLightColor * u_material.vColor;

	// Diffuse shading
	float fDiff = max(dot(vNormal, vLightDir), 0.0f);
	vec3 vDiffuse = light.vDiffuse * light.vLightColor * fDiff * u_material.vColor;

	// Attenuation
	float fDistance = length(light.vPosition - vFragPos);
	float fAttenuation = 1.0f / (light.fConstant + light.fLinear * fDistance + light.fQuadratic * (fDistance * fDistance));

	// We'll leave out attenuating the ambient shading
	vDiffuse = vDiffuse * fAttenuation;

	return (vAmbient + vDiffuse);
}

vec3 CalcDirLight(DirLight light, vec3 vNormal, vec3 vViewDir)
{
	vec3 vLightDir = normalize(-light.vDirection);

	// Ambient shading
	vec3 vAmbient = light.vAmbient * light.vLightColor * u_material.vColor;

	// Diffuse shading
	float fDiff = max(dot(vNormal, vLightDir), 0.0f);
	vec3 vDiffuse = light.vDiffuse * light.vLightColor * fDiff * u_material.vColor;

	vec3 vReflectDir = reflect(-vLightDir, vNormal);
	float fSpec = pow(max(dot(vViewDir, vReflectDir), 0.0f), u_material.fShininess);

	return (vAmbient + vDiffuse);
}

#endif