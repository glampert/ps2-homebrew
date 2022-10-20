
// ================================================================================================
// -*- C++ -*-
// File: renderer.cpp
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

#include "renderer.hpp"

// C/C++ libraries:
#include <cctype>
#include <math.h>

// PS2DEV SDK:
#include <dma.h>
#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_gp.h>

// ================================================================================================
// Helper constants and local macros:
// ================================================================================================

//
// Hardware constants:
//
enum
{
	// 16 Kilobytes (16384 bytes).
	SCRATCH_PAD_SIZE_BYTES = 0x4000,

	// 1024 128bit quadwords.
	SCRATCH_PAD_SIZE_QWORDS = SCRATCH_PAD_SIZE_BYTES / sizeof(qword_t),

	// The Scratch Pad (SPR) R/W memory is at a fixed address.
	SCRATCH_PAD_ADDRESS = 0x70000000,

	// ORing a pointer with this mask sets it to Uncached Accelerated (UCAB) space.
	UCAB_MEM_MASK = 0x30000000
};

//
// DMA GIF tag helpers:
//

#define BEGIN_DMA_TAG(qwordPtr) \
	qword_t * _dma_tag_ = (qwordPtr)++

#define END_DMA_TAG(qwordPtr) \
	DMATAG_CNT(_dma_tag_, (qwordPtr) - _dma_tag_ - 1, 0, 0, 0); \
	_dma_tag_ = nullptr

#define BEGIN_DMA_TAG_NAMED(tagName, qwordPtr) \
	tagName = (qwordPtr)++

#define END_DMA_TAG_NAMED(tagName, qwordPtr) \
	DMATAG_CNT((tagName), (qwordPtr) - (tagName) - 1, 0, 0, 0); \
	(tagName) = nullptr

#define END_DMA_TAG_AND_CHAIN(qwordPtr) \
	DMATAG_END(_dma_tag_, (qwordPtr) - _dma_tag_ - 1, 0, 0, 0); \
	_dma_tag_ = nullptr

// ================================================================================================
// RenderPacket implementation:
// ================================================================================================

// ========================================================
// RenderPacket::RenderPacket():
// ========================================================

RenderPacket::RenderPacket()
	: qwordBuffer(nullptr)
	, qwordCount(0)
	, type(NORMAL)
{
	// Leave uninitialized.
}

// ========================================================
// RenderPacket::RenderPacket():
// ========================================================

RenderPacket::RenderPacket(const uint quadwords, const Type packetType)
	: qwordBuffer(nullptr)
	, qwordCount(0)
	, type(NORMAL)
{
	init(quadwords, packetType);
}

// ========================================================
// RenderPacket::init():
// ========================================================

void RenderPacket::init(const uint quadwords, const Type packetType)
{
	ps2assert(qwordBuffer == nullptr && "RenderPacket already initialized!");
	ps2assert(quadwords != 0);

	type = packetType;

	if (type == SPR) // Use Scratch Pad memory:
	{
		if (quadwords > SCRATCH_PAD_SIZE_QWORDS)
		{
			fatalError("Scratch Pad memory can only fit up to %u quadwords!", SCRATCH_PAD_SIZE_QWORDS);
		}

		rcast<uint32 *>(qwordBuffer) = SCRATCH_PAD_ADDRESS;
		qwordCount = SCRATCH_PAD_SIZE_QWORDS;
	}
	else // Allocate from global heap:
	{
		ps2assert(type == NORMAL || type == UCAB);

		// Allocate the data area in bytes aligned to cache line:
		qwordBuffer = memAlloc<qword_t>(MEM_TAG_RENDERER, quadwords, 64);
		if (qwordBuffer == nullptr)
		{
			fatalError("Failed to allocate qword buffer for a RenderPacket!");
		}

		qwordCount = quadwords;
	}

	// Optionally set the pointer attribute to UCAB space:
	if (type == UCAB)
	{
		uint32 ucabAddr = rcast<uint32>(qwordBuffer);
		ucabAddr |= UCAB_MEM_MASK;

		rcast<uint32 *>(qwordBuffer) = ucabAddr;
	}
}

// ========================================================
// RenderPacket::~RenderPacket():
// ========================================================

RenderPacket::~RenderPacket()
{
	if (qwordBuffer == nullptr)
	{
		return;
	}

	if (type == NORMAL)
	{
		memFree(MEM_TAG_RENDERER, qwordBuffer); // Was allocated with `memAlloc()`.
	}
	else if (type == UCAB)
	{
		uint32 ucabAddr = rcast<uint32>(qwordBuffer);
		ucabAddr ^= UCAB_MEM_MASK;

		rcast<uint32 *>(qwordBuffer) = ucabAddr;

		memFree(MEM_TAG_RENDERER, qwordBuffer);
	}
	else
	{
		// Scratch Pad memory can't be freed
		// since it is a fixed hardware buffer.
	}

	qwordBuffer = nullptr;
}

// ================================================================================================
// Renderer implementation:
// ================================================================================================

uint gVRamUsedBytes = 0;
static inline int vramAlloc(const int width, const int height, const int psm, const int alignment)
{
	const int addr = graph_vram_allocate(width, height, psm, alignment);
	if (addr < 0)
	{
		fatalError("Failed to allocate VRam space!");
	}

	const int size = graph_vram_size(width, height, psm, alignment);
	gVRamUsedBytes += size * 4; // Size is in 32bit VRam words.

	return addr;
}

// Static members of Renderer:
bool Renderer::rendererInitialized = false;
bool Renderer::videoInitialized    = false;

// ========================================================
// Renderer::Renderer():
// ========================================================

Renderer::Renderer()
	: currentFramePacket(nullptr)
	, currentFrameQwPtr(nullptr)
	, dmaTagDraw2d(nullptr)
	, currentTex(nullptr)
	, frameIndex(0)
	, vramUserTextureStart(0)
	, drawCount2d(0)
	, drawCount3d(0)
	, trisCount3d(0)
	, texSwitches(0)
	, pipeFlushes(0)
	, globalTextScale(1.0f)
	, inMode2d(false)
	, inMode3d(false)
{
	if (rendererInitialized)
	{
		fatalError("Renderer is singleton! Only one instance is permitted!");
	}

	rendererInitialized = true;
}

// ========================================================
// Renderer::~Renderer():
// ========================================================

Renderer::~Renderer()
{
	rendererInitialized = false;
	videoInitialized    = false;
}

// ========================================================
// Renderer::initVideoGraphics():
// ========================================================

