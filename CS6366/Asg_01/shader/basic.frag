#version 330 core

smooth in vec3 pixelPos;

uniform sampler3D texMap;
uniform sampler2D texColorMap;
uniform vec3 emissiveColor;
uniform vec3 modelColor;

uniform float s0;
uniform float s1;
uniform float s2;
uniform float s3;
uniform float s4;
uniform float s5;	
uniform float s6;
uniform float s7;

uniform vec2 resolution;

uniform float sREL;

uniform bool bUseTransfer;

out vec4 color; // Color output from the frag shader.

void main(){
	float i = texture(texMap, vec3(1,1,1) - pixelPos).r;
	
	float i2 = i;
	if(i <= .01f) i2 = .01f;
	else if(i>=.9f) i2 = 0.9f;

	vec4 c = texture(texColorMap, vec2(i2,0));

	//color = texture(texColorMap, vec2(0.2f,0));
	

	if(!bUseTransfer){
		color = vec4(modelColor, 1.0f);
		return;
	}


	float alpha = 1;
	if(i <= (1.0f/7.0f) && i >= 0.0f){
		alpha = s0 + (s1-s0)*7.0f*(i-(0.0f/7.0f));
	}
	else if(i > 1.0f/7.0f && i <= 2.0f/7.0f){
		alpha = s1 + (s2-s1)*7.0f*(i-(1.0f/7.0f));
	}
	else if(i > 2.0f/7.0f && i <= 3.0f/7.0f){
		alpha = s2 + (s3-s2)*7.0f*(i-(2.0f/7.0f));
	}
	else if(i > 3.0f/7.0f && i <= 4.0f/7.0f){
		alpha = s3 + (s4-s3)*7.0f*(i-(3.0f/7.0f));
	}
	else if(i > 4.0f/7.0f && i <= 5.0f/7.0f){
		alpha = s4 + (s5-s4)*7.0f*(i-(4.0f/7.0f));
	}
	else if(i > 5.0f/7.0f && i <= 6.0f/7.0f){
		alpha = s5 + (s6-s5)*7.0f*(i-(5.0f/7.0f));
	}
	else if(i > 6.0f/7.0f && i <= 1.0f){
		alpha = s6 + (s7-s6)*7.0f*(i-(6.0f/7.0f));
	}
	else{
		alpha = 0.0f;
	}

	alpha = 1 - pow((1 - alpha), (sREL));
	color = vec4(c.xyz,alpha);
}