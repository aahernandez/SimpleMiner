//Some code based off of code by Squirrel Eiserloh
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Noise.hpp"

Game* g_theGame = nullptr;

Game::Game()
	: m_isPaused(false)
	, m_isQuitting(false)
	, m_isDrawingPlayer(false)
	, m_isRaining(true)
	, m_playerSightDistance(8.f)
	, m_gravity(9.8f)
	, m_currentlySelectedBlockType(BLOCK_TYPE_GLOWSTONE)
	, m_skyboxAlpha(1.f)
	, m_rainPerlinNoise(1.f)
{

	if (!g_isSavingAndLoading || !LoadPlayerData())
	{
		m_camera.m_yawAboutZ = m_camera.m_defaultOrientation.z;
		m_camera.m_pitchAboutY = m_camera.m_defaultOrientation.y;
		m_camera.m_rollAboutX = m_camera.m_defaultOrientation.x;
	}

	Texture* skyBoxDayTexture = g_theRenderer->CreateOrGetTexture("Data/Images/Skybox_Day.jpg");
	m_skyBoxDaySheet = new SpriteSheet(skyBoxDayTexture, 4, 3);

	Texture* skyBoxNightTexture = g_theRenderer->CreateOrGetTexture("Data/Images/Skybox_Night.png");
	m_skyBoxNightSheet = new SpriteSheet(skyBoxNightTexture, 4, 3);

	Texture* blockSheet = g_theRenderer->CreateOrGetTexture("Data/Images/SimpleMinerAtlas.png");
	m_weatherSheet = new SpriteSheet(blockSheet, 16, 16);
	for (int numAnimations = 0; numAnimations < 5; ++numAnimations)
	{
		float fractionElapsed = (float) numAnimations / 4.f;
		m_rainAnimation[numAnimations] = new SpriteAnimation(*m_weatherSheet, 0.5f, SPRITE_ANIM_MODE_LOOPING, 32, 36);
		m_rainAnimation[numAnimations]->SetFractionElapsed(fractionElapsed);

		m_snowAnimation[numAnimations] = new SpriteAnimation(*m_weatherSheet, 0.5f, SPRITE_ANIM_MODE_LOOPING, 48, 52);
		m_snowAnimation[numAnimations]->SetFractionElapsed(fractionElapsed);

		m_sandStormAnimation[numAnimations] = new SpriteAnimation(*m_weatherSheet, 0.5f, SPRITE_ANIM_MODE_LOOPING, 64, 68);
		m_snowAnimation[numAnimations]->SetFractionElapsed(fractionElapsed);
	}

	m_world = new World();
}

Game::~Game()
{
	delete m_world;
	m_world = nullptr;
}

void Game::Update(float deltaSeconds)
{
	deltaSeconds  = ChangeSimulationSpeed(deltaSeconds);
	UpdatePlayerMovement(deltaSeconds);
	UpdatePlayerSight();
	KeyDown();
	KeyUp();
	UpdateWorld(deltaSeconds);
}

void Game::UpdatePlayerMovement(float deltaSeconds)
{
	UpdatePlayerMouseLook();
	UpdatePlayerKeyboardMovement(deltaSeconds);
	UpdatePlayerPositionCorrective();
}

void Game::UpdatePlayerMouseLook()
{
	if (!g_theInput->DoesAppHaveFocus() || m_camera.m_cameraMode == FIXED_ANGLE)
		return;

	const float YAW_DEGREES_PER_MOUSE_MOVE = -0.05f;
	const float PITCH_DEGREES_PER_MOUSE_MOVE = 0.03f;

	Vector2 screenSize = g_theInput->GetScreenSize();
	Vector2 mouseRecentPos = screenSize * 0.5f;
	Vector2 mouseScreenCoords = g_theInput->GetMouseScreenCoords();
	Vector2 mouseMoveSinceLastFrame = mouseScreenCoords - mouseRecentPos;

	g_theInput->SetMouseScreenCoords(mouseRecentPos);
	m_camera.m_pitchAboutY += PITCH_DEGREES_PER_MOUSE_MOVE * mouseMoveSinceLastFrame.y;
	m_camera.m_pitchAboutY = Clamp(m_camera.m_pitchAboutY, -89.9f, 89.9f);
	m_camera.m_yawAboutZ += YAW_DEGREES_PER_MOUSE_MOVE * mouseMoveSinceLastFrame.x;
}

void Game::UpdatePlayerKeyboardMovement(float deltaSeconds)
{
	bool isMoving = false;

	if (!g_theInput->DoesAppHaveFocus())
		return;
	if (g_theInput->IsKeyDown(KEY_SHIFT))
		deltaSeconds = deltaSeconds * 8.f;

	const float moveDistanceThisFrame = MOVE_SPEED * deltaSeconds;
	Vector3 cameraForwardXY = m_camera.GetForwardXY();
	Vector3 cameraLeftXY = m_camera.GetLeftXY();
	Vector3 moveDisplacementThisFrame(0.f, 0.f, 0.f);

	if (g_theInput->IsKeyDown('W'))
	{
		moveDisplacementThisFrame += cameraForwardXY;
		isMoving = true;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		moveDisplacementThisFrame -= cameraForwardXY;
		isMoving = true;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		moveDisplacementThisFrame += cameraLeftXY;
		isMoving = true;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		moveDisplacementThisFrame -= cameraLeftXY;
		isMoving = true;
	}

	if ( (!m_player.IsPhysicsWalking() && g_theInput->IsKeyDown(' ')) )
	{
		moveDisplacementThisFrame.z += 1.f;
	}
	else if (m_player.IsPhysicsWalking() && g_theInput->WasKeyJustPressed(' ') && m_player.IsOnGround())//&& m_player.IsOnGround())
	{
		m_player.AddVelocity( Vector3(0.f, 0.f, JUMP_VELOCITY) );
		m_player.SetIsOnGround(false);
// 		moveDisplacementThisFrame.z += MOVE_SPEED * 10.f;
	}
	else if (m_player.IsPhysicsWalking() && m_camera.m_cameraMode != NO_CLIP)
	{
		moveDisplacementThisFrame.z -= m_gravity * deltaSeconds;
		m_player.CorrectTerminalVelocityDown();
	}

	if (!m_player.IsPhysicsWalking() && g_theInput->IsKeyDown('Z'))
		moveDisplacementThisFrame.z -= 1.0f;


	if (m_camera.m_cameraMode == NO_CLIP)
	{
		m_camera.m_position += (moveDisplacementThisFrame * moveDistanceThisFrame);
	}
	else
	{
		m_player.AddVelocity(moveDisplacementThisFrame * moveDistanceThisFrame);
		m_player.AddMovementToCenter(m_player.GetVelocity());
// 		m_player.AddMovementToCenter(moveDisplacementThisFrame * moveDistanceThisFrame);
		m_camera.SetCameraPositionAndOrientation( Vector3( m_player.GetCenter().x, m_player.GetCenter().y, m_player.GetCenter().z + m_player.GetEyeHeightAboveCenter() ) );

// 		m_player.SlowPlayerVertically(deltaSeconds);
	}

	if (!m_player.IsPhysicsWalking())
	{
		m_player.SlowPlayerVertically();
	}

// 	if (m_player.IsOnGround())
		m_player.UpdatePlayer();
}

