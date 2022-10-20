
// ================================================================================================
// -*- C++ -*-
// File: game_world.hpp
// Author: Guilherme R. Lampert
// Created on: 29/03/15
// Brief: Game world and simulation data.
//
// License:
//  This source code is released under the MIT License.
//  Copyright (c) 2015 Guilherme R. Lampert.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//
// ================================================================================================

#ifndef GAME_WORLD_HPP
#define GAME_WORLD_HPP

#include "tile_map.hpp"
#include "render_entity.hpp"
#include "game_entity.hpp"

#include "framework/game_pad.hpp"
#include "framework/particle_emitter.hpp"
#include "framework/third_person_camera.hpp"
#include "framework/first_person_camera.hpp"

// ========================================================
// struct PropDesc:
// ========================================================

struct ATTRIBUTE_ALIGNED(16) PropDesc
{
	// 3D model info (id, offset, rotation):
	ModelId modelId;
	float mpx, mpy, mpz;
	float mrx, mry, mrz;

	// Info for the lightmap or shadow blob (or none):
	LightShadowBlob::Type lightOrShadow;
	float lsx, lsy, lsz;
	float lsSize;

	// Tile x/z coords (used at load time; set to zero on declaration):
	float tx, tz;

	// Sort predicate for `qsort` and/or `Array::sort`.
	static int sortByModelType(const void * a, const void * b)
	{
		const PropDesc * ptrA = *rcast<PropDesc * const *>(a);
		const PropDesc * ptrB = *rcast<PropDesc * const *>(b);
		return scast<int>(ptrA->modelId) - scast<int>(ptrB->modelId);
	}
};

// ========================================================
// enum LevelId:
// ========================================================

enum LevelId
{
	// Levels available:
	LVL_THE_DUNGEONS,
	LVL_THE_GRAVEYARD,
	LVL_TEST_MAP,

	// Internal use constants:
	LEVEL_COUNT,
	NO_LEVEL = LEVEL_COUNT
};

// ========================================================
// class GameWorld:
// ========================================================

class GameWorld
{
public:

	GameWorld();

	//
	// Per-frame updates/rendering:
	//
	void updateGameLogic();
	void renderFrame3d();
	void renderMisc2d();
	void renderUiOverlays();

	//
	// Map/level loading:
	//
	void unloadLevel();
	void loadLevel(LevelId lvlId);
	void loadLevel(const char * mapName, const TileId * tileMap,
	               PropDesc * const * propMap, uint mapWidth, uint mapHeight);

	//
	// Miscellaneous accessors/helpers:
	//
	TileMap       & getTileMap() { return worldMap; }
	PlayerEntity  & getPlayer()  { return player;   }
	const Texture * getBuiltInTexture(const char * id) const;

	//
	// Entity allocation:
	//
	EnemyEntity     * allocEnemyEntity();
	RenderEntity    * allocRenderEntity();
	LightShadowBlob * allocShadowBlob();
	LightShadowBlob * allocLightmap();
	ParticleEmitter * allocParticleEmitter();

private:

	// Copy/assign disallowed.
	GameWorld(const GameWorld &);
	GameWorld & operator = (const GameWorld &);

	// Internal helpers:
	void beginFadeOut();
	void beginFadeIn();
	void onFadeOutFinished();
	void onFadeInFinished();
	static void drawMainMenuOpt(Vec2f & pos, const char * entryName, bool checked);

	// Render matrices, culling:
	Matrix  viewMatrix;
	Matrix  projectionMatrix;
	Frustum frustum;

	// Cameras / controls:
	GamePad    * gamePad;
	CameraBase * currCamera;
	ThirdPersonCamera thirdPersonCamera;
	FirstPersonCamera firstPersonCamera;

	// The world "tiles":
	TileMap worldMap;

	// The unique player instance:
	PlayerEntity player;

	// Debug counters/flags:
	uint entitiesDrawn;
	uint shadowsDrawn;
	uint lightmapsDrawn;
	uint prtsDrawn;
	bool drawModelBounds;
	bool showDevConsole;

	// Control screen fade-in/fade-out effects.
	ubyte fadeAlpha;
	bool  drawFadeScreen;
	bool  fadeOut;
	bool  fadeIn;

	// Menu/game-level, etc:
	bool    inMainMenu;
	LevelId currLevel;
	int     currMainMenuSel;

	// These values are per game-level/world instance, not per rendered frame!
	// My demo levels are quite small, so we don't need many at the moment.
	static const uint MAX_ENEMY_ENTITIES    = 24;
	static const uint MAX_RENDER_ENTITIES   = 64;
	static const uint MAX_SHADOW_BLOBS      = 64;
	static const uint MAX_LIGHTMAPS         = 32;
	static const uint MAX_PARTICLE_EMITTERS = 24;

	// Used count in the following arrays:
	uint enemyEntitiesInUse;
	uint renderEntitiesInUse;
	uint shadowBlobsInUse;
	uint lightmapsInUse;
	uint prtEmittersInUse;

	// Entity/object pools, for each level/world instance:
	EnemyEntity     enemyEntities  [MAX_ENEMY_ENTITIES]    ATTRIBUTE_ALIGNED(16);
	RenderEntity    renderEntities [MAX_RENDER_ENTITIES]   ATTRIBUTE_ALIGNED(16);
	LightShadowBlob shadowBlobs    [MAX_SHADOW_BLOBS]      ATTRIBUTE_ALIGNED(16);
	LightShadowBlob lightmaps      [MAX_LIGHTMAPS]         ATTRIBUTE_ALIGNED(16);
	ParticleEmitter prtEmitters    [MAX_PARTICLE_EMITTERS] ATTRIBUTE_ALIGNED(16);
};

#endif // GAME_WORLD_HPP
