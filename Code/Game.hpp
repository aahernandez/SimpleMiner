//Some code based off of code by Squirrel Eiserloh
#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimation.hpp"
#include "Engine/Math/MathUtilities.hpp"
#include "Engine/Input/XboxControl.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/BlockInfo.hpp"
#include "Game/Camera3D.hpp"
#include "Game/Player.hpp"
#include <vector>
#include <map>

class World;

//inputZeroToOne * inputZeroToOne * something
const float MOVE_SPEED = 1.0f;
const float JUMP_VELOCITY = 0.5f;

class Game
{
public:
	bool		m_isPaused;
	bool		m_isQuitting;
	bool		m_isDrawingPlayer;
	bool		m_isRaining;
	float		m_playerSightDistance;
	float		m_gravity;
	float		m_skyboxAlpha;
	float		m_rainPerlinNoise;
	World*		m_world;
	Player		m_player;
	Camera3D	m_camera;
	BlockInfo	m_farthestNonOpaqueBlock;
	BlockInfo	m_closestOpaqueBlock;
	SpriteSheet*	m_skyBoxDaySheet;
	SpriteSheet*	m_skyBoxNightSheet;
	SpriteSheet*	m_weatherSheet;
	unsigned char	m_currentlySelectedBlockType;
	SpriteAnimation*	m_rainAnimation[5];
	SpriteAnimation*	m_snowAnimation[5];
	SpriteAnimation*	m_sandStormAnimation[5];
	BlockDefinition*	m_blockDefinitions[BLOCK_TYPE_SIZE];

	Game();
	~Game();

	void Update(float deltaSeconds);
	void UpdatePlayerMovement(float deltaSeconds);
	void UpdatePlayerMouseLook();
	void UpdatePlayerKeyboardMovement(float deltaSeconds);
	void UpdatePlayerPositionCorrective();
	void UpdatePlayerGroundBlockCollisions();
	void UpdatePlayerSideBlockCollisions();
	void UpdatePlayerMiddleSideBlockCollisions();
	void UpdatePlayerSideCardinalBlockCollisions(const BlockInfo& playerCurrentBlock);
	void UpdatePlayerDiagonalBlockCollisions(const BlockInfo& playerBottomCurrentBlock, const Vector3& pointOnPlayer);
	void UpdatePlayerAboveBlockCollisions();
	void UpdatePlayerSight();
	void UpdateWorld(float deltaSeconds);

	void Render() const;
	void RenderPlayer() const;
	void RenderSkyBoxes() const;
	void RenderSkyBoxDay() const;
	void RenderSkyBoxNight() const;
	void RenderWeather() const;
	void RenderPlayerAlwaysVisible() const;
	void RenderCameraView() const;
	void RenderWorld() const;
	void RenderText() const;
	void RenderHUD() const;
	void RenderCrosshairs() const;
	void RenderBlockTypes() const;

	float ChangeSimulationSpeed(float deltaSeconds) const;
	void KeyDown();
	void KeyUp();
	bool IsQuitting();

	void SavePlayerData();
	void GetPlayerData(std::vector< float >& playerData);
	bool LoadPlayerData();
	float CalcSkyboxAlpha();
	void GetClosestOpaqueAndFarthestNonOpaqueBlock(const Vector3& startPos, const Vector3& endPos);
	void PushPlayerAwayFromGroundPoint(BlockInfo& blockUnderPointOnPlayer, const Vector3& pointOnBottomOfPlayer);
	void PushPlayerAwayFromPoint(Vector2& pointOnPlayer, const Vector2& cornerPoint);
	void PushPlayerAwayFromAbovePoint(BlockInfo& blockAbovePointOnPlayer, const Vector3& pointOnTopOfPlayer);
	bool IsPointInsidePointWithRadius(const Vector2& radiusPoint, float radius, const Vector2& point);
};

extern Game* g_theGame;