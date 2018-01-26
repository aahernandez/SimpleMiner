#include "Game/Chunk.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Core/Noise.hpp"
#include "Game/BlockInfo.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"

const int NUM_SIDES_OF_CUBE = 6;
const int NUM_CORNERS_PER_SIDE = 4;
const int NUM_VERTEXES = NUM_BLOCKS_PER_CHUNK * NUM_SIDES_OF_CUBE * NUM_CORNERS_PER_SIDE;

const float PERLIN_MULTIPLIER = CHUNK_BLOCKS_TALL_Z * 0.3f;

const float PERLIN_SCALE = 150.f;
const unsigned int PERLIN_NUM_OCTAVES = 100;
const float PERLIN_OCTAVE_PERSISTANCE = 0.3f;
const float PERLIN_OCTAVE_SCALE = 2.f;
const unsigned int PERLIN_SEED = 24;
const float LIGHT_LEVEL_DIVISOR = 1.f / 15.f;

Chunk::Chunk()
	: m_isDirty(false)
{
	m_eastNeighbor = nullptr;
	m_northNeighbor = nullptr;
	m_westNeighbor = nullptr;
	m_southNeighbor = nullptr;

	m_vboID = g_theRenderer->CreateVBO();
	m_numVertexes = 0;

	for (int blockTypeIndex = 0; blockTypeIndex < BLOCK_TYPE_SIZE; ++blockTypeIndex)
	{
		m_blockDefinitions[blockTypeIndex] = nullptr;
	}
	m_spriteSheet = nullptr;
	InitBlocks();
	InitIsSkyAndDirtyBlocks();
	GenerateVertexArray();
}

Chunk::Chunk(IntVector2 chunkCoords, BlockDefinition* blockDefs[])
{
	m_eastNeighbor = nullptr;
	m_northNeighbor = nullptr;
	m_westNeighbor = nullptr;
	m_southNeighbor = nullptr;

	m_vboID = g_theRenderer->CreateVBO();
	m_numVertexes = 0;

	m_chunkCoords = chunkCoords;
	m_worldBounds = AABB3D( Vector3( (float) chunkCoords.x * CHUNK_BLOCKS_WIDE_X, (float) chunkCoords.y * CHUNK_BLOCKS_DEEP_Y, 0.f), 
							Vector3( (float) chunkCoords.x * CHUNK_BLOCKS_WIDE_X + CHUNK_BLOCKS_WIDE_X, (float) chunkCoords.y * CHUNK_BLOCKS_DEEP_Y + CHUNK_BLOCKS_DEEP_Y, (float) CHUNK_BLOCKS_TALL_Z) );
	SetBlockDefs(blockDefs);
	InitBlocks();
	InitIsSkyAndDirtyBlocks();
	GenerateVertexArray();
}

Chunk::Chunk(IntVector2 chunkCoords, BlockDefinition* blockDefs[], const std::vector< unsigned char > chunkData)
{
	m_eastNeighbor = nullptr;
	m_northNeighbor = nullptr;
	m_westNeighbor = nullptr;
	m_southNeighbor = nullptr;

	m_vboID = g_theRenderer->CreateVBO();
	m_numVertexes = 0;

	m_chunkCoords = chunkCoords;
	m_worldBounds = AABB3D(	Vector3((float)chunkCoords.x * CHUNK_BLOCKS_WIDE_X, (float)chunkCoords.y * CHUNK_BLOCKS_DEEP_Y, 0.f),
							Vector3((float)chunkCoords.x * CHUNK_BLOCKS_WIDE_X + CHUNK_BLOCKS_WIDE_X, (float)chunkCoords.y * CHUNK_BLOCKS_DEEP_Y + CHUNK_BLOCKS_DEEP_Y, (float)CHUNK_BLOCKS_TALL_Z));
	SetBlockDefs(blockDefs);

	std::vector< unsigned char >::const_iterator iter;
	iter = chunkData.begin();

	++iter;
	if ( *iter != CHUNK_BLOCKS_WIDE_X)
		return;
	++iter;
	if (*iter != CHUNK_BLOCKS_DEEP_Y)
		return;
	++iter;
	if (*iter != CHUNK_BLOCKS_TALL_Z)
		return;

	++iter;
	int blockIndex = 0;
	for (iter; iter != chunkData.end(); ++iter)
	{
		unsigned char blockType = *iter;
		++iter;
		unsigned char loopCount = *iter;

		for (int loopBlockIndex = 0; loopBlockIndex < loopCount; ++loopBlockIndex)
		{
			m_blocks[blockIndex] = Block(blockType, m_blockDefinitions[blockType]->IsOpaque(), m_blockDefinitions[blockType]->IsSolid() );
			++blockIndex;
		}
	}

	InitIsSkyAndDirtyBlocks();
	GenerateVertexArray();
}

