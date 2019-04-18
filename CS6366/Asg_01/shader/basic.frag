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

out vec4 color; // Color output from the frag shader.

void main(){
	float sRate0 = 15;
	float i = texture(texMap, vec3(1,1,1) - pixelPos).r;

	//vec4 c = texture(texColorMap, vec2(i/resolution.x,i/resolution.y));

	//if(i == 255) {color = vec4(1,1,1,1);}
	//else{color = vec4(0);}

	float alpha = 1;

	//color = c*i;

	vec4 c = texture(texColorMap, vec2(i,1));

	if(i <= (1/7) && i >= 0){
		i = (i-s0)*7;
		alpha = s0 + i*(s1-s0); 
	}
	else if(i > 1/7 && i <= 2/7){
		i = (i-s1)*7;
		alpha = s1 + (i)*(s2-s1);
	}
	else if(i > 2/7 && i <= 3/7){
		i = (i-s2)*7;
		alpha = s2 + i*(s3-s2);
	}
	else if(i > 3/7 && i <= 4/7){
		i = (i-s3)*7;
		alpha = s3 + i*(s4-s3);
	}
	else if(i > 4/7 && i <= 5/7){
		i = (i-s4)*7;
		alpha = s4 + i*(s5-s4);
	}
	else if(i > 5/7 && i <= 6/7){
		i = (i-s5)*7;
		alpha = s5 + i*(s6-s5);
	}
	else if(i <= 1){
		i = (i-s6)*7;
		alpha = s6 + i*(s7-s6);
	}
	else{
		alpha = 0;
	}

	//alpha = 1 - pow((1 - alpha), (sREL));
	color = vec4(c.x,c.y,c.z,alpha);

	// JUST ORANGE:
	//color = vec4(modelColor, 1.0f);
}