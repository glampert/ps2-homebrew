
// ================================================================================================
// -*- C++ -*-
// File: texture.cpp
// Author: Guilherme R. Lampert
// Created on: 19/01/15
// Brief: Basic Texture and Texture Atlas types.
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

#include "texture.hpp"
#include "renderer.hpp"

// PS2DEV SDK:
#include <dma.h>
#include <dma_tags.h>

// ================================================================================================
// Texture implementation:
// ================================================================================================

// ========================================================
// Texture::Texture():
// ========================================================

Texture::Texture()
	: texData(nullptr)
	, texHeight(0)
	, texBuf()
	, texLod()
	, texClut()
{ }

// ========================================================
// Texture::initEmpty():
// ========================================================

bool Texture::initEmpty(const ubyte comps, const uint w, const uint h, const uint psm,
                        const ubyte func, const lod_t * lod, const clutbuffer_t * clut)
{
	ps2assert(w != 0);
	ps2assert(h != 0);

	if (w > MAX_SIZE)
	{
		logComment("Texture width (%u) exceeds max size (%u)!", w, MAX_SIZE);
	}
	if (h > MAX_SIZE)
	{
		logComment("Texture height (%u) exceeds max size (%u)!", h, MAX_SIZE);
	}

	texHeight              = h;
	texBuf.width           = w;
	texBuf.psm             = psm;
	texBuf.info.width      = draw_log2(w);
	texBuf.info.height     = draw_log2(h);
	texBuf.info.components = comps;
	texBuf.info.function   = func;

	//
	// All textures must share the same VRam space.
	// What this means is that we only have enough VRam
	// left for a single texture at a time, so every texture
	// must be copied to VRam on-the-fly, prior to any use.
	//
	texBuf.address = gRenderer.getVRamUserTextureStart();

	if (lod != nullptr)
	{
		texLod = *lod;
	}
	else
	{
		// Use defaults:
		texLod.calculation = LOD_USE_K;
		texLod.max_level   = 0;
		texLod.mag_filter  = LOD_MAG_NEAREST;
		texLod.min_filter  = LOD_MIN_NEAREST;
		texLod.l           = 0;
		texLod.k           = 0.0f;
	}

	if (clut != nullptr)
	{
		texClut = *clut;
	}
	else
	{
		// Use defaults:
		texClut.address      = 0;
		texClut.psm          = 0;
		texClut.storage_mode = CLUT_STORAGE_MODE1;
		texClut.start        = 0;
		texClut.load_method  = CLUT_NO_LOAD;
	}

	return true;
}

// ========================================================
// Texture::initFromMemory():
// ========================================================

bool Texture::initFromMemory(const ubyte * data, const ubyte comps, const uint w, const uint h,
                             const uint psm, const ubyte func, const lod_t * lod, const clutbuffer_t * clut)
{
	if (!initEmpty(comps, w, h, psm, func, lod, clut))
	{
		return false;
	}

	texData = data;
	return true;
}

// ================================================================================================
// TextureAtlas implementation:
// ================================================================================================

// ========================================================
// TextureAtlas::TextureAtlas():
// ========================================================

TextureAtlas::TextureAtlas()
	: texture()
	, nodes()
	, pixels(nullptr)
	, usedPixels(0)
	, width(0)
	, height(0)
	, bytesPerPixel(0)
	, initialized(false)
{
}

// ========================================================
// TextureAtlas::~TextureAtlas():
// ========================================================

TextureAtlas::~TextureAtlas()
{
	// Was allocated with `memAlloc()`.
	memFree(MEM_TAG_TEXTURE, pixels);
}

// ========================================================
// TextureAtlas::init():
// ========================================================

