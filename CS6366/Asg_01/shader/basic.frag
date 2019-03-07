#version 330 core

//in vec3 ourColor; // Color of the object
in vec3 pixelPos;
smooth in vec3 pixelNormalS;
flat in vec3 pixelNormalF;

uniform bool bUseSmooth;

uniform vec3 camPos;

uniform vec3 dLightDiffuse;
uniform vec3 dLightSpecular;
uniform vec3 dLightAmbient;

uniform vec3 pLightDiffuse;
uniform vec3 pLightSpecular;
uniform vec3 pLightAmbient;
uniform vec3 pLightloc;

uniform vec3 modelColor;

out vec4 color; // Color output from the frag shader.

void main(){
	vec3 pixelNormal;
	if(bUseSmooth == true){
		pixelNormal = pixelNormalS;
	}
	else{
		pixelNormal = pixelNormalS;
	}

	vec3 dDirection = vec3(0, 1, 1);
	vec3 pDirection = normalize(pLightloc - pixelPos);
	dDirection = normalize(dDirection);
	float shininess = 32;

	// Ambient
	vec3 ambient = dLightAmbient * 1.0f;

	vec3 pAmbient = pLightAmbient * 1.0f;
	ambient += pAmbient;

	// Diffuse
	vec3 diffuse = max(dot(pixelNormal, dDirection), 0.0f) * dLightDiffuse;

	vec3 pDiffuse = max(dot(pixelNormal, pDirection), 0.0f) * pLightDiffuse;
	diffuse += pDiffuse;

	// Specular
	vec3 viewVector = normalize(camPos - pixelPos);
	vec3 h = normalize(dDirection + viewVector);
	vec3 reflectVector = reflect(-1*dDirection, pixelNormal);
	float fSpecular = pow(dot(h, pixelNormal), shininess);
	vec3 specular = dLightSpecular * fSpecular;
	
	vec3 pH = normalize(pDirection + viewVector);
	vec3 pReflectVector = reflect(-1*pDirection, pixelNormal);
	float pFSpecular = pow(dot(pH, pixelNormal), shininess);
	vec3 pSpecular = pLightSpecular * pFSpecular;
	specular += pSpecular;

    color = vec4((ambient + diffuse + specular)*modelColor, 1.0f);
}