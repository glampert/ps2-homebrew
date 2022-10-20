
// ================================================================================================
// -*- C++ -*-
// File: texture.hpp
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

#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "common.hpp"
#include "array.hpp"

// PS2DEV SDK:
#include <draw.h>
#include <graph.h>
#include <gs_psm.h>

// ========================================================
// class Texture:
// ========================================================

class Texture
{
public:

	// Maximum square size of a texture in RGBA pixels.
	// I.e.: 256x256x4.
	static const uint MAX_SIZE = 256;

	Texture();

	// Initializes an empty texture. Pre-allocates VRam memory.
	// `lod` and `clut` are optional and may be null.
	bool initEmpty(ubyte comps, uint w, uint h, uint psm, ubyte func,
	               const lod_t * lod = nullptr, const clutbuffer_t * clut = nullptr);

	// Initializes the texture from a memory hunk containing the pixel data
	// (possibly originated from a file). The Texture class will keep references
	// to offsets into this buffer, so the memory should remain valid until
	// the class itself is disposed. `lod` and `clut` are optional and may be null.
	bool initFromMemory(const ubyte * data, ubyte comps, uint w, uint h, uint psm, ubyte func,
	                    const lod_t * lod = nullptr, const clutbuffer_t * clut = nullptr);

	// Access pixel data pointer (just a weak reference!):
	const ubyte * getPixels() const      { return texData;   }
	void setPixels(const ubyte * pixels) { texData = pixels; }

	// Tables access:
	texbuffer_t  & getTexBuffer() { return texBuf;  }
	clutbuffer_t & getTexClut()   { return texClut; }
	lod_t        & getTexLod()    { return texLod;  }

	// Misc accessors:
	uint getWidth()       const { return texBuf.width; }
	uint getHeight()      const { return texHeight;    }
	uint getPixelFormat() const { return texBuf.psm;   }

private:

	// Copy/assign disallowed.
	Texture(const Texture &);
	Texture & operator = (const Texture &);

	const ubyte * texData; // Pointer to external data. Never freed.
	uint          texHeight;
	texbuffer_t   texBuf;
	lod_t         texLod;
	clutbuffer_t  texClut;
};

// ========================================================
// class TextureAtlas:
// ========================================================

class TextureAtlas
{
public:

	 TextureAtlas();
	~TextureAtlas();

	// Atlas management:
	bool init(int w, int h, int bpp, ubyte fillVal);
	void reserveMemForRegions(uint numRegions);
	void clear(ubyte fillVal);

	// Region allocation:
	Rect4i allocRegion(int w, int h);
	void setRegion(int x, int y, int w, int h, uint32 color); // Assumes `color` and internal format are RGBA!
	void setRegion(int x, int y, int w, int h, const ubyte * newData, int stride);

	// Get a reference to the underlaying texture object.
	Texture & getTexture() { return texture; }

	// Other accessors:
	bool  isInitialized()     const { return initialized;   }
	const ubyte * getPixels() const { return pixels;        }
	uint getUsedPixelCount()  const { return usedPixels;    }
	uint getWidth()           const { return width;         }
	uint getHeight()          const { return height;        }
	uint getBpp()             const { return bytesPerPixel; }

private:

	// Copy/assign disallowed.
	TextureAtlas(const TextureAtlas &);
	TextureAtlas & operator = (const TextureAtlas &);

	// Internal helpers:
	int fit(int index, int w, int h);
	void merge();

	// Texture object:
	Texture texture;

	// "nodes" of the atlas (each sub-region):
	Array<Vec3i> nodes;

	// System memory copy of the texture data:
	ubyte * pixels;        // Atlas texture pixels.
	int     usedPixels;    // Allocated surface size in pixels.
	int     width;         // Width of the texture in pixels.
	int     height;        // Height of the texture in pixels.
	int     bytesPerPixel; // Size of a single pixels in bytes.
	bool    initialized;   // Simple initialization flag, to avoid duplicates.
};

// ========================================================
// struct Image & helper functions:
// ========================================================

struct ImageData
{
	ubyte * pixels;
	uint    width;
	uint    height;
	uint    comps;
};

// Loads a JPG, TGA or PNG from a memory buffer with the file contents.
// If `forceRgba` is true, output `image.comps` will always be 4.
bool loadImageFromMemory(const ubyte * data, uint sizeBytes, ImageData & image, bool forceRgba);

// Frees image data. Should be called on every ImageData instance when it gets disposed.
void imageCleanup(ImageData & image);

#endif // TEXTURE_HPP
