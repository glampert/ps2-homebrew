
// ================================================================================================
// -*- C++ -*-
// File: vector.hpp
// Author: Guilherme R. Lampert
// Created on: 11/03/15
// Brief: Generic Euclidean vector type, also utilized to represent 3D points.
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

#ifndef PS2MATH_VECTOR_HPP
#define PS2MATH_VECTOR_HPP

#include "math_funcs.hpp"

// ========================================================
// struct Vector:
// ========================================================

struct PS2MATH_ALIGNED(16) Vector
{
	Vector() { } // Uninitialized
	explicit Vector(float v); // Replicate to xyzw
	explicit Vector(const float v[4]);
	Vector(float xx, float yy, float zz, float ww);

	Vector & operator += (const Vector & rhs);
	Vector & operator -= (const Vector & rhs);

	Vector & operator *= (float s);
	Vector & operator /= (float s);

	bool operator == (const Vector & rhs) const;

	Vector cross(const Vector & rhs) const;
	float dot3(const Vector & rhs) const;
	float dot4(const Vector & rhs) const;

	Vector normalized() const;
	void normalizeSelf();
	void clampLength(float minLength, float maxLength);

	float length() const;
	float lengthSqr() const;

	float x;
	float y;
	float z;
	float w;
};

// ========================================================
// Global Vector operators and helpers:
// ========================================================

inline Vector operator - (const Vector & v1)
{
	return Vector(-v1.x, -v1.y, -v1.z, 1.0f);
}

inline Vector operator + (const Vector & v1, const Vector & v2)
{
	return Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, 1.0f);
}

inline Vector operator - (const Vector & v1, const Vector & v2)
{
	return Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, 1.0f);
}

inline Vector operator * (const Vector & v, const float s)
{
	return Vector(v.x * s, v.y * s, v.z * s, 1.0f);
}

inline Vector operator * (const float s, const Vector & v)
{
	return Vector(v.x * s, v.y * s, v.z * s, 1.0f);
}

inline Vector operator / (const Vector & v, const float s)
{
	return Vector(v.x / s, v.y / s, v.z / s, 1.0f);
}

inline Vector crossProduct(const Vector & v1, const Vector & v2)
{
	return Vector(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x, 1.0f);
}

inline float dotProduct4(const Vector & v1, const Vector & v2)
{
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w);
}

