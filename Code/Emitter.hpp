#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Game/Particle.hpp"
#include <vector>

class Emitter
{
public:
	float m_particleRadius;
	Vector2 m_cloudSpawnPoint;
	std::vector<Particle*> particles;

	Emitter(float particleRadius);
	~Emitter();

	void Update(float deltaSeconds);
	void Render();

	void SpawnParticles(ParticleType emitterType);
	void EmitExplosion(int numParticles);
	void EmitFirework(int numParticles);
	void EmitFountain(int numParticles);
	void EmitDebris(int numParticles);
	void EmitSmoke(int numParticles);
	void EmitPersonal(int numParticles);
	void ClearParticles();
};