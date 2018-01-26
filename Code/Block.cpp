#include "Game/Block.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Block::Block()
	: m_blockType(0)
{
	m_lightingAndFlags = 0;
	SetIsSky(false);
	SetIsOpaque(false);
	SetIsSolid(false);
	SetIsLightingDirty(false);
	SetLightLevel(0);
}

Block::Block(unsigned char blockType, bool isOpaque, bool isSolid)
{
	m_blockType = blockType;
	m_lightingAndFlags = 0;
	SetLightLevel(0);
	SetIsSky(false);
	SetIsOpaque(isOpaque);
	SetIsSolid(isSolid);
	SetIsLightingDirty(false);
}

Block::~Block()
{
}

void Block::SetLightLevel(int lightLevel)
{
	ASSERT_OR_DIE(lightLevel <= MASK_LIGHT, "Light level is greater than 15");
	m_lightingAndFlags &= ~MASK_LIGHT;
	m_lightingAndFlags |= lightLevel;
}

int Block::GetLightLevel() const
{
	return m_lightingAndFlags & MASK_LIGHT;
}

unsigned char Block::GetBlockType() const
{
	return m_blockType;
}

void Block::SetBlockType(unsigned char blockType)
{
	m_blockType = blockType;
}

void Block::SetIsSky(bool isSky)
{
	if (isSky)
	{
		m_lightingAndFlags |= MASK_IS_SKY;		
	}
	else
	{
		m_lightingAndFlags &= ~MASK_IS_SKY;
	}
}

bool Block::GetIsSky()
{
	return (m_lightingAndFlags & MASK_IS_SKY) == MASK_IS_SKY;
}

void Block::SetIsOpaque(bool isOpaque)
{
	if (isOpaque)
	{
		m_lightingAndFlags |= MASK_IS_OPAQUE;
	}
	else
	{
		m_lightingAndFlags &= ~MASK_IS_OPAQUE;
	}
}

bool Block::GetIsOpaque()
{
	return (m_lightingAndFlags & MASK_IS_OPAQUE) == MASK_IS_OPAQUE;
}

void Block::SetIsSolid(bool isSolid)
{
	if (isSolid)
	{
		m_lightingAndFlags |= MASK_IS_SOLID;
	}
	else
	{
		m_lightingAndFlags &= ~MASK_IS_SOLID;
	}
}

bool Block::GetIsSolid()
{
	return (m_lightingAndFlags & MASK_IS_SOLID) == MASK_IS_SOLID;
}

void Block::SetIsLightingDirty(bool isLightingDirty)
{
	if (isLightingDirty)
	{
		m_lightingAndFlags |= MASK_IS_LIGHTING_DIRTY;
	}
	else
	{
		m_lightingAndFlags &= ~MASK_IS_LIGHTING_DIRTY;
	}
}

bool Block::GetIsLightingDirty()
{
	return (m_lightingAndFlags & MASK_IS_LIGHTING_DIRTY) == MASK_IS_LIGHTING_DIRTY;
}
