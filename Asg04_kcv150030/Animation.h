#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Animation
{
public:

	Animation();
	~Animation();

	void init();
	void update(float delta_time);

	void reset();
	glm::mat4 get_model_mat() { return m_model_mat; };

	void rotateLocalX(float rot_degrees);

	void rotateGlobalY(float rot_degrees);

	void setTransform(const glm::mat4& in_trans);

private:
	glm::mat4 m_model_mat;
};

