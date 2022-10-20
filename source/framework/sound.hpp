
// ================================================================================================
// -*- C++ -*-
// File: sound.hpp
// Author: Guilherme R. Lampert
// Created on: 07/04/15
// Brief: Tiny sound library based on AUDSRV lib from the PS2DEV SDK.
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

#ifndef SOUND_HPP
#define SOUND_HPP

extern "C"
{

/*
 * Volume references:
 */
enum
{
	SOUND_MIN_VOLUME = 0x0000,
	SOUND_MAX_VOLUME = 0x3fff
};

/*
 * Initializes the sound library and IOP modules.
 * Must be called before using any of the following
 * functions. Return 0 on error and sets the global
 * error string. Non-zero on success.
 */
int soundInit();

/*
 * Starts playing an ADPCM encoded sound buffer.
 * Return 0 on error and sets the global
 * error string. Non-zero on success.
 */
int soundPlay(const void * adpcmData, int adpcmDataSize);

/*
 * Set the global volume.
 * Return 0 on error and sets the global
 * error string. Non-zero on success.
 */
int soundSetVolume(int volume);

/*
 * Stops all sound currently playing.
 * Return 0 on error and sets the global
 * error string. Non-zero on success.
 */
int soundStopAll();

/*
 * Get the current error string.
 * Returns an empty string if no error has occurred.
 * The returned string points to static memory and
 * should never be deallocated!
 */
const char * soundGetLastErrorStr();

} // extern "C"

#endif // SOUND_HPP
