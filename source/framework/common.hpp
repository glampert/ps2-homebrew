
// ================================================================================================
// -*- C++ -*-
// File: common.hpp
// Author: Guilherme R. Lampert
// Created on: 10/01/15
// Brief: Common includes and declarations.
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

#ifndef COMMON_HPP
#define COMMON_HPP

// ========================================================
// Common includes:
// ========================================================

// PS2DEV SDK:
#include <kernel.h>
#include <tamtypes.h>

// C/C++ standard libraries:
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// new/delete overrides and custom allocators:
#include "memory.hpp"

// ========================================================
// Type aliases:
// ========================================================

// Sized integers (base types from `tamtypes.h`):
typedef u8  uint8;
typedef u16 uint16;
typedef u32 uint32;
typedef u64 uint64;
typedef s8  int8;
typedef s16 int16;
typedef s32 int32;
typedef s64 int64;
typedef u32 uint;
typedef u8  ubyte;

// Ensure sizes are correct:
#define CT_ASSERT_SIZE(type, size) typedef int ct_assert_ ## type ## _size[(sizeof(type) == (size)) ? 1 : -1]
CT_ASSERT_SIZE(uint8   ,  1);
CT_ASSERT_SIZE(uint16  ,  2);
CT_ASSERT_SIZE(uint32  ,  4);
CT_ASSERT_SIZE(uint64  ,  8);
CT_ASSERT_SIZE(int8    ,  1);
CT_ASSERT_SIZE(int16   ,  2);
CT_ASSERT_SIZE(int32   ,  4);
CT_ASSERT_SIZE(int64   ,  8);
CT_ASSERT_SIZE(uint    ,  4);
CT_ASSERT_SIZE(ubyte   ,  1);
CT_ASSERT_SIZE(qword_t , 16);
#undef CT_ASSERT_SIZE

// ========================================================
// Miscellaneous macros:
// ========================================================

// C++ style casts are so damn verbose.
#define ccast const_cast
#define rcast reinterpret_cast
#define scast static_cast

// C++ uses the underscore prefixed name, which is an ugly sombitch to type.
#define restrict __restrict

// Ensures structure/union is byte packed (disallows compiler padding).
#define ATTRIBUTE_PACKED __attribute__((packed))

// Ensures structure/class is properly aligned to a give boundary.
#define ATTRIBUTE_ALIGNED(alignment) __attribute__((aligned(alignment)))

// Custom assert facility that prints to the screen and calls `die()`.
// Can be disabled and/or redirected to the standard `assert()` macro.
#if !defined(DISABLE_ASSERTIONS)
	#if USE_CUSTOM_ASSERT
		void ps2AssertFailed(const char * cond, const char * filename, int lineNum) __attribute__((noreturn));
		#define ps2assert(cond) if (!(cond)) { ps2AssertFailed(#cond, __FILE__, __LINE__); }
	#else // !USE_CUSTOM_ASSERT
		#include <cassert>
		#define ps2assert assert
	#endif // USE_CUSTOM_ASSERT
#else // DISABLE_ASSERTIONS defined
	#define ps2assert(cond) /* nothing */
#endif // DISABLE_ASSERTIONS

// ========================================================
// Logging macros / string formatting:
// ========================================================

// Strip trailing zeros from a string representing a floating-point number.
char * removeTrailingFloatZeros(char * str, size_t * newLen);

// Formats text into a statically allocated string.
// Similar functionality to `std::sprintf()`. NOT thread-safe!
char * format(const char * fmt, ...) __attribute__((format(printf, 1, 2)));

// Formats text into a statically allocated string.
// Similar functionality to `std::vsprintf()`. NOT thread-safe!
char * vformat(const char * fmt, va_list vaList) __attribute__((format(printf, 1, 0)));

// Generic log printing. This function is simplified with the
// following macros that provide default arguments for file and line numbers.
void logPrintf(const char * messageTag, const char * filename, int lineNum,
               const char * fmt, ...) __attribute__((format(printf, 4, 5)));

// Crashes the application with an error screen (if possible).
// This function never returns to the caller.
void die() __attribute__((noreturn));

// Macros that simplify the use of `logPrintf()`:
#define logComment(...) ::logPrintf("C", __FILE__, __LINE__, __VA_ARGS__)
#define logWarning(...) ::logPrintf("W", __FILE__, __LINE__, __VA_ARGS__)
#define logError(...)   ::logPrintf("E", __FILE__, __LINE__, __VA_ARGS__)

// Logs a fatal error and crashes spectacularly:
#define fatalError(...) do { ::logPrintf("E", __FILE__, __LINE__, __VA_ARGS__); ::die(); } while (0)

// ========================================================
// Timer routines:
// ========================================================

// Milliseconds elapsed since the application started.
// (Time since the last system reboot).
uint millisecondsSinceStartup();

