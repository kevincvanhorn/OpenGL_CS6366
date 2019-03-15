#version 330 core

// Vertex Inputs
layout(location = 0) in vec3 vPosition_modelspace;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;


out vec3 pixelPos;
out vec2 outTex;
smooth out vec3 pixelNormalS;
flat out vec3 pixelNormalF;

uniform mat4 MVP;

void main(){
	mat4 mMatrix = mat4(1.0f);

	pixelPos = vec3(mMatrix * vec4(vPosition_modelspace, 1.0));

	pixelNormalS = mat3(transpose(inverse(mMatrix))) * normal;
	pixelNormalF = mat3(transpose(inverse(mMatrix))) * normal;

	// Set vertex position:
    gl_Position = MVP * vec4(vPosition_modelspace, 1);

    outTex = texCoord;
}