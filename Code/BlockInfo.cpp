#include "Game/BlockInfo.hpp"
#include "Game/Block.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Game/Chunk.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtilities.hpp"

BlockInfo::BlockInfo()
{
	m_chunk = nullptr;
}

BlockInfo::BlockInfo(Chunk* chunk, int blockIndex)
{
	m_chunk = chunk;
	m_blockIndex = blockIndex;
}

BlockInfo::BlockInfo(const BlockInfo& copyInfo)
{
	m_chunk = copyInfo.m_chunk;
	m_blockIndex = copyInfo.m_blockIndex;
}

BlockInfo::BlockInfo(BlockInfo* copyInfo)
{
	m_chunk = copyInfo->m_chunk;
	m_blockIndex = copyInfo->m_blockIndex;
}

BlockInfo::~BlockInfo()
{
	m_chunk = nullptr;
}

BlockInfo BlockInfo::GetEastNeighbor() const
{
	if (m_chunk == nullptr)
		return BlockInfo(nullptr, 0);

	if ( (m_blockIndex & MASK_X) == MASK_X)
	{
		return BlockInfo(m_chunk->m_eastNeighbor, m_blockIndex & ~MASK_X);
	}
	else
	{
		return BlockInfo(m_chunk, 1 + m_blockIndex);
	}
}

BlockInfo BlockInfo::GetWestNeighbor() const
{
	if (m_chunk == nullptr)
		return BlockInfo(nullptr, 0);

	if ((m_blockIndex & MASK_X) == 0)
	{
		return BlockInfo(m_chunk->m_westNeighbor, m_blockIndex | MASK_X);
	}
	else
	{
		return BlockInfo(m_chunk, m_blockIndex - 1);
	}
}

BlockInfo BlockInfo::GetNorthNeighbor() const
{
	if (m_chunk == nullptr)
		return BlockInfo(nullptr, 0);

	if ((m_blockIndex & MASK_Y) == MASK_Y)
	{
		return BlockInfo(m_chunk->m_northNeighbor, m_blockIndex & ~MASK_Y);
	}
	else
	{
		return BlockInfo(m_chunk, CHUNK_BLOCKS_WIDE_X + m_blockIndex);
	}
}

BlockInfo BlockInfo::GetSouthNeighbor() const
{
	if (m_chunk == nullptr)
		return BlockInfo(nullptr, 0);

	if ((m_blockIndex & MASK_Y) == 0)
	{
		return BlockInfo(m_chunk->m_southNeighbor, m_blockIndex | MASK_Y);
	}
	else
	{
		return BlockInfo(m_chunk, m_blockIndex - CHUNK_BLOCKS_WIDE_X);
	}
}

BlockInfo BlockInfo::GetAboveNeighbor() const
{
	if (m_chunk == nullptr)
		return BlockInfo(nullptr, 0);

	if ((m_blockIndex & MASK_Z) == MASK_Z)
	{
		return BlockInfo(nullptr, 0);
	}
	else
	{
		return BlockInfo(m_chunk, CHUNK_BLOCKS_PER_LAYER + m_blockIndex);
	}
}

BlockInfo BlockInfo::GetBelowNeighbor() const
{
	if (m_chunk == nullptr)
		return BlockInfo(nullptr, 0);

	if ((m_blockIndex & MASK_Z) == 0)
	{
		return BlockInfo(nullptr, 0);
	}
	else
	{
		return BlockInfo(m_chunk, m_blockIndex - CHUNK_BLOCKS_PER_LAYER);
	}
}

BlockInfo BlockInfo::GetNorthEastNeighbor() const
{
	return GetNorthNeighbor().GetEastNeighbor();
}

BlockInfo BlockInfo::GetNorthWestNeighbor() const
{
	return GetNorthNeighbor().GetWestNeighbor();
}

BlockInfo BlockInfo::GetSouthEastNeighbor() const
{
	return GetSouthNeighbor().GetEastNeighbor();
}

BlockInfo BlockInfo::GetSouthWestNeighbor() const
{
	return GetSouthNeighbor().GetWestNeighbor();
}

BlockInfo BlockInfo::GetEastDownNeighbor() const
{
	return GetBelowNeighbor().GetEastNeighbor();
}

BlockInfo BlockInfo::GetWestDownNeighbor() const
{
	return GetBelowNeighbor().GetWestNeighbor();
}

BlockInfo BlockInfo::GetNorthDownNeighbor() const
{
	return GetBelowNeighbor().GetNorthNeighbor();
}

BlockInfo BlockInfo::GetSouthDownNeighbor() const
{
	return GetBelowNeighbor().GetSouthNeighbor();
}

void BlockInfo::SetBlockInfoFromWorldCoords(const Vector3& worldCoords)
{
	if (worldCoords.z >= CHUNK_BLOCKS_TALL_Z)
	{
		m_chunk = nullptr;
		return;
	}

	IntVector2 chunkCoords = GetChunkCoordsFromWorldPos(worldCoords);
	m_chunk = g_theGame->m_world->m_activeChunks[chunkCoords];
	if (m_chunk == nullptr)
		return;

	IntVector3 blockCoords = GetBlockCoordsFromWorldPos(worldCoords);
	m_blockIndex = m_chunk->GetBlockIndexForBlockCoords(blockCoords);
}

IntVector2 BlockInfo::GetChunkCoordsFromWorldPos(const Vector3& worldCoords)
{
	int x = 0;
	int y = 0;

	if (worldCoords.x < 0)
		x = ((int) (worldCoords.x - 1)>> CHUNK_BITS_X);
	else 
		x = ((int) worldCoords.x >> CHUNK_BITS_X);

	if (worldCoords.y < 0)
		y = ((int) (worldCoords.y - 1) >> CHUNK_BITS_Y);
	else 
		y = ((int) worldCoords.y >> CHUNK_BITS_Y);
	return IntVector2(x, y);
}

IntVector3 BlockInfo::GetBlockCoordsFromWorldPos(const Vector3& worldCoords)
{
	int x = 0;
	int y = 0;

	if (worldCoords.x < 0)
		x = (int) worldCoords.x - 1 & MASK_X;
	else 
		x = (int) worldCoords.x & MASK_X;

	if (worldCoords.y < 0)
		y = (int) worldCoords.y - 1 & (MASK_Y >> CHUNK_BITS_X);
	else
		y = (int) worldCoords.y & (MASK_Y >> CHUNK_BITS_X);

	int z = (int) worldCoords.z & (MASK_Z >> CHUNK_BITS_XY);
	return IntVector3(x, y, z);
}

Vector3 BlockInfo::GetWorldPosOfBlock()
{
	Vector3 worldPos;
	IntVector2 chunkCoords = m_chunk->GetChunkCoords();
	IntVector3 blockCoords = m_chunk->GetBlockCoordsForBlockIndex(m_blockIndex);

	worldPos.x = (float) blockCoords.x + ( (float) chunkCoords.x * CHUNK_BLOCKS_WIDE_X );
	worldPos.y = (float) blockCoords.y + ( (float) chunkCoords.y * CHUNK_BLOCKS_DEEP_Y );
	worldPos.z = (float) blockCoords.z;

	return worldPos;
}

Block* BlockInfo::GetBlock()
{
	return m_chunk->GetBlock(m_blockIndex);
}

bool BlockInfo::IsBlockSolid()
{
	return m_chunk->m_blocks[m_blockIndex].GetIsSolid();
}

