
// ================================================================================================
// -*- C++ -*-
// File: tile_map.cpp
// Author: Guilherme R. Lampert
// Created on: 15/03/15
// Brief: Tile Map management and rendering.
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

#include "tile_map.hpp"
#include "render_entity.hpp"
#include "framework/quick_sort.hpp"

// ================================================================================================
// Built-in tile data (include files generated with `obj2c` or `bin2c`):
// ================================================================================================

namespace
{

//
// Common textures used by all tiles:
// (Image files dumped with `bin2c`)
//
#include "tiles/wall_texture.h"
Texture wallTexture;

#include "tiles/floor_texture.h"
Texture floorTexture;

#include "tiles/floor_texture_2.h"
Texture floorTexture2;

#include "tiles/floor_texture_3.h"
Texture floorTexture3;

// ========================================================

struct TileRenderData
{
	const uint16 * indexes;
	uint indexCount;
	const float (*positions)[3];
	uint positionCount;
	const float (*texCoords)[2];
	uint texCoordCount;
};

#define TILE_INDEXES(prefix)        prefix ## Indexes
#define TILE_INDEX_COUNT(prefix)    prefix ## IndexesCount
#define TILE_POSITIONS(prefix)      prefix ## Vertexes
#define TILE_POSITION_COUNT(prefix) prefix ## VertexesCount
#define TILE_TEXCOORDS(prefix)      prefix ## TexCoords
#define TILE_TEXCOORD_COUNT(prefix) prefix ## TexCoordsCount

#define DECLARE_TILE_DATA(prefix)                        \
const TileRenderData prefix ATTRIBUTE_ALIGNED(16) =      \
{                                                        \
	TILE_INDEXES(prefix),   TILE_INDEX_COUNT(prefix),    \
	TILE_POSITIONS(prefix), TILE_POSITION_COUNT(prefix), \
	TILE_TEXCOORDS(prefix), TILE_TEXCOORD_COUNT(prefix)  \
}

#include "tiles/wall_corner_nw.h"
DECLARE_TILE_DATA(wall_corner_nw);

#include "tiles/wall_corner_ne.h"
DECLARE_TILE_DATA(wall_corner_ne);

#include "tiles/wall_corner_sw.h"
DECLARE_TILE_DATA(wall_corner_sw);

#include "tiles/wall_corner_se.h"
DECLARE_TILE_DATA(wall_corner_se);

#include "tiles/wall_corner_n.h"
DECLARE_TILE_DATA(wall_corner_n);

#include "tiles/wall_corner_s.h"
DECLARE_TILE_DATA(wall_corner_s);

#include "tiles/wall_corner_e.h"
DECLARE_TILE_DATA(wall_corner_e);

#include "tiles/wall_corner_w.h"
DECLARE_TILE_DATA(wall_corner_w);

#include "tiles/wall_center_ns.h"
DECLARE_TILE_DATA(wall_center_ns);

#include "tiles/wall_center_we.h"
DECLARE_TILE_DATA(wall_center_we);

#include "tiles/wall_centerpiece.h"
DECLARE_TILE_DATA(wall_centerpiece);

#include "tiles/floor_flat.h"
DECLARE_TILE_DATA(floor_flat);

// TILES ABOVE MUST BE REGISTERED HERE!
const TileRenderData * const tileRenderData[TILE_COUNT] ATTRIBUTE_ALIGNED(16) =
{
	{ &wall_corner_nw   },
	{ &wall_corner_ne   },
	{ &wall_corner_sw   },
	{ &wall_corner_se   },
	{ &wall_corner_n    },
	{ &wall_corner_s    },
	{ &wall_corner_e    },
	{ &wall_corner_w    },
	{ &wall_center_ns   },
	{ &wall_center_we   },
	{ &wall_centerpiece },
	{ &floor_flat       }, // FLR
	{ &floor_flat       }, // FLR2
	{ &floor_flat       }, // FLR3
};

// This one in filled on TileMap startup.
Aabb tileBounds[TILE_COUNT] ATTRIBUTE_ALIGNED(16);

} // namespace {}

// ================================================================================================
// Miscellaneous helpers:
// ================================================================================================

