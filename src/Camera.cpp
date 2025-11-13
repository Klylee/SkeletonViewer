#include "Camera.h"
#include "Input.h"
#include "GlobalTime.h"
#include <iostream>
#include <algorithm>

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

		if (Input::isKeyPressed(GLFW_KEY_R))
		{

			double k = 0.1;
			double delta_yaw = glm::radians(k * -dx);
			double delta_pitch = glm::radians(k * dy);

			// 绕target旋转
			glm::vec3 camPos = transform.position();
			glm::vec3 offset = camPos - target;

			// 绕Y轴旋转 yaw
			glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), (float)delta_yaw, glm::vec3(0, 1, 0));
			offset = glm::vec3(rotY * glm::vec4(offset, 1.0f));

			// 绕相机右轴旋转 pitch
			glm::vec3 right = transform.rotation() * glm::vec3(1, 0, 0);
			glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), (float)delta_pitch, right);
			offset = glm::vec3(rotX * glm::vec4(offset, 1.0f));

			camPos = target + offset;
			transform.position(camPos);

			yaw += glm::degrees(delta_yaw);
			pitch += glm::degrees(delta_pitch);
			transform.eulerAngles(yaw, pitch, 0.0f);
		}
		else
		{
			dx *= sensitivity;
			dy *= sensitivity;

			yaw += static_cast<float>(dx);
			pitch += static_cast<float>(-dy);

			transform.eulerAngles(yaw, pitch, 0.0f);
		}

		// glm::vec3 camUp = transform.rotation() * glm::vec3(0, 1, 0);
		// std::cout << std::format("{} {} {} {} {}", camUp.x, camUp.y, camUp.z, pitch, yaw) << std::endl;
	}
	else
	{
		firstMouse = true;
	}
}