void Game::UpdatePlayerPositionCorrective()
{
	UpdatePlayerMiddleSideBlockCollisions();

	UpdatePlayerGroundBlockCollisions();

	UpdatePlayerSideBlockCollisions();

	UpdatePlayerAboveBlockCollisions();

	m_camera.SetCameraPositionAndOrientation( Vector3( m_player.GetCenter().x, m_player.GetCenter().y, m_player.GetCenter().z + m_player.GetEyeHeightAboveCenter() ) );
}

void Game::UpdatePlayerGroundBlockCollisions()
{
	BlockInfo blockUnderPointOnPlayer;
	Vector3 originalPointOnBottomOfPlayer = m_player.GetBottomCenter();

	Vector3 pointOnBottomOfPlayer = m_player.GetBottomCenter();
	pointOnBottomOfPlayer.x += m_player.GetRadius();
	blockUnderPointOnPlayer.SetBlockInfoFromWorldCoords(pointOnBottomOfPlayer);
	PushPlayerAwayFromGroundPoint(blockUnderPointOnPlayer, pointOnBottomOfPlayer);

	pointOnBottomOfPlayer = m_player.GetBottomCenter();
	pointOnBottomOfPlayer.x -= m_player.GetRadius();
	blockUnderPointOnPlayer.SetBlockInfoFromWorldCoords(pointOnBottomOfPlayer);
	PushPlayerAwayFromGroundPoint(blockUnderPointOnPlayer, pointOnBottomOfPlayer);

	pointOnBottomOfPlayer = m_player.GetBottomCenter();
	pointOnBottomOfPlayer.y += m_player.GetRadius();
	blockUnderPointOnPlayer.SetBlockInfoFromWorldCoords(pointOnBottomOfPlayer);
	PushPlayerAwayFromGroundPoint(blockUnderPointOnPlayer, pointOnBottomOfPlayer);

	pointOnBottomOfPlayer = m_player.GetBottomCenter();
	pointOnBottomOfPlayer.y -= m_player.GetRadius();
	blockUnderPointOnPlayer.SetBlockInfoFromWorldCoords(pointOnBottomOfPlayer);
	PushPlayerAwayFromGroundPoint(blockUnderPointOnPlayer, pointOnBottomOfPlayer);
}

void Game::UpdatePlayerSideBlockCollisions()
{
	BlockInfo currentBlockOfPointInPlayer;
	currentBlockOfPointInPlayer.SetBlockInfoFromWorldCoords(m_player.GetBottomCenter());
	UpdatePlayerSideCardinalBlockCollisions(currentBlockOfPointInPlayer);
	UpdatePlayerDiagonalBlockCollisions(currentBlockOfPointInPlayer, m_player.GetBottomCenter());

// 	currentBlockOfPointInPlayer.SetBlockInfoFromWorldCoords(m_player.GetCenter());
// 	UpdatePlayerSideCardinalBlockCollisions(currentBlockOfPointInPlayer);
// 	UpdatePlayerDiagonalBlockCollisions(currentBlockOfPointInPlayer, m_player.GetCenter());

// 	currentBlockOfPointInPlayer.SetBlockInfoFromWorldCoords(m_player.GetTopCenter());
// 	UpdatePlayerSideCardinalBlockCollisions(currentBlockOfPointInPlayer);
// 	UpdatePlayerDiagonalBlockCollisions(currentBlockOfPointInPlayer, m_player.GetTopCenter());
}

void Game::UpdatePlayerMiddleSideBlockCollisions()
{
// 	BlockInfo currentBlockOfPointInPlayer;

// 	currentBlockOfPointInPlayer.SetBlockInfoFromWorldCoords(m_player.GetCenter());
// 	UpdatePlayerSideCardinalBlockCollisions(currentBlockOfPointInPlayer);
// 	UpdatePlayerDiagonalBlockCollisions(currentBlockOfPointInPlayer, m_player.GetCenter());

// 	currentBlockOfPointInPlayer.SetBlockInfoFromWorldCoords(m_player.GetTopCenter());
// 	UpdatePlayerSideCardinalBlockCollisions(currentBlockOfPointInPlayer);
// 	UpdatePlayerDiagonalBlockCollisions(currentBlockOfPointInPlayer, m_player.GetTopCenter());
}

void Game::UpdatePlayerSideCardinalBlockCollisions(const BlockInfo& playerCurrentBlock)
{
	BlockInfo eastBlock = playerCurrentBlock.GetEastNeighbor();
	BlockInfo westBlock = playerCurrentBlock.GetWestNeighbor();
	BlockInfo northBlock = playerCurrentBlock.GetNorthNeighbor();
	BlockInfo southBlock = playerCurrentBlock.GetSouthNeighbor();
	Vector3 playerPos = m_player.GetBottomCenter();
	float playerRadius = m_player.GetRadius();


	if (northBlock.m_chunk != nullptr && northBlock.IsBlockSolid())
	{
		float playerMaxY = playerPos.y + playerRadius;
		float northTileMinY = northBlock.GetWorldPosOfBlock().y;
		float overlapDist = playerMaxY - northTileMinY;

		if (overlapDist > 0.f)// && overlapDist < 0.1f)
			m_player.SetCenter(Vector3(playerPos.x, playerPos.y - overlapDist, m_player.GetCenter().z));
	}

	if (southBlock.m_chunk != nullptr && southBlock.IsBlockSolid())
	{
		float playerMinY = playerPos.y - playerRadius;
		float southTileMaxY = southBlock.GetWorldPosOfBlock().y + 1.f;
		float overlapDist = southTileMaxY - playerMinY;

		if (overlapDist > 0.f)// && overlapDist < 0.1f)
			m_player.SetCenter(Vector3(playerPos.x, playerPos.y + overlapDist, m_player.GetCenter().z));
	}

	if (eastBlock.m_chunk != nullptr && eastBlock.IsBlockSolid())
	{
		float playerMaxX = playerPos.x + playerRadius;
		float eastBlockMinX = eastBlock.GetWorldPosOfBlock().x;
		float overlapDist = playerMaxX - eastBlockMinX;

		if (overlapDist > 0.f)// && overlapDist < 0.1f)
			m_player.SetCenter(Vector3(playerPos.x - overlapDist, playerPos.y, m_player.GetCenter().z));
	}

	if (westBlock.m_chunk != nullptr && westBlock.IsBlockSolid())
	{
		float playerMinX = playerPos.x - playerRadius;
		float westTileMaxX = westBlock.GetWorldPosOfBlock().x + 1.f;
		float overlapDist = westTileMaxX - playerMinX;

		if (overlapDist > 0.f)// && overlapDist < 0.1f)
			m_player.SetCenter(Vector3(playerPos.x + overlapDist, playerPos.y, m_player.GetCenter().z));
	}


}

