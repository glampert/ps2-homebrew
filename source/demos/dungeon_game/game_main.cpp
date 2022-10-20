
// ================================================================================================
// -*- C++ -*-
// File: game_main.cpp
// Author: Guilherme R. Lampert
// Created on: 20/01/15
// Brief: A mini dungeon crawling game. Inspired by classics like Diablo and Dungeon Siege.
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

#include "game_world.hpp"
#include "framework/sound.hpp"
#include "framework/game_time.hpp"
#include "framework/ingame_console.hpp"

// ========================================================
// main():
// ========================================================

int main()
{
	initIop();
	soundInit();

	if (!gRenderer.initVideoGraphics(
		640, 448,        // 640x448 is the standard NTSC resolution
		GRAPH_MODE_AUTO, // Select best mode (PAL or NTSC)
		GS_PSM_32,       // Framebuffer color format
		GS_PSMZ_32,      // Z-buffer format
		true))           // Interlace ON
	{
		fatalError("Failed to initialized the graphics subsystems!");
	}

	// Default render states that never change:
	gRenderer.setClearScreenColor(0, 0, 0);
	gRenderer.setGlobalTextScale(0.67f);
	gRenderer.setPrimTextureMapping(true);
	gRenderer.setPrimAntialiasing(true);
	gRenderer.setPrimShading(PRIM_SHADE_GOURAUD);
	gConsole.preloadFonts();

	// Game instance initialization. `GameWorld` is quite
	// big, so it is best to allocate it dynamically.
	GameWorld * gameWorld = new(MEM_TAG_GENERIC) GameWorld();

	// Main game loop, until the PS2 is rebooted:
	for (;;)
	{
		// Update the global clock:
		gTime.beginFrame();

		// Game Logic update:
		gameWorld->updateGameLogic();

		// Rendering:
		gRenderer.beginFrame();
		{
			gRenderer.clearScreen();
			gameWorld->renderFrame3d();
			gameWorld->renderUiOverlays();
			gameWorld->renderMisc2d();
		}
		gRenderer.endFrame();

		// Update time deltas:
		gTime.endFrame();
	}
}
