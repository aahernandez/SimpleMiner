#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

const float FRICTION_AMOUNT = 0.8f;

Player::Player()
	: m_center(0.5f, 0.5f, 110.f)
	, m_velocity(0.f, 0.f, 0.f)
	, m_radius(0.3f)
	, m_height(1.86f)
	, m_eyeHeightAboveCenter(0.69f)
	, m_isPhysicsWalking(true)
	, m_terminalVelocity(3.f)
	, m_isOnGround(false)
{
}

Player::~Player()
{
}

void Player::UpdatePlayer()
{
	m_velocity.x *= FRICTION_AMOUNT;
// 	if (m_velocity.x < 0.0001f && m_velocity.x > -0.0001f)
// 		m_velocity.x = 0.f;

	m_velocity.y *= FRICTION_AMOUNT;
// 	if (m_velocity.y < 0.0001f && m_velocity.y > -0.0001f)
// 		m_velocity.y = 0.f;
	
}

void Player::SlowPlayerVertically()
{
	m_velocity.z *= FRICTION_AMOUNT;
// 	if (m_velocity.z < 0.0001f && m_velocity.z > -0.0001f)
// 		m_velocity.z = 0.f;
}

void Player::CorrectTerminalVelocityDown()
{
	if (m_velocity.z < -m_terminalVelocity)
	{
		m_velocity.z = -m_terminalVelocity;
	}
}

void Player::Render() const
{
	RenderPlayer();
}

void Player::RenderPlayer() const
{
// 	g_theRenderer->PushMatrix();
	g_theRenderer->Translate(m_center.x, m_center.y, m_center.z );
	g_theRenderer->SetDepthTesting(true);
	g_theRenderer->SetLineWidth(3.f);
	g_theRenderer->DrawCylinderHollow3D(m_radius, m_height, 30, RGBA::BLUE_LIGHT);
	g_theRenderer->SetDepthTesting(false);
// 	g_theRenderer->PopMatrix();

	RGBA color = RGBA::BLUE_LIGHT;
	color.a = (unsigned char)(0.3f * 255.f);
	g_theRenderer->Translate(m_center.x, m_center.y, m_center.z);
	g_theRenderer->SetLineWidth(1.f);
	g_theRenderer->DrawCylinderHollow3D(m_radius, m_height, 30, color);
}

void Player::RenderPlayerAlwaysVisible() const
{
	RGBA color = RGBA::BLUE_LIGHT;
	color.a =(unsigned char) (0.3f * 255.f);
	g_theRenderer->PushMatrix();
	g_theRenderer->Translate(m_center.x, m_center.y, m_center.z);
	g_theRenderer->DrawCylinderHollow3D(m_radius, m_height, 30, color);
	g_theRenderer->PopMatrix();
}

void Player::ClampVelocity(float moveSpeed)
{
	m_velocity.x = Clamp(m_velocity.x, -moveSpeed, moveSpeed);
	m_velocity.y = Clamp(m_velocity.x, -moveSpeed, moveSpeed);
}

void Player::SetCenter(const Vector3& center)
{
	m_center = center;
}

void Player::AddMovementToCenter(const Vector3& movement)
{
	m_center += movement;
}

void Player::AddVelocity(const Vector3& velocity)
{
	m_velocity += velocity;

	if (m_velocity.x > 1.f)
		m_velocity.x = 1.f;
	else if (m_velocity.x < -1.f)
		m_velocity.x = -1.f;

	if (m_velocity.y > 1.f)
		m_velocity.y = 1.f;
	else if (m_velocity.y < -1.f)
		m_velocity.y = -1.f;

	if (m_velocity.z < -3.f)
		m_velocity.z = -3.f;
}

Vector3 Player::GetVelocity() const
{
	return m_velocity;
}

Vector3 Player::GetCenter() const
{
	return m_center;
}

Vector3 Player::GetBottomCenter() const
{
	return Vector3(m_center.x, m_center.y, m_center.z - (m_height * 0.5f));
}

Vector3 Player::GetTopCenter() const
{
	return Vector3(m_center.x, m_center.y, m_center.z + (m_height * 0.5f));
}

void Player::SetRadius(float radius)
{
	m_radius = radius;
}

float Player::GetRadius() const
{
	return m_radius;
}

void Player::SetHeight(float height)
{
	m_height = height;
}

float Player::GetHeight() const
{
	return m_height;
}

void Player::SetEyeHeightAboveCenter(float eyeHeight)
{
	m_eyeHeightAboveCenter = eyeHeight;
}

float Player::GetEyeHeightAboveCenter() const
{
	return m_eyeHeightAboveCenter;
}

void Player::SetIsPhysicsWalking(bool isWalking)
{
	m_isPhysicsWalking = isWalking;
}

bool Player::IsPhysicsWalking() const
{
	return m_isPhysicsWalking;
}

bool Player::IsOnGround() const
{
	return m_isOnGround;
}

void Player::SetIsOnGround(bool isGrounded)
{
	m_isOnGround = isGrounded;
}