// ========================================================
// loadPermanentTexture():
// ========================================================

void loadPermanentTexture(Texture & texture, const ubyte * data, const uint sizeBytes, const ubyte textureFunction)
{
	ImageData img;
	if (!loadImageFromMemory(data, sizeBytes, img, /* forceRgba = */ true))
	{
		fatalError("Failed to load texture from mem location %p!", data);
	}
	ps2assert(img.comps == 4); // Should have been converted to RGBA

	// Note: Texture NOT mip-mapped!
	lod_t lod;
	lod.calculation = LOD_USE_K;
	lod.max_level   = 0;
	lod.mag_filter  = LOD_MAG_LINEAR;
	lod.min_filter  = LOD_MIN_LINEAR;
	lod.l           = 0;
	lod.k           = 0.0f;
	if (!texture.initFromMemory(img.pixels, TEXTURE_COMPONENTS_RGBA,
	     img.width, img.height, GS_PSM_32, textureFunction, &lod))
	{
		fatalError("Failed to init Texture!");
	}
}

// ========================================================
// computeTileAabbs():
// ========================================================

static void computeTileAabbs()
{
	static bool boundsComputed = false;
	if (boundsComputed)
	{
		return; // Do this only once.
	}

	for (uint t = 0; t < TILE_COUNT; ++t)
	{
		const TileRenderData * const tileData = tileRenderData[t];
		Aabb & tileAabb = tileBounds[t];

		tileAabb.clear();
		for (uint i = 0; i < tileData->indexCount; ++i)
		{
			Vector xyz;
			xyz.x = tileData->positions[tileData->indexes[i]][0];
			xyz.y = tileData->positions[tileData->indexes[i]][1];
			xyz.z = tileData->positions[tileData->indexes[i]][2];
			xyz.w = 1.0f;

			// Add to bounds if new min|max:
			tileAabb.mins = min3PerElement(xyz, tileAabb.mins);
			tileAabb.maxs = max3PerElement(xyz, tileAabb.maxs);
		}
	}

	boundsComputed = true;
	logComment("Tile AABBs computed!");
}

// ========================================================
// loadTilesetTextures():
// ========================================================

static void loadTilesetTextures()
{
	static bool texturesLoaded = false;
	if (texturesLoaded)
	{
		return; // Do this only once.
	}

	loadPermanentTexture(wallTexture,   wall_texture,    size_wall_texture,    TEXTURE_FUNCTION_MODULATE);
	loadPermanentTexture(floorTexture,  floor_texture,   size_floor_texture,   TEXTURE_FUNCTION_MODULATE);
	loadPermanentTexture(floorTexture2, floor_texture_2, size_floor_texture_2, TEXTURE_FUNCTION_MODULATE);
	loadPermanentTexture(floorTexture3, floor_texture_3, size_floor_texture_3, TEXTURE_FUNCTION_MODULATE);

	texturesLoaded = true;
	logComment("Tile map textures loaded successfully!");
}

// ========================================================
// TileDrawQSortPredicate / TileDrawCmd:
// ========================================================

namespace
{

struct TileDrawCmd
{
	float x, z;
	TileId id;
	bool fade;
};

struct TileDrawQSortPredicate
{
	int operator() (const TileDrawCmd & tileA, const TileDrawCmd & tileB) const
	{
		// Floor tiles must go first in the array.
		if (tileA.id != FLR && tileB.id == FLR) { return  1; }
		if (tileA.id == FLR && tileB.id != FLR) { return -1; }
		return scast<int>(tileA.id) - scast<int>(tileB.id);
	}
};

} // namespace {}

// ================================================================================================
// TileMap implementation:
// ================================================================================================

// ========================================================
// TileMap::TileMap():
// ========================================================

TileMap::TileMap()
	: tiles(nullptr)
	, mapTilesX(0)
	, mapTilesZ(0)
	, sceneProps(nullptr)
	, tileDrawCount(0)
	, drawTileBounds(false)
{
	tileRotationOffset.makeIdentity();
	memset(&activeArea, 0, sizeof(activeArea));

	mapColorTint.r = 0.8f;
	mapColorTint.g = 0.8f;
	mapColorTint.b = 0.8f;
	mapColorTint.a = 1.0f;

	fadedColorTint.r = 0.5f;
	fadedColorTint.g = 0.5f;
	fadedColorTint.b = 0.5f;
	fadedColorTint.a = 1.0f;
}

