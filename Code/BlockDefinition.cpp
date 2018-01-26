#include "Game/BlockDefinition.hpp"

BlockDefinition::BlockDefinition()
	: m_blockType(BLOCK_TYPE_AIR)
	, m_texCoordsXForward(0.f, 0.f)
	, m_texCoordsXBack(0.f, 0.f)
	, m_texCoordsYLeft(0.f, 0.f)
	, m_texCoordsYRight(0.f, 0.f)
	, m_texCoordsZUp(0.f, 0.f)
	, m_texCoordsZDown(0.f, 0.f)
	, m_isOpaque(false)
	, m_isSolid(false)
	, m_selfIllumination(0)
{
}

BlockDefinition::BlockDefinition(const AABB2D& texCoords, BlockType blockType, bool isOpaque, bool isSolid, char selfIllumination)
	: m_blockType(blockType)
	, m_texCoordsXForward(texCoords)
	, m_texCoordsXBack(texCoords)
	, m_texCoordsYLeft(texCoords)
	, m_texCoordsYRight(texCoords)
	, m_texCoordsZUp(texCoords)
	, m_texCoordsZDown(texCoords)
	, m_isOpaque(isOpaque)
	, m_isSolid(isSolid)
	, m_selfIllumination(selfIllumination)
{
}

BlockDefinition::BlockDefinition(const Vector2& texCoodsMin, const Vector2& texCoordsMax, BlockType blockType, bool isOpaque, bool isSolid, char selfIllumination)
	: m_blockType(blockType)
	, m_texCoordsXForward(texCoodsMin, texCoordsMax)
	, m_texCoordsXBack(texCoodsMin, texCoordsMax)
	, m_texCoordsYLeft(texCoodsMin, texCoordsMax)
	, m_texCoordsYRight(texCoodsMin, texCoordsMax)
	, m_texCoordsZUp(texCoodsMin, texCoordsMax)
	, m_texCoordsZDown(texCoodsMin, texCoordsMax)
	, m_isOpaque(isOpaque)
	, m_isSolid(isSolid)
	, m_selfIllumination(selfIllumination)
{
}

BlockDefinition::BlockDefinition(const AABB2D& texCoordsSides, const AABB2D& texCoordsTop, const AABB2D& texCoordsBottom, BlockType blockType, bool isOpaque, bool isSolid, char selfIllumination)
	: m_blockType(blockType)
	, m_texCoordsXForward(texCoordsSides)
	, m_texCoordsXBack(texCoordsSides)
	, m_texCoordsYLeft(texCoordsSides)
	, m_texCoordsYRight(texCoordsSides)
	, m_texCoordsZUp(texCoordsTop)
	, m_texCoordsZDown(texCoordsBottom)
	, m_isOpaque(isOpaque)
	, m_isSolid(isSolid)
	, m_selfIllumination(selfIllumination)
{
}

BlockDefinition::~BlockDefinition()
{
}

BlockType BlockDefinition::GetBlockType() const
{
	return m_blockType;
}

AABB2D BlockDefinition::GetTexCoordsXForward() const
{
	return m_texCoordsXForward;
}

AABB2D BlockDefinition::GetTexCoordsXBack() const
{
	return m_texCoordsXBack;
}

AABB2D BlockDefinition::GetTexCoordsYLeft() const
{
	return m_texCoordsYLeft;
}

AABB2D BlockDefinition::GetTexCoordsYRight() const
{
	return m_texCoordsYRight;
}

AABB2D BlockDefinition::GetTexCoordsZUp() const
{
	return m_texCoordsZUp;
}

AABB2D BlockDefinition::GetTexCoordsZDown() const
{
	return m_texCoordsZDown;
}

AABB2D BlockDefinition::GetTexCoordsSides() const
{
	return m_texCoordsXForward;
}

bool BlockDefinition::IsOpaque() const
{
	return m_isOpaque;
}

bool BlockDefinition::IsSolid() const
{
	return m_isSolid;
}

char BlockDefinition::GetSelfIllumination() const
{
	return m_selfIllumination;
}