// Get the elapsed time in milliseconds since the
// last call to this function. NOT thread-safe!
uint clockMilliseconds();

// ========================================================
// IOP helpers / File loading / misc IO:
// ========================================================

// Initializes common IOP features and modules.
// Can be called once `main()` enters to perform early initialization.
// If never called, first IO operation will implicitly call it.
void initIop();

// Test if an IOP module is already loaded. `moduleName` is a name such as "sio2man".
bool isIopModuleLoaded(const char * moduleName);

// Load and IRX module into IOP memory and execute.
// `irxFilePath` must be the full path or filename; example: "mass:irx/freesio2.irx".
bool loadExecIopModule(const char * irxFilePath);

// Print a formatted list with names of loaded IOP module and
// their ids/versions to the developer console.
void printLoadedIopModules(int maxModules = 100);

// ========================================================
// File loading:
// ========================================================

struct DataBlob
{
	ubyte * data;
	uint    size;
};

// Load a file into a memory buffer allocated with `memAlloc(align=128)`.
// Use `memFree()` to cleanup. Path should be properly prefixed with
// source device I.e.: "mass:", "rom0:", etc.
DataBlob loadBinaryFile(const char * filename);

// ========================================================
// Useful POD types:
// ========================================================

struct ATTRIBUTE_PACKED Vec2f
{
	float x, y;
};

struct ATTRIBUTE_PACKED Vec2i
{
	int x, y;
};

struct ATTRIBUTE_PACKED Vec3f
{
	float x, y, z;
};

struct ATTRIBUTE_PACKED Vec3i
{
	int x, y, z;
};

struct ATTRIBUTE_PACKED Rect4i
{
	int x, y;
	int width, height;
};

struct ATTRIBUTE_PACKED Color4f
{
	float r, g, b, a;
};

struct ATTRIBUTE_PACKED Color4b
{
	ubyte r, g, b, a;
};

// ========================================================
// Misc inline helpers:
// ========================================================

// Length in elements of statically allocated arrays.
template<class T, uint N>
inline uint arrayLength(const T (&)[N])
{
	return N;
}

// Swap two objects, without testing if they are already equal.
template<class T>
inline void swap(T & a, T & b)
{
	T c(a);
	a = b;
	b = c;
}

// Map scalar value from any range to any other range.
template<class T>
inline T mapValueRange(const T x, const T inMin, const T inMax, const T outMin, const T outMax)
{
	return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

// Map scalar value to [0,1] range.
template<class T>
inline T normalizeValue(const T x, const T minimum, const T maximum)
{
	return (x - minimum) / (maximum - minimum);
}

// Clamp value between minimum and maximum, inclusive.
template<class T>
inline T clamp(const T x, const T minimum, const T maximum)
{
	return (x < minimum) ? minimum : (x > maximum) ? maximum : x;
}

// Linear interpolation between two values. `t` is NOT clamped to 0,1.
template<class T>
inline T lerp(const T a, const T b, const T t)
{
	return a + t * (b - a);
}

// Make a Four Character Code from 4 separate bytes:
inline uint32 make4CC(const ubyte c0, const ubyte c1, const ubyte c2, const ubyte c3)
{
	return scast<uint32>(c0)        |
	      (scast<uint32>(c1) <<  8) |
	      (scast<uint32>(c2) << 16) |
	      (scast<uint32>(c3) << 24);
}

// Pack 4 bytes into a Color4b type.
// This could arguably be a Color4b constructor, but I want to keep it a POD type.
inline Color4b makeColor4b(const ubyte r, const ubyte g, const ubyte b, const ubyte a = 255)
{
	const Color4b c = { r, g, b, a };
	return c;
}

// Pack 4 floats into a Color4f type.
inline Color4f makeColor4f(const float r, const float g, const float b, const float a = 1.0f)
{
	const Color4f c = { r, g, b, a };
	return c;
}

// Degrees to Radians:
inline float degToRad(const float degrees)
{
	return degrees * (3.14159265359f / 180.0f);
}

// Radians to Degrees:
inline float radToDeg(const float radians)
{
	return radians * (180.0f / 3.14159265359f);
}

// Time scale conversion: Milliseconds to seconds.
inline float msecToSec(const float ms)
{
	return ms * 0.001f;
}

// Time scale conversion: Seconds to milliseconds.
inline float secToMsec(const float sec)
{
	return sec * 1000.0f;
}

// Time scale conversion: Seconds to milliseconds (overload for integer time values).
inline uint secToMsec(const uint sec)
{
	return sec * 1000;
}

// ========================================================
// Pseudo-random number generators:
// ========================================================

uint randomInt(); // => Integer in the [0, UINT_MAX] range
int  randomInt(int lowerBound, int upperBound);

float randomFloat(); // => Float in the [0.0, 1.0] range
float randomFloat(float lowerBound, float upperBound);

#endif // COMMON_HPP