// ========================================================
// TileMap::~TileMap():
// ========================================================

TileMap::~TileMap()
{
	memFree(MEM_TAG_GENERIC, sceneProps);
}

// ========================================================
// TileMap::initFromMemory():
// ========================================================

bool TileMap::initFromMemory(const TileId * tileMap, const uint tilesX, const uint tilesZ)
{
	if (tileMap == nullptr || tilesX == 0 || tilesZ == 0)
	{
		logError("Invalid TileMap data!");
		return false;
	}

	// This constraint is related to using ubytes for the `activeArea`.
	ps2assert(tilesX <= 256);
	ps2assert(tilesZ <= 256);

	// Each tile could in theory have a prop over it.
	// Currently, only floor tiles can have props.
	sceneProps = memClearedAlloc<CPropPtr>(MEM_TAG_GENERIC, (tilesX * tilesZ));

	// Save the `tileMap` pointer ref:
	tiles     = tileMap;
	mapTilesX = tilesX;
	mapTilesZ = tilesZ;

	// This default rotation is needed to fix the
	// tiles for our coordinate system. The tiles also seem
	// to fit better against each other when rotated, not sure why...
	tileRotationOffset.makeRotationY(degToRad(90.0f));

	// Load the persistent tile textures (used by all tiles):
	loadTilesetTextures();

	// One-time bounding box computation for the tile geometry:
	computeTileAabbs();

	return true;
}

// ========================================================
// TileMap::unload():
// ========================================================

void TileMap::unload()
{
	tiles          = nullptr;
	mapTilesX      = 0;
	mapTilesZ      = 0;
	tileDrawCount  = 0;

	tileRotationOffset.makeIdentity();
	memset(&activeArea, 0, sizeof(activeArea));

	mapColorTint.r = 0.8f;
	mapColorTint.g = 0.8f;
	mapColorTint.b = 0.8f;
	mapColorTint.a = 1.0f;

	fadedColorTint.r = 0.5f;
	fadedColorTint.g = 0.5f;
	fadedColorTint.b = 0.5f;
	fadedColorTint.a = 1.0f;

	memFree(MEM_TAG_GENERIC, sceneProps);
	sceneProps = nullptr;
}

// ========================================================
// TileMap::placeSceneProp():
// ========================================================

void TileMap::placeSceneProp(CPropPtr sceneProp, const float x, const float z)
{
	// Estimate tile position:
	uint objTileX = scast<uint>(x / TILE_SIZE);
	uint objTileZ = scast<uint>(z / TILE_SIZE);

	// Clamp it, just in case:
	if (objTileX > mapTilesX - 1) { objTileX = mapTilesX - 1; }
	if (objTileZ > mapTilesZ - 1) { objTileZ = mapTilesZ - 1; }

	// Save it:
	ps2assert(sceneProps != nullptr);
	sceneProps[objTileX + (objTileZ * mapTilesX)] = sceneProp;
}

// ========================================================
// TileMap::removeSceneProp():
// ========================================================

void TileMap::removeSceneProp(const Vector & worldPos)
{
	// Estimate tile position:
	const uint objTileX = scast<uint>(ceilf(worldPos.x / TILE_SIZE));
	const uint objTileZ = scast<uint>(ceilf(worldPos.z / TILE_SIZE));

	if (objTileX >= mapTilesX || objTileZ >= mapTilesZ)
	{
		return; // Invalid input.
	}

	// Clear it:
	ps2assert(sceneProps != nullptr);
	sceneProps[objTileX + (objTileZ * mapTilesX)] = nullptr;
}

// ========================================================
// TileMap::drawMap():
// ========================================================