Chunk::~Chunk()
{
	g_theRenderer->DestroyVBO(m_vboID);
}

void Chunk::InitBlocks()
{
	int seaLevel = (int) (CHUNK_BLOCKS_TALL_Z * 0.5f);
	int columnHeight = (int) (CHUNK_BLOCKS_TALL_Z * 0.4f);
	int baseColumnHeight = (int) (CHUNK_BLOCKS_TALL_Z * 0.15f);

	for (int blockIndexY = 0; blockIndexY < CHUNK_BLOCKS_DEEP_Y; ++blockIndexY)
	{
		for (int blockIndexX = 0; blockIndexX < CHUNK_BLOCKS_WIDE_X; ++blockIndexX)
		{
			float blockWorldPosX = (float) blockIndexX + m_worldBounds.mins.x;
			float blockWorldPosY = (float) blockIndexY + m_worldBounds.mins.y;
			float perlinNoise = Compute2dPerlinNoise( blockWorldPosX, blockWorldPosY, PERLIN_SCALE, PERLIN_NUM_OCTAVES, PERLIN_OCTAVE_PERSISTANCE, PERLIN_OCTAVE_SCALE, true, PERLIN_SEED);
			perlinNoise = perlinNoise * PERLIN_MULTIPLIER;
			float temperatureNoise = Compute2dPerlinNoise( blockWorldPosX, blockWorldPosY, 500.f, 1, 0.2f, 1.f, true, 8367);
			temperatureNoise += 1.f;
			temperatureNoise *= 2.f;
			//Snow: 0-1, Grass: 1-2, Beach: 2-3, Desert: 3-4;

			for (int blockIndexZ = 0; blockIndexZ < CHUNK_BLOCKS_TALL_Z; ++blockIndexZ)
			{
				int tempBaseColumnHeight = baseColumnHeight;
				if (temperatureNoise <= 2.f && temperatureNoise > 3.f)
					tempBaseColumnHeight *= 4;
				int blockPosInColumn = (int) Clamp( (blockIndexZ + perlinNoise + tempBaseColumnHeight), 0.0f, (float) CHUNK_BLOCKS_TALL_Z - 1 );
				int blockIndex = GetBlockIndexForBlockCoords( IntVector3(blockIndexX, blockIndexY, blockPosInColumn ));

				if (blockIndexZ == columnHeight && blockPosInColumn <= seaLevel)
				{
					if (temperatureNoise <= 1.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_SNOW, m_blockDefinitions[BLOCK_TYPE_SNOW]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_SNOW]->IsSolid() );
					else if (temperatureNoise <= 4.f && temperatureNoise > 2.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_SAND, m_blockDefinitions[BLOCK_TYPE_SAND]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_SAND]->IsSolid() );
				}
				else if (blockIndexZ == columnHeight && blockPosInColumn > seaLevel)
				{
					if (temperatureNoise <= 1.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_DIRTSNOW, m_blockDefinitions[BLOCK_TYPE_DIRTSNOW]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_DIRTSNOW]->IsSolid());
					else if (temperatureNoise <= 3.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_GRASS, m_blockDefinitions[BLOCK_TYPE_GRASS]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_GRASS]->IsSolid() );
					else if (temperatureNoise <= 4.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_SAND, m_blockDefinitions[BLOCK_TYPE_SAND]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_SAND]->IsSolid());
				}
				else if (blockIndexZ >= columnHeight - 5 && blockIndexZ <= columnHeight - 1 && blockPosInColumn >= seaLevel)
				{
					if (temperatureNoise <= 1.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_SNOW, m_blockDefinitions[BLOCK_TYPE_SNOW]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_SNOW]->IsSolid() );
					else if (temperatureNoise <= 3.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_DIRT, m_blockDefinitions[BLOCK_TYPE_DIRT]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_DIRT]->IsSolid() );
					else if (temperatureNoise <= 4.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_SAND, m_blockDefinitions[BLOCK_TYPE_SAND]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_SAND]->IsSolid());
				}
				else if (blockIndexZ >= columnHeight - 5 && blockIndexZ <= columnHeight - 1 && blockPosInColumn < seaLevel)
				{
					if (temperatureNoise < 2.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_DIRT, m_blockDefinitions[BLOCK_TYPE_DIRT]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_DIRT]->IsSolid() );
					else if (temperatureNoise < 4.f)
						m_blocks[blockIndex] = Block(BLOCK_TYPE_SAND, m_blockDefinitions[BLOCK_TYPE_SAND]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_SAND]->IsSolid() );
				}
				else if (blockIndexZ <= columnHeight - 6)
				{
					m_blocks[blockIndex] = Block(BLOCK_TYPE_STONE, m_blockDefinitions[BLOCK_TYPE_STONE]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_STONE]->IsSolid() );
				}
				if (blockIndexZ < seaLevel)
				{
					blockIndex = GetBlockIndexForBlockCoords(IntVector3(blockIndexX, blockIndexY, blockIndexZ));

					if (m_blocks[blockIndex].m_blockType == BLOCK_TYPE_AIR)
					{
						if (temperatureNoise <= 1.f)
							m_blocks[blockIndex] = Block(BLOCK_TYPE_SNOW, m_blockDefinitions[BLOCK_TYPE_SNOW]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_SNOW]->IsSolid() );
						else if (temperatureNoise <= 2.f)
							m_blocks[blockIndex] = Block(BLOCK_TYPE_DIRT, m_blockDefinitions[BLOCK_TYPE_DIRT]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_DIRT]->IsSolid() );
						else if (temperatureNoise <= 3.f)
							m_blocks[blockIndex] = Block(BLOCK_TYPE_WATER, m_blockDefinitions[BLOCK_TYPE_WATER]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_WATER]->IsSolid() );
						else if (temperatureNoise <= 4.f)
							m_blocks[blockIndex] = Block(BLOCK_TYPE_SAND, m_blockDefinitions[BLOCK_TYPE_SAND]->IsOpaque(), m_blockDefinitions[BLOCK_TYPE_SAND]->IsSolid() );
					}
				}
			}
		}
	}
}

