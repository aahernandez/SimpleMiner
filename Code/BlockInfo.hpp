#pragma once
class Chunk;
class Vector3;
class IntVector2;
class IntVector3;
class Block;

class BlockInfo
{
public: 
	Chunk* m_chunk;
	int m_blockIndex;

	BlockInfo();
	BlockInfo(BlockInfo* copyInfo);
	BlockInfo(const BlockInfo& copyInfo);
	BlockInfo(Chunk* chunk, int blockIndex);
	~BlockInfo();

	BlockInfo GetEastNeighbor() const;
	BlockInfo GetWestNeighbor() const;
	BlockInfo GetNorthNeighbor() const;
	BlockInfo GetSouthNeighbor() const;
	BlockInfo GetAboveNeighbor() const;
	BlockInfo GetBelowNeighbor() const;

	BlockInfo GetNorthEastNeighbor() const;
	BlockInfo GetNorthWestNeighbor() const;
	BlockInfo GetSouthEastNeighbor() const;
	BlockInfo GetSouthWestNeighbor() const;

	BlockInfo GetEastDownNeighbor() const;
	BlockInfo GetWestDownNeighbor() const;
	BlockInfo GetNorthDownNeighbor() const;
	BlockInfo GetSouthDownNeighbor() const;

	void SetBlockInfoFromWorldCoords(const Vector3& worldCoords);
	IntVector2 GetChunkCoordsFromWorldPos(const Vector3& worldCoords);
	IntVector3 GetBlockCoordsFromWorldPos(const Vector3& worldCoords);
	Vector3 GetWorldPosOfBlock();
	Block* GetBlock();

	bool IsBlockSolid();
};