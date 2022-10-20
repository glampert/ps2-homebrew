
// ================================================================================================
// -*- C++ -*-
// File: play_adpcm_demo.cpp
// Author: Guilherme R. Lampert
// Created on: 07/04/15
// Brief: Simple demo that loads and plays an ADPCM encoded sound file.
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
#include "framework/sound.hpp"
#include "framework/renderer.hpp"
#include "framework/ingame_console.hpp"

// The sound sample: A WAV converted to ADPCM with
// `adpenc` (tool provided by the PS2DEV SDK) and then
// dumped as a C-style array of bytes using `bin2c`.
#include "chiller_16bit_8k_mono.h"

// ========================================================
// main():
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
	gConsole.preloadFonts();

	// Start playing a sound.
	// We have created the ADPCM with loop enabled,
	// so it will continue playing forever once started.
	//
	logComment("==== Welcome to the Audio/Sound demo! ====");

	if (soundInit())
	{
		soundSetVolume(SOUND_MAX_VOLUME);

		if (soundPlay(chiller_16bit_8k_mono, size_chiller_16bit_8k_mono))
		{
			logComment("Playing a sound loop at full volume ...");
		}
		else
		{
			logError("soundPlay() failed: %s", soundGetLastErrorStr());
		}
	}
	else
	{
		logError("soundInit() failed: %s", soundGetLastErrorStr());
	}

	logComment("==========================================");

	// Main should not return.
	// To quit a game, the system must be rebooted.
	for (;;)
	{
		// Start a new render frame:
		gRenderer.beginFrame();

		// Clear screen to default color:
		gRenderer.clearScreen();

		// Any 2D drawing must be inside this block:
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
