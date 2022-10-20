
// ================================================================================================
// -*- C -*-
// File: sound.c
// Author: Guilherme R. Lampert
// Created on: 07/04/15
// Brief: Tiny wrapper around the PS2DEV SDK `audsrv` library. Written in C to allow the
//        `void` variable hack used by the `audsrv_irx` module. C++ doesn't seem to allow that.
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

// STD C:
#include <stdlib.h>

// PS2DEV SDK:
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <audsrv.h>

/*
 * This is the IRX module that runs the sound server and manages the SPU.
 * It is provided by the PS2DEV SDK. For conveniency, I've decided to make it
 * built into the application. The SDK file `audsrv.irx` must be processed with `bin2s`
 * when building the ELF executable. The simplest way is to add this rule to the makefile:
 *
 *   bin2s $(PS2SDK)/iop/irx/audsrv.irx audsrv_irx.s audsrv_irx
 *
 * Then just reference `audsrv_irx.o` in the OBJ list.
 *
 * This file is a C file just to allow the declaration of a `void` variable.
 * This should be the correct type, since `audsrv_irx` is not an array of bytes
 * per-say, but pure block addresses.
 */
extern void audsrv_irx;
extern int  size_audsrv_irx;

/*
 * Miscellaneous local data:
 */
static const char * sndLastErrStr = "";
static audsrv_adpcm_t sndSample   = {0};
static int sndIsInitialized       = 0;

// ========================================================
// sndSetErr():
// ========================================================

static void sndSetErr(const char * errMesssage)
{
	sndLastErrStr = errMesssage;
}

// ========================================================
// soundInit():
// ========================================================

int soundInit()
{
	if (sndIsInitialized)
	{
		sndSetErr("Sound already initialized!");
		return 0;
	}

	if (SifLoadModule("rom0:LIBSD", 0, NULL) <= 0)
	{
		sndSetErr("Failed to load \'rom0:LIBSD\'!");
		return 0;
	}

	int result = SifExecModuleBuffer(&audsrv_irx, size_audsrv_irx, 0, NULL, NULL);

	// This would indicate some internal error that was detected by loadfile.
	if (result <= 0)
	{
		sndSetErr("Failed to load \'audsrv.irx\'! loadfile error!");
		return 0;
	}

	// HACK:
	// This problem only happens on the emulator, AFAIK.
	// It doesn't return a negative error code, instead it returns a
	// huge value like 301312 and refuses to load the module. If I
	// still attempt to audsrv_init() after that the application stalls.
	// I'll assume any value above 1000 is an error, since the IOP
	// doesn't have enough memory to load that many modules.
	//
	if (result > 1000)
	{
		sndSetErr("Failed to load \'audsrv.irx\'! Ludicrous module id!");
		return 0;
	}

	if (audsrv_init() != 0)
	{
		sndSetErr("Failed to init AUDSRV lib!");
		return 0;
	}

	if (audsrv_adpcm_init() != 0)
	{
		sndSetErr("audsrv_adpcm_init() failed!");
		return 0;
	}

	return (sndIsInitialized = 1); // All went well.
}

// ========================================================
// soundPlay():
// ========================================================

int soundPlay(const void * adpcmData, int adpcmDataSize)
{
	if (!sndIsInitialized)
	{
		sndSetErr("Sound not initialized!");
		return 0;
	}

	if (adpcmData == NULL || adpcmDataSize <= 0)
	{
		sndSetErr("Invalid ADPCM data in soundPlay()!");
		return 0;
	}

	if (audsrv_load_adpcm(&sndSample, (void *)adpcmData, adpcmDataSize) != 0)
	{
		sndSetErr("audsrv_load_adpcm() failed!");
		return 0;
	}

	if (audsrv_play_adpcm(&sndSample) != 0)
	{
		sndSetErr("audsrv_play_adpcm() failed!");
		return 0;
	}

	return 1; // Sound should be playing on an available channel.
}

// ========================================================
// soundSetVolume():
// ========================================================

int soundSetVolume(int volume)
{
	if (!sndIsInitialized)
	{
		sndSetErr("Sound not initialized!");
		return 0;
	}

	if (audsrv_set_volume(volume) != 0)
	{
		sndSetErr("audsrv_set_volume() failed!");
		return 0;
	}

	return 1;
}

// ========================================================
// soundStopAll():
// ========================================================

int soundStopAll()
{
	if (!sndIsInitialized)
	{
		sndSetErr("Sound not initialized!");
		return 0;
	}

	if (audsrv_stop_audio() != 0)
	{
		sndSetErr("audsrv_stop_audio() failed!");
		return 0;
	}

	return 1;
}

// ========================================================
// soundGetLastErrorStr():
// ========================================================

const char * soundGetLastErrorStr()
{
	return sndLastErrStr;
}