bool Renderer::initVideoGraphics(const int scrW, const int scrH, const int vidMode,
                                 const int fbPsm, const int zPsm, const bool interlaced)
{
	if (videoInitialized)
	{
		fatalError("Duplicate video initialization! Shutdown first!");
	}

	logComment("Entering Renderer::initVideoGraphics() ...");

	// Reset the VRam pointer to the first address, just to be sure.
	graph_vram_clear();
	gVRamUsedBytes = 0;

	// Main renderer and video initialization:
	initGsBuffers(scrW, scrH, vidMode, fbPsm, zPsm, interlaced);
	initDrawingEnvironment();

	// Create double buffer render packets:
	//
	// `FRAME_PACKET_SIZE` is the number of quadwords per render packet
	// in our double buffer. We have two of them, so this adds up to
	// ~2 Megabytes of memory.
	//
	// NOTE: Currently no overflow checking is done, so drawing a
	// very big mesh could potentially crash the application!
	//
	const uint FRAME_PACKET_SIZE = 65535;
	framePackets[0].init(FRAME_PACKET_SIZE, RenderPacket::NORMAL);
	framePackets[1].init(FRAME_PACKET_SIZE, RenderPacket::NORMAL);

	// Extra packets for texture uploads:
	textureUploadPacket[0].init(128, RenderPacket::NORMAL);
	textureUploadPacket[1].init(128, RenderPacket::NORMAL);

	// One small UCAB packet used to send the flip buffer command:
	flipFbPacket.init(8, RenderPacket::UCAB);

	// Reset these, to be sure...
	currentFramePacket = nullptr;
	currentFrameQwPtr  = nullptr;
	currentTex         = nullptr;
	frameIndex         = 0;

	//
	// Init other aux data and default render states:
	//
	primDesc.type         = PRIM_TRIANGLE;
	primDesc.shading      = PRIM_SHADE_GOURAUD;
	primDesc.mapping      = DRAW_ENABLE;
	primDesc.fogging      = DRAW_DISABLE;
	primDesc.blending     = DRAW_DISABLE;
	primDesc.antialiasing = DRAW_DISABLE;
	primDesc.mapping_type = PRIM_MAP_ST;
	primDesc.colorfix     = PRIM_UNFIXED;

	primColor.r = 255;
	primColor.g = 255;
	primColor.b = 255;
	primColor.a = 255;
	primColor.q = 1.0f;

	screenColor.r = 0;
	screenColor.g = 0;
	screenColor.b = 0;
	screenColor.a = 255;
	screenColor.q = 1.0f;

	modelMatrix.makeIdentity();
	invModelMatrix.makeIdentity();
	vpMatrix.makeIdentity();
	mvpMatrix.makeIdentity();

	eyePosition = Vector(0.0f, 0.0f, 0.0f, 1.0f);
	eyePosModelSpace = eyePosition;

	inMode2d = false;
	inMode3d = false;

	memset(&fps, 0, sizeof(fps));

	// This only really affects 2D drawing. It simply sets a flag
	// inside the library, which gets tested by the 2D drawing routines.
	// So we can set this once on initialization and forget.
	draw_enable_blending();

	// PS2DEV SDK doesn't offers much error checking,
	// so we will just assume everything when well...
	videoInitialized = true;

	// All good.
	logComment("Renderer::initVideoGraphics() completed!");
	return videoInitialized;
}

// ========================================================
// Renderer::initGsBuffers():
// ========================================================