void TileMap::drawMap(const Frustum & frustum)
{
	ps2assert(tiles != nullptr);
	ps2assert(mapTilesX != 0);
	ps2assert(mapTilesZ != 0);

	// We batch the tiles to reduces texture changes to two:
	// 1 for the walls and 1 for the floor. The batch is sorted
	// before drawing to group each type of tile. Batch is fixed
	// size, so the amount of tiles drawn cannot exceed the max!
	//
	static const uint TILE_BATCH_SIZE = 1024;
	static TileDrawCmd tileBatch[TILE_BATCH_SIZE] ATTRIBUTE_ALIGNED(16);

	// Assemble the batch for visible tiles:
	//
	bool fadeTile = false;
	uint tilesBatched = 0;
	for (uint z = 0; z < mapTilesZ; ++z)
	{
		for (uint x = 0; x < mapTilesX; ++x)
		{
			const TileId tileId = tiles[x + (z * mapTilesX)];
			const float tx = x * TILE_SIZE;
			const float tz = z * TILE_SIZE;

			// Test if inside the active area/pattern:
			bool doDraw = false;
			for (uint i = 0; i < ActiveTiles::PATTERN_SIZE; ++i)
			{
				if (x == activeArea.pattern[i][0] && z == activeArea.pattern[i][1])
				{
					fadeTile = scast<bool>(activeArea.isEdge[i]);
					doDraw   = true;
					break;
				}
			}

			// This tile didn't make into our active area, discard it.
			if (!doDraw)
			{
				continue;
			}

			// Also skip tiles that are not inside the view frustum:
			if (!tileIsVisible(frustum, tileId, tx, tz))
			{
				continue;
			}

			ps2assert(tilesBatched < TILE_BATCH_SIZE && "Tile batch size exceeded!");

			// Add a draw command to the batch:
			TileDrawCmd & cmd = tileBatch[tilesBatched++];
			cmd.x    = tx;
			cmd.z    = tz;
			cmd.id   = tileId;
			cmd.fade = fadeTile;
		}
	}

	// Sort the batch:
	//
	TileDrawQSortPredicate pred;
	quickSort(tileBatch, tilesBatched, pred);

	// Now draw the sorted batch:
	//
	TileId prevTile = TILE_COUNT;
	for (uint t = 0; t < tilesBatched; ++t)
	{
		const TileDrawCmd & cmd = tileBatch[t];

		if (cmd.id != prevTile)
		{
			if (cmd.id == FLR) // Floor tile (type 1)
			{
				gRenderer.setTexture(floorTexture);
			}
			else if (cmd.id == FLR2) // Floor tile (type 2)
			{
				gRenderer.setTexture(floorTexture2);
			}
			else if (cmd.id == FLR3) // Floor tile (type 3)
			{
				gRenderer.setTexture(floorTexture3);
			}
			else // Wall tile
			{
				gRenderer.setTexture(wallTexture);
			}
			prevTile = cmd.id;
		}

		const Color4f & tileTint = cmd.fade ? fadedColorTint : mapColorTint;
		drawSingleTile(cmd.id, cmd.x, cmd.z, tileTint);
	}
}

// ========================================================
// TileMap::drawSingleTile():
// ========================================================

void TileMap::drawSingleTile(const TileId tileId, const float x, const float z, const Color4f & tileTint)
{
	Matrix matTrans;
	matTrans.makeTranslation(x, 0.0f, z);
	gRenderer.setModelMatrix(tileRotationOffset * matTrans);

	const TileRenderData * const tileData = tileRenderData[tileId];
	gRenderer.drawIndexedTriangles(
		tileData->indexes,   tileData->indexCount,
		tileData->positions, tileData->positionCount,
		tileData->texCoords, tileData->texCoordCount,
		tileTint);

	if (drawTileBounds)
	{
		if (tileIsFloor(tileId))
		{
			// Add a tiny offset to the bounds of floor tiles
			// so that they draw better with less flickering.
			Aabb bbox = tileBounds[tileId];
			bbox.mins.y += 0.1f;
			bbox.maxs.y += 0.1f;
			gRenderer.drawAabb(bbox, makeColor4f(1.0f, 0.0f, 1.0f));
		}
		else
		{
			gRenderer.drawAabb(tileBounds[tileId], makeColor4f(1.0f, 0.0f, 1.0f));
		}
	}

	++tileDrawCount;
}