bool TextureAtlas::init(const int w, const int h, const int bpp, const ubyte fillVal)
{
	ps2assert(!initialized && "Already initialized!");
	ps2assert(w != 0 && h != 0 && bpp != 0);

	width  = w;
	height = h;
	bytesPerPixel = bpp;

	if (bytesPerPixel != 4)
	{
		logError("Currently TextureAtlas only supports 4 bpp textures!");
		return false;
	}

	// We want a one pixel border around the whole atlas
	// to avoid any artifacts when sampling the texture.
	const Vec3i tmp = { 1, 1, width - 2 };
	nodes.pushBack(tmp);

	// Allocate a cleared image:
	pixels = memAlloc<ubyte>(MEM_TAG_TEXTURE, (width * height * bytesPerPixel), 128);
	memset(pixels, fillVal, (width * height * bytesPerPixel));

	usedPixels  = 0;
	initialized = true;

	// Create a texture with linear filter sampling. No mipmaps.
	lod_t lod;
	lod.calculation = LOD_USE_K;
	lod.max_level   = 0;
	lod.mag_filter  = LOD_MAG_LINEAR;
	lod.min_filter  = LOD_MIN_LINEAR;
	lod.l           = 0;
	lod.k           = 0.0f;
	return texture.initFromMemory(pixels, TEXTURE_COMPONENTS_RGBA, width, height,
			GS_PSM_32, TEXTURE_FUNCTION_MODULATE, &lod, nullptr);
}

// ========================================================
// TextureAtlas::reserveMemForRegions():
// ========================================================

void TextureAtlas::reserveMemForRegions(const uint numRegions)
{
	nodes.reserve(nodes.size() + numRegions);
}

// ========================================================
// TextureAtlas::clear():
// ========================================================

void TextureAtlas::clear(const ubyte fillVal)
{
	nodes.clear();

	if (pixels != nullptr)
	{
		memset(pixels, fillVal, width * height * bytesPerPixel);
	}

	usedPixels = 0;
}

// ========================================================
// TextureAtlas::allocRegion():
// ========================================================

