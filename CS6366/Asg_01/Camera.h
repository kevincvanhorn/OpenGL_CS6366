#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
	glm::vec3 Location;
	glm::vec3 FrontVector;
	glm::vec3 UpVector;
	glm::vec3 RightVector;
	glm::vec3 WorldUpVector;
	
	float Yaw;
	float Pitch;
	float Roll;

	// Constructor from vector values:
	Camera(glm::vec3 location = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 vUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = 0.0f, float pitch = 0.0f) : Front(glm::vec3(0.0f, 0.0f, -1.0f))
	{
		Location = location;
		WorldUpVector = vUp;
		Yaw = yaw;
		Pitch = pitch;
		Calibrate();
	}

	// Calculate the view matrix from camera properties.
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

private:
	void Calibrate()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		FrontVector = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		RightVector = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		UpVector = glm::normalize(glm::cross(Right, Front));
	}
};
