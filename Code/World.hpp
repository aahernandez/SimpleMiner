#pragma once
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Chunk.hpp"
#include "BlockInfo.hpp"
#include <map>
#include <deque>

typedef std::map<IntVector2, Chunk*>::iterator ChunkIterator;

const float DAY_LENGTH = 1000.f;
const float DAY_LENGTH_DIVISOR = 1.f / DAY_LENGTH;

class World
{
public:
	std::map< IntVector2, Chunk* > m_activeChunks;
	std::deque<BlockInfo*> m_dirtyLightingBlocks;
	BlockDefinition* m_blockDefinitions[BLOCK_TYPE_SIZE];
	ChunkIterator m_iterToManipulate;
	SpriteSheet* m_tileSheet;
	Chunk* m_farthestEastChunk;
	float m_distanceToIterToManipulate;
	float m_distanceToPlayer;
	float m_timeOfDay;
	int m_maxNumChunks;
	int m_minNumChunks;
	int m_minRangeOfActiveChunks;
	char m_fileVersionNumber;
	char m_outdoorLightLevel;
	char m_dayMaxLightLevel;
	char m_nightMinLightLevel;

	World();
	~World();

	void InitBlockDefs();
	void InitChunks();

	void Update(float deltaSeconds, Vector3& playerPos, const Vector3& cameraForwardXYZ, const Vector3& cameraPos);
	void UpdateChunks(Vector3& playerPos, const Vector3& cameraForwardXYZ, const Vector3& cameraPos);
	void UpdateTimeOfDay(float deltaSeconds);
	void UpdateLighting();
	void UpdateVertexArrays();

	void Render() const;
	void RenderAxes(float lineThickness, float alphaAmount) const;

	void SaveChunkToFile(const ChunkIterator& iter);
	bool LoadChunkFromFile(IntVector2 chunkCoords);
	void SaveAllChunks();
	void DeactivateChunk(const ChunkIterator& iter);
	void ActivateChunk(const IntVector2& chunkCoords);
	void SetNeighbors(const IntVector2& chunkCoords);
	float CalcPlayerDistanceToChunk(Vector3& playerPosition, const Vector3& chunkPos);
	void PlaceBlockAtFarthestOpaqueBlock(BlockInfo& farthestOpaqueBlockFromPlayer, unsigned char blockType);
	void RemoveBlockAtClosestNonOpaqueBlock(BlockInfo& closestOpaqueBlockToPlayer);
	void SetColumnIsNotSky(const BlockInfo& topBlockInfoOfColumn);
	void SetColumnIsSky(const BlockInfo& blockInfoInColumn);
	void SetFarthestEastBlock(const Vector3& playerPos);
	char GetSkyLightLevelForChunkCoords(const IntVector2& chunkCoords);
	void CalcOutdoorLightLevel();
	Vector3 CalcEastNeighborCenterWorldCoords(const IntVector2& chunkCoords);
	Vector3 CalcWestNeighborCenterWorldCoords(const IntVector2& chunkCoords);
	Vector3 CalcNorthNeighborCenterWorldCoords(const IntVector2& chunkCoords);
	Vector3 CalcSouthNeighborCenterWorldCoords(const IntVector2& chunkCoords);

	void SetBlockNeighborsDirty(BlockInfo* blockInfo);
};