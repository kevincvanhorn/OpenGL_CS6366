#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>	

class Bone_Animation
{
public:
	Bone_Animation();
	~Bone_Animation();

	void init();
	void update(float delta_time);
	void reset();

private:
	std::vector<glm::vec3> scale_vector;
	std::vector<glm::mat4> scale_matrices;

public:
	// Here the head of each vector is the root bone
	std::vector<glm::vec3> rotation_degree_vector;
	std::vector<glm::vec4> colors;
	std::vector<glm::mat4> base_transforms;

	glm::vec3 root_position;

public:
	float x1, x2, x3;
	float y1, y2, y3;
	float z1, z2, z3;

private:
	void SetBaseTransforms();

	void UpdateRotation(glm::mat4& base_rot, int index);

};