Rect4i TextureAtlas::allocRegion(const int w, const int h)
{
	int y, bestWidth, bestHeight, bestIndex;
	Vec3i * node, * prev;
	Rect4i region = { 0, 0, w, h };
	unsigned int i;

	bestWidth  = INT_MAX;
	bestHeight = INT_MAX;
	bestIndex  = -1;

	for (i = 0; i < nodes.size(); ++i)
	{
		y = fit(i, w, h);
		if (y >= 0)
		{
			node = &nodes[i];
			if (((y + h) < bestHeight) || (((y + h) == bestHeight) && (node->z < bestWidth)))
			{
				bestHeight = y + h;
				bestIndex  = i;
				bestWidth  = node->z;
				region.x   = node->x;
				region.y   = y;
			}
		}
	}

	if (bestIndex == -1) // No more room!
	{
		region.x      = -1;
		region.y      = -1;
		region.width  =  0;
		region.height =  0;
		return region;
	}

	Vec3i tempNode;
	node = &tempNode;
	node->x = region.x;
	node->y = region.y + h;
	node->z = w;
	nodes.insert(bestIndex, *node);

	for (i = bestIndex + 1; i < nodes.size(); ++i)
	{
		node = &nodes[i];
		prev = &nodes[i - 1];

		if (node->x < (prev->x + prev->z))
		{
			int shrink = (prev->x + prev->z - node->x);
			node->x += shrink;
			node->z -= shrink;

			if (node->z <= 0)
			{
				nodes.erase(i);
				--i;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	merge();
	usedPixels += (w * h);
	return region;
}

// ========================================================
// TextureAtlas::setRegion():
// ========================================================

void TextureAtlas::setRegion(const int x, const int y, const int w, const int h, const ubyte * newData, const int stride)
{
	// Validate the atlas object:
	ps2assert(pixels != nullptr && "Atlas has no image data assigned to it!");

	// Validate input parameters:
	ps2assert(x > 0 && y > 0);
	ps2assert(x < (width - 1));
	ps2assert((x + w) <= (width - 1));
	ps2assert(y < (height - 1));
	ps2assert((y + h) <= (height - 1));
	ps2assert(newData != nullptr);

	// 'stride' is the length in bytes of a row of pixels.
	for (int i = 0; i < h; ++i)
	{
		memcpy(/* dest     = */ (pixels + ((y + i) * width + x) * bytesPerPixel),
		       /* source   = */ (newData + (i * stride)),
		       /* numBytes = */ (w * bytesPerPixel));
	}
}

// ========================================================
// TextureAtlas::setRegion():
// ========================================================

void TextureAtlas::setRegion(const int x, const int y, const int w, const int h, const uint32 color)
{
	// Validate the atlas object:
	ps2assert(pixels != nullptr && "Atlas has no image data assigned to it!");

	// Validate input parameters:
	ps2assert(x > 0 && y > 0);
	ps2assert(x < (width - 1));
	ps2assert((x + w) <= (width - 1));
	ps2assert(y < (height - 1));
	ps2assert((y + h) <= (height - 1));

	uint32 * restrict rgbaPixels = rcast<uint32 *>(pixels);
	for (int yi = y; yi < (y + h); ++yi)
	{
		for (int xi = x; xi < (x + w); ++xi)
		{
			rgbaPixels[xi + yi * width] = color;
		}
	}
}

// ========================================================
// TextureAtlas::fit():
// ========================================================

int TextureAtlas::fit(const int index, const int w, const int h)
{
	ps2assert(!nodes.isEmpty());

	const Vec3i * node;
	int x, y, widthLeft;
	int i;

	node = &nodes[index];
	x = node->x;
	y = node->y;
	widthLeft = w;
	i = index;

	if ((x + w) > (width - 1))
	{
		return -1;
	}

	y = node->y;

	while (widthLeft > 0)
	{
		node = &nodes[i];

		if (node->y > y)
		{
			y = node->y;
		}
		if ((y + h) > (height - 1))
		{
			return -1;
		}

		widthLeft -= node->z;
		++i;
	}

	return y;
}

// ========================================================
// TextureAtlas::merge():
// ========================================================

void TextureAtlas::merge()
{
	if (nodes.isEmpty())
	{
		return;
	}

	Vec3i * node, * next;
	for (uint i = 0; i < nodes.size() - 1; ++i)
	{
		node = &nodes[i];
		next = &nodes[i + 1];
		if (node->y == next->y)
		{
			node->z += next->z;
			nodes.erase(i + 1);
			--i;

			if (nodes.isEmpty())
			{
				break;
			}
		}
	}
}

// ================================================================================================
// Image loading (provided by STBI):
// ================================================================================================

namespace
{

#define STB_IMAGE_IMPLEMENTATION

// Only need these for now:
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_TGA

// Don't need HDR nor loading from file.
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_STDIO
#define STBI_NO_SIMD

// Use our custom assert:
#define STBI_ASSERT ps2assert

// Use 128-aligned allocations.
// This will force STBI to use 128bytes aligned allocations for fresh allocs:
#define STBI_MALLOC(size)          memTagMalloc(MEM_TAG_TEXTURE, (size), 128)
#define STBI_REALLOC(ptr, newSize) memTagRealloc(MEM_TAG_TEXTURE, (ptr), (newSize))
#define STBI_FREE(ptr)             memTagFree(MEM_TAG_TEXTURE, (ptr))

// STBI header-only library:
#include "third_party/stb_image.h"

} // namespace {}

// ========================================================
// loadImageFromMemory():
// ========================================================

bool loadImageFromMemory(const ubyte * data, const uint sizeBytes, ImageData & image, const bool forceRgba)
{
	// First clear the output:
	memset(&image, 0, sizeof(image));

	if (data == nullptr || sizeBytes == 0)
	{
		logError("Invalid data buffer!");
		return false;
	}

	int w, h, c;
	stbi_uc * pixels = stbi_load_from_memory(data, sizeBytes, &w, &h, &c, (forceRgba ? 4 : 0));

	if (pixels == nullptr)
	{
		logError("\'stbi_load()\' failed with error: %s", stbi_failure_reason());
		return false;
	}

	// Non power-of-2 dimensions check:
	if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0)
	{
		// NOTE: This should be improved in the future.
		// We should define a configuration parameter that enables
		// downscaling a non-PoT image.
		//
		logWarning("Image doesn't have power-of-2 dimensions!");
		// Allow it to continue...
	}

	// Set image and we are done:
	image.pixels = pixels;
	image.width  = w;
	image.height = h;
	image.comps  = (forceRgba ? 4 : c);

	logComment("Loaded image from memory! %ux%u (%u components)",
			image.width, image.height, image.comps);

	return true;
}

// ========================================================
// imageCleanup():
// ========================================================

void imageCleanup(ImageData & image)
{
	if (image.pixels != nullptr)
	{
		stbi_image_free(image.pixels);
	}

	memset(&image, 0, sizeof(image));
}
