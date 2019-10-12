#pragma once
#include <vector>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

class Curve
{
public:
	Curve(bool bCatmullRomIn);
	~Curve();
	
	void init();
	void calculate_curve();

	std::vector<float> vTable;
	
public:
	float tau = 0.5; // Coefficient for catmull-rom spline
	glm::mat4 catmull_cmatrix;
	int num_points_per_segment = 200;
	float num_points_per_segment_inv;

	std::vector<glm::vec3> control_points_pos;
	std::vector<glm::vec3> curve_points_pos;
	std::vector<glm::quat> control_points_quaternion;

	float GetTotalDistance();

	// Converts a VTable index to control point index
	void GetIndexPointFromDist(int& im1, int& i, const int& realDist, float& interp);

protected:
	glm::vec3 catmull_rom(float u, const glm::vec3& p1_neg, const glm::vec3& p, const glm::vec3& p1_pos, const glm::vec3& p2_pos);

	void get_points(int index, glm::vec3& p1_neg, glm::vec3& p, glm::vec3& p1_pos, glm::vec3& p2_pos);

	bool bCatmullRom;
};