void Game::UpdatePlayerDiagonalBlockCollisions(const BlockInfo& playerCurrentBlock, const Vector3& pointOnPlayer)
{
	BlockInfo nEastBlock = playerCurrentBlock.GetNorthEastNeighbor();
	BlockInfo nWestBlock = playerCurrentBlock.GetNorthWestNeighbor();
	BlockInfo sEastBlock = playerCurrentBlock.GetSouthEastNeighbor();
	BlockInfo sWestBlock = playerCurrentBlock.GetSouthWestNeighbor();

	if ( nEastBlock.m_chunk != nullptr && nEastBlock.IsBlockSolid() )
	{
		Vector3 cornerPoint = nEastBlock.GetWorldPosOfBlock();
		Vector2 pointInPlayer = Vector2(pointOnPlayer.x, pointOnPlayer.y);
		PushPlayerAwayFromPoint( pointInPlayer, Vector2(cornerPoint.x, cornerPoint.y) );
	}

	if ( nWestBlock.m_chunk != nullptr && nWestBlock.IsBlockSolid() )
	{
		Vector3 cornerPoint = nWestBlock.GetWorldPosOfBlock();
		cornerPoint.x += 1.f;
		Vector2 pointInPlayer = Vector2(pointOnPlayer.x, pointOnPlayer.y);
		PushPlayerAwayFromPoint(pointInPlayer, Vector2(cornerPoint.x, cornerPoint.y));
	}

	if ( sEastBlock.m_chunk != nullptr && sEastBlock.IsBlockSolid() )
	{
		Vector3 cornerPoint = sEastBlock.GetWorldPosOfBlock();
		cornerPoint.y += 1.f;
		Vector2 pointInPlayer = Vector2(pointOnPlayer.x, pointOnPlayer.y);
		PushPlayerAwayFromPoint(pointInPlayer, Vector2(cornerPoint.x, cornerPoint.y));
	}

	if ( sWestBlock.m_chunk != nullptr && sWestBlock.IsBlockSolid() )
	{
		Vector3 cornerPoint = sWestBlock.GetWorldPosOfBlock();
		cornerPoint.x += 1.f;
		cornerPoint.y += 1.f;
		Vector2 pointInPlayer = Vector2(pointOnPlayer.x, pointOnPlayer.y);
		PushPlayerAwayFromPoint(pointInPlayer, Vector2(cornerPoint.x, cornerPoint.y));
	}
}

void Game::UpdatePlayerAboveBlockCollisions()
{
	BlockInfo blockAbovePointOnPlayer;

	Vector3 pointOnTopOfPlayer = m_player.GetTopCenter();
	pointOnTopOfPlayer.x += m_player.GetRadius();
	blockAbovePointOnPlayer.SetBlockInfoFromWorldCoords(pointOnTopOfPlayer);
	PushPlayerAwayFromAbovePoint(blockAbovePointOnPlayer, pointOnTopOfPlayer);

	pointOnTopOfPlayer = m_player.GetTopCenter();
	pointOnTopOfPlayer.x -= m_player.GetRadius();
	blockAbovePointOnPlayer.SetBlockInfoFromWorldCoords(pointOnTopOfPlayer);
	PushPlayerAwayFromAbovePoint(blockAbovePointOnPlayer, pointOnTopOfPlayer);

	pointOnTopOfPlayer = m_player.GetTopCenter();
	pointOnTopOfPlayer.y += m_player.GetRadius();
	blockAbovePointOnPlayer.SetBlockInfoFromWorldCoords(pointOnTopOfPlayer);
	PushPlayerAwayFromAbovePoint(blockAbovePointOnPlayer, pointOnTopOfPlayer);

	pointOnTopOfPlayer = m_player.GetTopCenter();
	pointOnTopOfPlayer.y -= m_player.GetRadius();
	blockAbovePointOnPlayer.SetBlockInfoFromWorldCoords(pointOnTopOfPlayer);
	PushPlayerAwayFromAbovePoint(blockAbovePointOnPlayer, pointOnTopOfPlayer);
}

void Game::UpdatePlayerSight()
{
	Vector3 playerPos = m_player.GetCenter();
	Vector3 playerFwd = m_camera.GetForwardXYZ();
	Vector3 playerSightEnd = playerPos + (playerFwd * m_playerSightDistance);
	GetClosestOpaqueAndFarthestNonOpaqueBlock(playerPos, playerSightEnd);
}

void Game::GetClosestOpaqueAndFarthestNonOpaqueBlock(const Vector3& startPos, const Vector3& endPos)
{
	Vector3 displacement = endPos - startPos;
	Vector3 displacementFraction = displacement * 0.001f;
	Vector3 currentPos = startPos;
	for (int lineOfSightStep = 0; lineOfSightStep < 1000; ++lineOfSightStep)
	{
		m_farthestNonOpaqueBlock.SetBlockInfoFromWorldCoords(currentPos);
		currentPos += displacementFraction;
		m_closestOpaqueBlock.SetBlockInfoFromWorldCoords(currentPos);
		if (m_closestOpaqueBlock.m_chunk != nullptr && m_closestOpaqueBlock.GetBlock()->GetIsOpaque())
			return;
	}
	m_closestOpaqueBlock.m_chunk = nullptr;
	m_farthestNonOpaqueBlock.m_chunk = nullptr;
}

void Game::PushPlayerAwayFromGroundPoint(BlockInfo& blockUnderPointOnPlayer, const Vector3& pointOnBottomOfPlayer)
{
	Vector3 playerPos = m_player.GetCenter();

	if (blockUnderPointOnPlayer.m_chunk != nullptr && blockUnderPointOnPlayer.IsBlockSolid())
	{
		float playerMinZ = pointOnBottomOfPlayer.z;
		float eastDownBlockMaxZ = blockUnderPointOnPlayer.GetWorldPosOfBlock().z + 1.f;
		float overlapDist = eastDownBlockMaxZ - playerMinZ;

		if ( ( overlapDist > 0.f) && overlapDist < 0.1f && m_player.IsOnGround() ) 
		{
			m_player.SetCenter(Vector3(playerPos.x, playerPos.y, playerPos.z + overlapDist));
			m_player.SetIsOnGround(true);
		}
		else if ( ( overlapDist > 0.f) )
		{
			m_player.SetCenter(Vector3(playerPos.x, playerPos.y, playerPos.z + overlapDist));
			m_player.SetIsOnGround(true);
		}
	}
}

