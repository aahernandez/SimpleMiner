#pragma once
#include "Engine/Math/Disc2D.hpp"
#include "Engine/Core/RGBA.hpp"

enum ParticleType
{
	PARTICLE_TYPE_EXPLOSION,
	PARTICLE_TYPE_FIREWORK,
	PARTICLE_TYPE_FOUNTAIN,
	PARTICLE_TYPE_DEBRIS,
	PARTICLE_TYPE_SMOKE,
	PARTICLE_TYPE_PERSONAL
};

class Particle : Disc2D
{
public:
	ParticleType m_particleType;
	Vector2 m_velocity;
	RGBA m_color;
	float m_elasticity;
	float m_deathTime;

	Particle();
	Particle(float centerX, float centerY, float radius, ParticleType particleType, RGBA color);
	Particle(Vector2 center, Vector2 initialVelocity, float radius, ParticleType particleType, RGBA color);
	~Particle();
	void Update(float deltaSeconds, float gravity);
	void Render() const;
	void ChangeSize(float scale);
};