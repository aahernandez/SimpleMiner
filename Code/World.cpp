#include "Game/World.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/FileUtils.hpp"

World::World()
	: m_maxNumChunks(10000)
	, m_minNumChunks(500) //runs out of memory when greater than 402
	, m_minRangeOfActiveChunks(100)
	, m_distanceToIterToManipulate(0.f)
	, m_distanceToPlayer(1000.f)
	, m_timeOfDay(550.f) //out of 1000
	, m_fileVersionNumber(1)
	, m_dayMaxLightLevel(15)
	, m_nightMinLightLevel(6)
{

	m_outdoorLightLevel = (unsigned char) Clamp( (sin( (m_timeOfDay * DAY_LENGTH_DIVISOR) * fPI ) * (m_dayMaxLightLevel - m_nightMinLightLevel) ) + m_nightMinLightLevel, m_nightMinLightLevel, m_dayMaxLightLevel);
	InitBlockDefs();

	//TEMPHACK begin
// 	m_tempHackVBOID = g_theRenderer->CreateVBO();
// 	m_tempHackVBONumVertexes = 4;
// 
// 	AABB2D tempHackTexCoords = m_tileSheet->GetTexCoordsForSpriteCoords(8,8);
// 	
// 	Vertex3_PCT vertexes[4];
// 	vertexes[0].m_position = Vector3( 0.f, 0.f, 60.f );
// 	vertexes[0].m_color = RGBA::WHITE;
// 	vertexes[0].m_texCoords = Vector2(tempHackTexCoords.mins.x, tempHackTexCoords.maxs.y);
// 	vertexes[1].m_position = Vector3(0.f, 1.f, 60.f);
// 	vertexes[1].m_color = RGBA::WHITE;
// 	vertexes[1].m_texCoords = tempHackTexCoords.mins;
// 	vertexes[2].m_position = Vector3(1.f, 1.f, 60.f);
// 	vertexes[2].m_color = RGBA::WHITE;
// 	vertexes[2].m_texCoords = Vector2(tempHackTexCoords.maxs.x, tempHackTexCoords.mins.y);
// 	vertexes[3].m_position = Vector3(1.f, 0.f, 60.f);
// 	vertexes[3].m_color = RGBA::WHITE;
// 	vertexes[3].m_texCoords = tempHackTexCoords.maxs;
// 
// 	g_theRenderer->UpdateVBO(m_tempHackVBOID, &vertexes[0], m_tempHackVBONumVertexes);
	//TEMPHACK end

	InitChunks();

	if (g_loadAllChunksOnStartup)
	{
		Vector3 playerPos = Vector3(0.f, 0.f, 0.f);
		for ( int chunkCount = 0; chunkCount <= m_minNumChunks; ++chunkCount )
		{
			UpdateChunks( playerPos, Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f));
		}

		m_farthestEastChunk->m_blocks[NUM_BLOCKS_PER_CHUNK - 1].SetIsLightingDirty(true);
		m_dirtyLightingBlocks.push_back(new BlockInfo(m_farthestEastChunk, NUM_BLOCKS_PER_CHUNK - 1));

		for ( int chunkCount = 0; chunkCount <= m_minNumChunks; ++chunkCount )
		{
			UpdateLighting();
			UpdateVertexArrays();
		}

		for (int chunkCount = 0; chunkCount <= m_minNumChunks; ++chunkCount)
		{
			UpdateVertexArrays();
		}
	}
}

World::~World()
{
// 	for (int blockDefIndex = BLOCK_TYPE_SIZE; blockDefIndex > 0; --blockDefIndex)
// 	{
// 		delete m_blockDefinitions[blockDefIndex];
// 		m_blockDefinitions[blockDefIndex] = nullptr;
// 	}
// 
// 	delete m_tileSheet;
// 	m_tileSheet = nullptr;
// 
// 	for (int dirtyLightsCount = 0; dirtyLightsCount < (int) m_dirtyLightingBlocks.size(); ++dirtyLightsCount)
// 	{
// 		delete m_dirtyLightingBlocks[dirtyLightsCount];
// 		m_dirtyLightingBlocks[dirtyLightsCount] = nullptr;
// 	}

// 	ChunkIterator chunkMapIter;
// 	for (chunkMapIter = m_activeChunks.end(); chunkMapIter != m_activeChunks.begin(); --chunkMapIter)
// 	{
// 		if (chunkMapIter->second == nullptr)
// 			continue;
// 		delete chunkMapIter->second;
// 		chunkMapIter->second = nullptr;
// 	}
// 
// 	delete m_farthestEastChunk;
// 	m_farthestEastChunk = nullptr;
}