void Chunk::InitIsSkyAndDirtyBlocks()
{
	for (int blockIndexY = 0; blockIndexY < CHUNK_BLOCKS_DEEP_Y; ++blockIndexY)
	{
		for (int blockIndexX = 0; blockIndexX < CHUNK_BLOCKS_WIDE_X; ++blockIndexX)
		{
			bool isSettingOpaqueToSky = true;
			for (int blockIndexZ = CHUNK_BLOCKS_TALL_Z - 1; blockIndexZ >= 0; --blockIndexZ)
			{
				int blockIndex = GetBlockIndexForBlockCoords(IntVector3(blockIndexX, blockIndexY, blockIndexZ));

				if (m_blocks[blockIndex].GetBlockType() == (unsigned char) BLOCK_TYPE_AIR)
				{
					if (isSettingOpaqueToSky)
						m_blocks[blockIndex].SetIsSky(true);
					if (isSettingOpaqueToSky || blockIndexX == 0 || blockIndexX == CHUNK_BLOCKS_WIDE_X - 1 || blockIndexY == 0 || blockIndexY == CHUNK_BLOCKS_DEEP_Y - 1 || blockIndexZ == 0 || blockIndexZ == CHUNK_BLOCKS_TALL_Z - 1)
					{
						m_blocks[blockIndex].SetIsLightingDirty(true);
						if (g_theGame != nullptr)
							g_theGame->m_world->m_dirtyLightingBlocks.push_back( new BlockInfo(this, blockIndex) );
					}
				}
				else
				{
					isSettingOpaqueToSky = false;
				}
			}
		}
	}
}

void Chunk::Render() const
{
	RenderBlocks();
}

