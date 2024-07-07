#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "OpenGL_Graphics.h"

enum class CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
public:
	glm::vec3 vCameraPos;
	glm::vec3 vCameraFront;
	glm::vec3 vCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

private:
	float fPitch = 0.0f;
	float fYaw = -90.0f;
	float fFov = 80.0f;

	float fLastX = 400.0f;
	float fLastY = 300.0f;

	//bool bMouseStarted = true;

	glm::mat4 matView;

public:
	float fCameraSpeed = 5.0f;

	Camera() = default;	

	void init(glm::vec3 vPos, glm::vec3 vFront);
	const glm::mat4& getLookAt();

	// We essentially change vCamPos using vCamFront and vCamUp
	void ProcessKeyboard(CameraMovement movement, float fDeltaTime);

	// We essentially change vCamDir using pitch and yaw values
	void ProcessMouse(OpenGL_Graphics* gl, float fMousePosX, float fMousePosY);

	void SetCameraPos(glm::vec3 vPos);

	void UpdateView(Shader shader, const std::string& viewMat4ID);
};

void Camera::init(glm::vec3 vPos, glm::vec3 vFront)
{
	vCameraPos = vPos;
	vCameraFront = vFront;
	matView = glm::lookAt(vCameraPos, vCameraPos + vCameraFront, vCameraUp);
}

const glm::mat4& Camera::getLookAt()
{
	return matView;
}

// We essentially change vCamPos using vCamFront and vCamUp
void Camera::ProcessKeyboard(CameraMovement movement, float fDeltaTime)
{
	float fDistance = fCameraSpeed * fDeltaTime;
	switch (movement)
	{
	case CameraMovement::FORWARD:
		vCameraPos += vCameraFront * fDistance;
		break;

	case CameraMovement::BACKWARD:
		vCameraPos -= vCameraFront * fDistance;
		break;

	case CameraMovement::LEFT:
		vCameraPos -= glm::normalize(glm::cross(vCameraFront, vCameraUp)) * fDistance;
		break;

	case CameraMovement::RIGHT:
		vCameraPos += glm::normalize(glm::cross(vCameraFront, vCameraUp)) * fDistance;
		break;

	case CameraMovement::UP:
		vCameraPos += vCameraUp * fDistance;
		break;

	case CameraMovement::DOWN:
		vCameraPos -= vCameraUp * fDistance;
		break;
	}

	matView = glm::lookAt(vCameraPos, vCameraPos + vCameraFront, vCameraUp);
}

// We essentially change vCamDir using pitch and yaw values
void Camera::ProcessMouse(OpenGL_Graphics* gl, float fMousePosX, float fMousePosY)
{
	if (gl->bFirstMouse)
	{
		fLastX = gl->ScreenWidth() / 2.0f;
		fLastY = gl->ScreenHeight() / 2.0f;
	}

	else
	{
		float fOffsetX = fMousePosX - fLastX;
		float fOffsetY = fLastY - fMousePosY;

		fLastX = fMousePosX;
		fLastY = fMousePosY;

		float fSensitivity = 0.2f;
		fOffsetX *= fSensitivity;
		fOffsetY *= fSensitivity;

		fYaw += fOffsetX;
		fPitch += fOffsetY;

		if (fPitch > 89.0f)
			fPitch = 89.0f;
		else if (fPitch < -89.0f)
			fPitch = -89.0f;

		glm::vec3 vDirection;
		vDirection.x = cosf(glm::radians(fYaw)) * cosf(glm::radians(fPitch));
		vDirection.y = sinf(glm::radians(fPitch));
		vDirection.z = sinf(glm::radians(fYaw)) * cosf(glm::radians(fPitch));
		vCameraFront = glm::normalize(vDirection);
	}

	matView = glm::lookAt(vCameraPos, vCameraPos + vCameraFront, vCameraUp);
}

void Camera::SetCameraPos(glm::vec3 vPos)
{
	vCameraPos = vPos;
}

void Camera::UpdateView(Shader shader, const std::string& viewMat4ID)
{
	shader.use();
	shader.setMat4(viewMat4ID, matView);
}