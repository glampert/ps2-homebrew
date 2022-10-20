
// ================================================================================================
// -*- C++ -*-
// File: common.cpp
// Author: Guilherme R. Lampert
// Created on: 14/01/15
// Brief: Common functions and shared data.
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

#include "common.hpp"
#include "renderer.hpp"
#include "ingame_console.hpp"

// PS2DEV SDK:
#include <smod.h>
#include <sifrpc.h>
#include <loadfile.h>

// C library:
#include <cstdarg>
#include <time.h>

// ========================================================
// ps2AssertFailed():
// ========================================================

#if USE_CUSTOM_ASSERT
void ps2AssertFailed(const char * cond, const char * filename, const int lineNum)
{
	gConsole.setTextColor(0xFF0000FF); // Red text

	gConsole.print(format(
		"\n**** Assertion failure ****\n"
		"%s\n"
		"In file %s:%d\n",
		cond, filename, lineNum));

	die();
}
#endif // USE_CUSTOM_ASSERT

// ========================================================
// format():
// ========================================================

char * format(const char * fmt, ...)
{
	ps2assert(fmt != nullptr);

	va_list vaList;
	va_start(vaList, fmt);
	char * s = vformat(fmt, vaList);
	va_end(vaList);

	return s;
}

// ========================================================
// vformat():
// ========================================================

char * vformat(const char * fmt, va_list vaList)
{
	ps2assert(fmt != nullptr);

	// Multiple buffers, in case called by nested functions.
	enum { MAX_BUF_CHARS = 2048 };
	static int index = 0;
	static char strings[4][MAX_BUF_CHARS] ATTRIBUTE_ALIGNED(16);

	char * buf = strings[index];
	index = (index + 1) & 3;

	uint charsWritten = scast<uint>(vsnprintf(buf, MAX_BUF_CHARS, fmt, vaList));
	if (charsWritten >= MAX_BUF_CHARS)
	{
		charsWritten = MAX_BUF_CHARS - 1; // Truncate.
	}

	buf[charsWritten] = '\0'; // Ensure null terminated.
	return buf;
}

// ========================================================
// logPrintf():
// ========================================================

void logPrintf(const char * messageTag, const char * filename, const int lineNum, const char * fmt, ...)
{
	// Supply defaults if needed:
	if (messageTag == nullptr) { messageTag = "???"; }
	if (filename   == nullptr) { filename   = "???"; }
	if (fmt        == nullptr) { fmt        = "???"; }

	// Set proper text color according to message type:
	switch (*messageTag)
	{
	case 'C' : // Comment/info message, white text:
		gConsole.setTextColor(0xFFFFFFFF);
		break;
	case 'W' : // Warning message, yellow text:
		gConsole.setTextColor(0xFFFF00FF);
		break;
	case 'E' : // Error message, red text:
		gConsole.setTextColor(0xFF0000FF);
		break;
	default : // Unknown token, but accept it anyway.
		gConsole.restoreTextColor();
		break;
	} // switch (*messageTag)

	// (Optional) message prefix: "<TAG> filename lineNum : "
	#if LOG_PRINTF_ADD_MESSAGE_PREFIX
	const char * trimmedFilename = strrchr(filename, '/');
	gConsole.print(format("<%s> %-15.14s%-4d: ", messageTag,
		(trimmedFilename != nullptr) ? trimmedFilename + 1 : filename, lineNum));
	#endif // LOG_PRINTF_ADD_MESSAGE_PREFIX

	// The message itself:
	va_list vaList;
	va_start(vaList, fmt);
	gConsole.print(vformat(fmt, vaList));
	va_end(vaList);

	// A default line break is implicit.
	gConsole.print("\n");

	// Play nice and restore the default text color.
	gConsole.restoreTextColor();
}

// ========================================================
// die():
// ========================================================

