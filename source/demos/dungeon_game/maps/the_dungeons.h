
// ========================================================
//
// Hardcoded tile map arrays for the "dungeons" level.
// This is not meant to be used as a header file but
// as a raw textual include.
//
// ========================================================

const char * dungeonMapName   = "The Dungeons";
const uint   dungeonMapWidth  = 17;
const uint   dungeonMapHeight = 14;

const TileId dungeonTileMap[] =
{
	C_NW, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, C_NE,
	W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	W_NS, FLR,  FLR,  FLR,  W_CP, W_CP, FLR,  FLR,  FLR,  C_S,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	C_SW, C_NE, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	W_CP, W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	W_CP, W_NS, FLR,  FLR,  C_W,  W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, C_SE,
	C_NW, C_SE, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	W_NS, FLR,  FLR,  FLR,  FLR,  W_CP, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	W_NS, FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  FLR,  W_NS,
	C_SW, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, W_WE, C_SE,
};

// ========================================================

PropDesc dungeonTorches[10] =
{
#define TORCH_INSTANCE \
{ \
	MDL_TORCH, \
	0.0f, -1.9f, 0.0f, \
	0.0f,  0.0f, 0.0f, \
	LightShadowBlob::LIGHTMAP, \
	0.0f, -1.4f, 0.0f, \
	0.2f,  0.0f, 0.0f  \
}

	TORCH_INSTANCE,
	TORCH_INSTANCE,
	TORCH_INSTANCE,
	TORCH_INSTANCE,
	TORCH_INSTANCE,
	TORCH_INSTANCE,
	TORCH_INSTANCE,
	TORCH_INSTANCE,
	TORCH_INSTANCE,
	TORCH_INSTANCE,

#undef TORCH_INSTANCE
};

// ========================================================

PropDesc dungeonBarrels[5] =
{
#define BARREL_INSTANCE(rx, py, pz) \
{ \
	MDL_BARREL, \
	0.0f, (py), (pz), \
	(rx), 0.0f, 0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.5f, 0.0f, \
	1.2f,  0.0f, 0.0f  \
}

	BARREL_INSTANCE(90.0f, -1.6f, -0.5f),
	BARREL_INSTANCE( 0.0f, -2.0f,  0.0f),
	BARREL_INSTANCE( 0.0f, -2.0f,  0.0f),
	BARREL_INSTANCE(90.0f, -1.6f, -0.5f),
	BARREL_INSTANCE(90.0f, -1.6f, -0.5f),

#undef BARREL_INSTANCE
};

// ========================================================

PropDesc dungeonCorpses[2] =
{
#define HANGING_CORPSE_INSTANCE \
{ \
	MDL_HANGING_CORPSE, \
	1.0f, -1.5f, 0.0f,  \
	0.0f,  0.0f, 0.0f,  \
	LightShadowBlob::NONE, \
	0.0f,  0.0f, 0.0f, \
	0.0f,  0.0f, 0.0f  \
}

	HANGING_CORPSE_INSTANCE,
	HANGING_CORPSE_INSTANCE,

#undef HANGING_CORPSE_INSTANCE
};

// ========================================================

PropDesc dungeonPlayers[1] =
{
#define PLAYER_INSTANCE \
{ \
	MDL_PLAYER, \
	0.0f, -1.3f, 0.0f, \
	0.0f,  0.0f, 0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.6f, 0.0f, \
	1.5f,  0.0f, 0.0f  \
}

	PLAYER_INSTANCE,

#undef PLAYER_INSTANCE
};

// ========================================================

PropDesc dungeonSkeletons[2] =
{
#define SKELETON_INSTANCE(ry, px, pz) \
{ \
	MDL_SKELETON, \
	(px), -2.0f,  (pz), \
	0.0f,  (ry),  0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.5f,  (pz), \
	1.5f,  0.0f,  0.0f  \
}

	SKELETON_INSTANCE(180.0f,  0.0f, -0.5f),
	SKELETON_INSTANCE(-90.0f, -0.5f,  0.0f),

#undef SKELETON_INSTANCE
};

// ========================================================

PropDesc dungeonGuillotines[1] =
{
#define GUILLOTINE_INSTANCE \
{ \
	MDL_GUILLOTINE, \
	0.0f, -1.9f, 0.0f, \
	0.0f,  0.0f, 0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.5f, 0.0f, \
	2.0f,  0.0f, 0.0f  \
}

	GUILLOTINE_INSTANCE,

#undef GUILLOTINE_INSTANCE
};

// ========================================================

PropDesc dungeonStaircases[1] =
{
#define STAIRCASE_INSTANCE \
{ \
	MDL_SPIRAL_STAIRCASE, \
	0.0f, -1.9f, 0.0f, \
	0.0f,  0.0f, 0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.5f, 0.0f, \
	4.0f,  0.0f, 0.0f  \
}

	STAIRCASE_INSTANCE,

#undef STAIRCASE_INSTANCE
};

// ========================================================

PropDesc dungeonEnemies[9] =
{
#define ENEMY_INSTANCE(ry) \
{ \
	MDL_ENEMY, \
	0.0f, -1.0f, 0.0f, \
	0.0f,  (ry), 0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.6f, 0.0f, \
	1.1f,  0.0f, 0.0f  \
}

	ENEMY_INSTANCE(-90.0f),
	ENEMY_INSTANCE(-90.0f),
	ENEMY_INSTANCE( 90.0f),
	ENEMY_INSTANCE(-90.0f),
	ENEMY_INSTANCE( 90.0f),
	ENEMY_INSTANCE(180.0f),
	ENEMY_INSTANCE(180.0f),
	ENEMY_INSTANCE(  0.0f),
	ENEMY_INSTANCE(-90.0f),

#undef ENEMY_INSTANCE
};

// ========================================================

#define PNUL nullptr
#define T(num) &dungeonTorches[num]
#define B(num) &dungeonBarrels[num]
#define C(num) &dungeonCorpses[num]
#define P(num) &dungeonPlayers[num]
#define S(num) &dungeonSkeletons[num]
#define G(num) &dungeonGuillotines[num]
#define O(num) &dungeonStaircases[num]
#define E(num) &dungeonEnemies[num]

//
// B = barrels
// C = hanging corpse
// E = generic enemy
// G = guillotine
// P = player start location
// S = skeleton
// T = torch
// O = spiral-staircase (level exit)
//
// PNUL = empty tile / no prop
//
PropDesc * dungeonPropMap[] =
{
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  B(0),  PNUL,  E(0),  T(1),  E(1),  PNUL,  PNUL,  PNUL,  PNUL,  T(3),  PNUL,  S(0),  B(2),  B(3),  T(5),  PNUL,
	PNUL,  B(1),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  T(0),  PNUL,  PNUL,  T(2),  PNUL,  PNUL,  PNUL,  T(4),  PNUL,  P(0),  PNUL,  PNUL,  C(1),  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  E(8),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  T(6),  PNUL,
	PNUL,  PNUL,  E(7),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  G(0),  PNUL,  PNUL,  PNUL,  T(8),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  S(1),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  E(3),  PNUL,  PNUL,  PNUL,  PNUL,  E(5),  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  C(0),  PNUL,  T(7),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  O(0),  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  E(6),  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  B(4),  PNUL,  E(2),  PNUL,  PNUL,  PNUL,  PNUL,  E(4),  PNUL,  PNUL,  T(9),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
};

#undef E
#undef O
#undef G
#undef S
#undef P
#undef C
#undef B
#undef T
#undef PNUL

// ========================================================
