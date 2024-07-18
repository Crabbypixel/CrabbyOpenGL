#ifdef SHADER_VERTEX

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;

out vec3 vNormal;
out vec3 vFragPos;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

void main()
{
	gl_Position = matProjection * matView * matModel * vec4(vPos, 1.0f);

	vNormal = mat3(transpose(inverse(matModel))) * vNorm;
	vFragPos = vec3(matModel * vec4(vPos, 1.0f));
}

#endif

#ifdef SHADER_FRAGMENT

struct PointLight
{
	vec3 vPosition;
	vec3 vLightColor;

	float fConstant;
	float fLinear;
	float fQuadratic;

	vec3 vAmbient;
	vec3 vDiffuse;
	vec3 vSpecular;
};

struct SpotLight
{
	vec3 vPosition;
	vec3 vDirection;
	vec3 vLightColor;

	vec3 vAmbient;
	vec3 vDiffuse;
	vec3 vSpecular;

	float fCutOff;
	float fOuterCutOff;

	float fConstant;
	float fLinear;
	float fQuadratic;
};

#define NR_POINT_LIGHTS 1

in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

uniform vec3 vViewPos;
uniform vec3 vMaterialColor;

uniform PointLight pointlights[NR_POINT_LIGHTS];
uniform SpotLight spotlight;

uniform sampler2D texture_diffuse1;

vec3 CalcPointLight(PointLight light, vec3 vNormal, vec3 vFragPos, vec3 vViewDir);
vec3 CalcSpotLight(SpotLight light, vec3 vNormal, vec3 vFragPos, vec3 vViewDir);

void main()
{
	vec3 vNorm = normalize(vNormal);
	vec3 vViewDir = normalize(vViewPos - vFragPos);

	vec3 vResult = vec3(0.0f, 0.0f, 0.0f);

	// Point lights
	vResult += CalcPointLight(pointlights[0], vNorm, vFragPos, vViewDir);

	// Spot light
	vResult += CalcSpotLight(spotlight, vNorm, vFragPos, vViewDir);

	FragColor = vec4(vResult, 1.0f);
}

vec3 CalcPointLight(PointLight light, vec3 vNormal, vec3 vFragPos, vec3 vViewDir)
{
	vec3 vLightDir = normalize(light.vPosition - vFragPos);

	// Ambient shading
	vec3 vAmbient = light.vAmbient * light.vLightColor * vMaterialColor;

	// Diffuse shading
	float fDiff = max(dot(vNormal, vLightDir), 0.0f);
	vec3 vDiffuse = light.vDiffuse * light.vLightColor * fDiff * vMaterialColor;

	// Specular shading
	vec3 vReflectDir = reflect(-vLightDir, vNormal);
	float fSpec = pow(max(dot(vViewDir, vReflectDir), 0.0f), 128);
	vec3 vSpecular = light.vSpecular * light.vLightColor * fSpec * vMaterialColor;

	// Attenuation
	//float fDistance = length(light.vPosition - vFragPos);
	//float fAttenuation = 1.0f / (light.fConstant + light.fLinear * fDistance + light.fQuadratic * fDistance * fDistance);

	float fAttenuation = 1.0f;

	// We'll leave out attenuating the ambient shading
	vDiffuse = vDiffuse * fAttenuation;
	vSpecular = vSpecular * fAttenuation;

	// Return the combined shading
	return (vAmbient + vDiffuse + vSpecular);
}

vec3 CalcSpotLight(SpotLight light, vec3 vNormal, vec3 vFragPos, vec3 vViewDir)
{
	vec3 vLightDir = normalize(light.vPosition - vFragPos);

	// Ambient shading
	vec3 vAmbient = light.vAmbient * light.vLightColor * vMaterialColor;

	// Diffuse shading
	float fDiff = max(dot(vLightDir, vNormal), 0.0f);
	vec3 vDiffuse = light.vDiffuse * light.vLightColor * fDiff * vMaterialColor;

	// Specular shading
	vec3 vReflectDir = reflect(-vLightDir, vNormal);
	float fSpec = pow(max(dot(vViewDir, vReflectDir), 0.0f), 128);
	vec3 vSpecular = light.vSpecular * light.vLightColor * fSpec * vMaterialColor;

	// SpotLight
	float fTheta = dot(vLightDir, normalize(-light.vDirection));
	float fEpsilion = light.fCutOff - light.fOuterCutOff;
	float fIntensity = clamp((fTheta - light.fOuterCutOff) / fEpsilion, 0.0f, 1.0f);
	vDiffuse = vDiffuse * fIntensity;
	vSpecular = vSpecular * fIntensity;

	// Attenuation
	float fDistance = length(light.vPosition - vFragPos);
	float fAttenuation = 1.0f / (light.fConstant + light.fLinear * fDistance + light.fQuadratic * (fDistance * fDistance));
	
	vAmbient = vAmbient * fAttenuation;
	vDiffuse = vDiffuse * fAttenuation;
	vSpecular = vSpecular * fAttenuation;

	return (vAmbient + vDiffuse + vSpecular);
}

#endif