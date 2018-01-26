#include "Game/Emitter.hpp"
#include "Game/Particle.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"

Emitter::Emitter(float particleRadius)
	: m_particleRadius(particleRadius)
{
	m_cloudSpawnPoint = Vector2(SCREEN_RATIO_WIDTH * 0.5f, SCREEN_RATIO_HEIGHT * 0.5f);
}

Emitter::~Emitter()
{

}

void Emitter::Update(float deltaSeconds)
{
	float moveSpeed = 10.f;
	if (g_theInput->IsKeyDown(KEY_UPARROW))
	{
		m_cloudSpawnPoint.y += moveSpeed;
	}
	if (g_theInput->IsKeyDown(KEY_DOWNARROW))
	{
		m_cloudSpawnPoint.y -= moveSpeed;
	}
	if (g_theInput->IsKeyDown(KEY_LEFTARROW))
	{
		m_cloudSpawnPoint.x -= moveSpeed;
	}
	if (g_theInput->IsKeyDown(KEY_RIGHTARROW))
	{
		m_cloudSpawnPoint.x += moveSpeed;
	}

	std::vector<Particle*>::iterator iter;
	for (iter = particles.begin(); iter != particles.end(); ++iter)
	{
		switch ((*iter)->m_particleType)
		{
		case PARTICLE_TYPE_EXPLOSION:
			(*iter)->Update(deltaSeconds, 0.f);
			break;
		case PARTICLE_TYPE_FIREWORK:
			(*iter)->Update(deltaSeconds, -20.f);
			break;
		case PARTICLE_TYPE_FOUNTAIN:
			(*iter)->Update(deltaSeconds, -20.f);
			break;
		case PARTICLE_TYPE_DEBRIS:
			(*iter)->Update(deltaSeconds, -40.f);
			break;
		case PARTICLE_TYPE_SMOKE:
			(*iter)->Update(deltaSeconds, -20.f);
			break;
		case PARTICLE_TYPE_PERSONAL:
			(*iter)->Update(deltaSeconds, -20.f);
			break;
		}
	}
}

void Emitter::Render()
{
	std::vector<Particle*>::iterator iter;
	for (iter = particles.begin(); iter != particles.end(); ++iter)
	{
		(*iter)->Render();
	}
}

void Emitter::SpawnParticles(ParticleType emitter)
{
	switch (emitter)
	{
	case PARTICLE_TYPE_EXPLOSION:
		EmitExplosion(20);
		break;
	case PARTICLE_TYPE_FIREWORK:
		EmitFirework(20);
		break;
	case PARTICLE_TYPE_FOUNTAIN:
		EmitFountain(1);
		break;
	case PARTICLE_TYPE_DEBRIS:
		EmitDebris(20);
		break;
	case PARTICLE_TYPE_SMOKE:
		EmitSmoke(1);
		break;
	case PARTICLE_TYPE_PERSONAL:
		EmitPersonal(1);
		break;
	}
}

void Emitter::EmitExplosion(int numParticles)
{
	for (int i = 0; i < numParticles; i++)
	{
		float yVel = GetRandomFloatInRange(-50.f, 50.f);
		float xVel = GetRandomFloatInRange(-50.f, 50.f);
		RGBA color(GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne());
		particles.push_back(new Particle(Vector2(SCREEN_RATIO_WIDTH * 0.5f, SCREEN_RATIO_HEIGHT * 0.5f), Vector2(xVel, yVel), m_particleRadius, PARTICLE_TYPE_EXPLOSION, color));
	}
}

void Emitter::EmitFirework(int numParticles)
{
	for (int i = 0; i < numParticles; i++)
	{
		float yVel = GetRandomFloatInRange(-50.f, 50.f);
		float xVel = GetRandomFloatInRange(-50.f, 50.f);
		RGBA color(GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne());
		particles.push_back(new Particle(Vector2(SCREEN_RATIO_WIDTH * 0.5f, SCREEN_RATIO_HEIGHT * 0.5f), Vector2(xVel, yVel), m_particleRadius, PARTICLE_TYPE_FIREWORK, color));
	}
}

void Emitter::EmitFountain(int numParticles)
{
	for (int i = 0; i < numParticles; i++)
	{
		float yVel = GetRandomFloatInRange(90.f, 100.f);
		float xVel = GetRandomFloatInRange(-35.f, 35.f);
		RGBA color(GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne());
		particles.push_back(new Particle(Vector2(SCREEN_RATIO_WIDTH * 0.5f, SCREEN_RATIO_HEIGHT * 0.5f), Vector2(xVel, yVel), m_particleRadius, PARTICLE_TYPE_FOUNTAIN, color));
	}
}

void Emitter::EmitDebris(int numParticles)
{
	for (int i = 0; i < numParticles; i++)
	{
		float yVel = GetRandomFloatInRange(60.f, 110.f);
		float xVel = GetRandomFloatInRange(-50.f, 50.f);
		RGBA color(GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne());
		particles.push_back(new Particle(Vector2(SCREEN_RATIO_WIDTH * 0.5f, SCREEN_RATIO_HEIGHT * 0.2f), Vector2(xVel, yVel), m_particleRadius, PARTICLE_TYPE_DEBRIS, color));
	}
}

void Emitter::EmitSmoke(int numParticles)
{
	for (int i = 0; i < numParticles; i++)
	{
		float yVel = GetRandomFloatInRange(-10.f, 10.f);
		float xVel = GetRandomFloatInRange(-50.f, 50.f);
		if (xVel < 0)
		{
			xVel = -50.f;
		}
		else 
		{
			xVel = 50.f;
		}
		float ySpawn = GetRandomFloatInRange(-10.f, 10.f);
		particles.push_back(new Particle(Vector2(SCREEN_RATIO_WIDTH * 0.5f, (SCREEN_RATIO_HEIGHT * 0.2f) + ySpawn), Vector2(xVel, yVel), m_particleRadius, PARTICLE_TYPE_SMOKE, RGBA::SILVER));
	}
}

void Emitter::EmitPersonal(int numParticles)
{
	for (int i = 0; i < numParticles; i++)
	{
		float yVel = GetRandomFloatInRange(-50.f, 50.f);
		float xVel = GetRandomFloatInRange(-50.f, 50.f);
		if (xVel < 0)
			xVel = -50.f;
		else
			xVel = 50.f;
		if (yVel < 0)
			yVel = -50.f;
		else
			yVel = 50.f;
		float ySpawn = GetRandomFloatInRange(-10.f, 10.f);
		float xSpawn = GetRandomFloatInRange(-10.f, 10.f);
		particles.push_back(new Particle(Vector2(m_cloudSpawnPoint.x + xSpawn, m_cloudSpawnPoint.y + ySpawn), Vector2(xVel, yVel), m_particleRadius, PARTICLE_TYPE_PERSONAL, RGBA::TEAL));
	}
}

void Emitter::ClearParticles()
{
	particles.clear();
}
