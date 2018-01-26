#pragma once
#include "Engine/Math/AABB2D.hpp"

enum BlockType
{
	BLOCK_TYPE_AIR,
	BLOCK_TYPE_GRASS,
	BLOCK_TYPE_DIRT,
	BLOCK_TYPE_STONE,
	BLOCK_TYPE_WATER,
	BLOCK_TYPE_COBBLESTONE,
	BLOCK_TYPE_SAND,
	BLOCK_TYPE_GLOWSTONE,
	BLOCK_TYPE_DIRTSNOW,
	BLOCK_TYPE_SNOW,
	BLOCK_TYPE_SIZE
};

enum BiomeType
{
	BIOME_ARCTIC,
	BIOME_GRASSLAND,
	BIOME_BEACH,
	BIOME_DESERT,
	BIOME_SIZE
};

class BlockDefinition
{
public:
	BlockDefinition();
	BlockDefinition(const AABB2D& texCoords, BlockType blockType, bool isOpaque, bool isSolid, char selfIllumination);
	BlockDefinition(const Vector2& texCoodsMin, const Vector2& texCoordsMax, BlockType blockType, bool isOpaque, bool isSolid, char selfIllumination);
	BlockDefinition(const AABB2D& texCoordsSides, const AABB2D& texCoordsTop, const AABB2D& texCoordsBottom, BlockType blockType, bool isOpaque, bool isSolid, char selfIllumination);
	~BlockDefinition();

	BlockType GetBlockType() const;
	AABB2D GetTexCoordsXForward() const;
	AABB2D GetTexCoordsXBack() const;
	AABB2D GetTexCoordsYLeft() const;
	AABB2D GetTexCoordsYRight() const;
	AABB2D GetTexCoordsZUp() const;
	AABB2D GetTexCoordsZDown() const;
	AABB2D GetTexCoordsSides() const;

	bool IsOpaque() const;
	bool IsSolid() const;
	char GetSelfIllumination() const;

private:
	AABB2D m_texCoordsXForward;
	AABB2D m_texCoordsXBack;
	AABB2D m_texCoordsYLeft;
	AABB2D m_texCoordsYRight;
	AABB2D m_texCoordsZUp;
	AABB2D m_texCoordsZDown;
	BlockType m_blockType;
	bool m_isOpaque;
	bool m_isSolid;
	char m_selfIllumination; //0 - 15
};