void World::InitBlockDefs()
{
	Texture* SimpleMinerTiles = g_theRenderer->CreateOrGetTexture("Data/Images/SimpleMinerAtlas.png");
	m_tileSheet = new SpriteSheet(SimpleMinerTiles, 16, 16);

	AABB2D airTile = m_tileSheet->GetTexCoordsForSpriteCoords(0, 0);
	AABB2D grassTileSides = m_tileSheet->GetTexCoordsForSpriteCoords(8, 8);
	AABB2D grassTileTop = m_tileSheet->GetTexCoordsForSpriteCoords(9, 8);
	AABB2D dirtTile = m_tileSheet->GetTexCoordsForSpriteCoords(7, 8);
	AABB2D stoneTile = m_tileSheet->GetTexCoordsForSpriteCoords(2, 10);
	AABB2D waterTile = m_tileSheet->GetTexCoordsForSpriteCoords(15, 11);
	AABB2D cobblestoneTile = m_tileSheet->GetTexCoordsForSpriteCoords(3, 10);
	AABB2D sand = m_tileSheet->GetTexCoordsForSpriteCoords(1, 8);
	AABB2D glowstone = m_tileSheet->GetTexCoordsForSpriteCoords(4, 11);
	AABB2D snowSides = m_tileSheet->GetTexCoordsForSpriteCoords(8, 7);
	AABB2D snowTop = m_tileSheet->GetTexCoordsForSpriteCoords(0, 8);	

	m_blockDefinitions[0] = new BlockDefinition(airTile, BLOCK_TYPE_AIR, false, false, 0);
	m_blockDefinitions[1] = new BlockDefinition(grassTileSides, grassTileTop, dirtTile, BLOCK_TYPE_GRASS, true, true, 0);
	m_blockDefinitions[2] = new BlockDefinition(dirtTile, BLOCK_TYPE_DIRT, true, true, 0);
	m_blockDefinitions[3] = new BlockDefinition(stoneTile, BLOCK_TYPE_STONE, true, true, 0);
	m_blockDefinitions[4] = new BlockDefinition(waterTile, BLOCK_TYPE_WATER, true, true, 0);
	m_blockDefinitions[5] = new BlockDefinition(cobblestoneTile, BLOCK_TYPE_COBBLESTONE, true, true, 0);
	m_blockDefinitions[6] = new BlockDefinition(sand, BLOCK_TYPE_SAND, true, true, 0);
	m_blockDefinitions[7] = new BlockDefinition(glowstone, BLOCK_TYPE_GLOWSTONE, true, true, 12);
	m_blockDefinitions[8] = new BlockDefinition(snowSides, snowTop, dirtTile, BLOCK_TYPE_DIRTSNOW, true, true, 0);
	m_blockDefinitions[9] = new BlockDefinition(snowTop, BLOCK_TYPE_SNOW, true, true, 0);
}

void World::InitChunks()
{
	IntVector2 worldPos(0, 0);
	if (!LoadChunkFromFile(worldPos))
		m_activeChunks[worldPos] = new Chunk(worldPos, m_blockDefinitions);
}

void World::Update(float deltaSeconds, Vector3& playerPos, const Vector3& cameraForwardXYZ, const Vector3& cameraPos)
{
 	UpdateChunks(playerPos, cameraForwardXYZ, cameraPos);
	UpdateTimeOfDay(deltaSeconds);
	UpdateLighting();
	UpdateVertexArrays();
}

