#version 330 core

layout(location = 0) in vec3 vPosition_modelspace;
layout (location = 1) in vec3 color;

uniform mat4 MVP;
out vec3 ourColor;

void main(){
 

	// Set vertex position:
    gl_Position = MVP * vec4(vPosition_modelspace, 1);

   
    ourColor = color;
}