void Chunk::RenderBlocks() const
{
	if (!m_isVisible)
		return;

	g_theRenderer->PushMatrix();
	g_theRenderer->Translate(m_worldBounds.mins.x, m_worldBounds.mins.y);
	g_theRenderer->DrawVBO3D_PCT(m_vboID, m_numVertexes, PRIMITIVE_QUADS);
// 	if ((int) m_vertexArray.size() > 0)
// 		g_theRenderer->DrawVertexArray3D_PCT(&m_vertexArray[0], (int) m_vertexArray.size(), PRIMITIVE_QUADS); 
	g_theRenderer->PopMatrix();
}

void Chunk::SetBlockDefs(BlockDefinition* blockDefs[])
{
	for (int blockTypeIndex = 0; blockTypeIndex < BLOCK_TYPE_SIZE; ++blockTypeIndex)
	{
		m_blockDefinitions[blockTypeIndex] = blockDefs[blockTypeIndex];
	}
}

Block* Chunk::GetBlock(int blockIndex)
{
	return &m_blocks[blockIndex];
}

Vector3 Chunk::GetWorldCoords()
{
	return m_worldBounds.mins;
}

IntVector2 Chunk::GetChunkCoords()
{
	return m_chunkCoords;
}

Vector3 Chunk::GetChunkCenterWorldCoords()
{
	return m_worldBounds.CalcCenter() + m_worldBounds.mins;
}

