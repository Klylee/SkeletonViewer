#include "Camera.h"
#include "Input.h"
#include "GlobalTime.h"
#include <iostream>

void Camera::update()
{
	// Key Events
	Vector3 forward = GetForward();
	Vector3 forwardXZ = glm::normalize(Vector3(forward.x, 0, forward.z));
	Vector3 rightXZ = glm::normalize(Vector3(-forwardXZ.z, 0, forwardXZ.x));

	float dt = GlobalTime::GetFrameDeltaTime();
	if (Input::isKeyPressed(GLFW_KEY_W))
		transform.position(transform.position() + forwardXZ * speed * dt);
	if (Input::isKeyPressed(GLFW_KEY_S))
		transform.position(transform.position() - forwardXZ * speed * dt);
	if (Input::isKeyPressed(GLFW_KEY_A))
		transform.position(transform.position() - rightXZ * speed * dt);
	if (Input::isKeyPressed(GLFW_KEY_D))
		transform.position(transform.position() + rightXZ * speed * dt);
	if (Input::isKeyPressed(GLFW_KEY_E))
		transform.position(transform.position() + Vector3(0, speed * dt, 0));
	if (Input::isKeyPressed(GLFW_KEY_Q))
		transform.position(transform.position() - Vector3(0, speed * dt, 0));

	if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
	{
		// Mouse Event
		auto [xpos, ypos] = Input::getMousePosition();
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}
		double dx = xpos - lastX;
		double dy = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		dx *= sensitivity;
		dy *= sensitivity;

		yaw += static_cast<float>(dx);
		pitch += static_cast<float>(-dy);

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
		transform.eulerAngles(yaw, pitch, 0.0f);
	}
	else
	{
		firstMouse = true;
	}
	// std::cout << std::format("{} {}", yaw, pitch) << std::endl;
}
