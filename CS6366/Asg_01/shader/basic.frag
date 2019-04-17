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

out vec4 color; // Color output from the frag shader.

void main(){

	//vec4 a = texture(texMap, pixelPos);
	//float alpha = a.r;

	//color = vec4(modelColor, alpha);

	//vec4 c4 = vec4(modelColor, 1.0f);
	//color = c4*alpha;//vec4(modelColor, 1.0f);
	float i = texture(texMap, vec3(1,1,1) - pixelPos).r;

	vec4 c = texture(texColorMap, vec2(i*255-1,0));
	//color = texture(texMap, vec3(1,1,1) - pixelPos).rrrr;

	float alpha = 0;

	if(i <= (1/7)){
		alpha = s0 + i*(s1-s0);
	}
	else if(i > 1/7 && i <= 2/7){
		alpha = s1 + i*(s2-s1);
	}
	else if(i > 2/7 && i <= 3/7){
		alpha = s2 + i*(s3-s2);
	}
	else if(i > 3/7 && i <= 4/7){
		alpha = s3 + i*(s4-s3);
	}
	else if(i > 4/7 && i <= 5/7){
		alpha = s4 + i*(s5-s4);
	}
	else if(i > 5/7 && i <= 6/7){
		alpha = s5 + i*(s6-s5);
	}
	else{
		alpha = s6 + i*(s7-s6);
	}

	color = vec4(c.x,c.y,c.z,alpha);
	//color = vec4(modelColor, 1.0f);
}