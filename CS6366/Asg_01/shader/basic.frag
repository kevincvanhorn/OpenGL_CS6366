#version 330 core
in vec3 pixelPos;
in vec2 texCoord;
flat in vec3 pixelNormalF;

uniform bool bDiffuseTex;
uniform bool bNormalTex;

uniform vec3 camPos;

uniform vec3 modelColor;

// Texture sampler:
uniform sampler2D TexDiffuse;
uniform sampler2D TexNormal;

out vec4 color; // Color output from the frag shader.

void main(){
	color = vec4(modelColor, 1.0f);
}