// ========================================================
// TileMap::updateActiveArea():
// ========================================================

void TileMap::updateActiveArea(const Vector & center)
{
	// This is a bit of an approximation, but seems work fine.
	const ubyte objTileX = scast<ubyte>(ceilf(center.x / TILE_SIZE));
	const ubyte objTileZ = scast<ubyte>(ceilf(center.z / TILE_SIZE));

	// Last one will hold the estimate tile the player is on.
	static const uint N = ActiveTiles::PATTERN_SIZE - 1;
	activeArea.pattern[N][0] = objTileX;
	activeArea.pattern[N][1] = objTileZ;

	// The following saves the positions of each tile in the 11x11
	// square tiles active area around the current player's position.
	// Also marks the ones that are in the edge of this square
	// so that they are properly darkened/faded-out.
	//
	// The magic number 5|-5 is the max distance from the center tile.
	// (any tile with this offset is at the edge)
	//
	int8 offset = 1;
	int8 incr   = 1;
	for (uint i = 0; i < 10; ++i)
	{
		activeArea.pattern[i][0] = objTileX;
		activeArea.pattern[i][1] = objTileZ + offset;

		activeArea.isEdge[i] = scast<ubyte>(offset == 5 || offset == -5);

		if (offset == 5)
		{
			offset =  0;
			incr   = -1;
		}
		offset += incr;
	}

	offset = 1;
	incr   = 1;
	for (uint i = 10; i < N - 10; i += 11)
	{
		activeArea.pattern[i +  0][0] = objTileX + offset;
		activeArea.pattern[i +  0][1] = objTileZ + 0;
		activeArea.pattern[i +  1][0] = objTileX + offset;
		activeArea.pattern[i +  1][1] = objTileZ + 1;
		activeArea.pattern[i +  2][0] = objTileX + offset;
		activeArea.pattern[i +  2][1] = objTileZ + 2;
		activeArea.pattern[i +  3][0] = objTileX + offset;
		activeArea.pattern[i +  3][1] = objTileZ + 3;
		activeArea.pattern[i +  4][0] = objTileX + offset;
		activeArea.pattern[i +  4][1] = objTileZ + 4;
		activeArea.pattern[i +  5][0] = objTileX + offset;
		activeArea.pattern[i +  5][1] = objTileZ + 5;
		activeArea.pattern[i +  6][0] = objTileX + offset;
		activeArea.pattern[i +  6][1] = objTileZ - 1;
		activeArea.pattern[i +  7][0] = objTileX + offset;
		activeArea.pattern[i +  7][1] = objTileZ - 2;
		activeArea.pattern[i +  8][0] = objTileX + offset;
		activeArea.pattern[i +  8][1] = objTileZ - 3;
		activeArea.pattern[i +  9][0] = objTileX + offset;
		activeArea.pattern[i +  9][1] = objTileZ - 4;
		activeArea.pattern[i + 10][0] = objTileX + offset;
		activeArea.pattern[i + 10][1] = objTileZ - 5;

		const ubyte isEdge = scast<ubyte>(offset == 5 || offset == -5);
		activeArea.isEdge[i +  0] = isEdge;
		activeArea.isEdge[i +  1] = isEdge;
		activeArea.isEdge[i +  2] = isEdge;
		activeArea.isEdge[i +  3] = isEdge;
		activeArea.isEdge[i +  4] = isEdge;
		activeArea.isEdge[i +  5] = 1; // Z +5, always edge
		activeArea.isEdge[i +  6] = isEdge;
		activeArea.isEdge[i +  7] = isEdge;
		activeArea.isEdge[i +  8] = isEdge;
		activeArea.isEdge[i +  9] = isEdge;
		activeArea.isEdge[i + 10] = 1; // Z -5, always edge

		if (offset == 5)
		{
			offset =  0;
			incr   = -1;
		}
		offset += incr;
	}
}

// ========================================================
// TileMap::collidesWithWallOrProp():
// ========================================================

