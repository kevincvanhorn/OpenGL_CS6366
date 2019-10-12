#include "Aircraft_Animation.h"


void Aircraft_Animation::SetDistance()
{
	totalDist = m_animation_curve->GetTotalDistance();
	//v0 = (2 * totalDist) / (2 * total_moving_time - t1 - t2);
	CalcV0();
}

float Aircraft_Animation::ease()
{
	// using t | d [0, 1]
	float t = totalTime / total_moving_time;

	if (t <= t1) {
		return v0* (glm::pow(t, 2) / (2*t1));
	}
	else if(t <= t2) {
		return v0 * (t1 * 0.5f) + v0*(t - t1);
	}
	else if (t <= 1.0f) {
		return v0 * (0.5f * t1) + v0 * (t2 - t1) + v0 * (1 - (0.5f * (t - t2) / (1 - t2))) * (t - t2);
	}
	else {
		return -1.1f;
	}

	return 0.0f;
}

Aircraft_Animation::Aircraft_Animation()
{
	t = 0;
	prev = 0;
	this->m_model_mat = glm::mat4(1.0f);
}


Aircraft_Animation::~Aircraft_Animation()
{
}

void Aircraft_Animation::init()
{
	reset();
}

void Aircraft_Animation::init(Curve* animation_curve)
{
	m_animation_curve = animation_curve;
	reset();
}

void Aircraft_Animation::update(float delta_time)
{
	CalcV0();
	if (bAircraftMoving) {
		totalTime += delta_time;
		float s = ease();

		if (s >= 1.0f || s < -1.0f) {
			bAircraftMoving = false;
			return;
		}

		float realDist = totalDist*s;
		int i = prev;
		for (; i < m_animation_curve->vTable.size(); ++i) {
			if (m_animation_curve->vTable[i] >= realDist) {
				break;
			}
		}
		int im1 = (i - 1 > 0) ? i - 1: 0;
		prev = im1;

		float d1 = m_animation_curve->vTable[im1];
		float d2 = m_animation_curve->vTable[i];

		float interp = ((float)(realDist)-d1) / (d2-d1);

		if (d2 - d1 == 0) {
			interp = 0;
		}

		glm::vec3 pos1 = m_animation_curve->curve_points_pos[im1];
		glm::vec3 pos2 = m_animation_curve->curve_points_pos[i];

		glm::vec3 newPos = pos1 + interp*(pos2-pos1);

		//printf("%f %i\n",s, im1);
		m_model_mat = glm::mat4(1.0f);
		m_model_mat = glm::translate(m_model_mat, newPos);
	}
}

void Aircraft_Animation::CalcV0()
{
	v0 = 2 / (-1 * t1 + t2 + 1);
}

void Aircraft_Animation::reset()
{
	m_model_mat = glm::mat4(1.0f);
	totalTime = 0;
	prev = 0;
	CalcV0();
	bAircraftMoving = false;

	if (m_animation_curve != nullptr && m_animation_curve->control_points_pos.size() > 0)
	{
		m_model_mat = glm::translate(m_model_mat, m_animation_curve->control_points_pos[0]);
	}
}