void Game::PushPlayerAwayFromPoint(Vector2& pointOnPlayer, const Vector2& cornerPoint)
{
	float radius = m_player.GetRadius();
	if (IsPointInsidePointWithRadius( pointOnPlayer, radius, cornerPoint))
	{
		float distance = pointOnPlayer.CalcDistanceToVector(cornerPoint);
		float amountToPush = radius - distance;
		Vector2 displacement = pointOnPlayer - cornerPoint;
		displacement.Normalize();
		displacement *= amountToPush;
		m_player.AddMovementToCenter( Vector3(displacement.x, displacement.y, 0.f) );
// 		entity->m_center += displacement;
	}
}

void Game::PushPlayerAwayFromAbovePoint(BlockInfo& blockAbovePointOnPlayer, const Vector3& pointOnTopOfPlayer)
{
	Vector3 playerPos = m_player.GetCenter();

	if (blockAbovePointOnPlayer.m_chunk != nullptr && blockAbovePointOnPlayer.IsBlockSolid())
	{
		float playerMaxZ = pointOnTopOfPlayer.z;
		float eastDownAboveMaxZ = blockAbovePointOnPlayer.GetWorldPosOfBlock().z;
		float overlapDist = playerMaxZ - eastDownAboveMaxZ;

		if (overlapDist > 0.f && overlapDist < 0.1f)
			m_player.SetCenter(Vector3(playerPos.x, playerPos.y, playerPos.z - overlapDist));
	}
}

bool Game::IsPointInsidePointWithRadius(const Vector2& radiusPoint, float radius, const Vector2& point)
{
	float distance = CalcDistance(radiusPoint, point);
	if (distance > radius)
	{
		return false;
	}
	return true;
}

void Game::UpdateWorld(float deltaSeconds)
{
	Vector3 playerPos = m_player.GetCenter();
	m_world->Update(deltaSeconds, playerPos, m_camera.GetForwardXYZ(), m_camera.m_position);
	m_skyboxAlpha = CalcSkyboxAlpha();

	for (int numAnimations = 0; numAnimations < 5; ++numAnimations)
	{
		m_rainAnimation[numAnimations]->Update(deltaSeconds);
	}

	m_rainPerlinNoise = Compute1dPerlinNoise( (float) GetCurrentTimeSeconds(), 150.f, 100, 0.3f, 2.f, true, 2 ); //7246
	if (m_rainPerlinNoise > 0.f || g_isWeatherActive)
	{
		m_isRaining = true;
	}
	else
	{
		m_isRaining = false;
	}
}

void Game::Render() const
{
	RenderCameraView();
	RenderWorld();
	RenderPlayer();
	RenderSkyBoxes();
	RenderWeather();
	RenderHUD();
}

void Game::RenderPlayer() const
{
	if (m_isDrawingPlayer)
	{
		m_player.RenderPlayer();
	}
}

void Game::RenderSkyBoxes() const
{
	RenderSkyBoxNight();
	RenderSkyBoxDay();
}

void Game::RenderSkyBoxDay() const
{
	std::vector< Vertex3_PCT >	vertexArray;
	vertexArray.resize(8);

	AABB2D texBoundsZUp = m_skyBoxDaySheet->GetTexCoordsForSpriteCoords(1, 0);
	AABB2D texBoundsXLeft = m_skyBoxDaySheet->GetTexCoordsForSpriteCoords(2, 1);
	AABB2D texBoundsYForward = m_skyBoxDaySheet->GetTexCoordsForSpriteCoords(1, 1);
	AABB2D texBoundsXRight = m_skyBoxDaySheet->GetTexCoordsForSpriteCoords(0, 1);
	AABB2D texBoundsYBack = m_skyBoxDaySheet->GetTexCoordsForSpriteCoords(3, 1);
	AABB2D texBoundsZDown = m_skyBoxDaySheet->GetTexCoordsForSpriteCoords(1, 2);
	float boxSideRadus = 399.f;
	RGBA skyBoxColor = RGBA::WHITE;
	skyBoxColor.a = (unsigned char)(m_skyboxAlpha * 255);

	//down
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsZDown.maxs.x, texBoundsZDown.mins.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsZDown.maxs));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsZDown.mins.x, texBoundsZDown.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsZDown.mins));

	//up
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsZUp.maxs.x, texBoundsZUp.mins.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsZUp.maxs));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsZUp.mins.x, texBoundsZUp.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsZUp.mins));

	// 	//north
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsYForward.maxs));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsYForward.mins.x, texBoundsYForward.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsYForward.mins));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsYForward.maxs.x, texBoundsYForward.mins.y)));

	// 	//south
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsYBack.mins.x, texBoundsYBack.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsYBack.mins));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsYBack.maxs.x, texBoundsYBack.mins.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsYBack.maxs));

	// 	//east
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsXRight.mins.x, texBoundsXRight.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsXRight.mins));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsXRight.maxs.x, texBoundsXRight.mins.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsXRight.maxs));

	// 	//west
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsXLeft.maxs));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsXLeft.mins.x, texBoundsXLeft.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsXLeft.mins));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsXLeft.maxs.x, texBoundsXLeft.mins.y)));

	g_theRenderer->BindTexture2D(m_skyBoxDaySheet->GetSpriteSheetTexture());

	int numVerts = (int)vertexArray.size();
	g_theRenderer->DrawVertexArray3D_PCT(&vertexArray[0], numVerts, PRIMITIVE_QUADS);
}

void Game::RenderSkyBoxNight() const
{
	std::vector< Vertex3_PCT >	vertexArray;
	vertexArray.resize(8);

	AABB2D texBoundsZUp = m_skyBoxNightSheet->GetTexCoordsForSpriteCoords(1, 0);
	AABB2D texBoundsXLeft = m_skyBoxNightSheet->GetTexCoordsForSpriteCoords(2, 1);
	AABB2D texBoundsYForward = m_skyBoxNightSheet->GetTexCoordsForSpriteCoords(1, 1);
	AABB2D texBoundsXRight = m_skyBoxNightSheet->GetTexCoordsForSpriteCoords(0, 1);
	AABB2D texBoundsYBack = m_skyBoxNightSheet->GetTexCoordsForSpriteCoords(3, 1);
	AABB2D texBoundsZDown = m_skyBoxNightSheet->GetTexCoordsForSpriteCoords(1, 2);
	float boxSideRadus = 400.f;
	RGBA skyBoxColor = RGBA::WHITE;
	skyBoxColor.a = (unsigned char)( (1.f - m_skyboxAlpha) * 255);

	//down
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsZDown.maxs.x, texBoundsZDown.mins.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsZDown.maxs));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsZDown.mins.x, texBoundsZDown.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsZDown.mins));

	//up
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsZUp.maxs.x, texBoundsZUp.mins.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsZUp.maxs));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsZUp.mins.x, texBoundsZUp.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsZUp.mins));

	// 	//north
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsYForward.maxs));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsYForward.mins.x, texBoundsYForward.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsYForward.mins));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsYForward.maxs.x, texBoundsYForward.mins.y)));

	// 	//south
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsYBack.mins.x, texBoundsYBack.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsYBack.mins));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsYBack.maxs.x, texBoundsYBack.mins.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsYBack.maxs));

	// 	//east
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsXRight.mins.x, texBoundsXRight.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsXRight.mins));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsXRight.maxs.x, texBoundsXRight.mins.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x + boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsXRight.maxs));

	// 	//west
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, texBoundsXLeft.maxs));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z - boxSideRadus), skyBoxColor, Vector2(texBoundsXLeft.mins.x, texBoundsXLeft.maxs.y)));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y + boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, texBoundsXLeft.mins));
	vertexArray.push_back(Vertex3_PCT(Vector3(m_player.GetCenter().x - boxSideRadus, m_player.GetCenter().y - boxSideRadus, m_player.GetCenter().z + boxSideRadus), skyBoxColor, Vector2(texBoundsXLeft.maxs.x, texBoundsXLeft.mins.y)));

	g_theRenderer->BindTexture2D(m_skyBoxNightSheet->GetSpriteSheetTexture());

	int numVerts = (int)vertexArray.size();
	g_theRenderer->DrawVertexArray3D_PCT(&vertexArray[0], numVerts, PRIMITIVE_QUADS);
}

