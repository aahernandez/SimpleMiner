#pragma once
#include "Engine/Math/Vector3.hpp"

class Player
{
public:
	Player();
	~Player();

	void UpdatePlayer();
	void SlowPlayerVertically();
	void CorrectTerminalVelocityDown();
	void Render() const;
	void RenderPlayer() const;
	void RenderPlayerAlwaysVisible() const;
	void ClampVelocity(float moveSpeed);

	void SetCenter(const Vector3& center);
	void AddMovementToCenter(const Vector3& movement);
	Vector3 GetCenter() const;
	Vector3 GetBottomCenter() const;
	Vector3 GetTopCenter() const;
	void AddVelocity(const Vector3& velocity);
	Vector3 GetVelocity() const;
	void SetRadius(float radius);
	float GetRadius() const;
	void SetHeight(float height);
	float GetHeight() const;
	void SetEyeHeightAboveCenter(float eyeHeight);
	float GetEyeHeightAboveCenter() const;
	void SetIsPhysicsWalking(bool isWalking);
	bool IsPhysicsWalking() const;
	bool IsOnGround() const;
	void SetIsOnGround(bool isGrounded);

private:
	Vector3		m_center;
	Vector3		m_velocity;
	float		m_radius;
	float		m_height;
	float		m_eyeHeightAboveCenter;
	bool		m_isPhysicsWalking;
	float		m_terminalVelocity;
	bool		m_isOnGround;
};