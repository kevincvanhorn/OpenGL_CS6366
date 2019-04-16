#version 330 core

smooth in vec3 pixelPos;

uniform sampler3D texMap;
uniform vec3 emissiveColor;
uniform vec3 modelColor;


out vec4 color; // Color output from the frag shader.

void main(){

	vec4 a = texture(texMap, pixelPos);
	//float alpha = a.r;

	//color = vec4(modelColor, alpha);

	//vec4 c4 = vec4(modelColor, 1.0f);
	//color = c4*alpha;//vec4(modelColor, 1.0f);
	color = texture(texMap, vec3(1,1,1) - pixelPos).rrrr;
}