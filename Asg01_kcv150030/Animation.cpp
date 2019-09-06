#define GLM_FORCE_SWIZZLE
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

void Animation::rotateLocalX(float rot_degrees)
{
	//glm::vec3 localAxis_x = m_model_mat[0].xyz;
	//m_model_mat = glm::rotate(m_model_mat, glm::radians(rot_degrees), localAxis_x);

	glm::mat4 rotMatrix = glm::mat4();
	rotMatrix = glm::rotate(rotMatrix, glm::radians(rot_degrees), glm::vec3(1, 0, 0));

	m_model_mat = m_model_mat*rotMatrix;
}

void Animation::rotateGlobalY(float rot_degrees)
{
	// multiply 
	glm::mat4 rotMatrix = glm::mat4();
	rotMatrix = glm::rotate(rotMatrix, glm::radians(rot_degrees), glm::vec3(0,1,0));

	m_model_mat = rotMatrix*m_model_mat;
}

void Animation::setTransform(const glm::mat4& in_trans)
{
	m_model_mat = in_trans;
}
