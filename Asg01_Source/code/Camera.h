/*
 * @author Kevin VanHorn - kcv150030
 * Responsible for creating and managing the model-view-projection matrix, handling rotation and translations
 * to the camera in local space.
*/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class Camera {
public:
	// Default values:
	float const NEAR_PLANE = 0.1f;
	float const FAR_PLANE = 100.0f;
	float const PERSP = 45.0f;
	float POS_X = 0;
	float POS_Y = 0;
	float POS_Z = 10;
	float DIST = 10;

	// Local camera values
	float perspective;
	float screenWidth, screenHeight;
	float nearPlane, farPlane;
	glm::vec3 localPos, objectLoc, initialLoc;
	glm::vec3 axisX, axisY, axisZ, localUp; // Left/Right, Up/Down, Forward/Backward respectively
	float RotX, RotY, RotZ;

	glm::vec3 pointVector = glm::vec3(0, 0, -1);
	glm::mat4 MV;


	// Constructor from vector values:
	Camera(float width, float height)
	{
		screenWidth = width;
		screenHeight = height;
		perspective = PERSP;
		nearPlane = NEAR_PLANE;
		farPlane = FAR_PLANE;
		
		initialLoc = glm::vec3(0, 0, 10);
		localPos = glm::vec3(0,0,10);
		objectLoc = glm::vec3(0, 0, 0);

		axisY = glm::vec3(0, 1, 0);
		axisX = glm::vec3(1, 0, 0);
		axisZ = glm::vec3(0, 0, -1);

		localUp = glm::vec3(0,1,0);

		RotX = 0;
		RotY = 0;
		RotZ = 0;
	}

	// Called after yaw/pitch/roll values have changed.
	void OnUpdateRotation() {
	}

	glm::vec3 GetCameraPos() {
		return glm::vec3(localPos.x, localPos.y, localPos.z);
	}

	glm::vec3 GetCameraDir() {
		return glm::normalize(pointVector);
		//return glm::normalize(glm::vec3(MV[1][3], MV[2][3], MV[3][3]));
	}

	glm::mat4 GetMVPMatrix()
	{
		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		glm::mat4 Projection = glm::perspective(glm::radians(perspective), screenWidth / screenHeight, nearPlane, farPlane);

		pointVector = glm::vec3(0,0,-1);
		glm::vec3 defaultUp = glm::vec3(0,1,0);

		// Rotate Up:
		glm::mat4 rot = glm::mat4(1.0f);
		glm::vec4 rVec = glm::vec4(pointVector.x, pointVector.y, pointVector.z, 1);
		rot = glm::rotate(rot, glm::radians(RotY), glm::vec3(0, 1, 0));
		rVec = rVec * rot;
		pointVector = glm::vec3(rVec.x, rVec.y, rVec.z);

		// Rotate along Forward:
		rot = glm::mat4(1.0f);
		glm::vec4 tempUp = glm::vec4(defaultUp.x, defaultUp.y, defaultUp.z, 1);
		rot = glm::rotate(rot, glm::radians(RotZ), glm::vec3(0, 0, 1));
		tempUp = tempUp * rot;
		axisY = glm::vec3(tempUp.x, tempUp.y, tempUp.z);

		// Rotate along Right
		rot = glm::mat4(1.0f);
		rot = glm::rotate(rot, glm::radians(RotX), glm::vec3(1, 0, 0));
		rVec = rVec * rot;
		tempUp = tempUp * rot;
		axisY = glm::vec3(tempUp.x, tempUp.y, tempUp.z);
		pointVector = glm::vec3(rVec.x, rVec.y, rVec.z);

		// Set Camera View (where initalLoc is the camera origin)
		glm::mat4 View = glm::lookAt(localPos, pointVector + localPos, axisY); // This should act as the new origin

		glm::vec3 objectLoc(0.5f,0.5f,0.5f);
		pointVector.x = cos(glm::radians(RotX)) * cos(glm::radians(RotY));
		pointVector.y = sin(glm::radians(RotX));
		pointVector.z = cos(glm::radians(RotX)) * sin(glm::radians(RotY));
		glm::vec3 vCamPos = objectLoc + pointVector*4.0f;
		
		View = glm::lookAt(vCamPos, objectLoc, glm::vec3(0,1,0)); // This should act as the new origin

		// Model matrix : Model is at origin
		glm::mat4 Model = glm::mat4(1.0f);

		// ModelViewProjection
		glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
		MV = View;

		return mvp;
	}

	/* Centers the model with an object center and dist from the object. */
	void SetModelCenter(float center, float dist) {
		DIST = dist;
		POS_X = 0;
		POS_Y = center;
		POS_Z = -1 * DIST;

		localPos = glm::vec3(POS_X, POS_Y, POS_Z);
	}

	/* Set camera to default settings. */
	void Reset() {
		perspective = PERSP;
		nearPlane = NEAR_PLANE;
		farPlane = FAR_PLANE;

		initialLoc = glm::vec3(0, 0, 10);
		localPos = glm::vec3(POS_X, POS_Y, POS_Z);
		objectLoc = glm::vec3(0, 0, 0);

		axisY = glm::vec3(0, 1, 0);
		axisX = glm::vec3(1, 0, 0);
		axisZ = glm::vec3(0, 0, -1);

		localUp = glm::vec3(0, 1, 0);

		RotX = 0;
		RotY = 0;
		RotZ = 0;
	}
};