void Chunk::GenerateVertexArray()
{
	std::vector< Vertex3_PCT >	vertexArray;
	vertexArray.resize(CHUNK_BLOCKS_WIDE_X * CHUNK_BLOCKS_DEEP_Y * 40 );

	for (int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		IntVector3 blockCoords = GetBlockCoordsForBlockIndex(blockIndex);
		int blockIndexX = blockCoords.x;
		int blockIndexY = blockCoords.y;
		int blockIndexZ = blockCoords.z;

		BlockInfo blockInfo(this, blockIndex);
		BlockInfo neighbor;
		RGBA faceColor;

		if (m_blocks[blockIndex].m_blockType != BLOCK_TYPE_AIR)
		{
			AABB2D texBoundsSides = m_blockDefinitions[m_blocks[blockIndex].m_blockType]->GetTexCoordsXForward();
			AABB2D texBoundsZUp = m_blockDefinitions[m_blocks[blockIndex].m_blockType]->GetTexCoordsZUp();
			AABB2D texBoundsZDown = m_blockDefinitions[m_blocks[blockIndex].m_blockType]->GetTexCoordsZDown();

			//down
			neighbor = blockInfo.GetBelowNeighbor();
			if (blockIndexZ == 0 || (neighbor.m_chunk != nullptr && neighbor.GetBlock()->GetIsOpaque() == false) )
			{
				if (blockIndexZ != 0)
				{
					float grayScale = (float)neighbor.GetBlock()->GetLightLevel() * LIGHT_LEVEL_DIVISOR;
					faceColor = RGBA(grayScale, grayScale, grayScale, 1.f);
				}
				else
					faceColor = RGBA::WHITE;
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY,		blockIndexZ),		faceColor, Vector2(texBoundsZDown.maxs.x, texBoundsZDown.mins.y)));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY + 1,	blockIndexZ),		faceColor, texBoundsZDown.maxs));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1, blockIndexY + 1,	blockIndexZ),		faceColor, Vector2(texBoundsZDown.mins.x, texBoundsZDown.maxs.y)));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1, blockIndexY,		blockIndexZ),		faceColor, texBoundsZDown.mins));	
			}

			//up
			neighbor = blockInfo.GetAboveNeighbor();
			if (blockIndexZ == CHUNK_BLOCKS_TALL_Z - 1 || (neighbor.m_chunk != nullptr && neighbor.GetBlock()->GetIsOpaque() == false))
			{
				if (blockIndexZ != CHUNK_BLOCKS_TALL_Z - 1)
				{
					float grayScale = (float) neighbor.GetBlock()->GetLightLevel() * LIGHT_LEVEL_DIVISOR;
					faceColor = RGBA(grayScale, grayScale, grayScale, 1.f);
				}
				else
					faceColor = RGBA::WHITE;
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY,		blockIndexZ + 1),	faceColor, texBoundsZUp.maxs));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1,	blockIndexY,		blockIndexZ + 1),	faceColor, Vector2(texBoundsZUp.maxs.x, texBoundsZUp.mins.y)));		 
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1,	blockIndexY + 1,	blockIndexZ + 1),	faceColor, texBoundsZUp.mins));		 
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY + 1,	blockIndexZ + 1),	faceColor, Vector2(texBoundsZUp.mins.x, texBoundsZUp.maxs.y)));
			}
															  			
			//north
			neighbor = blockInfo.GetNorthNeighbor();
			if (neighbor.m_chunk != nullptr && neighbor.GetBlock()->GetIsOpaque() == false) //blockIndexY == CHUNK_BLOCKS_DEEP_Y - 1 || 
			{
// 				if (blockIndexY != CHUNK_BLOCKS_DEEP_Y - 1)
// 				{
					float grayScale = (float)neighbor.GetBlock()->GetLightLevel() * LIGHT_LEVEL_DIVISOR;
					faceColor = RGBA(grayScale, grayScale, grayScale, 1.f);
// 				}
// 				else
// 					faceColor = RGBA::WHITE;
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY + 1,	blockIndexZ),		faceColor, Vector2(texBoundsSides.mins.x, texBoundsSides.maxs.y)));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY + 1,	blockIndexZ + 1),	faceColor, texBoundsSides.mins));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1, blockIndexY + 1,	blockIndexZ + 1),	faceColor, Vector2(texBoundsSides.maxs.x, texBoundsSides.mins.y)));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1, blockIndexY + 1,	blockIndexZ),		faceColor, texBoundsSides.maxs));
			}
							
			//south
			neighbor = blockInfo.GetSouthNeighbor();
			if ( neighbor.m_chunk != nullptr && neighbor.GetBlock()->GetIsOpaque() == false) //blockIndexY == 0 ||
			{
// 				if (blockIndexY != 0)
// 				{
					float grayScale = (float)neighbor.GetBlock()->GetLightLevel() * LIGHT_LEVEL_DIVISOR;
					faceColor = RGBA(grayScale, grayScale, grayScale, 1.f);
// 				}
// 				else
// 					faceColor = RGBA::WHITE;
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY,		blockIndexZ),		faceColor, Vector2(texBoundsSides.mins.x, texBoundsSides.maxs.y)));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1,	blockIndexY,		blockIndexZ),		faceColor, texBoundsSides.maxs));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1,	blockIndexY,		blockIndexZ + 1),	faceColor, Vector2(texBoundsSides.maxs.x, texBoundsSides.mins.y)));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY,		blockIndexZ + 1),	faceColor, texBoundsSides.mins));
			}
							
			//east
			neighbor = blockInfo.GetEastNeighbor();
			if ( neighbor.m_chunk != nullptr && neighbor.GetBlock()->GetIsOpaque() == false) //blockIndexX == CHUNK_BLOCKS_WIDE_X - 1 ||
			{
// 				if (blockIndexX != CHUNK_BLOCKS_WIDE_X - 1)
// 				{
					float grayScale = (float)neighbor.GetBlock()->GetLightLevel() * LIGHT_LEVEL_DIVISOR;
					faceColor = RGBA(grayScale, grayScale, grayScale, 1.f);
// 				}
// 				else
// 					faceColor = RGBA::WHITE;
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1,	blockIndexY,		blockIndexZ),		faceColor, texBoundsSides.maxs));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1,	blockIndexY + 1,	blockIndexZ),		faceColor, Vector2(texBoundsSides.mins.x, texBoundsSides.maxs.y)));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1,	blockIndexY + 1,	blockIndexZ + 1),	faceColor, texBoundsSides.mins));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX + 1,	blockIndexY,		blockIndexZ + 1),	faceColor, Vector2(texBoundsSides.maxs.x, texBoundsSides.mins.y)));
			}
							
			//west
			neighbor = blockInfo.GetWestNeighbor();
			if ( neighbor.m_chunk != nullptr && neighbor.GetBlock()->GetIsOpaque() == false) //blockIndexX == 0 ||
			{
// 				if (blockIndexX != 0)
// 				{
					float grayScale = (float)neighbor.GetBlock()->GetLightLevel() * LIGHT_LEVEL_DIVISOR;
					faceColor = RGBA(grayScale, grayScale, grayScale, 1.f);
// 				}
// 				else
// 					faceColor = RGBA::WHITE;
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY,		blockIndexZ),		faceColor, texBoundsSides.maxs));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY,		blockIndexZ + 1),	faceColor, Vector2(texBoundsSides.maxs.x, texBoundsSides.mins.y)));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY + 1,	blockIndexZ + 1),	faceColor, texBoundsSides.mins));
				vertexArray.push_back(Vertex3_PCT(IntVector3(blockIndexX,		blockIndexY + 1,	blockIndexZ),		faceColor, Vector2(texBoundsSides.mins.x, texBoundsSides.maxs.y)));
			}
		}
	}

	m_isDirty = false;

	m_numVertexes = (int) vertexArray.size();
	g_theRenderer->UpdateVBO(m_vboID, &vertexArray[0], m_numVertexes);
	vertexArray.clear();
}

