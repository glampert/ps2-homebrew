
// ========================================================
//
// Hardcoded tile map arrays for a 9x9 tiles test level.
// This is not meant to be used as a header file but
// as a raw textual include.
//
// ========================================================

const char * testMapName   = "Test Map";
const uint   testMapWidth  = 9;
const uint   testMapHeight = 9;

const TileId testTileMap[] =
{
	C_NW, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, C_NE,
	W_NS, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, W_NS,
	W_NS, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, W_NS,
	W_NS, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, W_NS,
	W_NS, FLR3, FLR3, FLR3, W_CP, FLR3, FLR3, FLR3, W_NS,
	W_NS, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, W_NS,
	W_NS, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, W_NS,
	W_NS, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, FLR3, W_NS,
	C_SW, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, C_SE,
};

// ========================================================

PropDesc testPlayerProp =
{
	MDL_PLAYER,
	0.0f, -1.3f,  0.0f,
	0.0f,  0.0f,  0.0f,
	LightShadowBlob::SHADOW_BLOB,
	0.0f, -1.6f,  0.0f,
	1.5f,  0.0f,  0.0f
};

#define PNUL nullptr
#define PLYR &testPlayerProp

PropDesc * testPropMap[] =
{
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL, PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL, PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL, PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL, PNUL,
	PNUL,  PLYR,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL, PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL, PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL, PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL, PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL, PNUL,
};

#undef PLYR
#undef PNUL

// ========================================================