void Game::RenderWeather() const
{
	if (g_isWeatherActive || m_isRaining)
	{
		const int numRainAway = 4;
		const int numRainWide = 8;
		const int numRainTall = 6;

		std::vector< Vertex3_PCT >	vertexArray;
		BlockInfo playerBlock;
		playerBlock.SetBlockInfoFromWorldCoords(m_player.GetCenter());
		BlockInfo northBlockStart;
		BlockInfo southBlockStart;
		BlockInfo eastBlockStart;
		BlockInfo westBlockStart;
		northBlockStart = playerBlock;
		southBlockStart = playerBlock;
		eastBlockStart = playerBlock;
		westBlockStart = playerBlock;

		for (int blockPositionAway = 0; blockPositionAway < numRainAway; ++blockPositionAway)
		{
			northBlockStart = northBlockStart.GetNorthNeighbor();
			southBlockStart = southBlockStart.GetSouthNeighbor();
			eastBlockStart = eastBlockStart.GetEastNeighbor();
			westBlockStart = westBlockStart.GetWestNeighbor();
		}
		for (int blockPositionWide = 0; blockPositionWide < numRainWide; blockPositionWide += 2)
		{
			northBlockStart = northBlockStart.GetWestNeighbor();
			southBlockStart = southBlockStart.GetWestNeighbor();
			eastBlockStart = eastBlockStart.GetSouthNeighbor();
			westBlockStart = westBlockStart.GetSouthNeighbor();
		}
		for (int blockPositionTall = 0; blockPositionTall < numRainTall; blockPositionTall += 2)
		{
			northBlockStart = northBlockStart.GetBelowNeighbor();
			southBlockStart = southBlockStart.GetBelowNeighbor();
			eastBlockStart = eastBlockStart.GetBelowNeighbor();
			westBlockStart = westBlockStart.GetBelowNeighbor();
		}

		for (int blockPositionAway = 0; blockPositionAway < numRainAway; ++blockPositionAway)
		{
			northBlockStart = northBlockStart.GetSouthNeighbor();
			southBlockStart = southBlockStart.GetNorthNeighbor();
			eastBlockStart = eastBlockStart.GetWestNeighbor();
			westBlockStart = westBlockStart.GetEastNeighbor();
			BlockInfo northBlockToEast = northBlockStart;
			BlockInfo southBlockToEast = southBlockStart;
			BlockInfo eastBlockToNorth = eastBlockStart;
			BlockInfo westBlockToNorth = westBlockStart;

			RGBA color = RGBA::WHITE;
			float colorValue = (float) blockPositionAway + 1.f / (float) numRainAway + 2.f;
			unsigned char colorValueChar = (unsigned char) (colorValue * 255.f);
			color.a = colorValueChar;

			for (int blockPositionWidth = 0; blockPositionWidth < numRainWide; ++blockPositionWidth)
			{
				northBlockToEast = northBlockToEast.GetEastNeighbor();
				southBlockToEast = southBlockToEast.GetEastNeighbor();
				eastBlockToNorth = eastBlockToNorth.GetNorthNeighbor();
				westBlockToNorth = westBlockToNorth.GetNorthNeighbor();
				
				BlockInfo northBlockUp = northBlockToEast;
				BlockInfo southBlockUp = southBlockToEast;
				BlockInfo eastBlockUp = eastBlockToNorth;
				BlockInfo westBlockUp = westBlockToNorth;

				for (int blockPositionTall = 0; blockPositionTall < numRainTall; ++blockPositionTall)
				{
					northBlockUp = northBlockUp.GetAboveNeighbor();
					southBlockUp = southBlockUp.GetAboveNeighbor();
					eastBlockUp = eastBlockUp.GetAboveNeighbor();
					westBlockUp = westBlockUp.GetAboveNeighbor();

					float animationIndexNoise = Compute3dFractalNoise( (float) blockPositionAway, (float) blockPositionWidth, (float) blockPositionTall);
					animationIndexNoise += 1.f;
					animationIndexNoise *= 2.5f;
					int animationIndex = (int) animationIndexNoise;
					AABB2D texBoundsRain = m_rainAnimation[animationIndex]->GetCurrentTexCoords();

					vertexArray.push_back(Vertex3_PCT(Vector3(northBlockUp.GetWorldPosOfBlock().x - 0.f, northBlockUp.GetWorldPosOfBlock().y + 1.f, northBlockUp.GetWorldPosOfBlock().z - 0.f), color, texBoundsRain.maxs));
					vertexArray.push_back(Vertex3_PCT(Vector3(northBlockUp.GetWorldPosOfBlock().x + 1.f, northBlockUp.GetWorldPosOfBlock().y + 1.f, northBlockUp.GetWorldPosOfBlock().z - 0.f), color, Vector2(texBoundsRain.mins.x, texBoundsRain.maxs.y)));
					vertexArray.push_back(Vertex3_PCT(Vector3(northBlockUp.GetWorldPosOfBlock().x + 1.f, northBlockUp.GetWorldPosOfBlock().y + 1.f, northBlockUp.GetWorldPosOfBlock().z + 1.f), color, texBoundsRain.mins));
					vertexArray.push_back(Vertex3_PCT(Vector3(northBlockUp.GetWorldPosOfBlock().x - 0.f, northBlockUp.GetWorldPosOfBlock().y + 1.f, northBlockUp.GetWorldPosOfBlock().z + 1.f), color, Vector2(texBoundsRain.maxs.x, texBoundsRain.mins.y)));

					vertexArray.push_back(Vertex3_PCT(Vector3(southBlockUp.GetWorldPosOfBlock().x - 0.f, southBlockUp.GetWorldPosOfBlock().y - 0.f, southBlockUp.GetWorldPosOfBlock().z - 0.f), color, Vector2(texBoundsRain.mins.x, texBoundsRain.maxs.y)));
					vertexArray.push_back(Vertex3_PCT(Vector3(southBlockUp.GetWorldPosOfBlock().x - 0.f, southBlockUp.GetWorldPosOfBlock().y - 0.f, southBlockUp.GetWorldPosOfBlock().z + 1.f), color, texBoundsRain.mins));
					vertexArray.push_back(Vertex3_PCT(Vector3(southBlockUp.GetWorldPosOfBlock().x + 1.f, southBlockUp.GetWorldPosOfBlock().y - 0.f, southBlockUp.GetWorldPosOfBlock().z + 1.f), color, Vector2(texBoundsRain.maxs.x, texBoundsRain.mins.y)));
					vertexArray.push_back(Vertex3_PCT(Vector3(southBlockUp.GetWorldPosOfBlock().x + 1.f, southBlockUp.GetWorldPosOfBlock().y - 0.f, southBlockUp.GetWorldPosOfBlock().z - 0.f), color, texBoundsRain.maxs));
				
					vertexArray.push_back(Vertex3_PCT(Vector3(eastBlockUp.GetWorldPosOfBlock().x + 1.f, eastBlockUp.GetWorldPosOfBlock().y - 0.f, eastBlockUp.GetWorldPosOfBlock().z - 0.f), color, Vector2(texBoundsRain.mins.x, texBoundsRain.maxs.y)));
					vertexArray.push_back(Vertex3_PCT(Vector3(eastBlockUp.GetWorldPosOfBlock().x + 1.f, eastBlockUp.GetWorldPosOfBlock().y - 0.f, eastBlockUp.GetWorldPosOfBlock().z + 1.f), color, texBoundsRain.mins));
					vertexArray.push_back(Vertex3_PCT(Vector3(eastBlockUp.GetWorldPosOfBlock().x + 1.f, eastBlockUp.GetWorldPosOfBlock().y + 1.f, eastBlockUp.GetWorldPosOfBlock().z + 1.f), color, Vector2(texBoundsRain.maxs.x, texBoundsRain.mins.y)));
					vertexArray.push_back(Vertex3_PCT(Vector3(eastBlockUp.GetWorldPosOfBlock().x + 1.f, eastBlockUp.GetWorldPosOfBlock().y + 1.f, eastBlockUp.GetWorldPosOfBlock().z - 0.f), color, texBoundsRain.maxs));
				
					vertexArray.push_back(Vertex3_PCT(Vector3(westBlockUp.GetWorldPosOfBlock().x - 0.f, westBlockUp.GetWorldPosOfBlock().y - 0.f, westBlockUp.GetWorldPosOfBlock().z - 0.f), color, texBoundsRain.maxs));
					vertexArray.push_back(Vertex3_PCT(Vector3(westBlockUp.GetWorldPosOfBlock().x - 0.f, westBlockUp.GetWorldPosOfBlock().y + 1.f, westBlockUp.GetWorldPosOfBlock().z - 0.f), color, Vector2(texBoundsRain.mins.x, texBoundsRain.maxs.y)));
					vertexArray.push_back(Vertex3_PCT(Vector3(westBlockUp.GetWorldPosOfBlock().x - 0.f, westBlockUp.GetWorldPosOfBlock().y + 1.f, westBlockUp.GetWorldPosOfBlock().z + 1.f), color, texBoundsRain.mins));
					vertexArray.push_back(Vertex3_PCT(Vector3(westBlockUp.GetWorldPosOfBlock().x - 0.f, westBlockUp.GetWorldPosOfBlock().y - 0.f, westBlockUp.GetWorldPosOfBlock().z + 1.f), color, Vector2(texBoundsRain.maxs.x, texBoundsRain.mins.y)));
				}
			}
		}

		g_theRenderer->BindTexture2D(m_rainAnimation[0]->GetTexture());

		int numVerts = (int)vertexArray.size();
		g_theRenderer->DrawVertexArray3D_PCT(&vertexArray[0], numVerts, PRIMITIVE_QUADS);
		vertexArray.clear();

		if (!g_theAudio->IsMusicPlaying())
		{
			SoundID stoneSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Rain.wav");
			g_theAudio->PlaySound(stoneSoundID, 0.15f, SOUND_MUSIC);
		}
	}
	else
	{
		g_theAudio->StopMusic();
	}
}