void World::UpdateChunks(Vector3& playerPos, const Vector3& cameraForwardXYZ, const Vector3& cameraPos)
{

	bool isDeactivatingChunk = false;
	bool isActivatingChunk = false;
	bool isGeneratingChunkVertex = false;
	IntVector2 chunkCoords;
	IntVector2 neightChunkCoords;
	Vector3 neighborWorldCoords;
	unsigned int direction = 0; //1 - east, 2 - north, 3 - west, 4 - south

	m_distanceToIterToManipulate = 0;
	m_distanceToPlayer = (float) m_minRangeOfActiveChunks;

	ChunkIterator chunkMapIter;
	for (chunkMapIter = m_activeChunks.begin(); chunkMapIter != m_activeChunks.end(); ++chunkMapIter)
	{
		Chunk* chunk = chunkMapIter->second;

		if (chunk == nullptr)
			continue;

		chunk->SetFrustumCulling(cameraForwardXYZ, cameraPos);

	}

	//Amortize
	for (chunkMapIter = m_activeChunks.begin(); chunkMapIter != m_activeChunks.end(); ++chunkMapIter)
	{
		Chunk* chunk = chunkMapIter->second;
		if (chunk == nullptr)
			continue;

		if ( (int)m_activeChunks.size() > m_maxNumChunks ) //if more than max chunks
		{
			isDeactivatingChunk = true;
			float distance = CalcPlayerDistanceToChunk(playerPos, chunk->GetChunkCenterWorldCoords());
			if (distance > m_distanceToIterToManipulate)
			{
				m_distanceToIterToManipulate = distance;
				m_iterToManipulate = chunkMapIter;
			}
		}

		if ( (int)m_activeChunks.size() < m_minNumChunks ) //if less than min chunks
		{
			float distance = 0;
			if (chunk->m_eastNeighbor == nullptr) //NEIGHBORS
			{
				chunkCoords = IntVector2( chunk->GetChunkCoords() );
				neighborWorldCoords = CalcEastNeighborCenterWorldCoords(chunkCoords);
				distance = CalcPlayerDistanceToChunk(playerPos, neighborWorldCoords);
				if (distance < m_distanceToPlayer)
				{
					m_distanceToPlayer = distance;
					m_iterToManipulate = chunkMapIter;
					direction = 1;
					neightChunkCoords = chunkCoords;
					neightChunkCoords.x++;
					isActivatingChunk = true;
				}
			}
			if (chunk->m_northNeighbor == nullptr)
			{
				chunkCoords = IntVector2(chunk->GetChunkCoords());
				neighborWorldCoords = CalcNorthNeighborCenterWorldCoords(chunkCoords);
				distance = CalcPlayerDistanceToChunk(playerPos, neighborWorldCoords);
				if (distance < m_distanceToPlayer)
				{
					m_distanceToPlayer = distance;
					m_iterToManipulate = chunkMapIter;
					direction = 2;
					neightChunkCoords = chunkCoords;
					neightChunkCoords.y++;
					isActivatingChunk = true;
				}
			}
			if (chunk->m_westNeighbor == nullptr)
			{
				chunkCoords = IntVector2(chunk->GetChunkCoords());
				neighborWorldCoords = CalcWestNeighborCenterWorldCoords(chunkCoords);
				distance = CalcPlayerDistanceToChunk(playerPos, neighborWorldCoords);
				if (distance < m_distanceToPlayer)
				{
					m_distanceToPlayer = distance;
					m_iterToManipulate = chunkMapIter;
					direction = 3;
					neightChunkCoords = chunkCoords;
					neightChunkCoords.x--;
					isActivatingChunk = true;
				}
			}
			if (chunk->m_southNeighbor == nullptr)
			{
				chunkCoords = IntVector2(chunk->GetChunkCoords());
				neighborWorldCoords = CalcSouthNeighborCenterWorldCoords(chunkCoords);
				distance = CalcPlayerDistanceToChunk(playerPos, neighborWorldCoords);
				if (distance < m_distanceToPlayer)
				{
					m_distanceToPlayer = distance;
					m_iterToManipulate = chunkMapIter;
					direction = 4;
					neightChunkCoords = chunkCoords;
					neightChunkCoords.y--;
					isActivatingChunk = true;
				}
			}
		}

		//Ensure all blocks are within minimum distance from the player
		if ( ((int)m_activeChunks.size() < m_maxNumChunks) && (int)m_activeChunks.size() >= m_minNumChunks )
		{
			float distance = CalcPlayerDistanceToChunk(playerPos, chunk->GetChunkCenterWorldCoords());
			if ( distance > m_minRangeOfActiveChunks && distance > m_distanceToIterToManipulate)
			{
				isDeactivatingChunk = true;
				m_distanceToIterToManipulate = distance;
				m_iterToManipulate = chunkMapIter;
			}

			//activate chunks in range
		}
	}

	if (isDeactivatingChunk)
	{
		if (g_isSavingAndLoading)
			SaveChunkToFile(m_iterToManipulate);
		DeactivateChunk(m_iterToManipulate);
		SetFarthestEastBlock(playerPos);
		return;
	}

	if (isActivatingChunk)
	{
		if (!g_isSavingAndLoading) 
			ActivateChunk(neightChunkCoords);
		else if( !LoadChunkFromFile(neightChunkCoords) )
			ActivateChunk(neightChunkCoords);
		SetNeighbors(neightChunkCoords);
		SetFarthestEastBlock(playerPos);
		return;
	}

	m_distanceToIterToManipulate = 2000;
	for (chunkMapIter = m_activeChunks.begin(); chunkMapIter != m_activeChunks.end(); ++chunkMapIter)
	{
		Chunk* chunk = chunkMapIter->second;

		if (chunk == nullptr)
			continue;

		//Ensure that all chunks' vertex arrays have been generated
		float distance = CalcPlayerDistanceToChunk(playerPos, chunk->GetChunkCenterWorldCoords());
		if ( chunk->IsChunkDirty() && distance < m_distanceToIterToManipulate )
		{
			m_distanceToIterToManipulate = distance;
			m_iterToManipulate = chunkMapIter;
			isGeneratingChunkVertex = true;
		}

	}

	if (isGeneratingChunkVertex)
	{
		m_iterToManipulate->second->GenerateVertexArray();
		return;
	}
}

