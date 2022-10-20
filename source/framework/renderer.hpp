
// ================================================================================================
// -*- C++ -*-
// File: renderer.hpp
// Author: Guilherme R. Lampert
// Created on: 11/01/15
// Brief: Renderer manager and other miscellaneous rendering tools.
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

#ifndef RENDERER_HPP
#define RENDERER_HPP

// Framework stuff:
#include "common.hpp"
#include "texture.hpp"

// Maths helpers:
#include "ps2_math/math_funcs.hpp"
#include "ps2_math/vector.hpp"
#include "ps2_math/matrix.hpp"
#include "ps2_math/aabb.hpp"

// PS2DEV SDK:
#include <draw.h>
#include <graph.h>
#include <gs_psm.h>

// ========================================================
// Renderer constants:
// ========================================================

// List of built-int fonts available:
enum BuiltInFontId
{
	// Consolas font-face, several sizes:
	FONT_CONSOLAS_10,
	FONT_CONSOLAS_12,
	FONT_CONSOLAS_16,
	FONT_CONSOLAS_20,
	FONT_CONSOLAS_24,
	FONT_CONSOLAS_30,
	FONT_CONSOLAS_36,
	FONT_CONSOLAS_40,
	FONT_CONSOLAS_48,

	// Built-in font count. Internal use.
	FONT_COUNT
};

// IDENTITY_MATRIX constant:
extern const Matrix IDENTITY_MATRIX;

// ========================================================
// struct DrawVertex:
// ========================================================

struct ATTRIBUTE_ALIGNED(16) DrawVertex
{
	Vector position; // Only XYZ are used. W always = 1
	Vector texCoord; // Only XY used. Z always = 0 and W always = 1
	Vector color;    // XYZW are used to store the RGBA color
};

// ASM optimized packing of a DrawVertex:
inline void packDrawVertex(DrawVertex & dv, const Vector & position, const Vector & texCoord, const Vector & color)
{
	asm volatile (
		"lqc2    vf4, 0x0(%1)  \n\t" // vf4 = position
		"lqc2    vf5, 0x0(%2)  \n\t" // vf5 = texCoord
		"lqc2    vf6, 0x0(%3)  \n\t" // vf6 = color
		"vmove.w vf4, vf0      \n\t" // vf4.w = 1.0 (set position.w to 1)
		"sqc2    vf6, 0x20(%0) \n\t" // dv.color    = vf6
		"sqc2    vf5, 0x10(%0) \n\t" // dv.texCoord = vf5
		"sqc2    vf4, 0x00(%0) \n\t" // dv.position = vf4
		: : "r" (&dv), "r" (&position), "r" (&texCoord), "r" (&color)
	);
}

// ========================================================
// class RenderPacket:
// ========================================================

class ATTRIBUTE_ALIGNED(64) RenderPacket
{
public:

	enum Type
	{
		NORMAL, // EE RAM
		UCAB,   // Uncached Accelerated memory (UCAB)
		SPR     // Scratch Pad memory (SPR)
	};

	// Create an empty packet with a null quadword buffer.
	RenderPacket();

	// Construct a new packet for use; size in quadwords.
	RenderPacket(uint quadwords, Type packetType);

	// Frees memory if this is not a Scratch Pad packet.
	~RenderPacket();

	// Allocate a new packet for use; size in quadwords.
	void init(uint quadwords, Type packetType);

	// Access the base quadword pointer.
	qword_t * getQwordPtr() const { return qwordBuffer; }

	// Number of quadwords in this packet.
	uint getQwordCount() const { return qwordCount; }

	// Given an incremented pointer to `qwordBuffer`, compute the displacement in qword_t's.
	uint getDisplacement(const qword_t * q) const { return scast<uint>(q - qwordBuffer); }

private:

	// Copy/assign disallowed.
	RenderPacket(const RenderPacket &);
	RenderPacket & operator = (const RenderPacket &);

	// Data:
	qword_t * qwordBuffer; // Aligned to a cache line (`memAlloc(align=64)`)
	uint      qwordCount;  // Size of `qwordBuffer` in qword_t's
	Type      type;
};

// ========================================================
// class Renderer:
// ========================================================

class Renderer
{
public:

	 Renderer();
	~Renderer();

	//
	// Initialization, video setup and rendering basics:
	//

	// Initializes video and screen plus the GS/GIF DMA channels.
	bool initVideoGraphics(int scrW, int scrH, int vidMode, int fbPsm, int zPsm, bool interlaced);

	// Start a new render frame.
	void beginFrame();

	// Finish rendering and waits for V-sync.
	void endFrame();

	// Entering 3D mode is mandatory before rendering 3D primitives/polygons.
	void begin3d();
	void end3d();