void Game::RenderPlayerAlwaysVisible() const
{
	if (m_isDrawingPlayer)
	{
		m_player.RenderPlayerAlwaysVisible();
	}
}

void Game::RenderCameraView() const
{
	g_theRenderer->ClearScreen( RGBA(0.15f, 0.15f, 0.15f, 1.f) );
	g_theRenderer->SetPerspective(55.f, 16.f / 9.f, 0.1f, 1000.f);

	//put +z up, +x forward,and +y left (instead of default -z forward, +x right, and +y up)
	g_theRenderer->Rotate(-90.f, Vector3(1.f, 0.f, 0.f));
	g_theRenderer->Rotate(90.f, Vector3(0.f, 0.f, 1.f));

	//Anti-rotate to the camera's yaw, pitch, and roll
	g_theRenderer->Rotate(-m_camera.m_rollAboutX, Vector3(1.f, 0.f, 0.f));
	g_theRenderer->Rotate(-m_camera.m_pitchAboutY, Vector3(0.f, 1.f, 0.f));
	g_theRenderer->Rotate(-m_camera.m_yawAboutZ, Vector3(0.f, 0.f, 1.f));

	//anti-translate to the camera's position (e.g. more world WEST to simulate camera is EAST)
	g_theRenderer->Translate(-m_camera.m_position.x, -m_camera.m_position.y, -m_camera.m_position.z);
}

void Game::RenderWorld() const
{
	m_world->Render();
}

