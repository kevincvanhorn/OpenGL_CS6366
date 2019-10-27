#include "Bone_Animation.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


Bone_Animation::Bone_Animation()
{
}


Bone_Animation::~Bone_Animation()
{
}

void Bone_Animation::init()
{
	root_position = { 2.0f,1.0f,2.0f };

	scale_vector =
	{
		{1.0f,1.0f,1.0f},
		{0.5f,4.0f,0.5f},
		{0.5f,3.0f,0.5f},
		{0.5f,2.0f,0.5f}
	};

	scale_matrices =
	{
		{glm::scale(scale_vector[0])},
		{glm::scale(scale_vector[1])},
		{glm::scale(scale_vector[2])},
		{glm::scale(scale_vector[3])},
	};

	rotation_degree_vector = 
	{
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	colors = 
	{
		{0.7f,0.0f,0.0f,1.0f},
		{0.7f,0.7f,0.0f,1.0f},
		{0.7f,0.0f,0.7f,1.0f},
		{0.0f,0.7f,0.7f,1.0f}
	};

	SetBaseTransforms();
}

void Bone_Animation::update(float delta_time)
{
	SetBaseTransforms();
}

void Bone_Animation::reset()
{
	x1 = x2 = x3 = 0;
	y1 = y2 = y3 = 0;
	z1 = z2 = z3 = 0;
}

void Bone_Animation::SetBaseTransforms()
{
	base_transforms = {
		glm::mat4(1.0f),
		glm::mat4(1.0f),
		glm::mat4(1.0f),
		glm::mat4(1.0f)
	};

	float acc_offset_y = 0;
	glm::mat4 acc_rotation = glm::mat4(1.0f);
	glm::mat4 global_transform = glm::mat4(1.0f);

	base_transforms[0] = glm::translate(base_transforms[0], glm::vec3(0, scale_vector[0].y * 0.5f, 0)) * scale_matrices[0];
	
	acc_rotation = glm::mat4(1.0f);
	base_transforms[1] = glm::translate(base_transforms[1], glm::vec3(0, scale_vector[1].y * 0.5f, 0)) * scale_matrices[1];
	UpdateRotation(/*out*/acc_rotation, 1);
	base_transforms[1] = acc_rotation * base_transforms[1];
	base_transforms[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0, scale_vector[0].y, 0)) * base_transforms[1];
	// --
	global_transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, scale_vector[0].y, 0));
	global_transform = global_transform * acc_rotation;

	acc_rotation = glm::mat4(1.0f);
	base_transforms[2] = glm::translate(base_transforms[2], glm::vec3(0, scale_vector[2].y * 0.5f, 0)) * scale_matrices[2];
	UpdateRotation(/*out*/acc_rotation, 2);
	base_transforms[2] = acc_rotation * base_transforms[2];
	base_transforms[2] = glm::translate(glm::mat4(1.0f), glm::vec3(0, scale_vector[1].y, 0)) * base_transforms[2];
	base_transforms[2] = global_transform * base_transforms[2];
	//--
	global_transform = glm::translate(global_transform, glm::vec3(0, scale_vector[1].y, 0));
	global_transform = global_transform * acc_rotation;
	
	acc_rotation = glm::mat4(1.0f);
	base_transforms[3] = glm::translate(base_transforms[0], glm::vec3(0, scale_vector[0].y * 0.5f, 0)) * scale_matrices[3];
	UpdateRotation(/*out*/acc_rotation, 3);
	base_transforms[3] = acc_rotation * base_transforms[3];
	base_transforms[3] = glm::translate(glm::mat4(1.0f), glm::vec3(0, scale_vector[2].y, 0)) * base_transforms[3];
	base_transforms[3] = global_transform * base_transforms[3];

	for (int i = 0; i < base_transforms.size(); ++i) {
		// Change pivot & scale (base object):
		//base_transforms[i] = glm::translate(base_transforms[i], glm::vec3(0, scale_vector[i].y * 0.5f, 0)) * scale_matrices[i];

		//if(i >0) base_transforms[i] = global_transform * base_transforms[i];

		// Rotations
		//if (i > 0) UpdateRotation(/*out*/acc_rotation, i);
		//base_transforms[i] = acc_rotation * base_transforms[i];

		// Relative offsets:
		//if (i > 0) acc_offset_y += scale_vector[i - 1].y;
		//base_transforms[i] = glm::translate(glm::mat4(1.0f), glm::vec3(0, acc_offset_y,0)) * base_transforms[i];
		
		// LAST: Translate all to desired root offset
		base_transforms[i] = glm::translate(glm::mat4(1.0f), root_position) * base_transforms[i];
	}
}

void Bone_Animation::UpdateRotation(glm::mat4& base_rot, int index)
{
	glm::vec3 local_rot;
	if (index == 1) local_rot = glm::vec3(x1, y1, z1);
	else if (index == 2) local_rot = glm::vec3(x2, y2, z2);
	else if (index == 3) local_rot = glm::vec3(x3, y3, z3);
	
	base_rot = glm::rotate(base_rot, glm::radians(local_rot.z), glm::vec3(0, 0, 1));
	base_rot = glm::rotate(base_rot, glm::radians(local_rot.y), glm::vec3(0, 1, 0));
	base_rot = glm::rotate(base_rot, glm::radians(local_rot.x), glm::vec3(1,0,0));
}