	// Entering 2D mode is mandatory before rendering 2D primitives or text.
	void begin2d(const Texture * tex = nullptr);
	void end2d();

	// Clears the screen framebuffer to the current clear color. Black is the default.
	void clearScreen();

	// Flushes batched geometry. Should be called BEFORE a texture switch,
	// except for the first and last ones.
	void flushPipeline();

	// This is only called when we need to abort the current
	// frame rendering to display an error message.
	void resetFrameStates();

	//
	// Render states / accessors:
	//

	// Query initialization states:
	bool isRendererInitialized() const;
	bool isVideoInitialized()    const;

	// Screen dimensions in pixels:
	uint getScreenWidth()  const;
	uint getScreenHeight() const;
	float getAspectRatio() const;

	// Get/set the Model View Projection matrices:
	const Matrix & getViewProjMatrix() const;
	const Matrix & getModelMatrix()    const;
	void setViewProjMatrix(const Matrix & vp);
	void setModelMatrix(const Matrix & m);

	// Get/set the eye (camera) position in world space:
	const Vector & getEyePosition() const;
	void setEyePosition(const Vector & eye);

	// Enable or disable writes to the depth buffer. Necessary for particle rendering.
	void disableDepthWriting();
	void enableDepthWriting();

	// VRam address for the user defined texture.
	// Addresses before this are filled with framebuffer(s) and z-buffer.
	uint getVRamUserTextureStart() const;

	// Applies a texture for all subsequent draw calls.
	// Texture must remain alive at least until the end of the current frame!
	void setTexture(const Texture & tex);

	// Set the color used to clear the screen framebuffer with `clearScreen()`. Default = black.
	void setClearScreenColor(ubyte r, ubyte g, ubyte b);

	// Set rendering attributes for 3D primitives:
	void setPrimShading(int shading);        // PRIM_SHADE_GOURAUD or PRIM_SHADE_FLAT. Default = PRIM_SHADE_GOURAUD
	void setPrimTopology(int topology);      // PRIM_TRIANGLE, PRIM_LINE, PRIM_POINT, etc. Default = PRIM_TRIANGLE
	void setPrimTextureMapping(bool enable); // Default = on
	void setPrimFogging(bool enable);        // Default = off
	void setPrimBlending(bool enable);       // Default = off
	void setPrimAntialiasing(bool enable);   // Default = off
	void setPrimBaseColor(ubyte r, ubyte g, ubyte b, ubyte a, float q); // Default = (255,255,255,255,1.0)

	//
	// 3D / mesh rendering:
	//

	// Draws a set of unindexed vertexes that in turn compose a set of triangle primitives.
	void drawUnindexedTriangles(const DrawVertex * verts, uint vertCount);

	// Draws a set of indexed vertexes that in turn compose a set
	// of triangle primitives. Index buffer and verts buffer must not overlap!
	void drawIndexedTriangles(const uint16 * indexes, uint indexCount,
	                          const DrawVertex * verts, uint vertCount);

	// Draws a set of indexed vertexes using raw values not packed into DrawVertexes.
	// Same behavior of the other `drawIndexed*` methods.
	void drawIndexedTriangles(const uint16 * indexes, uint indexCount,
	                          const float (*positions)[3], uint positionCount,
	                          const float (*texCoords)[2], uint texCoordCount,
	                          const Color4f & baseColor);

	// Same as `drawIndexedTriangles` but without performing back-face culling.
	// Off-screen triangle clipping is still done! This is used by the particle emitters.
	void drawIndexedTrianglesUnculled(const uint16 * indexes, uint indexCount,
	                                  const DrawVertex * verts, uint vertCount);

	//
	// Debug/line drawing:
	//

	// Draws a 3D line, transformed by the current model matrix.
	void drawLine(const Vector & from, const Vector & to, const Color4f & color);

	// Draws a 3D set of lines to represent an AABB. Transformed by the current model matrix.
	void drawAabb(const Aabb & aabb, const Color4f & color);

	//
	// Miscellaneous 2D primitives:
	//

	// Rectangle drawing (must be in 2D drawing mode!):
	void drawRectFilled(const Rect4i & rect, Color4b color);
	void drawRectOutline(const Rect4i & rect, Color4b color);

	// Draws a 2D rectangle applying the current texture to it.
	void drawRectTextured(const Rect4i & rect, Color4b color);

	// Draws a frames-per-second count at the top right corner of the screen.
	void drawFpsCounter();

	// Draws a few stats about the current frame at the lower left corner of the screen.
	void drawFrameStats();

	//
	// Built-in fronts and text rendering:
	//

