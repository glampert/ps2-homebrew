
// ========================================================
//
// Hardcoded tile map arrays for the "graveyard" level.
// This is not meant to be used as a header file but
// as a raw textual include.
//
// ========================================================

const char * graveyardMapName   = "The Graveyard";
const uint   graveyardMapWidth  = 11;
const uint   graveyardMapHeight = 22;

const TileId graveyardTileMap[] =
{
	C_NW,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  C_NE,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR2,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR3,  FLR3,  FLR3,  FLR3,  FLR3,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR3,  W_CP,  FLR3,  W_CP,  FLR3,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR3,  FLR3,  FLR3,  FLR3,  FLR3,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR3,  FLR3,  FLR3,  FLR3,  FLR3,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR3,  FLR3,  FLR3,  FLR3,  FLR3,  FLR3,  FLR3,  FLR3,  FLR3,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	W_NS,  FLR2,  FLR2,  FLR2,  FLR2,  FLR3,  FLR2,  FLR2,  FLR2,  FLR2,  W_NS,
	C_SW,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  W_WE,  C_SE,
};

// ========================================================

PropDesc graveyardPlayers[1] =
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

PropDesc graveyardTorches[2] =
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

#undef TORCH_INSTANCE
};

// ========================================================

PropDesc graveyardTreeHunks[2] =
{
#define TREE_INSTANCE \
{ \
	MDL_DEAD_TREE, \
	0.0f, -1.9f, 0.0f, \
	0.0f,  0.0f, 0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.5f, 0.0f, \
	3.5f,  0.0f, 0.0f  \
}

	TREE_INSTANCE,
	TREE_INSTANCE,

#undef TREE_INSTANCE
};

// ========================================================

PropDesc graveyardBanners[2] =
{
#define BANNER_INSTANCE \
{ \
	MDL_BANNER, \
	0.0f, -1.5f, -1.15f, \
	0.0f,  90.0f, 0.0f,  \
	LightShadowBlob::NONE, \
	0.0f,  0.0f,  0.0f, \
	0.0f,  0.0f,  0.0f  \
}

	BANNER_INSTANCE,
	BANNER_INSTANCE,

#undef BANNER_INSTANCE
};

// ========================================================

PropDesc graveyardStatues[2] =
{
#define STATUE_INSTANCE(ry) \
{ \
	MDL_STATUE, \
	0.0f, -2.0f, 0.0f, \
	0.0f,  (ry), 0.0f,  \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.5f, 0.0f, \
	2.5f,  0.0f, 0.0f  \
}

	STATUE_INSTANCE(-90.0f),
	STATUE_INSTANCE( 90.0f),

#undef STATUE_INSTANCE
};

// ========================================================

PropDesc graveyardTombstone1[10] = // Cross
{
#define TOMBS_INSTANCE(rx, ry) \
{ \
	MDL_TOMBSTONE_1, \
	0.0f, -2.0f,  0.0f, \
	(rx),  (ry),  0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.57f, 0.0f, \
	0.8f,  0.0f,  0.0f  \
}

	TOMBS_INSTANCE(0.0f, -90.0f ),
	TOMBS_INSTANCE(0.0f, -180.0f),
	TOMBS_INSTANCE(0.0f,  0.0f  ),
	TOMBS_INSTANCE(0.0f, -90.0f ),
	TOMBS_INSTANCE(0.0f, -90.0f ),
	TOMBS_INSTANCE(0.0f, -90.0f ),
	TOMBS_INSTANCE(0.0f, -90.0f ),
	TOMBS_INSTANCE(0.0f, -90.0f ),
	TOMBS_INSTANCE(0.0f, -90.0f ),
	TOMBS_INSTANCE(0.0f, -90.0f ),

#undef TOMBS_INSTANCE
};

// ========================================================

PropDesc graveyardTombstone2[12] = // Plain stone block
{
#define TOMBS_INSTANCE(rx) \
{ \
	MDL_TOMBSTONE_2, \
	0.0f, -2.0f,  0.0f, \
	(rx), -90.0f, 0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.5f,  0.0f, \
	1.0f,  0.0f,  0.0f  \
}

	TOMBS_INSTANCE( 0.0f ),
	TOMBS_INSTANCE( 0.0f ),
	TOMBS_INSTANCE(-90.0f),
	TOMBS_INSTANCE( 0.0f ),
	TOMBS_INSTANCE( 0.0f ),
	TOMBS_INSTANCE( 0.0f ),
	TOMBS_INSTANCE(-90.0f),
	TOMBS_INSTANCE(-90.0f),
	TOMBS_INSTANCE( 0.0f ),
	TOMBS_INSTANCE( 0.0f ),
	TOMBS_INSTANCE( 0.0f ),
	TOMBS_INSTANCE(-90.0f),

#undef TOMBS_INSTANCE
};

// ========================================================

PropDesc graveyardSkeletons[1] =
{
#define SKELETON_INSTANCE(ry, px, pz) \
{ \
	MDL_SKELETON, \
	(px), -2.0f, (pz), \
	0.0f,  (ry), 0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	(px), -1.5f, (pz), \
	1.0f,  0.0f, 0.0f  \
}

	SKELETON_INSTANCE(-90.0f, -0.5f, 0.0f),

#undef SKELETON_INSTANCE
};

