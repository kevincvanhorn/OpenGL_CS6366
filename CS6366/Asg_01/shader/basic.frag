#version 330 core
in vec3 pixelPos;
in vec3 texCoord;

uniform vec3 camPos;

uniform vec3 modelColor;
uniform vec3 emmisiveColor;

// Texture sampler:
uniform sampler3D TexMap3D;

out vec4 color; // Color output from the frag shader.

void main(){
	//float a = texture(TexMap3D, texCoord); // opacity

	//color = vec4(emmisiveColor, a);
	color = vec4(modelColor, 1.0f);
}