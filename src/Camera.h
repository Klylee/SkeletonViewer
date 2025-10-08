#pragma once

#include "config.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SceneObject.h"
#include "Event.h"

class Camera : public SceneObject
{
public:
	REGISTER_SCENE_OBJECT(Camera)
	float fieldView = 45.0f;
	float nearPlane = 0.001f;
	float farPlane = 100.0f;
	float speed = 0.04f;

	float yaw = 0.0f;
	float pitch = 0.0f;
	double sensitivity = 0.1;
	double lastX = 0, lastY = 0;
	bool firstMouse = true;

	void awake() override
	{
		transform.eulerAngles(yaw, pitch, 0.0f);
	}
	void update() override;
	void draw() override {}

	glm::vec3 GetForward() const
	{
		return glm::normalize(transform.rotation() * glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 GetRight() const
	{
		return glm::normalize(glm::cross(GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	glm::vec3 GetUp() const
	{
		return glm::normalize(glm::cross(GetRight(), GetForward()));
	}
	Mat4 getViewMatrix()
	{
		Vector3 camPos = transform.position();
		Vector3 camUp = Vector3(0, 1, 0);
		return glm::lookAt(camPos, camPos + GetForward(), camUp);
	}
	Mat4 GetProjectionMatrix(float aspectRatio = 1) const
	{
		return glm::perspective(glm::radians(fieldView), aspectRatio, nearPlane, farPlane);
	}
};
