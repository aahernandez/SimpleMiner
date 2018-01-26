#pragma once
#include "Game/GameCommon.hpp"

 class Block
 {
 public:
	 unsigned char m_blockType;
	 unsigned char m_lightingAndFlags; //Flags: sky, opaque, solid, dirty lighting
	 unsigned char m_biomeTemp; 

	 Block();
	 Block(unsigned char blockType, bool isOpaque, bool isSolid);
	 ~Block();

	 void SetLightLevel(int lightLevel);
	 int GetLightLevel() const;
	 unsigned char GetBlockType() const;
	 void SetBlockType(unsigned char blockType); //set flags for that type of block as well
	 void SetIsSky(bool isSky);
	 bool GetIsSky();
	 void SetIsOpaque(bool isOpaque);
	 bool GetIsOpaque();
	 void SetIsSolid(bool isSolid);
	 bool GetIsSolid();
	 void SetIsLightingDirty(bool isLightingDirty);
	 bool GetIsLightingDirty();
 };