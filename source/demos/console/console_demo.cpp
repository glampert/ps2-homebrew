
// ================================================================================================
// -*- C++ -*-
// File: console_demo.cpp
// Author: Guilherme R. Lampert
// Created on: 20/01/15
// Brief: Very simple demo that showcases the in-game developer console and game pad input.
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
#include "framework/renderer.hpp"
#include "framework/ingame_console.hpp"

// ========================================================

void testGamePad(GamePad * gamePad)
{
	using namespace padlib;

	static bool messagePrinted = false;
	if (gamePad == nullptr)
	{
		if (!messagePrinted)
		{
			logError("Failed to connect with Game Controller at slot 1 (pad0)!");
			messagePrinted = true;
		}
		return;
	}

	if (!gamePad->update() || !gamePad->isStable())
	{
		logWarning("Failed to update pad0 | unstable connection!");
		return;
	}

	const padlib::PadButtonStates & buttons = gamePad->getButtonStates();

	// Main buttons:
	//
	if (gamePad->isDown(PAD_LEFT))     { logComment("LEFT");   }
	if (gamePad->isDown(PAD_RIGHT))    { logComment("RIGHT");  }
	if (gamePad->isDown(PAD_UP))       { logComment("UP");     }
	if (gamePad->isDown(PAD_DOWN))     { logComment("DOWN");   }
	if (gamePad->isDown(PAD_START))    { logComment("START");  }
	if (gamePad->isDown(PAD_SELECT))   { logComment("SELECT"); }
	if (gamePad->isDown(PAD_SQUARE))   { logComment("SQUARE   (Pressure: %i)", scast<int>(buttons.square_p));   }
	if (gamePad->isDown(PAD_TRIANGLE)) { logComment("TRIANGLE (Pressure: %i)", scast<int>(buttons.triangle_p)); }
	if (gamePad->isDown(PAD_CIRCLE))   { logComment("CIRCLE   (Pressure: %i)", scast<int>(buttons.circle_p));   }
	if (gamePad->isDown(PAD_CROSS))    { logComment("CROSS    (Pressure: %i)", scast<int>(buttons.cross_p));    }

	// L:
	//
	if (gamePad->isDown(PAD_L1))
	{
		logComment("L1 (Start Little Motor)");
		gamePad->startActuator(PAD_ACT_SMALL_MOTOR, 1);
	}
	if (gamePad->isDown(PAD_L2))
	{
		logComment("L2 (Stop Little Motor)");
		gamePad->stopActuator(PAD_ACT_SMALL_MOTOR);
	}
	if (gamePad->isDown(PAD_L3))
	{
		logComment("L3");
	}

	// R:
	//
	if (gamePad->isDown(PAD_R1))
	{
		logComment("R1 (Start Big Motor)");
		gamePad->startActuator(PAD_ACT_BIG_MOTOR, 255);
	}
	if (gamePad->isDown(PAD_R2))
	{
		logComment("R2 (Stop Big Motor)");
		gamePad->stopActuator(PAD_ACT_BIG_MOTOR);
	}
	if (gamePad->isDown(PAD_R3))
	{
		logComment("R3");
	}
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

	// Clear the background to a light brown color and set console text scale:
	gRenderer.setClearScreenColor(65, 25, 0);
	gRenderer.setGlobalTextScale(0.67f);

	// Pre-loading the fonts is a good idea to avoid frame-rate drops.
	gConsole.preloadFonts();

	// Init game controller 0 (the first slot). Will get null if no controller attached.
	GamePad * gamePad = initGamePad(0, true, 200);

	// Main should not return. To quit a game, the system must be rebooted.
	logComment("==== Welcome to the Console/Input demo! ====");
	for (;;)
	{
		// Poll pad input and print some text:
		testGamePad(gamePad);

		// Start a new render frame:
		gRenderer.beginFrame();

		// Clear screen to default color:
		gRenderer.clearScreen();

		// 3D rendering would go inside this block.
		// Since there is nothing to render, no need to call begin/end.
		//
		//gRenderer.begin3d();
		//{
		//    renderMy3dGame();
		//}
		//gRenderer.end3d();

		// Any 2D drawing must be inside this block:
		//
		gRenderer.begin2d();
		{
			gConsole.draw();
			gRenderer.drawFpsCounter();
			gRenderer.drawFrameStats();
		}
		gRenderer.end2d();

		// Flush buffers / wait V-sync.
		gRenderer.endFrame();
	}
}