void Game::RenderText() const
{
	g_theRenderer->LoadIdentity();
	g_theRenderer->SetOrtho(Vector2(0.f, 0.f), Vector2(SCREEN_RATIO_WIDTH, SCREEN_RATIO_HEIGHT));

	if (!g_isHelpActive)
		return;

	BitmapFont* bitmapFont = g_theRenderer->CreateOrGetFont("SquirrelFixedFont");
	std::string cameraPosX = "x:" + std::to_string(m_player.GetCenter().x);
	std::string cameraPosY = "y:" + std::to_string(m_player.GetCenter().y);
	std::string cameraPosZ = "z:" + std::to_string(m_player.GetCenter().z);
	g_theRenderer->DrawText2D( Vector2(5.f, 885.f),cameraPosX, 1.f, RGBA::WHITE, 10.f, bitmapFont );
	g_theRenderer->DrawText2D( Vector2(5.f, 870.f), cameraPosY, 1.f, RGBA::WHITE, 10.f, bitmapFont );
	g_theRenderer->DrawText2D( Vector2(5.f, 855.f), cameraPosZ, 1.f, RGBA::WHITE, 10.f, bitmapFont );

	std::string cameraYaw = "yaw:" + std::to_string(m_camera.m_yawAboutZ);
	std::string cameraPitch = "pitch:" + std::to_string(m_camera.m_pitchAboutY);
	std::string cameraRoll = "roll:" + std::to_string(m_camera.m_rollAboutX);
	g_theRenderer->DrawText2D( Vector2(5.f, 840.f), cameraYaw, 1.f, RGBA::WHITE, 10.f, bitmapFont );
	g_theRenderer->DrawText2D( Vector2(5.f, 825.f), cameraPitch, 1.f, RGBA::WHITE, 10.f, bitmapFont );
	g_theRenderer->DrawText2D( Vector2(5.f, 810.f), cameraRoll, 1.f, RGBA::WHITE, 10.f, bitmapFont );

	BlockInfo blockInfo;
	IntVector2 currentChunkCoords = blockInfo.GetChunkCoordsFromWorldPos(m_player.GetCenter());
	std::string chunkCoordsText = "ChunkCoords: (" + std::to_string(currentChunkCoords.x) + "," + std::to_string(currentChunkCoords.y) + ")";
	g_theRenderer->DrawText2D(Vector2(5.f, 795.f), chunkCoordsText, 1.f, RGBA::WHITE, 10.f, bitmapFont);

	IntVector3 currentBlockCoords = blockInfo.GetBlockCoordsFromWorldPos(m_player.GetCenter());
	std::string blockCoordsText = "BlockCoords: (" + std::to_string(currentBlockCoords.x) + "," + std::to_string(currentBlockCoords.y) + "," + std::to_string(currentBlockCoords.z) +")";
	g_theRenderer->DrawText2D(Vector2(5.f, 780.f), blockCoordsText, 1.f, RGBA::WHITE, 10.f, bitmapFont);

// 	blockInfo.GetBlockInfoFromWorldCoords(m_player.GetCenter());
// 	if (blockInfo.m_chunk != nullptr)
// 	{
// 		int lightLevel = blockInfo.GetBlock()->GetLightLevel();
// 		std::string lightLevelText = "Light Level in Block: " + std::to_string(lightLevel) + ")";
// 		g_theRenderer->DrawText2D(Vector2(-1.f, 0.74f), lightLevelText, 1.f, RGBA::WHITE, 0.02f, bitmapFont);
// 	}

	IntVector3 rayTraceBlockCoords = m_farthestNonOpaqueBlock.m_chunk->GetBlockCoordsForBlockIndex(m_farthestNonOpaqueBlock.m_blockIndex);
	std::string raytraceText = "Raytrace Coords: (" + std::to_string(rayTraceBlockCoords.x) + "," + std::to_string(rayTraceBlockCoords.y) + "," + std::to_string(rayTraceBlockCoords.z) + ")";
	g_theRenderer->DrawText2D(Vector2(5.f, 765.f), raytraceText, 1.f, RGBA::WHITE, 10.f, bitmapFont);

	std::string isWalkingText = "Player is ";
	if (m_player.IsPhysicsWalking())
		isWalkingText += "WALKING";
	else 
		isWalkingText += "FLYING";
	g_theRenderer->DrawText2D(Vector2(5.f, 750.f), isWalkingText, 1.f, RGBA::WHITE, 10.f, bitmapFont);

	std::string cameraModeText = "CameraMode: ";
	if (m_camera.m_cameraMode == FIRST_PERSON)
		cameraModeText += "First Person";
	else if (m_camera.m_cameraMode == FROM_BEHIND)
		cameraModeText += "From Behind";
	else if (m_camera.m_cameraMode == FIXED_ANGLE)
		cameraModeText += "Fixed Angle";
	else if (m_camera.m_cameraMode == NO_CLIP)
		cameraModeText += "No Clip";
	g_theRenderer->DrawText2D(Vector2(5.f, 735.f), cameraModeText, 1.f, RGBA::WHITE, 10.f, bitmapFont);

	float timeOfDay = m_world->m_timeOfDay;
	std::string timeOfDayText = "Time: " + std::to_string( RangeMap1D(timeOfDay, Vector2(0.f, DAY_LENGTH), Vector2(0.f, 24.f)) );
	g_theRenderer->DrawText2D(Vector2(5.f, 720.f), timeOfDayText, 1.f, RGBA::WHITE, 10.f, bitmapFont);

	IntVector2 farthestEastChunkCoords = m_world->m_farthestEastChunk->GetChunkCoords();
	std::string farthestEastChunkCoordsText = "Farthest East ChunkCoords: (" + std::to_string(farthestEastChunkCoords.x) + "," + std::to_string(farthestEastChunkCoords.y) + ")";
	g_theRenderer->DrawText2D(Vector2(5.f, 705.f), farthestEastChunkCoordsText, 1.f, RGBA::WHITE, 10.f, bitmapFont);

	std::string lightLevelText = "Light Level: " + std::to_string( m_world->m_outdoorLightLevel ) ;
	g_theRenderer->DrawText2D(Vector2(5.f, 690.f), lightLevelText, 1.f, RGBA::WHITE, 10.f, bitmapFont);

	std::string currentTimeString = "Current Time: " + std::to_string( (float) GetCurrentTimeSeconds() );
	g_theRenderer->DrawText2D(Vector2(5.f, 675.f), currentTimeString, 1.f, RGBA::WHITE, 10.f, bitmapFont);

	std::string rainNoiseString = "Rain Noise: " + std::to_string(m_rainPerlinNoise);
	g_theRenderer->DrawText2D(Vector2(5.f, 660.f), rainNoiseString, 1.f, RGBA::WHITE, 10.f, bitmapFont);
}

void Game::RenderHUD() const
{
	RenderText();
	RenderCrosshairs();
	RenderBlockTypes();
}

void Game::RenderCrosshairs() const
{
	g_theRenderer->SetLineWidth(4.f);
	g_theRenderer->DrawLine3D( Vector3( (SCREEN_RATIO_WIDTH * 0.5f - 25),	(SCREEN_RATIO_HEIGHT * 0.5f), 0.f ),	Vector3( (SCREEN_RATIO_WIDTH * 0.5f) + 25, (SCREEN_RATIO_HEIGHT * 0.5f), 0.f ), RGBA::WHITE, RGBA::WHITE );
	g_theRenderer->DrawLine3D( Vector3( (SCREEN_RATIO_WIDTH * 0.5f),		(SCREEN_RATIO_HEIGHT * 0.5f) - 25, 0.f ),		Vector3( (SCREEN_RATIO_WIDTH * 0.5f), (SCREEN_RATIO_HEIGHT * 0.5f) + 25, 0.f ), RGBA::WHITE, RGBA::WHITE );
}