void Renderer::initGsBuffers(const int scrW, const int scrH, int vidMode,
                             const int fbPsm, const int zPsm, const bool interlaced)
{
	// Init GIF DMA channel:
	dma_channel_initialize(DMA_CHANNEL_GIF, nullptr, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	// FB 0:
	framebuffers[0].width   = scrW;
	framebuffers[0].height  = scrH;
	framebuffers[0].mask    = 0;
	framebuffers[0].psm     = fbPsm;
	framebuffers[0].address = vramAlloc(framebuffers[0].width,
		framebuffers[0].height, framebuffers[0].psm, GRAPH_ALIGN_PAGE);

	// FB 1:
	framebuffers[1].width   = scrW;
	framebuffers[1].height  = scrH;
	framebuffers[1].mask    = 0;
	framebuffers[1].psm     = fbPsm;
	framebuffers[1].address = vramAlloc(framebuffers[1].width,
		framebuffers[1].height, framebuffers[1].psm, GRAPH_ALIGN_PAGE);

	// Z-buffer/depth-buffer (same size as the framebuffers):
	zBuffer.enable  = DRAW_ENABLE;
	zBuffer.mask    = 0;
	zBuffer.method  = ZTEST_METHOD_GREATER_EQUAL;
	zBuffer.zsm     = zPsm;
	zBuffer.address = vramAlloc(framebuffers[0].width,
		framebuffers[0].height, zBuffer.zsm, GRAPH_ALIGN_PAGE);

	// User textures start after the z-buffer.
	// Allocate space for a single 256x256 large texture (pretty much all the space we have left):
	vramUserTextureStart = vramAlloc(Texture::MAX_SIZE, Texture::MAX_SIZE, GS_PSM_32, GRAPH_ALIGN_BLOCK);

	//
	// Initialize the screen and tie the first framebuffer to the read circuits:
	//

	// Select between NTSC or PAL, based on region:
	if (vidMode == GRAPH_MODE_AUTO)
	{
		vidMode = graph_get_region();
	}

	// Set video mode with flicker filter:
	const int graphMode = interlaced ? GRAPH_MODE_INTERLACED : GRAPH_MODE_NONINTERLACED;
	graph_set_mode(graphMode, vidMode, GRAPH_MODE_FIELD, GRAPH_ENABLE);

	// Set screen dimensions and framebuffer:
	graph_set_screen(0, 0, framebuffers[0].width, framebuffers[0].height);
	graph_set_bgcolor(0, 0, 0);
	graph_set_framebuffer_filtered(framebuffers[0].address, framebuffers[0].width, framebuffers[0].psm, 0, 0);
	graph_enable_output();
}

// ========================================================
// Renderer::initDrawingEnvironment():
// ========================================================

void Renderer::initDrawingEnvironment()
{
	RenderPacket packet(50, RenderPacket::NORMAL);

	// Set framebuffer and virtual screen offsets:
	qword_t * q = packet.getQwordPtr();
	q = draw_setup_environment(q, 0, &framebuffers[0], &zBuffer);
	q = draw_primitive_xyoffset(q, 0, 2048 - (getScreenWidth() / 2), 2048 - (getScreenHeight() / 2));

	// Texture addressing mode will be fixed to REPEAT for now...
	texwrap_t wrap;
	wrap.horizontal = WRAP_REPEAT;
	wrap.vertical   = WRAP_REPEAT;
	wrap.minu = wrap.maxu = 0;
	wrap.minv = wrap.maxv = 0;
	q = draw_texture_wrapping(q, 0, &wrap);

	q = draw_finish(q);
	dma_channel_send_normal(DMA_CHANNEL_GIF, packet.getQwordPtr(), packet.getDisplacement(q), 0, 0);
	dma_wait_fast();
}

// ========================================================
// Renderer::beginFrame():
// ========================================================

void Renderer::beginFrame()
{
	// Make sure we didn't screw ourselves since the last frame:
	ps2assert(!inMode2d && "Missing end2d() call!");
	ps2assert(!inMode3d && "Missing end3d() call!");
	ps2assert(frameIndex == 0 || frameIndex == 1);

	drawCount2d = 0;
	drawCount3d = 0;
	trisCount3d = 0;
	texSwitches = 0;
	pipeFlushes = 0;

	currentFramePacket = &framePackets[frameIndex];
	currentFrameQwPtr  = currentFramePacket->getQwordPtr();
}

// ========================================================
// Renderer::endFrame():
// ========================================================

void Renderer::endFrame()
{
	ps2assert(!inMode2d && "Missing end2d() call!");
	ps2assert(!inMode3d && "Missing end3d() call!");

	// Add a finish command to the DMA chain:
	BEGIN_DMA_TAG(currentFrameQwPtr);
	currentFrameQwPtr = draw_finish(currentFrameQwPtr);
	END_DMA_TAG_AND_CHAIN(currentFrameQwPtr);

	dma_wait_fast();
	dma_channel_send_chain(DMA_CHANNEL_GIF, currentFramePacket->getQwordPtr(),
		currentFramePacket->getDisplacement(currentFrameQwPtr), 0, 0);

	// V-Sync wait:
	graph_wait_vsync();
	draw_wait_finish();

	graph_set_framebuffer_filtered(framebuffers[frameIndex].address,
		framebuffers[frameIndex].width, framebuffers[frameIndex].psm, 0, 0);

	// Switch context:
	//  1 XOR 1 = 0
	//  0 XOR 1 = 1
	frameIndex ^= 1;

	// Swap render framebuffer:
	flipBuffers(framebuffers[frameIndex]);
}

// ========================================================
// Renderer::flipBuffers():
// ========================================================

void Renderer::flipBuffers(framebuffer_t & fb)
{
	qword_t * q = flipFbPacket.getQwordPtr();

	q = draw_framebuffer(q, 0, &fb);
	q = draw_finish(q);

	dma_wait_fast();
	dma_channel_send_normal_ucab(DMA_CHANNEL_GIF, flipFbPacket.getQwordPtr(), flipFbPacket.getDisplacement(q), 0);
	draw_wait_finish();
}

// ========================================================
// Renderer::clearScreen():
// ========================================================

void Renderer::clearScreen()
{
	BEGIN_DMA_TAG(currentFrameQwPtr);

	currentFrameQwPtr = draw_disable_tests(currentFrameQwPtr, 0, &zBuffer);

	const uint width  = getScreenWidth();
	const uint height = getScreenHeight();

	currentFrameQwPtr = draw_clear(currentFrameQwPtr, 0,
		2048 - (width / 2), 2048 - (height / 2), width, height,
		screenColor.r, screenColor.g, screenColor.b);

	currentFrameQwPtr = draw_enable_tests(currentFrameQwPtr, 0, &zBuffer);

	END_DMA_TAG(currentFrameQwPtr);
}

// ========================================================
// Renderer::resetFrameStates():
// ========================================================

void Renderer::resetFrameStates()
{
	dma_wait_fast();
	draw_wait_finish();
	graph_wait_vsync();

	currentFramePacket = nullptr;
	currentFrameQwPtr  = nullptr;
	currentTex         = nullptr;
	frameIndex         = 0;

	drawCount2d = 0;
	drawCount3d = 0;
	trisCount3d = 0;
	texSwitches = 0;
	pipeFlushes = 0;
	inMode2d    = false;
	inMode3d    = false;

	modelMatrix.makeIdentity();
	invModelMatrix.makeIdentity();
	vpMatrix.makeIdentity();
	mvpMatrix.makeIdentity();

	eyePosition = Vector(0.0f, 0.0f, 0.0f, 1.0f);
	eyePosModelSpace = eyePosition;
}

// ========================================================
// Renderer::isRendererInitialized():
// ========================================================

bool Renderer::isRendererInitialized() const
{
	return rendererInitialized;
}

// ========================================================
// Renderer::isVideoInitialized():
// ========================================================

bool Renderer::isVideoInitialized() const
{
	return videoInitialized;
}

// ========================================================
// Renderer::getScreenWidth():
// ========================================================

uint Renderer::getScreenWidth() const
{
	return framebuffers[0].width;
}

// ========================================================
// Renderer::getScreenHeight():
// ========================================================

uint Renderer::getScreenHeight() const
{
	return framebuffers[0].height;
}

// ========================================================
// Renderer::getAspectRatio():
// ========================================================

float Renderer::getAspectRatio() const
{
	return 4.0f / 3.0f;
	//
	// 4:3 seems to produce better visual results...
	// return graph_aspect_ratio();
}

// ========================================================
// Renderer::disableDepthWriting():
// ========================================================

void Renderer::disableDepthWriting()
{
	ps2assert(currentFrameQwPtr != nullptr);

	BEGIN_DMA_TAG(currentFrameQwPtr);
	zBuffer.mask = 0xFFFFFFFF;
	currentFrameQwPtr = draw_zbuffer(currentFrameQwPtr, 0, &zBuffer);
	END_DMA_TAG(currentFrameQwPtr);
}

// ========================================================
// Renderer::enableDepthWriting():
// ========================================================

void Renderer::enableDepthWriting()
{
	ps2assert(currentFrameQwPtr != nullptr);

	BEGIN_DMA_TAG(currentFrameQwPtr);
	zBuffer.mask = 0;
	currentFrameQwPtr = draw_zbuffer(currentFrameQwPtr, 0, &zBuffer);
	END_DMA_TAG(currentFrameQwPtr);
}

// ========================================================
// Renderer::getVRamUserTextureStart():
// ========================================================

uint Renderer::getVRamUserTextureStart() const
{
	return vramUserTextureStart;
}

// ========================================================
// Renderer::setTexture():
// ========================================================

void Renderer::setTexture(const Texture & tex)
{
	if (&tex == currentTex)
	{
		return; // Avoid redundant state changes.
	}

	currentTex = ccast<Texture *>(&tex);
	ps2assert(currentTex->getPixels() != nullptr && "No pixel data associated with texture!");
	ps2assert(currentTex->getTexBuffer().address == uint(vramUserTextureStart));

	flushPipeline();
	texSwitches++;

	//
	// Upload the texture to GS VRam.
	// Since we are using almost all of our video memory with framebuffers
	// and z-buffer, the rest of it can only fit a single texture.
	//
	const uint width  = currentTex->getWidth();
	const uint height = currentTex->getHeight();
	const uint psm    = currentTex->getPixelFormat();
	ubyte * pixels    = ccast<ubyte *>(currentTex->getPixels());

	qword_t * q = textureUploadPacket[frameIndex].getQwordPtr();

	q = draw_texture_transfer(q, pixels, width, height, psm, vramUserTextureStart, width);
	q = draw_texture_flush(q);

	dma_channel_send_chain(DMA_CHANNEL_GIF, textureUploadPacket[frameIndex].getQwordPtr(),
			textureUploadPacket[frameIndex].getDisplacement(q), 0, 0);
	dma_wait_fast();
}

// ========================================================
// Renderer::setTextureBufferSampling():
// ========================================================

void Renderer::setTextureBufferSampling()
{
	ps2assert(currentTex != nullptr);
	ps2assert(currentFrameQwPtr != nullptr);
	ps2assert(currentTex->getTexBuffer().address == uint(vramUserTextureStart));

	currentFrameQwPtr = draw_texture_sampling(currentFrameQwPtr, 0, &currentTex->getTexLod());
	currentFrameQwPtr = draw_texturebuffer(currentFrameQwPtr, 0, &currentTex->getTexBuffer(), &currentTex->getTexClut());
}

// ========================================================
// Renderer::flushPipeline():
// ========================================================

void Renderer::flushPipeline()
{
	if (drawCount2d + drawCount3d == 0)
	{
		return; // Nothing draw, do nothing.
	}

	pipeFlushes++;

	// Add a finish command to the DMA chain:
	BEGIN_DMA_TAG(currentFrameQwPtr);
	currentFrameQwPtr = draw_finish(currentFrameQwPtr);
	END_DMA_TAG_AND_CHAIN(currentFrameQwPtr);

	dma_channel_send_chain(DMA_CHANNEL_GIF, currentFramePacket->getQwordPtr(),
		currentFramePacket->getDisplacement(currentFrameQwPtr), 0, 0);
	dma_wait_fast();

	// Reset packet data pointer to the next one:
	currentFramePacket = &framePackets[frameIndex];
	currentFrameQwPtr  = currentFramePacket->getQwordPtr();
}

// ========================================================
// Renderer::setClearScreenColor():
// ========================================================

void Renderer::setClearScreenColor(const ubyte r, const ubyte g, const ubyte b)
{
	screenColor.r = r;
	screenColor.g = g;
	screenColor.b = b;
}

// ========================================================
// Renderer::setPrimShading():
// ========================================================

void Renderer::setPrimShading(const int shading)
{
	primDesc.shading = shading;
}

// ========================================================
// Renderer::setPrimTopology():
// ========================================================

void Renderer::setPrimTopology(const int topology)
{
	primDesc.type = topology;
}

// ========================================================
// Renderer::setPrimTextureMapping():
// ========================================================

void Renderer::setPrimTextureMapping(const bool enable)
{
	if (enable)
	{
		primDesc.mapping      = DRAW_ENABLE;
		primDesc.mapping_type = PRIM_MAP_ST;
	}
	else
	{
		primDesc.mapping      = DRAW_DISABLE;
		primDesc.mapping_type = DRAW_DISABLE;
	}
}

// ========================================================
// Renderer::setPrimFogging():
// ========================================================

void Renderer::setPrimFogging(const bool enable)
{
	primDesc.fogging = enable ? DRAW_ENABLE : DRAW_DISABLE;
}

// ========================================================
// Renderer::setPrimBlending():
// ========================================================

void Renderer::setPrimBlending(const bool enable)
{
	primDesc.blending = enable ? DRAW_ENABLE : DRAW_DISABLE;
}

// ========================================================
// Renderer::setPrimAntialiasing():
// ========================================================

void Renderer::setPrimAntialiasing(const bool enable)
{
	primDesc.antialiasing = enable ? DRAW_ENABLE : DRAW_DISABLE;
}

// ========================================================
// Renderer::setPrimBaseColor():
// ========================================================

void Renderer::setPrimBaseColor(const ubyte r, const ubyte g, const ubyte b, const ubyte a, const float q)
{
	primColor.r = r;
	primColor.g = g;
	primColor.b = b;
	primColor.a = a;
	primColor.q = q;
}

// ================================================================================================
// Miscellaneous 3D drawing:
// ================================================================================================

/*
 * Defining this will disable all back-face triangle
 * culling. For tiny scenes, this might actually be faster,
 * but for any complex geometry, disabling back-face culling
 * will likely double the number of triangles processed every
 * frame, slowing down the application quite significantly.
 */
//#define NO_BACK_FACE_CULLING 1

/*
 * Defining this will disable off-screen triangle clipping.
 * Clipping of off-screen geometry is not very accurate,
 * sometimes things get clipped when they shouldn't, however,
 * without this hidden surface removal, positioning the viewer
 * behind any geometry would generate some wacky visual artifacts.
 * Off-screen triangle clipping is best left ON, unless you are
 * sure the viewer/camera will never go behind any scene geometry.
 */
//#define NO_TRIANGLE_CLIPPING 1

// ========================================================

// Vertex transformation and clipping routines (raw text include):
#include "vertex_xform.h"

// ========================================================
// Renderer::begin3d():
// ========================================================

void Renderer::begin3d()
{
	ps2assert(!inMode3d && "Already in 3D drawing mode!");
	inMode3d = true;
}

// ========================================================
// Renderer::end3d():
// ========================================================

void Renderer::end3d()
{
	ps2assert(inMode3d && "Not in 3D drawing mode!");
	inMode3d = false;
}

// ========================================================
// Renderer::getViewProjMatrix():
// ========================================================

const Matrix & Renderer::getViewProjMatrix() const
{
	return vpMatrix;
}

// ========================================================
// Renderer::getModelMatrix():
// ========================================================

const Matrix & Renderer::getModelMatrix() const
{
	return modelMatrix;
}

// ========================================================
// Renderer::setViewProjMatrix():
// ========================================================

void Renderer::setViewProjMatrix(const Matrix & vp)
{
	vpMatrix = vp;
}

// ========================================================
// Renderer::setModelMatrix():
// ========================================================

void Renderer::setModelMatrix(const Matrix & m)
{
	modelMatrix = m;
	mvpMatrix   = modelMatrix * vpMatrix;

	inverseMatrix(&invModelMatrix, &modelMatrix);
	applyXForm(&eyePosModelSpace, &invModelMatrix, &eyePosition);
}

// ========================================================
// Renderer::getEyePosition():
// ========================================================

const Vector & Renderer::getEyePosition() const
{
	return eyePosition;
}

// ========================================================
// Renderer::setEyePosition():
// ========================================================

void Renderer::setEyePosition(const Vector & eye)
{
	eyePosition = eye;
}

// ========================================================
// Renderer::drawUnindexedTriangles():
// ========================================================

void Renderer::drawUnindexedTriangles(const DrawVertex * restrict verts, const uint vertCount)
{
	ps2assert(verts != nullptr);
	ps2assert(vertCount != 0);
	ps2assert(inMode3d && "3D mode required!");

	// We are expecting triangles!
	if (vertCount % 3)
	{
		fatalError("drawUnindexedTriangles(%u) => vertCount must be evenly divisible by 3!", vertCount);
	}

	DRAW3D_PROLOGUE();

	uint trisSentToGs = 0;
	const uint triCount = vertCount / 3;

	for (uint t = 0; t < triCount; ++t)
	{
		const DrawVertex & v0 = verts[(t * 3) + 0];
		const DrawVertex & v1 = verts[(t * 3) + 1];
		const DrawVertex & v2 = verts[(t * 3) + 2];
		XFORM_TRIANGLE(v0, v1, v2); // Includes clipping and back-face culling.
		++trisSentToGs;
	}

	DRAW3D_EPILOGUE();
	trisCount3d += trisSentToGs;
}

// ========================================================
// Renderer::drawIndexedTriangles():
// ========================================================

void Renderer::drawIndexedTriangles(const uint16 * restrict indexes, const uint indexCount,
                                    const DrawVertex * restrict verts, const uint vertCount)
{
	ps2assert(indexes    != nullptr);
	ps2assert(indexCount != 0);
	ps2assert(verts      != nullptr);
	ps2assert(vertCount  != 0);
	ps2assert(inMode3d && "3D mode required!");

	// We are expecting triangles!
	if (indexCount % 3)
	{
		fatalError("drawIndexedTriangles(%u) => indexCount must be evenly divisible by 3!", indexCount);
	}

	DRAW3D_PROLOGUE();

	uint trisSentToGs = 0;
	const uint triCount = indexCount / 3;

	for (uint t = 0; t < triCount; ++t)
	{
		const DrawVertex & v0 = verts[indexes[(t * 3) + 0]];
		const DrawVertex & v1 = verts[indexes[(t * 3) + 1]];
		const DrawVertex & v2 = verts[indexes[(t * 3) + 2]];
		XFORM_TRIANGLE(v0, v1, v2); // Includes clipping and back-face culling.
		++trisSentToGs;
	}

	DRAW3D_EPILOGUE();
	trisCount3d += trisSentToGs;
}

// ========================================================
// Renderer::drawIndexedTriangles():
// ========================================================

void Renderer::drawIndexedTriangles(const uint16 * restrict indexes, const uint indexCount,
                                    const float restrict (*positions)[3], const uint positionCount,
                                    const float restrict (*texCoords)[2], const uint texCoordCount,
                                    const Color4f & baseColor)
{
	ps2assert(indexes != nullptr);
	ps2assert(indexCount    != 0);
	ps2assert(positionCount != 0);
	ps2assert(texCoordCount != 0);
	ps2assert(inMode3d && "3D mode required!");

	// We are expecting triangles!
	if (indexCount % 3)
	{
		fatalError("drawIndexedTriangles(%u) => indexCount must be evenly divisible by 3!", indexCount);
	}

	DRAW3D_PROLOGUE();

	uint trisSentToGs = 0;
	const uint triCount = indexCount / 3;
	const Vector vColor(baseColor.r, baseColor.g, baseColor.b, baseColor.a);

	DrawVertex v0, v1, v2;
	Vector tPos0, tPos1, tPos2;

	for (uint t = 0; t < triCount; ++t)
	{
		const uint index0 = indexes[(t * 3) + 0];
		const uint index1 = indexes[(t * 3) + 1];
		const uint index2 = indexes[(t * 3) + 2];

		v0.position.x = positions[index0][0];
		v0.position.y = positions[index0][1];
		v0.position.z = positions[index0][2];
		v0.position.w = 1.0f;

		v1.position.x = positions[index1][0];
		v1.position.y = positions[index1][1];
		v1.position.z = positions[index1][2];
		v1.position.w = 1.0f;

		v2.position.x = positions[index2][0];
		v2.position.y = positions[index2][1];
		v2.position.z = positions[index2][2];
		v2.position.w = 1.0f;

		// I know this looks shady... And it is!
		// Look at the implementation in `vertex_xform.h` for details.
		TRIANGLE_BACK_FACE_CULL(v0.position, v1.position, v2.position);
		TRIANGLE_XFORM_SCR_CLIP(v0.position, v1.position, v2.position);

		// Conv to fixed point & add to GS render packet:
		//
		setTexcColor(v0, Vector(texCoords[index0][0], texCoords[index0][1], 0.0f, 1.0f), vColor);
		setTexcColor(v1, Vector(texCoords[index1][0], texCoords[index1][1], 0.0f, 1.0f), vColor);
		setTexcColor(v2, Vector(texCoords[index2][0], texCoords[index2][1], 0.0f, 1.0f), vColor);
		emitVert(packetPtr, tPos0, q0, v0);
		emitVert(packetPtr, tPos1, q1, v1);
		emitVert(packetPtr, tPos2, q2, v2);

		++trisSentToGs;
	}

	DRAW3D_EPILOGUE();
	trisCount3d += trisSentToGs;
}

// ========================================================
// Renderer::drawIndexedTrianglesUnculled():
// ========================================================

void Renderer::drawIndexedTrianglesUnculled(const uint16 * restrict indexes, const uint indexCount,
                                            const DrawVertex * restrict verts, const uint vertCount)
{
	ps2assert(indexes    != nullptr);
	ps2assert(indexCount != 0);
	ps2assert(verts      != nullptr);
	ps2assert(vertCount  != 0);
	ps2assert(inMode3d && "3D mode required!");

	// We are expecting triangles!
	if (indexCount % 3)
	{
		fatalError("drawIndexedTrianglesUnculled(%u) => indexCount must be evenly divisible by 3!", indexCount);
	}

	DRAW3D_PROLOGUE();

	uint trisSentToGs = 0;
	const uint triCount = indexCount / 3;

	for (uint t = 0; t < triCount; ++t)
	{
		const DrawVertex & v0 = verts[indexes[(t * 3) + 0]];
		const DrawVertex & v1 = verts[indexes[(t * 3) + 1]];
		const DrawVertex & v2 = verts[indexes[(t * 3) + 2]];
		XFORM_TRIANGLE_NO_BF_CULL(v0, v1, v2); // Includes off-screen clipping, but no back-face culling.
		++trisSentToGs;
	}

	DRAW3D_EPILOGUE();
	trisCount3d += trisSentToGs;
}

// ========================================================
// Renderer::drawLine():
// ========================================================

void Renderer::drawLine(const Vector & from, const Vector & to, const Color4f & color)
{
	setPrimTopology(PRIM_LINE);
	setPrimTextureMapping(false);

	BEGIN_DMA_TAG(currentFrameQwPtr);
	qword_t * restrict packetPtr = draw_prim_start(currentFrameQwPtr, 0, &primDesc, &primColor);

	emitLine(packetPtr, mvpMatrix, from, to, color);

	currentFrameQwPtr = draw_prim_end(packetPtr, 2, DRAW_RGBAQ_REGLIST);
	END_DMA_TAG(currentFrameQwPtr);
	drawCount3d++;

	setPrimTextureMapping(true);
	setPrimTopology(PRIM_TRIANGLE);
}

// ========================================================
// Renderer::drawAabb():
// ========================================================

void Renderer::drawAabb(const Aabb & aabb, const Color4f & color)
{
	setPrimTopology(PRIM_LINE);
	setPrimTextureMapping(false);

	Vector points[8];
	aabb.toPoints(points);

	BEGIN_DMA_TAG(currentFrameQwPtr);
	qword_t * restrict packetPtr = draw_prim_start(currentFrameQwPtr, 0, &primDesc, &primColor);

	for (int i = 0; i < 4; ++i)
	{
		emitLine(packetPtr, mvpMatrix, points[i],     points[(i + 1) & 3],       color);
		emitLine(packetPtr, mvpMatrix, points[4 + i], points[4 + ((i + 1) & 3)], color);
		emitLine(packetPtr, mvpMatrix, points[i],     points[4 + i],             color);
	}

	currentFrameQwPtr = draw_prim_end(packetPtr, 2, DRAW_RGBAQ_REGLIST);
	END_DMA_TAG(currentFrameQwPtr);
	drawCount3d++;

	setPrimTextureMapping(true);
	setPrimTopology(PRIM_TRIANGLE);
}

// ================================================================================================
// Miscellaneous 2D drawing:
// ================================================================================================

// ========================================================
// Renderer::begin2d():
// ========================================================

void Renderer::begin2d(const Texture * tex)
{
	ps2assert(!inMode2d && "Already in 2D drawing mode!");
	inMode2d = true;

	// Bind the text atlas for further `drawText()` calls:
	setTexture((tex == nullptr) ? texAtlas.getTexture() : (*tex));

	// Reference the external tag `dmaTagDraw2d`:
	BEGIN_DMA_TAG_NAMED(dmaTagDraw2d, currentFrameQwPtr);
	currentFrameQwPtr = draw_primitive_xyoffset(currentFrameQwPtr, 0, 2048, 2048);
	setTextureBufferSampling(); // For `texAtlas` or the user supplied texture.
}

// ========================================================
// Renderer::end2d():
// ========================================================

void Renderer::end2d()
{
	ps2assert(inMode2d && "Not in 2D drawing mode!");

	currentFrameQwPtr = draw_primitive_xyoffset(currentFrameQwPtr, 0,
		2048 - (getScreenWidth() / 2), 2048 - (getScreenHeight() / 2));

	// Close `dmaTagDraw2d`.
	END_DMA_TAG_NAMED(dmaTagDraw2d, currentFrameQwPtr);
	inMode2d = false;
}

// ========================================================
// Renderer::drawRectFilled():
// ========================================================

void Renderer::drawRectFilled(const Rect4i & rect, const Color4b color)
{
	ps2assert(inMode2d && "2D mode required!");

	rect_t rc;
	rc.v0.x = rect.x;
	rc.v0.y = rect.y;
	rc.v0.z = 0xFFFFFFFF;
	rc.v1.x = rect.width  + rect.x;
	rc.v1.y = rect.height + rect.y;
	rc.v1.z = 0xFFFFFFFF;
	rc.color.r = color.r;
	rc.color.g = color.g;
	rc.color.b = color.b;
	rc.color.a = color.a;
	rc.color.q = 1.0f;

	currentFrameQwPtr = draw_rect_filled(currentFrameQwPtr, 0, &rc);
	drawCount2d++;
}

// ========================================================
// Renderer::drawRectOutline():
// ========================================================

void Renderer::drawRectOutline(const Rect4i & rect, const Color4b color)
{
	ps2assert(inMode2d && "2D mode required!");

	rect_t rc;
	rc.v0.x = rect.x;
	rc.v0.y = rect.y;
	rc.v0.z = 0xFFFFFFFF;
	rc.v1.x = rect.x + rect.width;
	rc.v1.y = rect.y + rect.height;
	rc.v1.z = 0xFFFFFFFF;
	rc.color.r = color.r;
	rc.color.g = color.g;
	rc.color.b = color.b;
	rc.color.a = color.a;
	rc.color.q = 1.0f;

	currentFrameQwPtr = draw_rect_outline(currentFrameQwPtr, 0, &rc);
	drawCount2d++;
}

// ========================================================
// Renderer::drawRectTextured():
// ========================================================

void Renderer::drawRectTextured(const Rect4i & rect, const Color4b color)
{
	uint w, h;
	if (currentTex != nullptr)
	{
		w = currentTex->getWidth();
		h = currentTex->getHeight();
	}
	else
	{
		w = Texture::MAX_SIZE;
		h = Texture::MAX_SIZE;
	}

	texrect_t rc;
	rc.v0.x = rect.x;
	rc.v0.y = rect.y;
	rc.v0.z = 0xFFFFFFFF;
	rc.t0.u = 0;
	rc.t0.v = 0;
	rc.v1.x = rect.x + rect.width;
	rc.v1.y = rect.y + rect.height;
	rc.v1.z = 0xFFFFFFFF;
	rc.t1.u = w;
	rc.t1.v = h;
	rc.color.r = color.r;
	rc.color.g = color.g;
	rc.color.b = color.b;
	rc.color.a = color.a;
	rc.color.q = 1.0f;

	currentFrameQwPtr = draw_rect_textured(currentFrameQwPtr, 0, &rc);
	drawCount2d++;
}

// ========================================================
// Renderer::drawFpsCounter():
// ========================================================

void Renderer::drawFpsCounter()
{
	const uint timeMillisec = clockMilliseconds(); // Real time clock
	const uint frameTime    = timeMillisec - fps.previousTime;

	fps.previousTimes[fps.index++] = frameTime;
	fps.previousTime = timeMillisec;

	if (fps.index == FpsCounter::MAX_FRAMES)
	{
		uint total = 0;
		for (uint i = 0; i < FpsCounter::MAX_FRAMES; ++i)
		{
			total += fps.previousTimes[i];
		}

		if (total == 0)
		{
			total = 1;
		}

		fps.fpsCount = (10000 * FpsCounter::MAX_FRAMES / total);
		fps.fpsCount = (fps.fpsCount + 5) / 10;
		fps.index    = 0;
	}

	// Draw it at the top-right corner of the screen, full-size text:
	Vec2f pos;
	pos.x = getScreenWidth() - 80.0f;
	pos.y = 5.0f;

	const float oldTextScale = globalTextScale;
	globalTextScale = 1.0f; // Ensure full size text.

	drawText(pos, makeColor4b(0, 200, 0), FONT_CONSOLAS_24, format("FPS:%u", fps.fpsCount));

	globalTextScale = oldTextScale;
}

// ========================================================
// Renderer::drawFrameStats():
// ========================================================

void Renderer::drawFrameStats()
{
	const Color4b white = { 255, 255, 255, 255 };

	Vec2f pos;
	pos.x = 5.0f;
	pos.y = getScreenHeight() - 88.0f;

	drawText(pos, white, FONT_CONSOLAS_24, format("Texture switches  : %u\n", texSwitches));
	drawText(pos, white, FONT_CONSOLAS_24, format("Pipeline flushes  : %u\n", pipeFlushes));
	drawText(pos, white, FONT_CONSOLAS_24, format("3D draw calls     : %u\n", drawCount3d));
	drawText(pos, white, FONT_CONSOLAS_24, format("2D draw calls     : %u\n", drawCount2d));
	drawText(pos, white, FONT_CONSOLAS_24, format("Tris sent to GS   : %u\n", trisCount3d));
}

// ================================================================================================
// Built-in text rendering:
// ================================================================================================

namespace
{

// ========================================================
// struct BuiltInFontGlyph:
// ========================================================

struct ATTRIBUTE_ALIGNED(16) BuiltInFontGlyph
{
	// Texture coordinates to use when rendering.
	// Stored *in pixels*, as required by the PS2 renderer.
	int32 u0, v0;
	int32 u1, v1;

	// Dimensions of the glyph's bitmap (in pixels):
	int32 w, h;

	// Pointer to start of glyph bitmap data:
	//  `builtInFontBitmap->bitmap[]`
	uint32 * pixels;
};

// ========================================================
// struct BuiltInFontBitmap:
// ========================================================

struct ATTRIBUTE_ALIGNED(16) BuiltInFontBitmap
{
	// Glyphs in use / first char:
	int32 numGlyphs;
	int32 charStart;

	// Dimensions of the bitmap, in pixels:
	int32 bmpWidth;
	int32 bmpHeight;

	// Bitmap allocated to store decompressed glyphs.
	// Bitmap glyphs are stored as a string of glyphs,
	// forming a very wide image, but with height equal
	// to the glyph's height. E.g.:
	// +--------------------
	// |1|2|3|...|A|B|C|...
	// +--------------------
	uint32 * bitmap;

	// Glyphs for this font:
	static const int MAX_GLYPHS = 96;
	BuiltInFontGlyph glyphs[MAX_GLYPHS];
};

// ========================================================
// Local text rendering data & compressed font data:
// ========================================================

// "Consolas" fonts (see `builtin_fonts/` dir):
#include "builtin_fonts/consolas10.h"
#include "builtin_fonts/consolas12.h"
#include "builtin_fonts/consolas16.h"
#include "builtin_fonts/consolas20.h"
#include "builtin_fonts/consolas24.h"
#include "builtin_fonts/consolas30.h"
#include "builtin_fonts/consolas36.h"
#include "builtin_fonts/consolas40.h"
#include "builtin_fonts/consolas48.h"

// All built-in fonts must be registered here,
// in the same order they appear in the BuiltInFontId enum!
static const uint32 * const compressedFontData[] ATTRIBUTE_ALIGNED(16) =
{
	font_10Con,
	font_12Con,
	font_16Con,
	font_20Con,
	font_24Con,
	font_30Con,
	font_36Con,
	font_40Con,
	font_48Con
};

// Expanded built-in fonts, loaded on demand:
static BuiltInFontBitmap builtInFonts[FONT_COUNT];

} // namespace {}

// ========================================================
// Renderer::fntDecompressGlyphs():
// ========================================================

void Renderer::fntDecompressGlyphs(const BuiltInFontId fontId)
{
	// Based on code found at: http://silverspaceship.com/inner/imgui/grlib/gr_extra.c
	// Originally written by Sean Barrett.

// --------------------------------------

#define SKIPBIT() \
	if ((buffer >>= 1), (--bitsLeft == 0)) { buffer = *source++; bitsLeft = 32; }

#define PACKRGBA(r, g, b, a) \
	(uint32)((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((r) & 0xFF))

// --------------------------------------

	BuiltInFontBitmap * font = &builtInFonts[fontId];
	font->numGlyphs = BuiltInFontBitmap::MAX_GLYPHS;

	const uint32 * source = compressedFontData[fontId];

	int32 i, j;
	int32 effectiveCount;
	int32 bitsLeft;
	int32 buffer;

	const int32 start = (source[0] >>  0) & 0xFF;
	const int32 count = (source[0] >>  8) & 0xFF;
	const int32 h     = (source[0] >> 16) & 0xFF;

	BuiltInFontGlyph * glyphs = &font->glyphs[0];
	int32 numGlyphs = font->numGlyphs;

	if (count > numGlyphs)
	{
		effectiveCount = numGlyphs;
	}
	else
	{
		effectiveCount = count;
	}

	font->charStart = start;
	font->bmpHeight = h; // Height is the same for every glyph
	font->bmpWidth  = 0;
	++source;

	// Count bitmap size:
	for (i = 0; i < effectiveCount; ++i)
	{
		const int32 w = (source[i >> 2] >> ((i & 3) << 3)) & 0xFF;
		font->bmpWidth += w;
	}

	// Allocate memory:
	font->bitmap = memAlloc<uint32>(MEM_TAG_RENDERER, (font->bmpWidth * font->bmpHeight), 128);

	// Set glyphs:
	uint32 * pBitmap = font->bitmap;
	for (i = 0; i < effectiveCount; ++i)
	{
		const int32 w = (source[i >> 2] >> ((i & 3) << 3)) & 0xFF;
		glyphs[i].w = w;
		glyphs[i].h = h;
		glyphs[i].pixels = pBitmap;
		pBitmap += w * h;
	}

	source  += (count + 3) >> 2;
	buffer   = *source++;
	bitsLeft = 32;

	while (--numGlyphs >= 0)
	{
		uint32 * c = glyphs->pixels;
		for (j = 0; j < glyphs->h; ++j)
		{
			int32 z = (buffer & 1);
			SKIPBIT();
			if (!z)
			{
				for (i = 0; i < glyphs->w; ++i)
				{
					*c++ = PACKRGBA(255, 255, 255, 0);
				}
			}
			else
			{
				for (i = 0; i < glyphs->w; ++i)
				{
					z = (buffer & 1);
					SKIPBIT();
					if (!z)
					{
						*c++ = PACKRGBA(255, 255, 255, 0);
					}
					else
					{
						int32 n = 0;
						n += n + (buffer & 1); SKIPBIT();
						n += n + (buffer & 1); SKIPBIT();
						n += n + (buffer & 1); SKIPBIT();
						n += 1;
						*c++ = PACKRGBA(255, 255, 255, ((255 * n) >> 3));
					}
				}
			}
		}
		++glyphs;
	}

	font->numGlyphs = count;

// Cleanup the namespace:
#undef PACKRGBA
#undef SKIPBIT
}

// ========================================================
// Renderer::fntLoadGlyphs():
// ========================================================

void Renderer::fntLoadGlyphs(const BuiltInFontId fontId)
{
	ps2assert(fontId >= 0 && fontId < FONT_COUNT);
	ps2assert(builtInFonts[fontId].bitmap == nullptr && "Font already loaded!");

	// Expand data and allocate the bitmap:
	fntDecompressGlyphs(fontId);
}

// ========================================================
// Renderer::fntUnloadGlyphs():
// ========================================================

void Renderer::fntUnloadGlyphs(const BuiltInFontId fontId)
{
	ps2assert(fontId >= 0 && fontId < FONT_COUNT);
	if (builtInFonts[fontId].bitmap == nullptr)
	{
		return;
	}

	memFree(MEM_TAG_RENDERER, builtInFonts[fontId].bitmap); // Was allocated with `memAlloc()`

	builtInFonts[fontId].numGlyphs = 0;
	builtInFonts[fontId].charStart = 0;
	builtInFonts[fontId].bmpWidth  = 0;
	builtInFonts[fontId].bmpHeight = 0;
	builtInFonts[fontId].bitmap    = nullptr;

	memset(builtInFonts[fontId].glyphs, 0, BuiltInFontBitmap::MAX_GLYPHS * sizeof(BuiltInFontGlyph));
}

// ========================================================
// Renderer::fntAllocateAtlasSpace():
// ========================================================

bool Renderer::fntAllocateAtlasSpace(const BuiltInFontId fontId)
{
	ps2assert(fontId >= 0 && fontId < FONT_COUNT);
	ps2assert(texAtlas.isInitialized());

	// Reserve some memory for every glyph, since we know the amount beforehand:
	texAtlas.reserveMemForRegions(builtInFonts[fontId].numGlyphs + 1);

	for (int i = 0; i < builtInFonts[fontId].numGlyphs; ++i)
	{
		BuiltInFontGlyph & glyph = builtInFonts[fontId].glyphs[i];
		if (glyph.pixels == nullptr)
		{
			continue;
		}

		// Alloc atlas space (+1 pixel each side for a safety border when sampling):
		const int BorderSize = 1;
		Rect4i region = texAtlas.allocRegion(glyph.w + (BorderSize * 2), glyph.h + (BorderSize * 2));
		if (region.x < 0)
		{
			logError("The default text atlas seems to be full! Discarding further text glyphs...");
			return false;
		}

		// Fill the whole region with a default initial value:
		texAtlas.setRegion(region.x, region.y, region.width, region.height, 0);

		// Copy the glyph pixels to its rectangle, preserving the border:
		region.x += BorderSize;
		region.y += BorderSize;
		texAtlas.setRegion(region.x, region.y, glyph.w, glyph.h,
			rcast<const ubyte *>(glyph.pixels), glyph.w * sizeof(uint32));

		// Store UVs for this glyph with pixels scale:
		glyph.u0 = region.x;
		glyph.v0 = region.y;
		glyph.u1 = region.x + glyph.w;
		glyph.v1 = region.y + glyph.h;
	}

	return true;
}

// ========================================================
// Renderer::fntGetWhiteSpaceWidth():
// ========================================================

float Renderer::fntGetWhiteSpaceWidth(const BuiltInFontId fontId) const
{
	ps2assert(fontId >= 0 && fontId < FONT_COUNT);
	// Get with in pixels of a white space.
	// Index zero should always be the white space glyph.
	return scast<float>(builtInFonts[fontId].glyphs[0].w) * globalTextScale;
}

// ========================================================
// Renderer::loadAllBuiltInFonts():
// ========================================================

void Renderer::loadAllBuiltInFonts()
{
	for (int i = 0; i < FONT_COUNT; ++i)
	{
		if (!loadBuiltInFont(scast<BuiltInFontId>(i)))
		{
			logError("Failed to load built-in font #%d", i);
		}
	}
}

// ========================================================
// Renderer::unloadAllBuiltInFonts():
// ========================================================

void Renderer::unloadAllBuiltInFonts()
{
	for (int i = 0; i < FONT_COUNT; ++i)
	{
		unloadBuiltInFont(scast<BuiltInFontId>(i));
	}
}

// ========================================================
// Renderer::loadBuiltInFont():
// ========================================================

bool Renderer::loadBuiltInFont(const BuiltInFontId fontId)
{
	// Load a font only once.
	if (isBuiltInFontLoaded(fontId))
	{
		logWarning("Built-in bitmap font #%d already loaded!", scast<int>(fontId));
		return true;
	}

	// Initialize our texture atlas if this is the first font loaded:
	if (!texAtlas.isInitialized())
	{
		texAtlas.init(
			/* width   = */ Texture::MAX_SIZE,
			/* height  = */ Texture::MAX_SIZE,
			/* bpp     = */ 4,
			/* fillVal = */ 0);
	}

	// Load & decompress the glyphs:
	fntLoadGlyphs(fontId);

	// Place them in the atlas texture:
	if (!fntAllocateAtlasSpace(fontId))
	{
		return false;
	}

	logComment("Successfully loaded built-in font #%d", scast<int>(fontId));
	return true;
}

// ========================================================
// Renderer::unloadBuiltInFont():
// ========================================================

void Renderer::unloadBuiltInFont(const BuiltInFontId fontId)
{
	// Note that unloading a font will not remove the glyphs from the texture atlas.
	// The glyphs will remain allocated in the atlas space until the whole atlas
	// texture is cleared.
	fntUnloadGlyphs(fontId);
}

// ========================================================
// Renderer::isBuiltInFontLoaded():
// ========================================================

bool Renderer::isBuiltInFontLoaded(const BuiltInFontId fontId)
{
	ps2assert(fontId >= 0 && fontId < FONT_COUNT);
	return builtInFonts[fontId].bitmap != nullptr;
}

// ========================================================
// Renderer::drawChar():
// ========================================================

float Renderer::drawChar(Vec2f & pos, const Color4b color, const BuiltInFontId fontId, const int c)
{
	ps2assert(inMode2d && "2D mode required!");

	if (!isBuiltInFontLoaded(fontId)) // Load font if needed
	{
		if (!loadBuiltInFont(fontId))
		{
			return 0;
		}
	}

	// - Text is aligned to the left.
	// - `pos.x` always incremented by glyph.with.
	// - Spaces increment `pos.x` by `glyph.with`; Tabs increment it by `glyph.width * 4` (4 spaces).
	// - New lines are not handled, since we don't know the initial line x to return to.
	float charWidth = 0.0f;
	switch (c)
	{
	case ' ' :
		charWidth = fntGetWhiteSpaceWidth(fontId);
		pos.x += charWidth;
		break;

	case '\t' :
		charWidth = fntGetWhiteSpaceWidth(fontId);
		pos.x += charWidth * 4.0f;
		break;

	default :
		if (isgraph(c)) // Only add if char is printable
		{
			const int charIndex = c - builtInFonts[fontId].charStart;
			if (charIndex >= 0 && charIndex < BuiltInFontBitmap::MAX_GLYPHS)
			{
				const BuiltInFontGlyph & glyph = builtInFonts[fontId].glyphs[charIndex];

				texrect_t rc;
				rc.v0.x = pos.x;
				rc.v0.y = pos.y;
				rc.v0.z = 0xFFFFFFFF;
				rc.t0.u = glyph.u0;
				rc.t0.v = glyph.v0;
				rc.v1.x = pos.x + (glyph.w * globalTextScale);
				rc.v1.y = pos.y + (glyph.h * globalTextScale);
				rc.v1.z = 0xFFFFFFFF;
				rc.t1.u = glyph.u1;
				rc.t1.v = glyph.v1;
				rc.color.r = color.r;
				rc.color.g = color.g;
				rc.color.b = color.b;
				rc.color.a = color.a;
				rc.color.q = 1.0f;

				currentFrameQwPtr = draw_rect_textured(currentFrameQwPtr, 0, &rc);
				drawCount2d++;

				charWidth = scast<float>(glyph.w);
				pos.x += charWidth * globalTextScale;
			}
		}
		break;
	} // switch (c)

	return charWidth;
}

// ========================================================
// Renderer::drawText():
// ========================================================

void Renderer::drawText(Vec2f & pos, const Color4b color, const BuiltInFontId fontId, const char * str)
{
	ps2assert(fontId >= 0 && fontId < FONT_COUNT);
	ps2assert(str != nullptr);

	// Unlike drawChar(), this function handles a newline by setting
	// `pos.x` back to its initial value, and `pos[1] += glyph.height`.
	// A '\r' restores `pos.x` but leaves `pos.y` unchanged.

	const float initialX = pos.x;
	for (uint c = 0; str[c] != '\0'; ++c)
	{
		switch (str[c])
		{
		case '\n' :
			pos.x  = initialX;
			pos.y += builtInFonts[fontId].bmpHeight * globalTextScale; // Every glyph of a font-face has the same height
			break;

		case '\r' :
			pos.x = initialX;
			break;

		default :
			drawChar(pos, color, fontId, str[c]);
			break;
		} // switch (str[c])
	}
}

// ========================================================
// Renderer::getTextLength():
// ========================================================

void Renderer::getTextLength(const char * str, const BuiltInFontId fontId, Vec2f & advance)
{
	ps2assert(str != nullptr);
	if (!isBuiltInFontLoaded(fontId)) // Load font if needed
	{
		if (!loadBuiltInFont(fontId))
		{
			return;
		}
	}

	advance.x = 0.0f;
	advance.y = 0.0f;

	// Same rules of drawText() and drawChar().
	const float initialX = advance.x;
	for (uint c = 0; str[c] != '\0'; ++c)
	{
		switch (str[c])
		{
		case '\n' :
			advance.x  = initialX;
			advance.y += builtInFonts[fontId].bmpHeight * globalTextScale; // Every glyph of a font has the same height
			break;

		case '\r' :
			advance.x = initialX;
			break;

		case '\t' :
			advance.x += fntGetWhiteSpaceWidth(fontId) * 4; // 4 spaces for each TAB
			break;

		case ' ' :
			advance.x += fntGetWhiteSpaceWidth(fontId);
			break;

		default :
			if (isgraph(str[c])) // Only increment if char is printable
			{
				const int charIndex = str[c] - builtInFonts[fontId].charStart;
				if (charIndex >= 0 && charIndex < BuiltInFontBitmap::MAX_GLYPHS)
				{
					const BuiltInFontGlyph & glyph = builtInFonts[fontId].glyphs[charIndex];
					advance.x += glyph.w * globalTextScale;
				}
			}
			break;
		} // switch (str[c])
	}
}

// ========================================================
// Renderer::setGlobalTextScale():
// ========================================================

void Renderer::setGlobalTextScale(const float scale)
{
	globalTextScale = scale;
}

// ========================================================
// Renderer::getGlobalTextScale():
// ========================================================

float Renderer::getGlobalTextScale() const
{
	return globalTextScale;
}

// ================================================================================================
// Miscellaneous:
// ================================================================================================

// IDENTITY_MATRIX constant:
const Matrix IDENTITY_MATRIX
(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

// Renderer global instance:
Renderer gRenderer;

// Cleanup the namespace:
#undef BEGIN_DMA_TAG
#undef END_DMA_TAG
#undef BEGIN_DMA_TAG_NAMED
#undef END_DMA_TAG_NAMED
#undef END_DMA_TAG_AND_CHAIN
