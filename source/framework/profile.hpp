
// ================================================================================================
// -*- C++ -*-
// File: profile.hpp
// Author: Guilherme R. Lampert
// Created on: 10/04/15
// Brief: Very simple macros to aid profiling and benchmarking the application.
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

#ifndef PROFILE_HPP
#define PROFILE_HPP

#include "common.hpp"
#include "renderer.hpp"

//
// Define `PROFILE_ENABLED` to non-zero before
// including this file to enable the profiling
// macros. If that is not defined, the macros
// default to no-ops.
//
#if PROFILE_ENABLED

// ========================================================

#define PROFILE_DECL(prof, printName) ProfileTimes prof = { 0,0,0, printName }

#define PROFILE_BEGIN(prof) \
do { \
	(prof).start = clockMilliseconds(); \
} while (0)

#define PROFILE_END(prof) \
do { \
	(prof).end   = clockMilliseconds(); \
	(prof).delta = (prof).end - (prof).start; \
} while (0)

#define PROFILE_PRINT(pos, ...) profilePrintN((pos), __VA_ARGS__, nullptr)

// ========================================================

struct ProfileTimes
{
	// In milliseconds!
	uint start;
	uint end;
	uint delta;
	const char * name;
};

inline void profilePrintN(Vec2f & pos, const ProfileTimes * first, ...)
{
	const Color4b white = { 255, 255, 255, 255 };

	va_list vaList;
	const ProfileTimes * ptr = first;

	va_start(vaList, first);
	while (ptr != nullptr)
	{
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24,
			format("%s ms: %u\n", ptr->name, ptr->delta));

		ptr = va_arg(vaList, const ProfileTimes *);
	}
	va_end(vaList);
}

// ========================================================

#else // !PROFILE_ENABLED

// All macros are no-ops:
#define PROFILE_DECL(prof, printName)
#define PROFILE_BEGIN(prof)
#define PROFILE_END(prof)
#define PROFILE_PRINT(...)

#endif // PROFILE_ENABLED

#endif // PROFILE_HPP
