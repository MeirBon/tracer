#pragma once

#include <glm/glm.hpp>

#include "Core/Ray.h"

namespace core
{

#define ROTATION_SPEED 0.002f
#define MOVEMENT_SPEED 0.003f

constexpr float MAX_PITCH = 89.f * 0.0174532925f;
constexpr float MIN_PITCH = -89.f * 0.0174532925f;

class Camera
{
  public:
	Camera() = default;

	Camera(int width, int height, float fov, float mouseSens = ROTATION_SPEED,
		   glm::vec3 pos = glm::vec3(0.f, 0.f, 1.f));

	Ray GenerateRay(float x, float y) const;

	Ray GenerateRandomRay(float x, float y, RandomGenerator &rng) const;

	void ProcessMouse(int x, int y) noexcept;

	glm::vec3 GetPosition() const noexcept;

	void MoveUp(float movementSpeed = MOVEMENT_SPEED) noexcept;

	void MoveDown(float movementSpeed = MOVEMENT_SPEED) noexcept;

	void MoveForward(float movementSpeed = MOVEMENT_SPEED) noexcept;

	void MoveBackward(float movementSpeed = MOVEMENT_SPEED) noexcept;

	void MoveLeft(float movementSpeed = MOVEMENT_SPEED) noexcept;

	void MoveRight(float movementSpeed = MOVEMENT_SPEED) noexcept;

	void RotateUp(float times = 1) noexcept;

	void RotateDown(float times = 1) noexcept;

	void RotateRight(float times = 1) noexcept;

	void RotateLeft(float times = 1) noexcept;

	void ChangeFOV(float fov) noexcept;

	const float &GetFOV() const noexcept;

	const float &GetPitch() const noexcept;

	const float &GetYaw() const noexcept;

	inline float GetFOVDistance() const noexcept { return m_FOV_Distance; }

	inline float GetInvWidth() const noexcept { return m_InvWidth; }

	inline float GetInvHeight() const noexcept { return m_InvHeight; }

	inline float GetAspectRatio() const noexcept { return m_AspectRatio; }

	inline glm::vec3 GetUp() const noexcept { return m_Up; }

	inline void SetPosition(glm::vec3 position) noexcept { this->m_Origin = position; }

	inline glm::vec3 GetViewDirection() const noexcept { return m_ViewDirection; }

	glm::vec3 getDirectionFromPitchAndYaw() noexcept;

	inline void SetWidth(int width)
	{
		m_Width = float(width);
		m_InvWidth = (float)1.0f / float(width);
		m_AspectRatio = float(m_Width) / float(m_Height);
	}

	inline void SetHeight(int height)
	{
		m_Height = float(height);
		m_InvHeight = (float)1.0f / float(height);
		m_AspectRatio = float(m_Width) / float(m_Height);
	}

	inline void SetWidthHeight(int width, int height)
	{
		m_Width = float(width);
		m_Height = float(height);
		m_InvWidth = (float)1.0f / float(width);
		m_InvHeight = (float)1.0f / float(height);
		m_AspectRatio = float(width) / float(height);
	}

  private:
	glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_ViewDirection;
	glm::vec3 m_Origin;

	struct
	{
		float x;
		float y;
	} m_FOV = {90.f, 90.f};

	float m_FOV_Distance;
	float m_RotationSpeed = 0.005f;
	float m_Width, m_Height;
	float m_InvWidth, m_InvHeight;
	float m_AspectRatio{};
	float m_Yaw = -0.04f;
	float m_Pitch = 0.2f;
};
} // namespace core
