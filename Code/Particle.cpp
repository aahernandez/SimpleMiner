#include "Game/Particle.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

Particle::Particle()
	: m_elasticity(0.5f)
{
	m_center = Vector2(0.f, 0.f);
	m_radius = 0.f;
	m_velocity = Vector2(0.f, 0.f);
	m_color = RGBA::WHITE;
}

Particle::Particle(float centerX, float centerY, float radius, ParticleType particleType, RGBA color)
	: m_elasticity(0.5f)
	, m_particleType(particleType)
{
	m_center = Vector2(centerX, centerY);
	m_radius = radius;
	m_velocity = Vector2(0.f, 0.f);
	m_color = color;
}

Particle::Particle(Vector2 center, Vector2 initialVelocity, float radius, ParticleType particleType, RGBA color)
	: m_particleType(particleType)
{
	m_center = center;
	m_velocity = initialVelocity;
	m_radius = radius;
	m_color = color;
}

Particle::~Particle()
{

}

void Particle::Update(float deltaSeconds, float gravity)
{
	m_deathTime -= deltaSeconds;
	float randFloat = GetRandomFloatZeroToOne();
	float randFloat2 = GetRandomFloatZeroToOne();

	switch (m_particleType)
	{
	case PARTICLE_TYPE_EXPLOSION:
		m_center += (m_velocity * deltaSeconds);
		break;
	case PARTICLE_TYPE_FIREWORK:
		m_velocity.y += (gravity * deltaSeconds);
		m_center += (m_velocity * deltaSeconds);
		break;
	case PARTICLE_TYPE_FOUNTAIN:
		m_velocity.y += (gravity * deltaSeconds);
		m_center += (m_velocity * deltaSeconds);
		break;
	case PARTICLE_TYPE_DEBRIS:
		m_velocity.y += (gravity * deltaSeconds);
		m_center += (m_velocity * deltaSeconds);
		if (m_center.y < 50.f)
		{
			m_center.y = 50.f;
			m_velocity.y = m_velocity.y * -0.5f;
			m_velocity.x = m_velocity.x * 0.7f;
		}
		break;
	case PARTICLE_TYPE_SMOKE:
		if (randFloat > 0.9f)
		{
			m_velocity.y = GetRandomFloatInRange(-10.f, 15.f);
		}
		m_center += (m_velocity * deltaSeconds);
		break;
	case PARTICLE_TYPE_PERSONAL:
		if (randFloat > 0.9f)
		{
			m_velocity.y = GetRandomFloatInRange(-10.f, 15.f);
		}
		if (randFloat2 > 0.9f)
		{
			m_velocity.x = GetRandomFloatInRange(-10.f, 15.f);
		}
		m_center += (m_velocity * deltaSeconds);
		break;
	}
}

void Particle::Render() const
{
	int numVertices = 0;
	switch (m_particleType)
	{
	case PARTICLE_TYPE_EXPLOSION:
		numVertices = 25;
		break;
	case PARTICLE_TYPE_FIREWORK:
		numVertices = 5;
		break;
	case PARTICLE_TYPE_FOUNTAIN:
		numVertices = 6;
		break;
	case PARTICLE_TYPE_DEBRIS:
		numVertices = 4;
		break;
	case PARTICLE_TYPE_SMOKE:
		numVertices = 10;
		break;
	case PARTICLE_TYPE_PERSONAL:
		numVertices = 25;
		break;
	}
	g_theRenderer->DrawDiscClosed2D(m_center, m_radius, numVertices, m_color);
}

void Particle::ChangeSize(float scale)
{
	m_radius *= scale;
}
