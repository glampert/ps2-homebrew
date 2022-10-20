
// ================================================================================================
// -*- C++ -*-
// File: cubes_demo.cpp
// Author: Guilherme R. Lampert
// Created on: 20/01/15
// Brief: Simple demo that showcases 3D drawing with texture and 3D camera movement.
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

#include "framework/common.hpp"
#include "framework/game_pad.hpp"
#include "framework/game_time.hpp"
#include "framework/renderer.hpp"
#include "framework/ingame_console.hpp"
#include "framework/first_person_camera.hpp"

// ========================================================

struct Cube
{
	static const uint VERT_COUNT  = 24;
	static const uint INDEX_COUNT = 36;

	DrawVertex * vertexes;
	uint16     * indexes;
};

// ========================================================

Cube makeCube(const Vector & color, const float scale)
{
	// Face indexes:
	static const uint16 cubeF[6][4] =
	{
		{ 0, 1, 5, 4 }, { 4, 5, 6, 7 }, { 7, 6, 2, 3 },
		{ 1, 0, 3, 2 }, { 1, 2, 6, 5 }, { 0, 4, 7, 3 }
	};

	// Positions/vertexes:
	static const float cubeV[8][3] =
	{
		{ -0.5f, -0.5f, -0.5f },
		{ -0.5f, -0.5f,  0.5f },
		{  0.5f, -0.5f,  0.5f },
		{  0.5f, -0.5f, -0.5f },

		{ -0.5f,  0.5f, -0.5f },
		{ -0.5f,  0.5f,  0.5f },
		{  0.5f,  0.5f,  0.5f },
		{  0.5f,  0.5f, -0.5f },
	};

	// Tex coords:
	static const float cubeT[4][2] =
	{
		{ 0.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f }
	};

	// Allocate a new data for a new cube:
	Cube cube;
	cube.vertexes = new(MEM_TAG_GEOMETRY) DrawVertex[Cube::VERT_COUNT];
	cube.indexes  = new(MEM_TAG_GEOMETRY) uint16[Cube::INDEX_COUNT];

	// Fill in the data:
    DrawVertex * restrict vertexPtr = cube.vertexes;
    uint16     * restrict facePtr   = cube.indexes;

	// `i` iterates over the faces, 2 triangles per face:
	uint16 vertIndex = 0;
	for (uint i = 0; i < 6; ++i, vertIndex += 4)
	{
		for (uint j = 0; j < 4; ++j)
		{
			vertexPtr->position.x = cubeV[cubeF[i][j]][0] * scale;
			vertexPtr->position.y = cubeV[cubeF[i][j]][1] * scale;
			vertexPtr->position.z = cubeV[cubeF[i][j]][2] * scale;
			vertexPtr->position.w = 1.0f;

			vertexPtr->texCoord.x = cubeT[j][0];
			vertexPtr->texCoord.y = cubeT[j][1];
			vertexPtr->texCoord.z = 0.0f;
			vertexPtr->texCoord.w = 1.0f;

			vertexPtr->color = color;
			++vertexPtr;
		}

		facePtr[0] = vertIndex;
		facePtr[1] = vertIndex + 1;
		facePtr[2] = vertIndex + 2;
		facePtr += 3;

		facePtr[0] = vertIndex + 2;
		facePtr[1] = vertIndex + 3;
		facePtr[2] = vertIndex;
		facePtr += 3;
	}

	return cube;
}

// ========================================================

void makeCheckerPatternTexture(Texture & texture)
{
	const int NUM_SQUARES  = 8;   // Number of checker squares
	const int IMG_SIZE     = 256; // Square size of the texture
	const int CHECKER_SIZE = IMG_SIZE / NUM_SQUARES; // Size of one checker square, in pixels.
	static Color4b data[IMG_SIZE * IMG_SIZE] ATTRIBUTE_ALIGNED(16);

	// Black/white:
	const Color4b pattern[] =
	{
		{ 0,   0,   0,   255 },
		{ 255, 255, 255, 255 }
	};

	int startY    = 0;
	int lastColor = 0;
	int color, rowX;

	while (startY < IMG_SIZE)
	{
		for (int y = startY; y < (startY + CHECKER_SIZE); ++y)
		{
			color = lastColor, rowX = 0;
			for (int x = 0; x < IMG_SIZE; ++x)
			{
				if (rowX == CHECKER_SIZE)
				{
					// Invert color every time we complete a checker square:
					color = !color;
					rowX  = 0;
				}
				data[x + y * IMG_SIZE] = pattern[color];
				++rowX;
			}
		}
		startY += CHECKER_SIZE;
		lastColor = !lastColor;
	}

	// NOTE: Texture NOT mip-mapped!
	//
	lod_t lod;
	lod.calculation = LOD_USE_K;
	lod.max_level   = 0;
	lod.mag_filter  = LOD_MAG_LINEAR;
	lod.min_filter  = LOD_MIN_LINEAR;
	lod.l           = 0;
	lod.k           = 0.0f;
	if (!texture.initFromMemory(rcast<ubyte *>(data), TEXTURE_COMPONENTS_RGBA,
			IMG_SIZE, IMG_SIZE, GS_PSM_32, TEXTURE_FUNCTION_MODULATE, &lod))
	{
		fatalError("Failed to init Texture!");
	}
}