bool TileMap::collidesWithWallOrProp(const Aabb & bbox, const Vector & boxPos) const
{
	// Estimate tile position:
	const uint objTileX = scast<uint>(ceilf(boxPos.x / TILE_SIZE));
	const uint objTileZ = scast<uint>(ceilf(boxPos.z / TILE_SIZE));

	// Max x/z indexes:
	const uint maxMapX  = mapTilesX - 1;
	const uint maxMapZ  = mapTilesZ - 1;

#define GET_TILE(t, x, z) \
do { \
	(t).tx = (((x) <= maxMapX) ? (x) : maxMapX);  \
	(t).tz = (((z) <= maxMapZ) ? (z) : maxMapZ);  \
	(t).id = tiles[(t).tx + ((t).tz * mapTilesX)]; \
} while (0)

	// Fetch the 8 tiles closest to the inferred position.
	// Include the center tile. This will limit our testing
	// to at most 9 tiles.
	struct { uint tx, tz; TileId id; } t[3*3];
	GET_TILE(t[0], objTileX + 0, objTileZ + 0);
	GET_TILE(t[1], objTileX + 0, objTileZ + 1);
	GET_TILE(t[2], objTileX + 0, objTileZ - 1);
	GET_TILE(t[3], objTileX + 1, objTileZ + 0);
	GET_TILE(t[4], objTileX + 1, objTileZ + 1);
	GET_TILE(t[5], objTileX + 1, objTileZ - 1);
	GET_TILE(t[6], objTileX - 1, objTileZ + 0);
	GET_TILE(t[7], objTileX - 1, objTileZ + 1);
	GET_TILE(t[8], objTileX - 1, objTileZ - 1);

#undef GET_TILE

	ps2assert(sceneProps != nullptr);

	// Test collision with any nearby walls or scene props:
	for (uint i = 0; i < 3*3; ++i)
	{
		// We only want to collide against walls.
		if (tileIsFloor(t[i].id))
		{
			// But also test if we are not colliding with any
			// scene props that might be on this floor tile.
			CPropPtr prop = sceneProps[t[i].tx + (t[i].tz * mapTilesX)];
			if (prop != nullptr)
			{
				if (Aabb::collision(bbox, boxPos, prop->getBounds(), prop->getWorldPosition()))
				{
					return true; // Collides with scene prop.
				}
			}

			// Skipp empty floor tile.
			continue;
		}

		// Test collision with this wall tile:
		const Vector tilePos(t[i].tx * TILE_SIZE, 0.0f, t[i].tz * TILE_SIZE, 1.0f);
		if (Aabb::collision(bbox, boxPos, tileBounds[t[i].id], tilePos))
		{
			return true; // Collides with wall.
		}
	}

	return false; // No intersections/collision.
}

// ========================================================
// TileMap::tileIsVisible():
// ========================================================

bool TileMap::tileIsVisible(const Frustum & frustum, const TileId tileId, const float x, const float z) const
{
	// Translate the tile bounding box:
	Aabb tileAabb = tileBounds[tileId];
	tileAabb.mins.x += x;
	tileAabb.mins.z += z;
	tileAabb.maxs.x += x;
	tileAabb.maxs.z += z;
	return frustum.testAabb(tileAabb);
}

// ========================================================
// TileMap::inActiveArea():
// ========================================================

bool TileMap::inActiveArea(const Vector & position, bool * isInsideEdge) const
{
	// Estimate tile position:
	const uint objTileX = scast<uint>(ceilf(position.x / TILE_SIZE));
	const uint objTileZ = scast<uint>(ceilf(position.z / TILE_SIZE));

	for (uint i = 0; i < ActiveTiles::PATTERN_SIZE; ++i)
	{
		if (objTileX == activeArea.pattern[i][0] && objTileZ == activeArea.pattern[i][1])
		{
			if (isInsideEdge != nullptr)
			{
				(*isInsideEdge) = scast<bool>(activeArea.isEdge[i]);
			}
			return true;
		}
	}

	// Not in the active area or edge.
	if (isInsideEdge != nullptr)
	{
		(*isInsideEdge) = false;
	}
	return false;
}

// ========================================================
// TileMap::getTileDrawCount():
// ========================================================

uint TileMap::getTileDrawCount()
{
	const uint count = tileDrawCount;
	tileDrawCount = 0;
	return count;
}
