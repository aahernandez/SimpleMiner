#pragma once

class Renderer;
class InputSystem;
class AudioSystem;

const float SCREEN_RATIO_WIDTH = 1600.f;
const float SCREEN_RATIO_HEIGHT = 900.f;

const int CHUNK_BITS_X = 4;
const int CHUNK_BITS_Y = 4;
const int CHUNK_BITS_Z = 7; //7
const int CHUNK_BITS_XY = CHUNK_BITS_X + CHUNK_BITS_Y;
const int CHUNK_BITS_XYZ = CHUNK_BITS_XY + CHUNK_BITS_Z;
const int CHUNK_BLOCKS_WIDE_X = 1 << CHUNK_BITS_X;
const int CHUNK_BLOCKS_DEEP_Y = 1 << CHUNK_BITS_Y;
const int CHUNK_BLOCKS_TALL_Z = 1 << CHUNK_BITS_Z;
const int CHUNK_BLOCKS_PER_LAYER = 1 << CHUNK_BITS_XY;
const int NUM_BLOCKS_PER_CHUNK = CHUNK_BLOCKS_WIDE_X * CHUNK_BLOCKS_DEEP_Y * CHUNK_BLOCKS_TALL_Z;
const int TEST_NUM = 1 >> CHUNK_BLOCKS_WIDE_X;
const int MASK_X = (1 << CHUNK_BITS_X) - 1; //15 - 0000000 0000 1111;
const int MASK_Y = ( (1 << CHUNK_BITS_XY) - 1) & ~(MASK_X); //240 - 0000000 1111 0000;
const int MASK_Z = ( (1 << CHUNK_BITS_XYZ) - 1) & ~(MASK_X | MASK_Y); //7936 - 0011111 0000 0000
const unsigned char MASK_IS_SKY =			 0b10000000;
const unsigned char MASK_IS_OPAQUE =		 0b01000000;
const unsigned char MASK_IS_SOLID =			 0b00100000;
const unsigned char MASK_IS_LIGHTING_DIRTY = 0b00010000;
const unsigned char MASK_LIGHT =			 0b00001111;

extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;

extern float g_deltaSeconds;
extern bool g_isSavingAndLoading;
extern bool g_loadAllChunksOnStartup;
extern bool g_isWeatherActive;
extern bool g_isHelpActive;