void World::UpdateTimeOfDay(float deltaSeconds)
{
	m_timeOfDay += deltaSeconds;
	if (m_timeOfDay > DAY_LENGTH)
		m_timeOfDay = 0.f;
	CalcOutdoorLightLevel();
}

void World::UpdateLighting()
{
	if (m_dirtyLightingBlocks.empty())
		return;

	//set west block lighting
	m_farthestEastChunk->m_blocks[NUM_BLOCKS_PER_CHUNK - 1].GetIsLightingDirty();
	int numLightingLoops = NUM_BLOCKS_PER_CHUNK * 2;
	if ( (int) m_dirtyLightingBlocks.size() < numLightingLoops )
		numLightingLoops = (int) m_dirtyLightingBlocks.size();

	for (int dirtyLightsCount = 0; dirtyLightsCount < numLightingLoops; --numLightingLoops)
	{
		if (m_dirtyLightingBlocks.empty())
			return;

		BlockInfo* blockInfo = m_dirtyLightingBlocks[dirtyLightsCount];
		if (blockInfo->m_chunk == nullptr || blockInfo == nullptr)
			return;
		Block* block = blockInfo->GetBlock();
		int originalLightLevel = block->GetLightLevel();

		if (block->GetIsOpaque())
		{
			block->SetLightLevel( m_blockDefinitions[block->GetBlockType()]->GetSelfIllumination() );
		}
		else
		{
			if (block->GetIsSky())
			{
// 				char skyLightLevelForChunk = GetSkyLightLevelForChunkCoords(blockInfo->m_chunk->GetChunkCoords());
				//find sky light level based on position from west most block
				block->SetLightLevel( m_outdoorLightLevel );
			}

			if (m_blockDefinitions[block->GetBlockType()]->GetSelfIllumination() > block->GetLightLevel())
			{
				block->SetLightLevel(m_blockDefinitions[block->GetBlockType()]->GetSelfIllumination());
			}

			BlockInfo neighbor;
			neighbor = blockInfo->GetAboveNeighbor();
			if (neighbor.m_chunk != nullptr)
			{
				int neighborLightLevel = neighbor.GetBlock()->GetLightLevel();
				if (neighborLightLevel - 1 > block->GetLightLevel())
				{
					block->SetLightLevel(neighborLightLevel - 1);
				}
			}

			neighbor = blockInfo->GetBelowNeighbor();
			if (neighbor.m_chunk != nullptr)
			{
				int neighborLightLevel = neighbor.GetBlock()->GetLightLevel();
				if (neighborLightLevel - 1 > block->GetLightLevel())
				{
					block->SetLightLevel(neighborLightLevel - 1);
				}
			}

			neighbor = blockInfo->GetNorthNeighbor();
			if (neighbor.m_chunk != nullptr)
			{
				int neighborLightLevel = neighbor.GetBlock()->GetLightLevel();
				if (neighborLightLevel - 1 > block->GetLightLevel())
				{
					block->SetLightLevel(neighborLightLevel - 1);
				}
			}

			neighbor = blockInfo->GetSouthNeighbor();
			if (neighbor.m_chunk != nullptr)
			{
				int neighborLightLevel = neighbor.GetBlock()->GetLightLevel();
				if (neighborLightLevel - 1 > block->GetLightLevel())
				{
					block->SetLightLevel(neighborLightLevel - 1);
				}
			}

			neighbor = blockInfo->GetEastNeighbor();
			if (neighbor.m_chunk != nullptr)
			{
				int neighborLightLevel = neighbor.GetBlock()->GetLightLevel();
				if (neighborLightLevel - 1 > block->GetLightLevel())
				{
					block->SetLightLevel(neighborLightLevel - 1);
				}
			}

			neighbor = blockInfo->GetWestNeighbor();
			if (neighbor.m_chunk != nullptr)
			{
				int neighborLightLevel = neighbor.GetBlock()->GetLightLevel();
				if (neighborLightLevel - 1 > block->GetLightLevel())
				{
					block->SetLightLevel(neighborLightLevel - 1);
				}
			}
		}

		if (originalLightLevel != block->GetLightLevel())
		{
			SetBlockNeighborsDirty(blockInfo);
			blockInfo->m_chunk->SetIsDirty(true);
		}
		block->SetIsLightingDirty(false);

		delete m_dirtyLightingBlocks[dirtyLightsCount];
		m_dirtyLightingBlocks[dirtyLightsCount] = nullptr;
		m_dirtyLightingBlocks.pop_front();
	}
}