inline float dotProduct3(const Vector & v1, const Vector & v2)
{
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

inline Vector normalize(const Vector & v)
{
	return v / v.length();
}

inline float * toFloatPtr(Vector & v)
{
	return reinterpret_cast<float *>(&v);
}

inline const float * toFloatPtr(const Vector & v)
{
	return reinterpret_cast<const float *>(&v);
}

inline Vector min3PerElement(const Vector & vec0, const Vector & vec1)
{
	return Vector(
		ps2math::min(vec0.x, vec1.x),
		ps2math::min(vec0.y, vec1.y),
		ps2math::min(vec0.z, vec1.z), 1.0f);
}

inline Vector max3PerElement(const Vector & vec0, const Vector & vec1)
{
	return Vector(
		ps2math::max(vec0.x, vec1.x),
		ps2math::max(vec0.y, vec1.y),
		ps2math::max(vec0.z, vec1.z), 1.0f);
}

inline void lerp(Vector & v0, const Vector & v1, const Vector & v2, const float t)
{
#if 1

	asm volatile (
		"lqc2      vf4, 0x0(%1)  \n\t" // vf4 = v1
		"lqc2      vf5, 0x0(%2)  \n\t" // vf5 = v2
		"mfc1      $8,  %3       \n\t" // vf6 = t
		"qmtc2     $8,  vf6      \n\t" // lerp:
		"vsub.xyz  vf7, vf5, vf4 \n\t" // vf7 = v2 - v1
		"vmulx.xyz vf8, vf7, vf6 \n\t" // vf8 = vf7 * t
		"vadd.xyz  vf9, vf8, vf4 \n\t" // vf9 = vf8 + vf4
		"sqc2      vf9, 0x0(%0)  \n\t" // v0  = vf9
		: : "r" (&v0), "r" (&v1), "r" (&v2), "f" (t)
		: "$8"
	);

#else

	v0.x = v1.x + t * (v2.x - v1.x);
	v0.y = v1.y + t * (v2.y - v1.y);
	v0.z = v1.z + t * (v2.z - v1.z);
	// v0.w is undefined!

#endif
}

inline void lerpScale(Vector & v0, const Vector & v1, const Vector & v2, const float t, const float s)
{
#if 1

	asm volatile (
		"mfc1      $8,  %3       \n\t"
		"mfc1      $9,  %4       \n\t"
		"lqc2      vf4, 0x0(%1)  \n\t" // vf4 = v1
		"lqc2      vf5, 0x0(%2)  \n\t" // vf5 = v2
		"qmtc2     $8,  vf6      \n\t" // vf6 = t
		"qmtc2     $9,  vf7      \n\t" // vf7 = s
		"vsub.xyz  vf8, vf5, vf4 \n\t" // vf8 = v2 - v1
		"vmulx.xyz vf8, vf8, vf6 \n\t" // vf8 = vf8 * t
		"vadd.xyz  vf9, vf8, vf4 \n\t" // vf9 = vf8 + vf4
		"vmulx.xyz vf9, vf9, vf7 \n\t" // vf9 = vf9 * s
		"sqc2      vf9, 0x0(%0)  \n\t" // v0  = vf9
		: : "r" (&v0), "r" (&v1), "r" (&v2), "f" (t), "f" (s)
		: "$8", "$9"
	);

#else

	v0.x = v1.x + t * (v2.x - v1.x);
	v0.y = v1.y + t * (v2.y - v1.y);
	v0.z = v1.z + t * (v2.z - v1.z);
	v0.x *= s;
	v0.y *= s;
	v0.z *= s;
	// v0.w is undefined!

#endif
}

inline float distanceSqr(const Vector & a, const Vector & b)
{
#if 1

	register float dist;
	asm volatile (
		"lqc2     vf4, 0x0(%1)  \n\t" // vf4 = a
		"lqc2     vf5, 0x0(%2)  \n\t" // vf5 = b
		"vsub.xyz vf6, vf4, vf5 \n\t" // vf6 = vf4(a) - vf5(b)
		"vmul.xyz vf7, vf6, vf6 \n\t" // vf7 = vf6 * vf6
		"vaddy.x  vf7, vf7, vf7 \n\t" // dot(vf7, vf7)
		"vaddz.x  vf7, vf7, vf7 \n\t"
		"qmfc2    $2,  vf7      \n\t" // Store result on `dist`
		"mtc1     $2,  %0       \n\t"
		: "=f" (dist)
		: "r" (&a), "r" (&b)
		: "$2"
	);
	return dist;

#else

	return (a - b).lengthSqr();

#endif
}

inline Vector lerp(const Vector & v1, const Vector & v2, const float t)
{
	Vector v0;
	lerp(v0, v1, v2, t); // Forward to the optimized ASM routine
	v0.w = 1.0f;         // This one ensures w=1
	return v0;
}

inline void rotateAroundAxis(Vector & result, const Vector & vec, const Vector & axis, const float radians)
{
	// Rotate `vec` around an arbitrary `axis` by an angle in radians.
	//
	const float sinAng = ps2math::sin(radians);
	const float cosAng = ps2math::cos(radians);
	const float oneMinusCosAng = (1.0f - cosAng);
	const float aX = axis.x;
	const float aY = axis.y;
	const float aZ = axis.z;

	// Calculate X component:
	float xxx = (aX * aX * oneMinusCosAng + cosAng)      * vec.x +
				(aX * aY * oneMinusCosAng + aZ * sinAng) * vec.y +
				(aX * aZ * oneMinusCosAng - aY * sinAng) * vec.z;

	// Calculate Y component:
	float yyy = (aX * aY * oneMinusCosAng - aZ * sinAng) * vec.x +
				(aY * aY * oneMinusCosAng + cosAng)      * vec.y +
				(aY * aZ * oneMinusCosAng + aX * sinAng) * vec.z;

	// Calculate Z component:
	float zzz = (aX * aZ * oneMinusCosAng + aY * sinAng) * vec.x +
				(aY * aZ * oneMinusCosAng - aX * sinAng) * vec.y +
				(aZ * aZ * oneMinusCosAng + cosAng)      * vec.z;

	result.x = xxx;
	result.y = yyy;
	result.z = zzz;
	result.w = 1.0f;
}

// ========================================================
// Vector inline methods and operators:
// ========================================================

inline Vector::Vector(const float v)
	: x(v), y(v), z(v), w(v)
{
}

inline Vector::Vector(const float xx, const float yy, const float zz, const float ww)
	: x(xx), y(yy), z(zz), w(ww)
{
}

inline Vector::Vector(const float v[4])
	: x(v[0]), y(v[1]), z(v[2]), w(v[3])
{
}

inline Vector & Vector::operator += (const Vector & rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w  = 1.0f;
	return *this;
}

inline Vector & Vector::operator -= (const Vector & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w  = 1.0f;
	return *this;
}

inline float Vector::dot3(const Vector & rhs) const
{
	return (x * rhs.x + y * rhs.y + z * rhs.z);
}

inline float Vector::dot4(const Vector & rhs) const
{
	return (x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w);
}

inline float Vector::length() const
{
	return ps2math::sqrt(x * x + y * y + z * z);
}

inline float Vector::lengthSqr() const
{
	return (x * x + y * y + z * z);
}

inline Vector Vector::cross(const Vector & rhs) const
{
	return Vector(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x, 1.0f);
}

inline Vector Vector::normalized() const
{
	return (*this) / length();
}

inline void Vector::normalizeSelf()
{
	(*this) /= length();
}

inline void Vector::clampLength(const float minLength, const float maxLength)
{
	const float length2 = lengthSqr();
	if (length2 > (maxLength * maxLength))
	{
		const float invLen = maxLength * ps2math::invSqrt(length2);
		(*this) *= invLen;
		return;
	}
	if (length2 < (minLength * minLength))
	{
		const float invLen = minLength * ps2math::invSqrt(length2);
		(*this) *= invLen;
		return;
	}
}

inline Vector & Vector::operator *= (const float s)
{
	x *= s;
	y *= s;
	z *= s;
	w  = 1.0f;
	return *this;
}

inline Vector & Vector::operator /= (const float s)
{
	x /= s;
	y /= s;
	z /= s;
	w  = 1.0f;
	return *this;
}

inline bool Vector::operator == (const Vector & rhs) const
{
	return (x == rhs.x) && (y == rhs.y) &&
	       (z == rhs.z) && (w == rhs.w);
}

#endif // PS2MATH_VECTOR_HPP
