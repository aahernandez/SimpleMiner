#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"

enum CameraMode
{
	FIRST_PERSON,
	FROM_BEHIND,
	FIXED_ANGLE,
	NO_CLIP,
	CAMERA_MODE_SIZE
};

class Camera3D
{
public:
	Vector3 m_position;
	CameraMode m_cameraMode;
	float m_rollAboutX;
	float m_pitchAboutY;
	float m_yawAboutZ;
	Vector3 m_defaultOrientation;

	Camera3D();

	void CycleCameraMode();
	void SetCameraPositionAndOrientation(const Vector3& playerPosAtEyeHeight);

	Vector3 GetForwardXYZ() const;
	Vector3 GetForwardXY() const;
	Vector3 GetLeftXY() const;
};