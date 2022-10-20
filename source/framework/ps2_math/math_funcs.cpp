
// ================================================================================================
// -*- C++ -*-
// File: math_funcs.cpp
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

#include "math_funcs.hpp"

//
// Asm code originally from: Morten "Sparky" Mikkelsen's fast maths routines
// (From a post on the forums at www.playstation2-linux.com)
//

namespace ps2math
{

// ========================================================
// ps2math::mod():
// ========================================================

float mod(float a, float b)
{
	float r;
	asm volatile (
		".set push                 \n\t"
		".set noreorder            \n\t"
		"div.s  %0,  %1,    %2     \n\t"
		"mtc1   $0,  $f8           \n\t"
		"mfc1   $10, %0            \n\t"
		"srl    $8,  $10,   23     \n\t"
		"addiu  $9,  $0,    127+23 \n\t"
		"andi   $8,  $8,    0xff   \n\t"
		"addiu  $12, $0,    1      \n\t"
		"addiu  $11, $8,   -127    \n\t"
		"subu   $8,  $9,    $8     \n\t"
		"pmaxw  $8,  $8,    $0     \n\t"
		"sllv   $8,  $12,   $8     \n\t"
		"lui    $9,  0x8000        \n\t"
		"negu   $8,  $8            \n\t"
		"srl    $11, $11,   31     \n\t"
		"movz   $9,  $8,    $11    \n\t"
		"and    $10, $9,    $10    \n\t"
		"mtc1   $10, %0            \n\t"
		"adda.s %1,  $f8           \n\t"
		"msub.s %0,  %2,    %0     \n\t"
		".set pop                  \n\t"
		: "=&f" (r)
		: "f" (a), "f" (b)
		: "$8", "$9", "$10", "$11", "$12", "$f8"
	);
	return r;
}

// ========================================================
// ps2math::asin():
// ========================================================

float asin(float x)
{
	float r;
	asm volatile (
		".set noreorder             \n\t"
		".align 3                   \n\t"
		"lui     $8,  0x3f80        \n\t"
		"mtc1    $0,  $f8           \n\t"
		"mtc1    $8,  $f1           \n\t"
		"lui     $8,  0x3f35        \n\t"
		"mfc1    $9,  %1            \n\t"
		"ori     $8,  $8,    0x04f3 \n\t"
		"adda.s  $f1, $f8           \n\t"
		"lui     $10, 0x8000        \n\t"
		"msub.s  $f2, %1,    %1     \n\t"
		"not     $11, $10           \n\t"
		"and     $11, $9,    $11    \n\t"
		"subu    $8,  $8,    $11    \n\t"
		"nop                        \n\t"
		"and     $9,  $9,    $10    \n\t"
		"sqrt.s  $f2, $f2           \n\t"
		"srl     $8,  $8,    31     \n\t"
		"abs.s   %0,  %1            \n\t"
		"lui     $11, 0x3fc9        \n\t"
		"ori     $11, 0x0fdb        \n\t"
		"or      $11, $9,    $11    \n\t"
		"movz    $11, $0,    $8     \n\t"
		"xor     $10, $9,    $10    \n\t"
		"mtc1    $11, $f6           \n\t"
		"movz    $10, $9,    $8     \n\t"
		"min.s   %0,  $f2,   %0     \n\t"
		"lui     $9,  0x3e2a        \n\t"
		"lui     $8,  0x3f80        \n\t"
		"ori     $9,  $9,    0xaaab \n\t"
		"or      $8,  $10,   $8     \n\t"
		"or      $9,  $10,   $9     \n\t"
		"mtc1    $8,  $f3           \n\t"
		"lui     $8,  0x3d99        \n\t"
		"mul.s   $f7, %0,    %0     \n\t"
		"ori     $8,  $8,    0x999a \n\t"
		"mtc1    $9,  $f4           \n\t"
		"lui     $9,  0x3d36        \n\t"
		"or      $8,  $10,   $8     \n\t"
		"ori     $9,  $9,    0xdb6e \n\t"
		"mtc1    $8,  $f5           \n\t"
		"or      $9,  $10,   $9     \n\t"
		"mul.s   $f1, %0,    $f7    \n\t"
		"lui     $8,  0x3cf8        \n\t"
		"adda.s  $f6, $f8           \n\t"
		"ori     $8,  $8,    0xe38e \n\t"
		"mul.s   $f8, $f7,   $f7    \n\t"
		"or      $8,  $10,   $8     \n\t"
		"madda.s $f3, %0            \n\t"
		"mul.s   $f2, $f1,   $f7    \n\t"
		"madda.s $f4, $f1           \n\t"
		"mtc1    $9,  $f6           \n\t"
		"mul.s   $f1, $f1,   $f8    \n\t"
		"mul.s   %0,  $f2,   $f8    \n\t"
		"mtc1    $8,  $f7           \n\t"
		"madda.s $f5, $f2           \n\t"
		"madda.s $f6, $f1           \n\t"
		"madd.s  %0,  $f7,   %0     \n\t"
		".set reorder               \n\t"
		: "=&f" (r)
		: "f" (x)
		: "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7", "$f8"
	);
	return r;
}

// ========================================================
// ps2math::cos():
// ========================================================

float cos(float x)
{
	float r;
	asm volatile (
		"lui     $9,  0x3f00        \n\t"
		".set noreorder             \n\t"
		".align 3                   \n\t"
		"abs.s   %0,  %1            \n\t"
		"lui     $8,  0xbe22        \n\t"
		"mtc1    $9,  $f1           \n\t"
		"ori     $8,  $8,    0xf983 \n\t"
		"mtc1    $8,  $f8           \n\t"
		"lui     $9,  0x4b00        \n\t"
		"mtc1    $9,  $f3           \n\t"
		"lui     $8,  0x3f80        \n\t"
		"mtc1    $8,  $f2           \n\t"
		"mula.s  %0,  $f8           \n\t"
		"msuba.s $f3, $f2           \n\t"
		"madda.s $f3, $f2           \n\t"
		"lui     $8,  0x40c9        \n\t"
		"msuba.s %0,  $f8           \n\t"
		"ori     $8,  0x0fdb        \n\t"
		"msub.s  %0,  $f1,   $f2    \n\t"
		"lui     $9,  0xc225        \n\t"
		"abs.s   %0,  %0            \n\t"
		"lui     $10, 0x3e80        \n\t"
		"mtc1    $10, $f7           \n\t"
		"ori     $9,  0x5de1        \n\t"
		"sub.s   %0,  %0,    $f7    \n\t"
		"lui     $10, 0x42a3        \n\t"
		"mtc1    $8,  $f3           \n\t"
		"ori     $10, 0x3458        \n\t"
		"mtc1    $9,  $f4           \n\t"
		"lui     $8,  0xc299        \n\t"
		"mtc1    $10, $f5           \n\t"
		"ori     $8,  0x2663        \n\t"
		"mul.s   $f8, %0,    %0     \n\t"
		"lui     $9,  0x421e        \n\t"
		"mtc1    $8,  $f6           \n\t"
		"ori     $9,  0xd7bb        \n\t"
		"mtc1    $9,  $f7           \n\t"
		"nop                        \n\t"
		"mul.s   $f1, %0,    $f8    \n\t"
		"mul.s   $f9, $f8,   $f8    \n\t"
		"mula.s  $f3, %0            \n\t"
		"mul.s   $f2, $f1,   $f8    \n\t"
		"madda.s $f4, $f1           \n\t"
		"mul.s   $f1, $f1,   $f9    \n\t"
		"mul.s   %0,  $f2,   $f9    \n\t"
		"madda.s $f5, $f2           \n\t"
		"madda.s $f6, $f1           \n\t"
		"madd.s  %0,  $f7,   %0     \n\t"
		".set reorder               \n\t"
		: "=&f" (r)
		: "f" (x)
		: "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7", "$f8", "$f9", "$8", "$9", "$10"
	);
	return r;
}

} // namespace ps2math {}
