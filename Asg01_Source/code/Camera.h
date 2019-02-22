#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class Camera {
public:
	float perspective;
	float screenWidth, screenHeight;
	float nearPlane, farPlane;
	glm::vec3 localPos, objectLoc, initialLoc;

	glm::vec3 axisX, axisY, axisZ, localUp; // Left/Right, Up/Down, Forward/Backward respectively
	float RotX, RotY, RotZ;

	// Constructor from vector values:
	Camera(float width, float height)
	{
		screenWidth = width;
		screenHeight = height;

		perspective = 45.0f;
		nearPlane = 0.1f;
		farPlane = 100.0f;

		initialLoc = glm::vec3(0, 0, -10);
		localPos = glm::vec3(0,0,-10);
		objectLoc = glm::vec3(0, 0, 0);

		axisY = glm::vec3(0, 1, 0);
		axisX = glm::vec3(1, 0, 0);
		axisZ = glm::vec3(0, 0, 1);

		localUp = glm::vec3(0,1,0);

		RotX = 0;
		RotY = 0;
		RotZ = 0;
	}

	void UpdateCoordinates() {
	}

	// Called after yaw/pitch/roll values have changed.
	void OnUpdateRotation() {
	
	}

	glm::mat4 GetMVPMatrix()
	{
		// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		glm::mat4 Projection = glm::perspective(glm::radians(perspective), screenWidth / screenHeight, nearPlane, farPlane);

		glm::vec3 pointVector = glm::vec3(0,0,1);
		glm::vec3 defaultUp = glm::vec3(0,1,0);

		// Rotate Up:
		glm::mat4 rot = glm::mat4(1.0f);
		glm::vec4 rVec = glm::vec4(pointVector.x, pointVector.y, pointVector.z, 1);
		rot = glm::rotate(rot, glm::radians(RotY), glm::vec3(0, 1, 0));
		rVec = rVec * rot;
		pointVector = glm::vec3(rVec.x, rVec.y, rVec.z);

		// Rotate along Forward
		rot = glm::mat4(1.0f);
		glm::vec4 tempUp = glm::vec4(defaultUp.x, defaultUp.y, defaultUp.z, 1);
		rot = glm::rotate(rot, glm::radians(RotZ), glm::vec3(0, 0, 1));
		tempUp = tempUp * rot;
		axisY = glm::vec3(tempUp.x, tempUp.y, tempUp.z);

		// Rotate along Right
		//rVec = glm::vec4(pointVector.x, pointVector.y, pointVector.z, 1);
		rot = glm::mat4(1.0f);
		rot = glm::rotate(rot, glm::radians(RotX), glm::vec3(1, 0, 0));
		rVec = rVec * rot;
		tempUp = tempUp * rot;
		axisY = glm::vec3(tempUp.x, tempUp.y, tempUp.z);
		pointVector = glm::vec3(rVec.x, rVec.y, rVec.z);

		// Set Camera View (where initalLoc is the camera origin)
		glm::mat4 View = glm::lookAt(localPos, pointVector + localPos, axisY); // This should act as the new origin

		// Model matrix : Model is at origin
		glm::mat4 Model = glm::mat4(1.0f);

		// ModelViewProjection
		glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

		return mvp;
	}

void Reset() {
		perspective = 45.0f;
		nearPlane = 0.1f;
		farPlane = 100.0f;

		initialLoc = glm::vec3(0, 0, -10);
		localPos = glm::vec3(0, 0, -10);
		objectLoc = glm::vec3(0, 0, 0);

		axisY = glm::vec3(0, 1, 0);
		axisX = glm::vec3(1, 0, 0);
		axisZ = glm::vec3(0, 0, 1);

		localUp = glm::vec3(0, 1, 0);

		RotX = 0;
		RotY = 0;
		RotZ = 0;
	}
};
