#pragma once

#include <vector>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Curve.h"

class Aircraft_Animation
{

public:
	float total_moving_time = 10;
	float t1 = 0.1;
	float t2 = 0.7;
	bool bAircraftMoving;

	void SetDistance();

private:
	glm::mat4 m_model_mat;
	Curve* m_animation_curve = nullptr;

	float t; // 0 -> 1 as 
	float totalTime;
	float v0;

	int prev;

	float ease();

	float totalDist;

public:
	Aircraft_Animation();
	~Aircraft_Animation();

	void init();
	void init(Curve* animation_curve);

	void update(float delta_time);

	void CalcV0();

	void reset();
	glm::mat4 get_model_mat() { return m_model_mat; };
};

