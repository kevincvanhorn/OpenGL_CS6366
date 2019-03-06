#version 330 core

in vec3 ourColor; // Color of the object

//uniform vec3 pLightColor;
//uniform vec3 dLightColor;
uniform vec3 modelColor;


out vec4 color; // Color output from the frag shader.

void main(){
    color = vec4(modelColor, 1.0f);
}