void Game::RenderBlockTypes() const
{
	g_theRenderer->SetLineWidth(3.f);
	if (m_currentlySelectedBlockType == BLOCK_TYPE_STONE)//stone
		g_theRenderer->DrawBoxOpen2D(AABB2D(50.f, 50.f, 125.f, 125.f), RGBA::WHITE);
	else if (m_currentlySelectedBlockType == BLOCK_TYPE_GLOWSTONE)
		g_theRenderer->DrawBoxOpen2D(AABB2D(150.f, 50.f, 225.f, 125.f), RGBA::WHITE);

	g_theRenderer->DrawAABB2DTexturedFromSpriteSheet(AABB2D(50.f, 50.f, 125.f, 125.f), m_world->m_blockDefinitions[BLOCK_TYPE_STONE]->GetTexCoordsSides(), m_world->m_tileSheet->GetSpriteSheetTexture());
	g_theRenderer->DrawAABB2DTexturedFromSpriteSheet(AABB2D(150.f, 50.f, 225.f, 125.f), m_world->m_blockDefinitions[BLOCK_TYPE_GLOWSTONE]->GetTexCoordsSides(), m_world->m_tileSheet->GetSpriteSheetTexture());
}

float Game::ChangeSimulationSpeed(float deltaSeconds) const
{
	if (m_isPaused)
		deltaSeconds = 0.f;

	return deltaSeconds;
}

void Game::KeyDown()
{
	if (g_theInput->WasKeyJustPressed('P'))
	{
		DebuggerPrintf("Game paused");
		m_isPaused == true ? m_isPaused = false : m_isPaused = true;
	}

	if (g_theInput->WasKeyJustPressed('Q'))
	{
		SoundID testSoundID = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		g_theAudio->PlaySound(testSoundID);
	}

	if (g_theInput->WasKeyJustPressed(KEY_ESCAPE))
	{
		if (g_isSavingAndLoading)
		{
			SavePlayerData();
			m_world->SaveAllChunks();
		}
		m_isQuitting = true;
	}

	if (g_theInput->WasKeyJustPressed(MOUSE_LEFT))
	{
		if (m_farthestNonOpaqueBlock.m_chunk != nullptr)
		{
			m_world->PlaceBlockAtFarthestOpaqueBlock(m_farthestNonOpaqueBlock, m_currentlySelectedBlockType);
			if (m_currentlySelectedBlockType == BLOCK_TYPE_STONE)
			{
				SoundID stoneSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Place_Stone.wav");
				g_theAudio->PlaySound(stoneSoundID);
			}
			else if (m_currentlySelectedBlockType == BLOCK_TYPE_GLOWSTONE)
			{
				SoundID glowstoneSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Place_Glowstone.wav");
				g_theAudio->PlaySound(glowstoneSoundID);
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(MOUSE_RIGHT))
	{
		if (m_closestOpaqueBlock.m_chunk != nullptr)
		{
			unsigned char closestOpaqueBlockType = m_closestOpaqueBlock.GetBlock()->GetBlockType();
			if (closestOpaqueBlockType == BLOCK_TYPE_DIRT)
			{
				SoundID dirtSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Break_Dirt.wav");
				g_theAudio->PlaySound(dirtSoundID);
			}
			else if (closestOpaqueBlockType == BLOCK_TYPE_GLOWSTONE)
			{
				SoundID glowstoneSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Break_Glowstone.wav");
				g_theAudio->PlaySound(glowstoneSoundID);
			}
			else if (closestOpaqueBlockType == BLOCK_TYPE_GRASS)
			{
				SoundID grassSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Break_Grass.wav");
				g_theAudio->PlaySound(grassSoundID);
			}
			else if (closestOpaqueBlockType == BLOCK_TYPE_STONE)
			{
				SoundID stoneSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Break_Stone.wav");
				g_theAudio->PlaySound(stoneSoundID);
			}
			m_world->RemoveBlockAtClosestNonOpaqueBlock(m_closestOpaqueBlock);
		}
	}

	if (g_theInput->WasKeyJustPressed('R'))
	{
		g_isWeatherActive = !g_isWeatherActive;
	}

	if (g_theInput->WasKeyJustPressed(KEY_F1))
	{
		g_isHelpActive = !g_isHelpActive;
	}

	if (g_theInput->WasKeyJustPressed(KEY_F5))
	{
		m_camera.CycleCameraMode();
		if (m_camera.m_cameraMode == FIRST_PERSON)
			m_isDrawingPlayer = false;
		else
			m_isDrawingPlayer = true;
	}

	if (g_theInput->WasKeyJustPressed(KEY_F6))
	{
		m_player.SetIsPhysicsWalking( !m_player.IsPhysicsWalking() );
	}

	if (g_theInput->WasKeyJustPressed('1'))
	{
		m_currentlySelectedBlockType = BLOCK_TYPE_STONE; 
		SoundID selectStoneSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Select_Stone.wav");
		g_theAudio->PlaySound(selectStoneSoundID);
	}

	if (g_theInput->WasKeyJustPressed('2'))
	{
		m_currentlySelectedBlockType = BLOCK_TYPE_GLOWSTONE;
		SoundID selectGlowstoneSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Select_Glowstone.wav");
		g_theAudio->PlaySound(selectGlowstoneSoundID);
	}
}

void Game::KeyUp()
{

}

bool Game::IsQuitting()
{
	return m_isQuitting;
}

void Game::SavePlayerData()
{
	std::vector< float > playerData;
	GetPlayerData(playerData);
	SaveBinaryFileFromBuffer("Data/Saves/PlayerData.player", playerData);
}

void Game::GetPlayerData(std::vector< float >& playerData)
{
	playerData.push_back(m_player.GetCenter().x);
	playerData.push_back(m_player.GetCenter().y);
	playerData.push_back(m_player.GetCenter().z);
	playerData.push_back(m_camera.m_rollAboutX);
	playerData.push_back(m_camera.m_pitchAboutY);
	playerData.push_back(m_camera.m_yawAboutZ);

}

bool Game::LoadPlayerData()
{
	std::vector< float > playerData;
	if (!LoadBinaryFileToBuffer("Data/Saves/PlayerData.player", playerData))
		return false;

	m_player.SetCenter( Vector3(playerData[0], playerData[1], playerData[2]) );
	m_camera.m_rollAboutX = playerData[3];
	m_camera.m_pitchAboutY = playerData[4];
	m_camera.m_yawAboutZ = playerData[5];
	return true;
}

float Game::CalcSkyboxAlpha()
{
	return Clamp( sin( (m_world->m_timeOfDay * DAY_LENGTH_DIVISOR) * fPI ), 0.f, 1.f);
}