void World::UpdateVertexArrays()
{
	std::map<IntVector2, Chunk*>::iterator iter;
	for (iter = m_activeChunks.begin(); iter != m_activeChunks.end(); ++iter)
	{
		Chunk* chunk = iter->second;

		if (chunk != nullptr && chunk->IsChunkDirty())
		{
			chunk->GenerateVertexArray();
			return;
		}
	}
}

void World::Render() const
{
	//TEMPHACK
	g_theRenderer->BindTexture2D(m_tileSheet->GetSpriteSheetTexture());
// 	g_theRenderer->DrawVBO3D_PCT(m_tempHackVBOID, m_tempHackVBONumVertexes, PRIMITIVE_QUADS);

	RenderAxes(3.f, 1.f);
	g_theRenderer->BindTexture2D(m_tileSheet->GetSpriteSheetTexture());

	std::map<IntVector2, Chunk*>::const_iterator iter;
	for (iter = m_activeChunks.begin(); iter != m_activeChunks.end(); ++iter)
	{
		Chunk* chunk = iter->second;
		if (chunk != nullptr)
			chunk->Render();
	}

	RenderAxes(1.f, 0.3f);
}

void World::RenderAxes(float lineThickness, float alphaAmount) const
{
	RGBA color = RGBA::RED;
	color.ScaleAlpha(alphaAmount);
	g_theRenderer->SetLineWidth(lineThickness);
	g_theRenderer->DrawLine3D(Vector3(0.f, 0.f, 0.f), Vector3(2.f, 0.f, 0.f), color, color);

	color = RGBA::GREEN;
	color.ScaleAlpha(alphaAmount);
	g_theRenderer->SetLineWidth(lineThickness);
	g_theRenderer->DrawLine3D(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 2.f, 0.f), color, color);

	color = RGBA::BLUE;
	color.ScaleAlpha(alphaAmount);
	g_theRenderer->SetLineWidth(lineThickness);
	g_theRenderer->DrawLine3D(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 2.f), color, color);
}

void World::SaveChunkToFile(const ChunkIterator& iter)
{
	Chunk* chunk = iter->second;

	std::vector< unsigned char > chunkData;
	chunk->GetRLEBlockData(chunkData);

	IntVector2 chunkCoords = IntVector2(chunk->GetChunkCoords());
	chunkData.insert(chunkData.begin(), (unsigned char) CHUNK_BLOCKS_TALL_Z);
	chunkData.insert(chunkData.begin(), (unsigned char) CHUNK_BLOCKS_DEEP_Y);
	chunkData.insert(chunkData.begin(), (unsigned char) CHUNK_BLOCKS_WIDE_X);
	chunkData.insert( chunkData.begin(),  m_fileVersionNumber);

	std::string fileName = Stringf("Data/Saves/Chunk_at_(%i,%i).chunk", chunkCoords.x, chunkCoords.y);
	SaveBinaryFileFromBuffer(fileName, chunkData);
}

bool World::LoadChunkFromFile(IntVector2 chunkCoords)
{
	std::vector< unsigned char > chunkData;
	std::string fileName = Stringf("Data/Saves/Chunk_at_(%i,%i).chunk", chunkCoords.x, chunkCoords.y);
	if (!LoadBinaryFileToBuffer(fileName, chunkData))
		return false;
	if (chunkData[0] != m_fileVersionNumber)
		return false;

	m_activeChunks[chunkCoords] = new Chunk(chunkCoords, m_blockDefinitions, chunkData);
	return true;
}

void World::SaveAllChunks()
{
	std::map<IntVector2, Chunk*>::iterator iter;
	for (iter = m_activeChunks.begin(); iter != m_activeChunks.end(); ++iter)
	{
		SaveChunkToFile(iter);
	}
}

void World::DeactivateChunk(const ChunkIterator& iter)
{
	Chunk* chunk = iter->second;
	
	if (chunk->m_eastNeighbor)
	{
		chunk->m_eastNeighbor->m_westNeighbor = nullptr;
		chunk->m_eastNeighbor = nullptr;
	}
	if (chunk->m_westNeighbor)
	{
		chunk->m_westNeighbor->m_eastNeighbor = nullptr;
		chunk->m_westNeighbor = nullptr;
	}
	if (chunk->m_northNeighbor)
	{
		chunk->m_northNeighbor->m_southNeighbor = nullptr;
		chunk->m_northNeighbor = nullptr;
	}
	if (chunk->m_southNeighbor)
	{
		chunk->m_southNeighbor->m_northNeighbor = nullptr;
		chunk->m_southNeighbor = nullptr;
	}

	delete iter->second;
	m_activeChunks.erase(iter);
}

