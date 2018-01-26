#pragma once
#include "Game/Block.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/AABB3D.hpp"
#include <vector>

class SpriteSheet;
class IntVector3;
struct Vertex3_PCT;

class Chunk
{
public:
	Block m_blocks[NUM_BLOCKS_PER_CHUNK];
	BiomeType m_biomes[CHUNK_BLOCKS_PER_LAYER];
	Chunk* m_eastNeighbor;
	Chunk* m_westNeighbor;
	Chunk* m_northNeighbor;
	Chunk* m_southNeighbor;

	Chunk();
	Chunk(IntVector2 chunkCoords, BlockDefinition* blockDefs[]);
	Chunk(IntVector2 chunkCoords, BlockDefinition* blockDefs[], const std::vector< unsigned char > chunkData); 
	~Chunk();

	void InitBlocks();
	void InitIsSkyAndDirtyBlocks();
	void GenerateVertexArray();

	void SetFrustumCulling(const Vector3& cameraForwardXYZ, const Vector3& cameraPos);
	Vector3 GetCornerWorldPosFromIndex(int cornerIndex);

	void Render() const;
	void RenderBlocks() const;

	void SetBlockDefs(BlockDefinition* blockDefs[]);
	Block* GetBlock(int blockIndex);
	Vector3 GetWorldCoords();
	IntVector2 GetChunkCoords();
	Vector3 GetChunkCenterWorldCoords();
	void GetRLEBlockData(std::vector< unsigned char >& blockData);

	int GetBlockIndexForBlockCoords(const IntVector3& blockCoords) const;
	IntVector3 GetBlockCoordsForBlockIndex(int blockIndex) const;

	void SetIsDirty(bool isDirty);
	bool IsChunkDirty();

private:
	bool m_isDirty;
	bool m_isVisible;

	IntVector2 m_chunkCoords;
	AABB3D m_worldBounds; //the position in the world //make this an AABB3D
	BlockDefinition* m_blockDefinitions[BLOCK_TYPE_SIZE];
	SpriteSheet* m_spriteSheet;
	unsigned int m_vboID;
	int m_numVertexes;
};