void Chunk::SetFrustumCulling(const Vector3& cameraForwardXYZ, const Vector3& cameraPos)
{
	m_isVisible = false;
	for (int cornerIndex = 0; cornerIndex < 8; cornerIndex++)
	{
		const Vector3& cornerPosition = GetCornerWorldPosFromIndex(cornerIndex);
		Vector3 displacementToCorner = cornerPosition - cameraPos;
		float dotProduct = DotProduct(cameraForwardXYZ, displacementToCorner);
		if (dotProduct > 0) {
			m_isVisible = true;
			return;
		}
	}
}

Vector3 Chunk::GetCornerWorldPosFromIndex(int cornerIndex)
{
	if (cornerIndex == 0)
		return m_worldBounds.mins;
	else if (cornerIndex == 1)
		return Vector3(m_worldBounds.maxs.x, m_worldBounds.mins.y, m_worldBounds.mins.z);
	else if (cornerIndex == 2)
		return Vector3(m_worldBounds.mins.x, m_worldBounds.maxs.y, m_worldBounds.mins.z);
	else if (cornerIndex == 3)
		return Vector3(m_worldBounds.maxs.x, m_worldBounds.maxs.y, m_worldBounds.mins.z);
	else if (cornerIndex == 4)
		return Vector3(m_worldBounds.mins.x, m_worldBounds.mins.y, m_worldBounds.maxs.z);
	else if (cornerIndex == 5)
		return Vector3(m_worldBounds.maxs.x, m_worldBounds.mins.y, m_worldBounds.maxs.z);
	else if (cornerIndex == 6)
		return Vector3(m_worldBounds.mins.x, m_worldBounds.maxs.y, m_worldBounds.maxs.z);
	else
		return m_worldBounds.maxs;
}

bool Chunk::IsChunkDirty()
{
	return m_isDirty;
}

int Chunk::GetBlockIndexForBlockCoords(const IntVector3& blockCoords) const
{
	return blockCoords.x | (blockCoords.y << CHUNK_BITS_Y) | (blockCoords.z << CHUNK_BITS_XY);
}

IntVector3 Chunk::GetBlockCoordsForBlockIndex(int blockIndex) const
{
	IntVector3 blockCoords;
	blockCoords.x = blockIndex & MASK_X;
	blockCoords.y = (blockIndex & MASK_Y) >> CHUNK_BITS_X;
	blockCoords.z = (blockIndex & MASK_Z) >> CHUNK_BITS_XY;
	return blockCoords;
}

void Chunk::SetIsDirty(bool isDirty)
{
	m_isDirty = isDirty;
}

void Chunk::GetRLEBlockData(std::vector< unsigned char >& blockData)
{
	unsigned char currentBlockType = 0;
	unsigned char numBlocksForBlockType = 0;
	for (int blockIndex = 0; blockIndex <= NUM_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		if (m_blocks[blockIndex].m_blockType == currentBlockType)
		{
			numBlocksForBlockType++;
		}
		else if (m_blocks[blockIndex].m_blockType != currentBlockType)
		{
			if ( !blockData.empty() )
			{
				blockData.push_back( (unsigned char)currentBlockType );
				blockData.push_back(numBlocksForBlockType);
			}
			currentBlockType = m_blocks[blockIndex].m_blockType;
			numBlocksForBlockType = 1;
		}

		if (numBlocksForBlockType == 255)
		{
			blockData.push_back( (unsigned char) currentBlockType );
			blockData.push_back(numBlocksForBlockType);
			numBlocksForBlockType = 0;
		}
	}
}

