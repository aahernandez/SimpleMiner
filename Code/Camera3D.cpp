#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtilities.hpp"

Camera3D::Camera3D()
	: m_position(0.f, 0.f, 0.f)
	, m_yawAboutZ(0.f)
	, m_pitchAboutY(0.f)
	, m_rollAboutX(0.f)
	, m_cameraMode(FIRST_PERSON)
	, m_defaultOrientation(0.f, 0.f, 0.f)
{
}

void Camera3D::CycleCameraMode()
{
	if (m_cameraMode < CAMERA_MODE_SIZE - 1)
		m_cameraMode = (CameraMode) ((int) m_cameraMode + 1);
	else 
		m_cameraMode = FIRST_PERSON;
}

void Camera3D::SetCameraPositionAndOrientation(const Vector3& playerPosAtEyeHeight)
{
	if (m_cameraMode == FIRST_PERSON)
	{
		m_position = playerPosAtEyeHeight;
	}
	else if (m_cameraMode == FROM_BEHIND)
	{
		Vector3 cameraPos = (GetForwardXYZ() * -4.f) + playerPosAtEyeHeight; //#TODO: Raycasting
		m_position = cameraPos;
	}
	else if (m_cameraMode == FIXED_ANGLE)
	{
		m_rollAboutX = 0.f;
		m_pitchAboutY = 20.f;
		m_yawAboutZ = 45.f;

		Vector3 cameraPos = (GetForwardXYZ() * -4.f) + playerPosAtEyeHeight; //#TODO: Raycasting
		m_position = cameraPos; 
	}
}

Vector3 Camera3D::GetForwardXYZ() const
{
	float xPos = CosDegrees(m_yawAboutZ) * CosDegrees(m_pitchAboutY);
	float yPos = SinDegrees(m_yawAboutZ) * CosDegrees(m_pitchAboutY);
	float zPos = -SinDegrees(m_pitchAboutY);
	return Vector3( xPos, yPos, zPos );
}

Vector3 Camera3D::GetForwardXY() const
{
	Vector3 mForwardDirection = Vector3( CosDegrees(m_yawAboutZ), SinDegrees(m_yawAboutZ), 0 );
	return mForwardDirection;
}

Vector3 Camera3D::GetLeftXY() const
{
	Vector3 mLeftDirection = Vector3( -SinDegrees(m_yawAboutZ), CosDegrees(m_yawAboutZ), 0 );
	return mLeftDirection;
}