// ========================================================

PropDesc graveyardCorpses[2] =
{
#define CORPSE_INSTANCE(px, py, pz, ry) \
{ \
	MDL_HANGING_CORPSE, \
	(px), (py), (pz), \
	0.0f, (ry), 0.0f, \
	LightShadowBlob::NONE, \
	0.0f, 0.0f, 0.0f, \
	0.0f, 0.0f, 0.0f  \
}

	CORPSE_INSTANCE(1.0f, -1.1f,  0.0f,   0.0f),
	CORPSE_INSTANCE(0.0f, -1.5f, -1.0f,  90.0f),

#undef CORPSE_INSTANCE
};

// ========================================================

PropDesc graveyardBarrels[2] =
{
#define BARREL_INSTANCE(rx, py, pz) \
{ \
	MDL_BARREL, \
	0.0f,  (py), (pz), \
	(rx),  0.0f, 0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.5f, 0.0f, \
	1.2f,  0.0f, 0.0f  \
}

	BARREL_INSTANCE( 0.0f, -2.0f,  0.0f),
	BARREL_INSTANCE(90.0f, -1.6f, -0.5f),

#undef BARREL_INSTANCE
};

// ========================================================

PropDesc graveyardEnemies[6] =
{
#define ENEMY_INSTANCE(ry) \
{ \
	MDL_ENEMY, \
	0.0f, -1.0f,  0.0f, \
	0.0f,  (ry),  0.0f, \
	LightShadowBlob::SHADOW_BLOB, \
	0.0f, -1.53f, 0.0f, \
	1.1f,  0.0f,  0.0f  \
}

	ENEMY_INSTANCE(-90.0f),
	ENEMY_INSTANCE(-90.0f),
	ENEMY_INSTANCE( 0.0f ),
	ENEMY_INSTANCE(-90.0f),
	ENEMY_INSTANCE(180.0f),
	ENEMY_INSTANCE( 0.0f ),

#undef ENEMY_INSTANCE
};

// ========================================================

PropDesc graveyardBoss[1] =
{
#define BOSS_ENEMY_INSTANCE \
{ \
	MDL_BOSS, \
	0.0f, -0.8f,  0.0f, \
	0.0f, -90.0f, 0.0f, \
	LightShadowBlob::NONE, \
	0.0f,  0.0f,  0.0f, \
	0.0f,  0.0f,  0.0f  \
}

	BOSS_ENEMY_INSTANCE,

#undef BOSS_ENEMY_INSTANCE
};

// ========================================================

#define PNUL nullptr
#define P(num)  &graveyardPlayers[num]
#define T(num)  &graveyardTorches[num]
#define H(num)  &graveyardTreeHunks[num]
#define F(num)  &graveyardBanners[num]
#define S(num)  &graveyardStatues[num]
#define T1(num) &graveyardTombstone1[num]
#define T2(num) &graveyardTombstone2[num]
#define B(num)  &graveyardSkeletons[num]
#define E(num)  &graveyardEnemies[num]
#define C(num)  &graveyardCorpses[num]
#define K(num)  &graveyardBarrels[num]
#define X(num)  &graveyardBoss[num]

//
// P = player start location
// T = torch
// H = dead tree hunk
// F = flags/banners
// S = statue
// T1/T2 = tombstones (two models)
// B = bones/skeleton
// E = generic enemy
// C = corpse
// K = keg/barrel
// X = level 'boss' (super badie)
//
// PNUL = empty tile / no prop
//
PropDesc * graveyardPropMap[] =
{
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  C(1),  PNUL,  PNUL,  K(0),  K(1),  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  T(0),  PNUL,  PNUL,  PNUL,  T(1),  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  F(0),  X(0),  F(1),  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  T1(9), T2(10),PNUL,  E(0),  PNUL,  E(1),  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  T2(9), T2(8), T2(11),PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  T1(3), C(0),  H(1),  PNUL,  PNUL,
	PNUL,  T1(8), T1(7), PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  E(3),  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  E(5),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  E(4),  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  T2(7), T1(5), PNUL,  PNUL,  PNUL,  T2(6), T1(4), PNUL,  PNUL,
	PNUL,  E(2),  PNUL,  T1(6), PNUL,  PNUL,  PNUL,  T2(5), T2(4), PNUL,  PNUL,
	PNUL,  S(0),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  S(1),  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  T2(0), PNUL,  T2(1), PNUL,  PNUL,
	PNUL,  PNUL,  H(0),  T1(2), PNUL,  P(0),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  B(0),  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  T1(1), PNUL,
	PNUL,  PNUL,  T2(3), PNUL,  PNUL,  PNUL,  T1(0), PNUL,  T2(2), PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
	PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,  PNUL,
};

#undef X
#undef K
#undef C
#undef E
#undef B
#undef T2
#undef T1
#undef S
#undef F
#undef H
#undef T
#undef P
#undef PNUL

// ========================================================
