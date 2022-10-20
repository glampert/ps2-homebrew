
// ================================================================================================
// -*- C++ -*-
// File: math_funcs.hpp
// Author: Guilherme R. Lampert
// Created on: 11/03/15
// Brief: Optimized mathematical routines (uses PS2/MIPS inline assembly).
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

#ifndef PS2MATH_MATH_FUNCS_HPP
#define PS2MATH_MATH_FUNCS_HPP

// For FLT_EPSILON
#include <float.h>
#include <math.h>

#define PS2MATH_PI     3.1415926535897932384626433832795f
#define PS2MATH_TWOPI  6.283185307179586476925286766559f
#define PS2MATH_HALFPI 1.5707963267948966192313216916398f

// Ensures structure/class is properly aligned to a give boundary.
#define PS2MATH_ALIGNED(alignment) __attribute__((aligned(alignment)))

namespace ps2math
{

// ========================================================

// NOTE: For sine/cosine and friends, angles are in radians.
float asin(float x);
float cos(float x);
float mod(float a, float b);

// ========================================================

inline float abs(float x)
{
	float r;
	asm volatile (
		"abs.s %0, %1 \n\t"
		: "=&f" (r) : "f" (x)
	);
	return r;
}

// ========================================================

inline float min(float a, float b)
{
	float r;
	asm volatile (
		"min.s %0, %1, %2 \n\t"
		: "=&f" (r) : "f" (a), "f" (b)
	);
	return r;
}

// ========================================================

inline float max(float a, float b)
{
	float r;
	asm volatile (
		"max.s %0, %1, %2 \n\t"
		: "=&f" (r) : "f" (a), "f" (b)
	);
	return r;
}

// ========================================================

inline float sqrt(float x)
{
	float r;
	asm volatile (
		"sqrt.s %0, %1 \n\t"
		: "=&f" (r) : "f" (x)
	);
	return r;
}

// ========================================================

inline float invSqrt(float x)
{
	return 1.0f / ps2math::sqrt(x);
}

// ========================================================

inline float sin(float x)
{
	return ps2math::cos(x - PS2MATH_HALFPI);
}

// ========================================================

inline float acos(float x)
{
	return PS2MATH_HALFPI - ps2math::asin(x);
}

// ========================================================

inline bool floatEquals(float a, float b, float tolerance = FLT_EPSILON)
{
	return ps2math::abs(a - b) < tolerance;
}

// ========================================================

inline bool floatGreaterOrEqual(float a, float b, float tolerance = FLT_EPSILON)
{
	return (a - b) > (-tolerance);
}

// ========================================================

} // namespace ps2math {}

#endif // PS2MATH_MATH_FUNCS_HPP
