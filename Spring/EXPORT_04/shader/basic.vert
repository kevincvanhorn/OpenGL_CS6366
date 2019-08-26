#version 330 core

// Vertex Inputs
layout(location = 0) in vec3 vPosition_modelspace;

smooth out vec3 pixelPos;

uniform mat4 MVP;

void main(){
	mat4 mMatrix = mat4(1.0f);

	pixelPos = vec3(mMatrix * vec4(vPosition_modelspace, 1.0));

	// Set vertex position:
    gl_Position = MVP * vec4(vPosition_modelspace, 1);
}