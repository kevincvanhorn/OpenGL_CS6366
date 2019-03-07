/*
 * @author Kevin VanHorn - kcv150030
 * Responsible for creating and managing the model-view-projection matrix, handling rotation and translations
 * to the camera in local space.
*/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class PointLight {
public:
	glm::vec3 Loc; // Initial location.

	glm::vec3 MovedLoc; // Updated location.
	float totalRotX;
	float totalRotY;
	float totalRotZ;

	float speed = 1;
	float prevTime;

	PointLight(glm::vec3 intialLoc) {
		Loc = intialLoc;
		MovedLoc = Loc;
		prevTime = glfwGetTime();
	}

	glm::vec3 getLoc() {
		return MovedLoc;
	}

	void UpdatePos(bool bRotX, bool bRotY, bool bRotZ) {
		//float deltaTime = glfwGetTime() - prevTime;
		//prevTime = deltaTime;
		//float dist = deltaTime * speed;
		
		float dist = .4; // essentially the speed

		// Slow down the distance traveled for each axis used

		float numTrue = 0;
		if (bRotX) { numTrue++; }
		if (bRotY) { numTrue++; }
		if (bRotZ) { numTrue++; }
		if (numTrue > 0) { dist /= numTrue; }

		glm::mat4 rot = glm::mat4(1.0f);

		if (bRotX) {
			rot = glm::rotate(rot, glm::radians(dist), glm::vec3(1, 0, 0));
			glm::vec4 rot4 = glm::vec4(MovedLoc.x, MovedLoc.y, MovedLoc.z, 1);
			MovedLoc = rot4 * rot;
		}
		if (bRotY) {
			rot = glm::rotate(rot, glm::radians(dist), glm::vec3(0, 1, 0));
			glm::vec4 rot4 = glm::vec4(MovedLoc.x, MovedLoc.y, MovedLoc.z, 1);
			MovedLoc = rot4 * rot;
		}
		if (bRotZ) {
			rot = glm::rotate(rot, glm::radians(dist), glm::vec3(0, 0, 1));
			glm::vec4 rot4 = glm::vec4(MovedLoc.x, MovedLoc.y, MovedLoc.z, 1);
			MovedLoc = rot4 * rot;
		}
	}



};