void World::ActivateChunk(const IntVector2& chunkCoords)
{
	m_activeChunks[chunkCoords] = new Chunk(chunkCoords, m_blockDefinitions);
}

void World::SetNeighbors(const IntVector2& chunkCoords)
{
// 	Chunk* chunk = m_iterToManipulate->second;
	Chunk* chunk = m_activeChunks[chunkCoords];

	IntVector2 neighborChunkCoords = chunkCoords;
	neighborChunkCoords.x++;
	if (m_activeChunks.find(neighborChunkCoords) != m_activeChunks.end()) 
	{
		Chunk* neighbor = m_activeChunks[neighborChunkCoords];
		if (neighbor != nullptr)
		{
			chunk->m_eastNeighbor = neighbor;
			neighbor->m_westNeighbor = chunk;
		}
	}

	neighborChunkCoords = chunkCoords;
	neighborChunkCoords.x--;
	if (m_activeChunks.find(neighborChunkCoords) != m_activeChunks.end())
	{
		Chunk* neighbor = m_activeChunks[neighborChunkCoords];
		if (neighbor != nullptr)
		{
			chunk->m_westNeighbor = neighbor;
			neighbor->m_eastNeighbor = chunk;
		}
	}

	neighborChunkCoords = chunkCoords;
	neighborChunkCoords.y++;
	if (m_activeChunks.find(neighborChunkCoords) != m_activeChunks.end())
	{
		Chunk* neighbor = m_activeChunks[neighborChunkCoords];
		if (neighbor != nullptr)
		{
			chunk->m_northNeighbor = neighbor;
			neighbor->m_southNeighbor = chunk;
		}	
	}

	neighborChunkCoords = chunkCoords;
	neighborChunkCoords.y--;
	if (m_activeChunks.find(neighborChunkCoords) != m_activeChunks.end())
	{
		Chunk* neighbor = m_activeChunks[neighborChunkCoords];
		if (neighbor != nullptr)
		{
			chunk->m_southNeighbor = neighbor;
			neighbor->m_northNeighbor = chunk;
		}
	}
}

float World::CalcPlayerDistanceToChunk(Vector3& playerPosition, const Vector3& chunkPos)
{
	return playerPosition.CalcDistanceToVector(chunkPos);
}

void World::PlaceBlockAtFarthestOpaqueBlock(BlockInfo& farthestOpaqueBlockFromPlayer, unsigned char blockType)
{
	farthestOpaqueBlockFromPlayer.m_chunk->m_blocks[farthestOpaqueBlockFromPlayer.m_blockIndex] = Block(blockType, m_blockDefinitions[blockType]->IsOpaque(), m_blockDefinitions[blockType]->IsSolid() );
	farthestOpaqueBlockFromPlayer.m_chunk->m_blocks[farthestOpaqueBlockFromPlayer.m_blockIndex].SetIsLightingDirty(true);
	farthestOpaqueBlockFromPlayer.m_chunk->m_blocks[farthestOpaqueBlockFromPlayer.m_blockIndex].SetLightLevel(0);
	farthestOpaqueBlockFromPlayer.m_chunk->m_blocks[farthestOpaqueBlockFromPlayer.m_blockIndex].SetIsSky(false);
	m_dirtyLightingBlocks.push_back( new BlockInfo(farthestOpaqueBlockFromPlayer));
	SetColumnIsNotSky(farthestOpaqueBlockFromPlayer);
}

void World::RemoveBlockAtClosestNonOpaqueBlock(BlockInfo& closestOpaqueBlockToPlayer)
{
// 	Block* block = closestOpaqueBlockToPlayer.GetBlock();
	// closestOpaqueBlockToPlayer.m_chunk->m_blocks[closestOpaqueBlockToPlayer.m_blockIndex] = Block();
	closestOpaqueBlockToPlayer.m_chunk->m_blocks[closestOpaqueBlockToPlayer.m_blockIndex].SetBlockType(BLOCK_TYPE_AIR);
	closestOpaqueBlockToPlayer.m_chunk->m_blocks[closestOpaqueBlockToPlayer.m_blockIndex].SetIsOpaque(false);
	closestOpaqueBlockToPlayer.m_chunk->m_blocks[closestOpaqueBlockToPlayer.m_blockIndex].SetIsSolid(false);//#FIXME: don't make a new block, change the block
	closestOpaqueBlockToPlayer.m_chunk->m_blocks[closestOpaqueBlockToPlayer.m_blockIndex].SetIsLightingDirty(true); //make this a combined function with the line below
	m_dirtyLightingBlocks.push_back( new BlockInfo(closestOpaqueBlockToPlayer) );
	SetColumnIsSky(closestOpaqueBlockToPlayer);
}

