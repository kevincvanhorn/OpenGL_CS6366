#include "Animation.h"

Animation::Animation()
{
	this->m_model_mat = glm::mat4();
}

Animation::~Animation()
{
}

void Animation::init()
{
	reset();
}

void Animation::update(float delta_time)
{
}

void Animation::reset()
{
	m_model_mat = glm::mat4();
	m_model_mat = glm::translate(m_model_mat, glm::vec3(5.0, 0.0, 0.0));
}