// ========================================================

Texture gTexture;
Matrix  gViewMatrix;
Matrix  gProjectionMatrix;
FirstPersonCamera gCamera;

Cube gRedCube;
Cube gWhiteCube;
Cube gGreenCube;

GamePad * gGamePad      = nullptr;
bool      gShowConsole  = false;
float     gCubeRotation = 0.0f;

// ========================================================

void demoSetup()
{
	// Create a few cubes:
	//
	gRedCube   = makeCube(Vector(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);
	gWhiteCube = makeCube(Vector(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);
	gGreenCube = makeCube(Vector(0.0f, 1.0f, 0.0f, 1.0f), 2.0f);

	// Set projection:
	//
	gViewMatrix.makeIdentity();
	gProjectionMatrix.makePerspectiveProjection(degToRad(60.0f), gRenderer.getAspectRatio(),
			gRenderer.getScreenWidth(), gRenderer.getScreenHeight(), 2.0f, 1000.0f);

	// A texture for the cubes:
	//
	makeCheckerPatternTexture(gTexture);
}

// ========================================================

void padInput()
{
	static bool selectDown = false;
	if (gGamePad->isDown(padlib::PAD_SELECT))
	{
		selectDown = true;
	}
	if (gGamePad->isUp(padlib::PAD_SELECT) && selectDown)
	{
		selectDown = false;
		gShowConsole = !gShowConsole;
	}

	// Only update the camera if there is input.
	gCamera.update(*gGamePad, Vector(0.0f));
	gRenderer.setEyePosition(gCamera.getEyePosition());
}

// ========================================================

void drawCubeSet(const Cube & cube, const float y)
{
	for (uint z = 0; z < 3; ++z)
	{
		for (uint x = 0; x < 3; ++x)
		{
			Matrix trans, rotX, rotY;
			trans.makeTranslation((x * 5.0f) - 5.0f, y, (z * 5.0f) + 20.0f);
			rotX.makeRotationX(degToRad(gCubeRotation));
			rotY.makeRotationY(degToRad(gCubeRotation));

			gRenderer.setModelMatrix(rotX * rotY * trans);
			gRenderer.drawIndexedTriangles(cube.indexes, Cube::INDEX_COUNT, cube.vertexes, Cube::VERT_COUNT);
		}
	}
}

// ========================================================

void drawCubes()
{
	gViewMatrix = gCamera.getViewMatrix();
	gRenderer.setViewProjMatrix(gViewMatrix * gProjectionMatrix);

	gCubeRotation += 0.3f;
	if (gCubeRotation >= 360.0f)
	{
		gCubeRotation = 0.0f;
	}

	gRenderer.setTexture(gTexture);
	drawCubeSet(gRedCube,  -5.0f);
	drawCubeSet(gWhiteCube, 0.0f);
	drawCubeSet(gGreenCube, 5.0f);
}

// ========================================================

int main()
{
	initIop();

	if (!gRenderer.initVideoGraphics(
		640, 448,        // 640x448 is the standard NTSC resolution
		GRAPH_MODE_AUTO, // Select best mode (PAL or NTSC)
		GS_PSM_32,       // Framebuffer color format
		GS_PSMZ_32,      // Z-buffer format
		true))           // Interlace ON
	{
		fatalError("Failed to initialized the graphics subsystems!");
	}

	gRenderer.setClearScreenColor(65, 25, 0);
	gRenderer.setGlobalTextScale(0.67f);
	gRenderer.setPrimTextureMapping(true);
	gRenderer.setPrimAntialiasing(true);
	gRenderer.setPrimShading(PRIM_SHADE_FLAT);

	gConsole.preloadFonts();

	// Init game controller 0 (the first slot).
	// Will get a null if no controller attached.
	gGamePad = initGamePad(0, true, 200);

	// Other misc initialization for this demo app:
	demoSetup();

	// Enter the game loop:
	logComment("==== Welcome to the Cubes demo! ====");
	for (;;)
	{
		gTime.beginFrame();

		if (gGamePad != nullptr)
		{
			if (!gGamePad->update())
			{
				logWarning("Failed to update Game Controller! Connection lost?");
			}
			padInput();
		}

		gRenderer.beginFrame();
		gRenderer.clearScreen();

		gRenderer.begin3d();
		{
			drawCubes();
		}
		gRenderer.end3d();

		gRenderer.begin2d();
		{
			if (gShowConsole)
			{
				gConsole.draw();
			}
			else
			{
				Vec2f pos = { 5.0f, 5.0f };
				gRenderer.drawText(pos, makeColor4b(255, 255, 255),
					FONT_CONSOLAS_24, "Press [SELECT] to toggle the console");
			}

			gRenderer.drawFpsCounter();
			gRenderer.drawFrameStats();
		}
		gRenderer.end2d();

		gRenderer.endFrame();

		gTime.endFrame();
	}
}