void World::SetColumnIsNotSky(const BlockInfo& topBlockInfoOfColumn)
{
	IntVector3 blockCoords = topBlockInfoOfColumn.m_chunk->GetBlockCoordsForBlockIndex(topBlockInfoOfColumn.m_blockIndex);
	for (int columnIndexZ = blockCoords.z - 1; columnIndexZ >= 0; --columnIndexZ)
	{
		int blockIndex = topBlockInfoOfColumn.m_chunk->GetBlockIndexForBlockCoords(IntVector3(blockCoords.x, blockCoords.y, columnIndexZ));

		if (!topBlockInfoOfColumn.m_chunk->m_blocks[blockIndex].GetIsOpaque())
		{
			topBlockInfoOfColumn.m_chunk->m_blocks[blockIndex].SetIsSky(false);
			topBlockInfoOfColumn.m_chunk->m_blocks[blockIndex].SetLightLevel(0);
			topBlockInfoOfColumn.m_chunk->m_blocks[blockIndex].SetIsLightingDirty(true);
			m_dirtyLightingBlocks.push_back( new BlockInfo(topBlockInfoOfColumn.m_chunk, blockIndex) );
		}
		else
		{
			break;	
		}
	}
}

void World::SetColumnIsSky(const BlockInfo& blockInfoInColumn)
{
	IntVector3 blockCoords = blockInfoInColumn.m_chunk->GetBlockCoordsForBlockIndex(blockInfoInColumn.m_blockIndex);

	for (int columnIndexZ = CHUNK_BLOCKS_TALL_Z - 1; columnIndexZ >= 0; --columnIndexZ)
	{
		int blockIndex = blockInfoInColumn.m_chunk->GetBlockIndexForBlockCoords( IntVector3(blockCoords.x, blockCoords.y, columnIndexZ) );

		if (!blockInfoInColumn.m_chunk->m_blocks[blockIndex].GetIsOpaque())
		{
			blockInfoInColumn.m_chunk->m_blocks[blockIndex].SetIsSky(true);
			blockInfoInColumn.m_chunk->m_blocks[blockIndex].SetIsLightingDirty(true);
			blockInfoInColumn.m_chunk->m_blocks[blockIndex].SetLightLevel(0);
			m_dirtyLightingBlocks.push_back( new BlockInfo(blockInfoInColumn.m_chunk, blockIndex) );
		}
		else
		{
			break;
		}
	}
}

void World::SetFarthestEastBlock(const Vector3& playerPos)
{
	Chunk* chunkAtPlayerPos = m_activeChunks[ IntVector2( (int) playerPos.x, (int) playerPos.y ) ];
	
	
	while (chunkAtPlayerPos != nullptr)
	{
		m_farthestEastChunk = chunkAtPlayerPos;
		chunkAtPlayerPos = chunkAtPlayerPos->m_eastNeighbor;
	}
}

char World::GetSkyLightLevelForChunkCoords(const IntVector2& chunkCoords)
{
	chunkCoords;
	//set outdoor light level based on time of day

// 	float farthestEastPosition = (float) m_farthestEastChunk->GetChunkCoords().x;
// 	float chunkEastPosition = (float) chunkCoords.x;
// 
// 	float lightLevel = m_outdoorLightLevel;
// 
// 	if (chunkEastPosition < 0)
// 	{
// 		lightLevel = RangeMap1D( chunkEastPosition - farthestEastPosition, Vector2(chunkEastPosition, farthestEastPosition), Vector2(m_nightMinLightLevel, m_outdoorLightLevel) );
// 	}
// 	else
// 	{
// 		lightLevel = RangeMap1D( farthestEastPosition - chunkEastPosition, Vector2(chunkEastPosition, farthestEastPosition), Vector2(m_nightMinLightLevel, m_outdoorLightLevel) );
// 	}
// 
// 	return (char) Clamp( lightLevel, m_nightMinLightLevel, m_outdoorLightLevel);
	return m_outdoorLightLevel;
}

void World::CalcOutdoorLightLevel()
{
	unsigned char originalLightLevel = m_outdoorLightLevel;
	m_outdoorLightLevel = (unsigned char) Clamp( (sin( (m_timeOfDay * DAY_LENGTH_DIVISOR) * fPI ) * (m_dayMaxLightLevel - m_nightMinLightLevel) ) + m_nightMinLightLevel, m_nightMinLightLevel, m_dayMaxLightLevel);
	if (m_outdoorLightLevel != originalLightLevel)
	{
		m_farthestEastChunk->m_blocks[NUM_BLOCKS_PER_CHUNK - 1].SetIsLightingDirty(true);
		m_farthestEastChunk->m_blocks[NUM_BLOCKS_PER_CHUNK - 1].SetLightLevel(0);
		m_dirtyLightingBlocks.push_back( new BlockInfo(m_farthestEastChunk, NUM_BLOCKS_PER_CHUNK - 1));
	}
}