void die()
{
	fprintf(stderr, "\'die()\' called! Aborting due to a fatal error or assertion failure...\n");

	if (!gRenderer.isVideoInitialized())
	{
		// Early error, before full renderer initialization.
		// Attempt a quick init anyway, so we might still get
		// a chance of displaying the error...
		//
		gRenderer.initVideoGraphics(640, 512, GRAPH_MODE_AUTO, GS_PSM_32, GS_ZBUF_32, true);
		gConsole.preloadFonts();
	}
	else
	{
		gRenderer.resetFrameStates();
	}

	for (;;)
	{
		gRenderer.beginFrame();
		gRenderer.clearScreen();

		// Draw the console with whatever messages where last printed
		// and draw a fatal error message at the bottom to make our
		// situation clear to the user.
		//
		gRenderer.begin2d();
		{
			gRenderer.setGlobalTextScale(0.67f);
			gConsole.draw();

			Vec2f pos;
			pos.x = 50.0f;
			pos.y = 10.0f + (gConsole.getHeight() * gRenderer.getGlobalTextScale());
			gRenderer.setGlobalTextScale(1.5f);
			gRenderer.drawText(pos, makeColor4b(255, 0, 0, 255), FONT_CONSOLAS_24, "FATAL ERROR, ABORTING!\n");
		}
		gRenderer.end2d();

		gRenderer.endFrame();
		SleepThread();
	}
}

// ========================================================
// removeTrailingFloatZeros():
// ========================================================

char * removeTrailingFloatZeros(char * str, size_t * newLen)
{
	ps2assert(str != nullptr);

	register char * p = str;
	size_t n = 0;

	for (; *p; ++p, ++n)
	{
		if (*p == '.')
		{
			while (*++p) // Find the end of the string
			{
				++n;
			}
			++n;
			while (*--p == '0') // Remove trailing zeros
			{
				*p = '\0', --n;
			}
			if (*p == '.') // If the dot was left alone at the end, remove it to
			{
				*p = '\0', --n;
			}
			break;
		}
	}

	if (newLen)
	{
		*newLen = n;
	}
	return str;
}

// ========================================================
// millisecondsSinceStartup():
// ========================================================

uint millisecondsSinceStartup()
{
	return clock() / (CLOCKS_PER_SEC / 1000);
}

// ========================================================
// clockMilliseconds():
// ========================================================

uint clockMilliseconds()
{
	static uint baseTime         = 0;
	static uint currentTime      = 0;
	static bool timerInitialized = false;

	if (!timerInitialized)
	{
		baseTime = millisecondsSinceStartup();
		timerInitialized = true;
	}

	currentTime = millisecondsSinceStartup() - baseTime;
	return currentTime;
}

// ========================================================
// loadBinaryFile():
// ========================================================

DataBlob loadBinaryFile(const char * filename)
{
	ps2assert(filename != nullptr);
	DataBlob fileContents = { nullptr, 0 };

	const int fd = fioOpen(filename, O_RDONLY);
	if (fd < 0)
	{
		logWarning("Failed to open file \"%s\"! (fd=%d)", filename, fd);
		return fileContents;
	}

	// Get size in bytes:
	const int fileSize = fioLseek(fd, 0, SEEK_END);
	if (fileSize <= 0)
	{
		logWarning("File \"%s\" was empty or fioLseek error! (size=%d)", filename, fileSize);
		fioClose(fd);
		return fileContents;
	}

	// Rewind:
	fioLseek(fd, 0, SEEK_SET);

	// Data might be used on a DMA transfer, so make it properly aligned:
	fileContents.data = memAlloc<ubyte>(MEM_TAG_GENERIC, fileSize, 128);
	if (fileContents.data == nullptr)
	{
		logWarning("loadBinaryFile: Out-of-memory!");
		fioClose(fd);
		return fileContents;
	}

	const int numRead = fioRead(fd, fileContents.data, fileSize);
	if (numRead != fileSize)
	{
		logWarning("fioRead: Requested %d bytes, got %d", fileSize, numRead);
	}
	fileContents.size = numRead;

	fioClose(fd);
	return fileContents;
}

// ========================================================
// initIop():
// ========================================================

void initIop()
{
	SifInitRpc(0);
	fioInit();
}

// ========================================================
// isIopModuleLoaded():
// ========================================================

bool isIopModuleLoaded(const char * moduleName)
{
	ps2assert(moduleName  != nullptr);
	ps2assert(*moduleName != '\0');

	smod_mod_info_t moduleInfo;
	return smod_get_mod_by_name(moduleName, &moduleInfo) > 0;
}

// ========================================================
// loadExecIopModule():
// ========================================================

