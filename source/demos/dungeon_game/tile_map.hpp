
// ================================================================================================
// -*- C++ -*-
// File: tile_map.hpp
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

#ifndef TILE_MAP_HPP
#define TILE_MAP_HPP

#include "framework/common.hpp"
#include "framework/renderer.hpp"
#include "framework/ps2_math/frustum.hpp"

// ========================================================

// Forward declarations/aliases:
class RenderEntity;
typedef const RenderEntity * CPropPtr;

// Size of a single tile:
const float TILE_SIZE = 2.5f;

// ========================================================
// enum TileId:
// ========================================================

enum TileId
{
	/*
	 * ---- Wall corners: ----
	 */
	C_NW,
	// +--
	// |

	C_NE,
	// --+
	//   |

	C_SW,
	// |
	// +--

	C_SE,
	//   |
	// --+

	C_N,
	// +--+
	// |  |

	C_S,
	// |  |
	// +--+

	C_E,
	// --+
	//   |
	// --+

	C_W,
	// +--
	// |
	// +--

	/*
	 * ---- Wall centers: ----
	 */
	W_NS,
	// | |
	// | |

	W_WE,
	// ---
	// ---

	W_CP,
	// +--+
	// |++|
	// +--+

	/*
	 * ---- Floor tiles: ----
	 */
	FLR,  // Main floor tile
	FLR2, // Alternate floor 1 (with another texture)
	FLR3, // Alternate floor 2 (with another texture)

	// Internal use.
	TILE_COUNT
};

// ========================================================
// class TileMap:
// ========================================================

class TileMap
{
public:

	//
	// Initialization/misc:
	//
	 TileMap();
	~TileMap();

	bool initFromMemory(const TileId * tileMap, uint tilesX, uint tilesZ);
	void placeSceneProp(CPropPtr sceneProp, float x, float z); // Prop pointer may be null.
	void removeSceneProp(const Vector & worldPos);
	void unload();

	//
	// Map rendering/updating:
	//
	void drawMap(const Frustum & frustum);
	void drawSingleTile(TileId tileId, float x, float z, const Color4f & tileTint);
	void updateActiveArea(const Vector & center); // `center` is usually the player's position.

	//
	// Collision/visibility tests:
	//
	bool collidesWithWallOrProp(const Aabb & bbox, const Vector & boxPos) const;
	bool tileIsVisible(const Frustum & frustum, TileId tileId, float x, float z) const;
	bool inActiveArea(const Vector & position, bool * isInsideEdge = nullptr) const;

	//
	// Get/set methods:
	//
	static bool tileIsFloor(const TileId tileId)  { return tileId == FLR || tileId == FLR2 || tileId == FLR3; }
	uint getTotalTiles() const                    { return mapTilesX * mapTilesZ; }

	void setMapColorTint(const Color4f & color)   { mapColorTint = color;    }
	const Color4f & getMapColorTint() const       { return mapColorTint;     }

	void setFadedColorTint(const Color4f & color) { fadedColorTint = color;  }
	const Color4f & getFadedColorTint() const     { return fadedColorTint;   }

	void setDrawTileBounds(const bool enable)     { drawTileBounds = enable; }
	bool getDrawTileBoundsFlag() const            { return drawTileBounds;   }

	// Get and clear the per-frame tile draw count. Use this for debug printing.
	uint getTileDrawCount();

private:

	// Copy/assign disallowed.
	TileMap(const TileMap &);
	TileMap & operator = (const TileMap &);

	struct ActiveTiles
	{
		// We can get away with `ubyte` for maps smaller than 256x256 tiles!
		static const uint PATTERN_SIZE = 11*11;
		ubyte pattern[PATTERN_SIZE][2];
		ubyte isEdge[PATTERN_SIZE];
	};

private:

	// Pointer to an external array of tile ids (not to be deleted!):
	const TileId * tiles;

	// Width height of map, in tiles:
	uint mapTilesX;
	uint mapTilesZ;

	// Currently, only floor tiles can have a prop over them,
	// but we keep one pointer for each tile nevertheless so
	// that this array can be indexed with `x + y * width`.
	CPropPtr * sceneProps;

	// Tiles must be rotated to adjust the map to our PoV.
	Matrix tileRotationOffset;

	// Set of active tiles that are drawn and tested for player collision.
	ActiveTiles activeArea;

	// Color modulated with the map's textures (AKA tint color).
	Color4f mapColorTint;
	Color4f fadedColorTint;

	// Number of tiles drawn in a frame. Cleared when `getTileDrawCount` is called.
	uint tileDrawCount;

	// `drawTileBounds` = debug draw the tile AABBs (false by default).
	bool drawTileBounds;
};

// ========================================================
// Miscellaneous helpers:
// ========================================================

// Loads a texture into memory, texture data is never freed.
// This function is usually used to create textures from static
// arrays of image data that are built into the application.
void loadPermanentTexture(Texture & texture, const ubyte * data, uint sizeBytes, ubyte textureFunction);

#endif // TILE_MAP_HPP