Vector3 World::CalcEastNeighborCenterWorldCoords(const IntVector2& chunkCoords)
{
	return Vector3( ( (chunkCoords.x + 1) * CHUNK_BLOCKS_WIDE_X) + (CHUNK_BLOCKS_WIDE_X * 0.5f), (chunkCoords.y * CHUNK_BLOCKS_DEEP_Y) + (CHUNK_BLOCKS_DEEP_Y * 0.5f), CHUNK_BLOCKS_TALL_Z * 0.5f );
}

Vector3 World::CalcWestNeighborCenterWorldCoords(const IntVector2& chunkCoords)
{
	return Vector3( ( (chunkCoords.x - 1) * CHUNK_BLOCKS_WIDE_X) + (CHUNK_BLOCKS_WIDE_X * 0.5f), (chunkCoords.y * CHUNK_BLOCKS_DEEP_Y) + (CHUNK_BLOCKS_DEEP_Y * 0.5f), CHUNK_BLOCKS_TALL_Z * 0.5f );
}

Vector3 World::CalcNorthNeighborCenterWorldCoords(const IntVector2& chunkCoords)
{
	return Vector3( ( (chunkCoords.x) * CHUNK_BLOCKS_WIDE_X) + (CHUNK_BLOCKS_WIDE_X * 0.5f), ( (chunkCoords.y + 1) * CHUNK_BLOCKS_DEEP_Y) + (CHUNK_BLOCKS_DEEP_Y * 0.5f), CHUNK_BLOCKS_TALL_Z * 0.5f );
}

Vector3 World::CalcSouthNeighborCenterWorldCoords(const IntVector2& chunkCoords)
{
	return Vector3( ( (chunkCoords.x) * CHUNK_BLOCKS_WIDE_X) + (CHUNK_BLOCKS_WIDE_X * 0.5f), ( (chunkCoords.y - 1) * CHUNK_BLOCKS_DEEP_Y) + (CHUNK_BLOCKS_DEEP_Y * 0.5f), CHUNK_BLOCKS_TALL_Z * 0.5f );
}

void World::SetBlockNeighborsDirty(BlockInfo* blockInfo)
{
	BlockInfo* neighborAbove = new BlockInfo(blockInfo->GetAboveNeighbor());
	if (neighborAbove->m_chunk != nullptr)
	{
		neighborAbove->GetBlock()->SetIsLightingDirty(true);
		m_dirtyLightingBlocks.push_back(neighborAbove);
	}
// 	delete neighborAbove;
// 	neighborAbove = nullptr;

	BlockInfo* neighborBelow = new BlockInfo(blockInfo->GetBelowNeighbor());
	if (neighborBelow->m_chunk != nullptr)
	{
		neighborBelow->GetBlock()->SetIsLightingDirty(true);
		m_dirtyLightingBlocks.push_back(neighborBelow);
	}
// 	delete neighborBelow;
// 	neighborBelow = nullptr;

	BlockInfo* neighborEast = new BlockInfo(blockInfo->GetEastNeighbor());
	if (neighborEast->m_chunk != nullptr)
	{
		neighborEast->GetBlock()->SetIsLightingDirty(true);
		m_dirtyLightingBlocks.push_back(neighborEast);
	}
// 	delete neighborEast;
// 	neighborEast = nullptr;

	BlockInfo* neighborWest = new BlockInfo(blockInfo->GetWestNeighbor());
	if (neighborWest->m_chunk != nullptr)
	{
		neighborWest->GetBlock()->SetIsLightingDirty(true);
		m_dirtyLightingBlocks.push_back(neighborWest);
	}
// 	delete neighborWest;
// 	neighborWest = nullptr;

	BlockInfo* neighborNorth = new BlockInfo(blockInfo->GetNorthNeighbor());
	if (neighborNorth->m_chunk != nullptr)
	{
		neighborNorth->GetBlock()->SetIsLightingDirty(true);
		m_dirtyLightingBlocks.push_back(neighborNorth);
	}
// 	delete neighborNorth;
// 	neighborNorth = nullptr;

	BlockInfo* neighborSouth = new BlockInfo(blockInfo->GetSouthNeighbor());
	if (neighborSouth->m_chunk != nullptr)
	{
		neighborSouth->GetBlock()->SetIsLightingDirty(true);
		m_dirtyLightingBlocks.push_back(neighborSouth);
	}
// 	delete neighborSouth;
// 	neighborSouth = nullptr;
}