	// Load/unload all fonts:
	void loadAllBuiltInFonts();
	void unloadAllBuiltInFonts();

	// Load/unload specific:
	bool loadBuiltInFont(BuiltInFontId fontId);
	void unloadBuiltInFont(BuiltInFontId fontId);

	// Check if built-in font is loaded.
	bool isBuiltInFontLoaded(BuiltInFontId fontId);

	// Draw a single char using a built-in font. Loads the font if not loaded yet.
	// Returns the number of units that `pos.x` was advanced by. This is the effective
	// width of the rendered char. Zero is returned if no char was rendered.
	// Text origin is the top-left corner of the screen, measured in pixels.
	float drawChar(Vec2f & pos, Color4b color, BuiltInFontId fontId, int c);

	// Draws a text string using a built-in font. Loads the font if not loaded yet.
	// Text origin is the top-left corner of the screen, measured in pixels.
	// Handles tabs and newlines ('\t' and '\n').
	void drawText(Vec2f & pos, Color4b color, BuiltInFontId fontId, const char * str);

	// Query the length in pixels a string would occupy if it were to be renderer with `drawText()`.
	// On completion, `advance` is set to the amount `pos` would be advanced in the x and y for the
	// given `drawText()` call with the same string. This method does not draw any text.
	void getTextLength(const char * str, BuiltInFontId fontId, Vec2f & advance);

	// Scale applied to rendered glyphs. 1.0 by default (no scale).
	void  setGlobalTextScale(const float scale);
	float getGlobalTextScale() const;

private:

	// Copy/assign disallowed.
	Renderer(const Renderer &);
	Renderer & operator = (const Renderer &);

	// Built-in font helpers:
	void  fntLoadGlyphs(BuiltInFontId fontId);
	void  fntUnloadGlyphs(BuiltInFontId fontId);
	bool  fntAllocateAtlasSpace(BuiltInFontId fontId);
	float fntGetWhiteSpaceWidth(BuiltInFontId fontId) const;
	void  fntDecompressGlyphs(BuiltInFontId fontId);

	// Misc internal helpers:
	void initGsBuffers(int scrW, int scrH, int vidMode, int fbPsm, int zPsm, bool interlaced);
	void initDrawingEnvironment();
	void flipBuffers(framebuffer_t & fb);
	void setTextureBufferSampling();

private:

	// The Renderer is an implicit singleton. Only one instance is allowed to exist.
	// Trying to create more than one Renderer will result in a runtime fatal error.
	static bool rendererInitialized;

	// Set to true only if we've managed to successfully
	// initialize the video and graphics subsystems.
	static bool videoInitialized;

	// We draw 3D using two packets to implement a double-buffer
	// scheme and minimize waiting on DMA data transfers.
	zbuffer_t      zBuffer;
	framebuffer_t  framebuffers[2];
	RenderPacket   framePackets[2];
	RenderPacket   flipFbPacket;
	RenderPacket * currentFramePacket;
	qword_t      * currentFrameQwPtr;
	qword_t      * dmaTagDraw2d;
	Texture      * currentTex;
	uint           frameIndex;

	// Texture mapping aux data:
	RenderPacket textureUploadPacket[2];
	int vramUserTextureStart;

	// 3D primitive/geometry attributes:
	prim_t  primDesc;
	color_t primColor;
	color_t screenColor;

	// Debug counters (per frame):
	uint drawCount2d; // Number of 2D draw calls
	uint drawCount3d; // Number of 3D draw calls
	uint trisCount3d; // Number of 3D triangles sent to the GS
	uint texSwitches; // Number of Texture switches
	uint pipeFlushes; // Number of `flushPipeline()` calls

	// Current render matrices for 3D geometry transformation:
	Matrix modelMatrix;
	Matrix invModelMatrix;
	Matrix vpMatrix;
	Matrix mvpMatrix;
	Vector eyePosition;
	Vector eyePosModelSpace;

	// Optional default scale for text glyphs. Initially no scale (1.0):
	float globalTextScale;

	// Local atlas used by the built-in font glyphs:
	TextureAtlas texAtlas;

	// 2D rendering, including text, can only take place
	// between `begin2d()` and `end2d()` calls.
	bool inMode2d;

	// 3D rendering can only take place between `begin3d()` and `end3d()` calls.
	bool inMode3d;

	// Average multiple frames together to smooth changes out a bit.
	struct FpsCounter
	{
		static const uint MAX_FRAMES = 4;
		uint previousTimes[MAX_FRAMES];
		uint previousTime;
		uint fpsCount;
		uint index;
	} fps;
};

// ========================================================
// Renderer global instance:
// ========================================================

extern Renderer gRenderer;

#endif // RENDERER_HPP