bool loadExecIopModule(const char * irxFilePath)
{
	ps2assert(irxFilePath  != nullptr);
	ps2assert(*irxFilePath != '\0');

	// If trying to load a stock IRX from ROM, do it
	// directly with `SifLoadModule()`:
	//
	if (strncmp(irxFilePath, "rom", 3) == 0)
	{
		const int moduleId = SifLoadModule(irxFilePath, 0, nullptr);
		if (moduleId <= 0)
		{
			logError("Failed to load ROM module %s! res=%d", irxFilePath, moduleId);
			return false;
		}

		logComment("ROM module %s loaded successfully! id=%d", irxFilePath, moduleId);
		return true;
	}

	// Load into memory first, then execute.
	// This will ensure modules loaded from an USB drive
	// or equivalent get loaded and executed properly.
	//
	DataBlob irx = loadBinaryFile(irxFilePath);
	if (irx.data == nullptr || irx.size == 0)
	{
		logError("Failed to load IOP module file %s!", irxFilePath);
		return false;
	}

	int startupResult = 0;
	const int moduleId = SifExecModuleBuffer(irx.data, irx.size, 0, nullptr, &startupResult);
	if (moduleId <= 0)
	{
		logError("Failed to execute IOP module %s! res=%d", irxFilePath, moduleId);
		memFree(MEM_TAG_GENERIC, irx.data);
		return false;
	}

	logComment("IOP module %s loaded! id=%d, res=%d", irxFilePath, moduleId, startupResult);
	memFree(MEM_TAG_GENERIC, irx.data);
	return true;
}

// ========================================================
// printLoadedIopModules():
// ========================================================

void printLoadedIopModules(const int maxModules)
{
	static char moduleNameStr[128] ATTRIBUTE_ALIGNED(64);

	smod_mod_info_t moduleInfo;
	SifRpcReceiveData_t rcpData;
	int evenOdd = 0, listedCount = 0;

	if (!smod_get_next_mod(nullptr, &moduleInfo))
	{
		logError("Couldn't get module list!");
		return;
	}

	// Table header:
	// (Print two tables side-by side, since our console has very few lines).
	gConsole.print(
		"|    iop-module-name    | version | id "
		"|    iop-module-name    | version | id \n");

	moduleNameStr[64] = '\0';
	do
	{
		SyncDCache(moduleNameStr, moduleNameStr + 64);

		if (SifRpcGetOtherData(&rcpData, moduleInfo.name, moduleNameStr, 64, 0) >= 0)
		{
			if (moduleNameStr[0] == '\0')
			{
				// Unnamed module.
				strcpy(moduleNameStr, "???");
			}
			else
			{
				// Truncate to 21 chars, the size of the name column:
				moduleNameStr[21] = '\0';
			}

			// Expand version word into major.minor:
			char moduleVersionStr[64];
			sprintf(moduleVersionStr, "%d.%d",
				(moduleInfo.version & 0xFF00) >> 8, // Major version number
				(moduleInfo.version & 0x00FF));     // Minor version number

			// Print a table row (we print two tables side-by-side to save lines):
			if (!(evenOdd++ & 1))
			{
				gConsole.print(format("| %-21s | %-7s | %-2u |",
					moduleNameStr, moduleVersionStr, moduleInfo.id));
			}
			else
			{
				gConsole.print(format(" %-21s | %-7s | %-2u |\n",
					moduleNameStr, moduleVersionStr, moduleInfo.id));
			}

			if (++listedCount == maxModules)
			{
				break;
			}
		}
	}
	while (smod_get_next_mod(&moduleInfo, &moduleInfo) != 0);

	if (evenOdd & 1)
	{
		gConsole.print("\n");
	}
	gConsole.print(format("listed %u modules\n", listedCount));
}

// ========================================================
// Pseudo-random number generators:
// ========================================================

uint randomInt()
{
	// Initial constants presented on this paper:
	//  http://www.jstatsoft.org/v08/i14/paper
	static uint x = 123456789u;
	static uint y = 362436069u;
	static uint z = 521288629u;
	static uint w = 88675123u;

	// XOR-Shift-128 RNG:
	uint t = x ^ (x << 11);
	x = y;
	y = z;
	z = w;
	w = w ^ (w >> 19) ^ t ^ (t >> 8);

	return w;
}

float randomFloat()
{
	return scast<float>(randomInt()) / UINT_MAX;
}

int randomInt(const int lowerBound, const int upperBound)
{
	if (lowerBound == upperBound)
	{
		return lowerBound;
	}
	return (randomInt() % (upperBound - lowerBound)) + lowerBound;
}

float randomFloat(const float lowerBound, const float upperBound)
{
	return (randomFloat() * (upperBound - lowerBound)) + lowerBound;
}
