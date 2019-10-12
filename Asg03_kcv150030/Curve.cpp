#include "Curve.h"

Curve::Curve(bool bCatmullRomIn)
{
	num_points_per_segment_inv = 1 / (float)(num_points_per_segment);
	bCatmullRom = bCatmullRomIn;

}

Curve::~Curve()
{
}

void Curve::init()
{
	this->control_points_pos = {
		{ 0.0, 8.5, -2.0 },
		{ -3.0, 11.0, 2.3 },
		{ -6.0, 8.5, -2.5 },
		{ -4.0, 5.5, 2.8 },
		{ 1.0, 2.0, -4.0 },
		{ 4.0, 2.0, 3.0 },
		{ 7.0, 8.0, -2.0 },
		{ 3.0, 10.0, 3.7 }
	};

	catmull_cmatrix[0] = glm::vec4(-1, 2, -1, 0);
	catmull_cmatrix[1] = glm::vec4(3, -5, 0, 2);
	catmull_cmatrix[2] = glm::vec4(-3, 4, 1, 0);
	catmull_cmatrix[3] = glm::vec4(1, -1, 0, 0);
	catmull_cmatrix = tau*catmull_cmatrix;
}

void Curve::calculate_curve()
{
	if (bCatmullRom) {
		glm::vec3 p1_neg, p, p1_pos, p2_pos;
		for (int i = 0; i < control_points_pos.size(); ++i) {
			get_points(i, p1_neg, p, p1_pos, p2_pos);
			
			for (int u = 0; u < num_points_per_segment; ++u) {
				curve_points_pos.emplace_back(catmull_rom(u * num_points_per_segment_inv, p1_neg, p, p1_pos, p2_pos));
			}
		}
	}
	else {
		this->curve_points_pos = {
		{ 0.0, 8.5, -2.0 },
		{ -3.0, 11.0, 2.3 },
		{ -6.0, 8.5, -2.5 },
		{ -4.0, 5.5, 2.8 },
		{ 1.0, 2.0, -4.0 },
		{ 4.0, 2.0, 3.0 },
		{ 7.0, 8.0, -2.0 },
		{ 3.0, 10.0, 3.7 }
		};
	}
}

float Curve::GetTotalDistance()
{
	float totalDist = 0;
	glm::vec3 prev = curve_points_pos[curve_points_pos.size()-1];
	for (int i = 0; i < curve_points_pos.size(); ++i) {
		totalDist += glm::length(curve_points_pos[i] - prev);
		vTable.push_back(totalDist);
		prev = curve_points_pos[i];
	}

	return totalDist;
}

glm::vec3 Curve::catmull_rom(float u, const glm::vec3& p1_neg, const glm::vec3& p, const glm::vec3& p1_pos, const glm::vec3& p2_pos)
{
	glm::mat3x4 M(catmull_cmatrix*glm::transpose(glm::mat4x3(p1_neg, p, p1_pos, p2_pos)));
	glm::vec3 newPoint =  glm::vec4(glm::vec4(pow(u, 3), pow(u, 2), u, 1)) * M;
	return newPoint;
}

int get_next_index(int index, int size, int delta) {

	index += delta;

	if (index == size) {
		return 0;
	}
	else if (index == size + 1) {
		return 1;
	}
	else if (index == -1) {
		return size - 1;
	}
	return index;
}

void Curve::get_points(int index, glm::vec3& p1_neg, glm::vec3& p, glm::vec3& p1_pos, glm::vec3& p2_pos)
{
	int size = control_points_pos.size();
	if (index >= 0 && index < size) {
		p = control_points_pos[index];
		p1_neg = control_points_pos[get_next_index(index, size,-1)];
		p1_pos = control_points_pos[get_next_index(index, size, 1)];
		p2_pos = control_points_pos[get_next_index(index, size, 2)];
	}
}
