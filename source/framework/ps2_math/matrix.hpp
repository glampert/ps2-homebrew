
// ================================================================================================
// -*- C++ -*-
// File: matrix.hpp
// Author: Guilherme R. Lampert
// Created on: 11/03/15
// Brief: Homogeneous 4x4 matrix, used for all kinds of object transforms.
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

#ifndef PS2MATH_MATRIX_HPP
#define PS2MATH_MATRIX_HPP

#include "vector.hpp"
#include <string.h> // For `memcpy()`

// ========================================================
// struct Matrix:
// ========================================================

struct PS2MATH_ALIGNED(16) Matrix
{
	Matrix() { } // Uninitialized
	explicit Matrix(const float data[][4]);
	Matrix(float m11, float m12, float m13, float m14,
	       float m21, float m22, float m23, float m24,
	       float m31, float m32, float m33, float m34,
	       float m41, float m42, float m43, float m44);

	Matrix(const Matrix & other);
	Matrix & operator = (const Matrix & other);

	void makeIdentity();

	void makeTranslation(float x, float y, float z);
	void makeTranslation(const Vector & v);

	void makeScaling(float x, float y, float z);
	void makeScaling(const Vector & v);

	void makeRotationX(float radians);
	void makeRotationY(float radians);
	void makeRotationZ(float radians);

	void makeLookAt(const Vector & vFrom, const Vector & vTo, const Vector & vUp);
	void makePerspectiveProjection(float fovy, float aspect, float scrW, float scrH,
	                               float zNear, float zFar, float projScale = 4096.0f);

	float & operator() (unsigned int row, unsigned int column)       { return elem[row][column]; }
	float   operator() (unsigned int row, unsigned int column) const { return elem[row][column]; }

	float elem[4][4];
};

// ========================================================
// Matrix inline methods and operators:
// ========================================================

inline Matrix::Matrix(const float m11, const float m12, const float m13, const float m14,
                      const float m21, const float m22, const float m23, const float m24,
                      const float m31, const float m32, const float m33, const float m34,
                      const float m41, const float m42, const float m43, const float m44)
{
	elem[0][0] = m11;  elem[0][1] = m12;  elem[0][2] = m13;  elem[0][3] = m14;
	elem[1][0] = m21;  elem[1][1] = m22;  elem[1][2] = m23;  elem[1][3] = m24;
	elem[2][0] = m31;  elem[2][1] = m32;  elem[2][2] = m33;  elem[2][3] = m34;
	elem[3][0] = m41;  elem[3][1] = m42;  elem[3][2] = m43;  elem[3][3] = m44;
}

inline Matrix::Matrix(const float data[][4])
{
	// Input is not necessarily aligned, so default to a memcpy.
	memcpy(elem, data, sizeof(float) * 16);
}

inline Matrix::Matrix(const Matrix & other)
{
#if 1

	asm volatile (
		"lq $6, 0x00(%1) \n\t"
		"lq $7, 0x10(%1) \n\t"
		"lq $8, 0x20(%1) \n\t"
		"lq $9, 0x30(%1) \n\t"
		"sq $6, 0x00(%0) \n\t"
		"sq $7, 0x10(%0) \n\t"
		"sq $8, 0x20(%0) \n\t"
		"sq $9, 0x30(%0) \n\t"
		: : "r" (elem), "r" (other.elem)
		: "$6", "$7", "$8", "$9"
	);

#else

	memcpy(elem, other.elem, sizeof(float) * 16);

#endif
}

inline Matrix & Matrix::operator = (const Matrix & other)
{
#if 1

	asm volatile (
		"lq $6, 0x00(%1) \n\t"
		"lq $7, 0x10(%1) \n\t"
		"lq $8, 0x20(%1) \n\t"
		"lq $9, 0x30(%1) \n\t"
		"sq $6, 0x00(%0) \n\t"
		"sq $7, 0x10(%0) \n\t"
		"sq $8, 0x20(%0) \n\t"
		"sq $9, 0x30(%0) \n\t"
		: : "r" (elem), "r" (other.elem)
		: "$6", "$7", "$8", "$9"
	);
	return *this;

#else

	memcpy(elem, other.elem, sizeof(float) * 16);
	return *this;

#endif
}

inline void Matrix::makeIdentity()
{
#if 1

	asm volatile (
		"vsub.xyzw  vf4, vf0, vf0 \n\t"
		"vadd.w     vf4, vf4, vf0 \n\t"
		"vmr32.xyzw vf5, vf4      \n\t"
		"vmr32.xyzw vf6, vf5      \n\t"
		"vmr32.xyzw vf7, vf6      \n\t"
		"sqc2       vf4, 0x30(%0) \n\t"
		"sqc2       vf5, 0x20(%0) \n\t"
		"sqc2       vf6, 0x10(%0) \n\t"
		"sqc2       vf7, 0x0(%0)  \n\t"
		: : "r" (elem)
	);

#else

	elem[0][0] = 1.0f;  elem[0][1] = 0.0f;  elem[0][2] = 0.0f;  elem[0][3] = 0.0f;
	elem[1][0] = 0.0f;  elem[1][1] = 1.0f;  elem[1][2] = 0.0f;  elem[1][3] = 0.0f;
	elem[2][0] = 0.0f;  elem[2][1] = 0.0f;  elem[2][2] = 1.0f;  elem[2][3] = 0.0f;
	elem[3][0] = 0.0f;  elem[3][1] = 0.0f;  elem[3][2] = 0.0f;  elem[3][3] = 1.0f;

#endif
}

inline void Matrix::makeTranslation(const float x, const float y, const float z)
{
	makeIdentity();
	elem[3][0] = x;
	elem[3][1] = y;
	elem[3][2] = z;
}

inline void Matrix::makeTranslation(const Vector & v)
{
	makeIdentity();
	elem[3][0] = v.x;
	elem[3][1] = v.y;
	elem[3][2] = v.z;
}

inline void Matrix::makeScaling(const float x, const float y, const float z)
{
	makeIdentity();
	elem[0][0] = x;
	elem[1][1] = y;
	elem[2][2] = z;
}

inline void Matrix::makeScaling(const Vector & v)
{
	makeIdentity();
	elem[0][0] = v.x;
	elem[1][1] = v.y;
	elem[2][2] = v.z;
}

inline void Matrix::makeRotationX(const float radians)
{
	makeIdentity();
	const float c = ps2math::cos(radians);
	const float s = ps2math::sin(radians);
	elem[1][1] =  c;
	elem[1][2] =  s;
	elem[2][1] = -s;
	elem[2][2] =  c;
}

inline void Matrix::makeRotationY(const float radians)
{
	makeIdentity();
	const float c = ps2math::cos(radians);
	const float s = ps2math::sin(radians);
	elem[0][0] =  c;
	elem[2][0] =  s;
	elem[0][2] = -s;
	elem[2][2] =  c;
}

inline void Matrix::makeRotationZ(const float radians)
{
	makeIdentity();
	const float c = ps2math::cos(radians);
	const float s = ps2math::sin(radians);
	elem[0][0] =  c;
	elem[0][1] =  s;
	elem[1][0] = -s;
	elem[1][1] =  c;
}

inline void Matrix::makeLookAt(const Vector & vFrom, const Vector & vTo, const Vector & vUp)
{
	const Vector vZ = normalize(vFrom - vTo);
	const Vector vX = normalize(vUp.cross(vZ));
	const Vector vY = vZ.cross(vX);

	elem[0][0] = vX.x;  elem[0][1] = vY.x;  elem[0][2] = vZ.x;  elem[0][3] = 0.0f;
	elem[1][0] = vX.y;  elem[1][1] = vY.y;  elem[1][2] = vZ.y;  elem[1][3] = 0.0f;
	elem[2][0] = vX.z;  elem[2][1] = vY.z;  elem[2][2] = vZ.z;  elem[2][3] = 0.0f;

	elem[3][0] = -vX.dot3(vFrom);
	elem[3][1] = -vY.dot3(vFrom);
	elem[3][2] = -vZ.dot3(vFrom);
	elem[3][3] = 1.0f;
}

inline void Matrix::makePerspectiveProjection(const float fovy, const float aspect, const float scrW, const float scrH,
                                              const float zNear, const float zFar, const float projScale)
{
	const float fovYdiv2 = fovy * 0.5f;
	const float cotFOV   = 1.0f / (ps2math::sin(fovYdiv2) / ps2math::cos(fovYdiv2));
	const float w = cotFOV * (scrW / projScale) / aspect;
	const float h = cotFOV * (scrH / projScale);

	(*this) = Matrix(
		w,    0.0f, 0.0f, 0.0f,
		0.0f, -h,   0.0f, 0.0f,
		0.0f, 0.0f, (zFar + zNear) / (zFar - zNear),       -1.0f,
		0.0f, 0.0f, (2.0f * zFar * zNear) / (zFar - zNear), 0.0f);
}

inline Matrix operator - (const Matrix & M)
{
	return Matrix(-M(0,0), -M(0,1), -M(0,2), -M(0,3),
	              -M(1,0), -M(1,1), -M(1,2), -M(1,3),
	              -M(2,0), -M(2,1), -M(2,2), -M(2,3),
	              -M(3,0), -M(3,1), -M(3,2), -M(3,3));
}

inline Matrix operator - (const Matrix & M1, const Matrix & M2)
{
	return Matrix(M1(0,0) - M2(0,0), M1(0,1) - M2(0,1), M1(0,2) - M2(0,2), M1(0,3) - M2(0,3),
	              M1(1,0) - M2(1,0), M1(1,1) - M2(1,1), M1(1,2) - M2(1,2), M1(1,3) - M2(1,3),
	              M1(2,0) - M2(2,0), M1(2,1) - M2(2,1), M1(2,2) - M2(2,2), M1(2,3) - M2(2,3),
	              M1(3,0) - M2(3,0), M1(3,1) - M2(3,1), M1(3,2) - M2(3,2), M1(3,3) - M2(3,3));
}

inline Matrix operator + (const Matrix & M1, const Matrix & M2)
{
	return Matrix(M1(0,0) + M2(0,0), M1(0,1) + M2(0,1), M1(0,2) + M2(0,2), M1(0,3) + M2(0,3),
	              M1(1,0) + M2(1,0), M1(1,1) + M2(1,1), M1(1,2) + M2(1,2), M1(1,3) + M2(1,3),
	              M1(2,0) + M2(2,0), M1(2,1) + M2(2,1), M1(2,2) + M2(2,2), M1(2,3) + M2(2,3),
	              M1(3,0) + M2(3,0), M1(3,1) + M2(3,1), M1(3,2) + M2(3,2), M1(3,3) + M2(3,3));
}

inline Matrix operator * (const Matrix & M, const float s)
{
	return Matrix(M(0,0) * s, M(0,1) * s, M(0,2) * s, M(0,3) * s,
	              M(1,0) * s, M(1,1) * s, M(1,2) * s, M(1,3) * s,
	              M(2,0) * s, M(2,1) * s, M(2,2) * s, M(2,3) * s,
	              M(3,0) * s, M(3,1) * s, M(3,2) * s, M(3,3) * s);
}

inline Matrix operator * (const float s, const Matrix & M)
{
	return Matrix(M(0,0) * s, M(0,1) * s, M(0,2) * s, M(0,3) * s,
	              M(1,0) * s, M(1,1) * s, M(1,2) * s, M(1,3) * s,
	              M(2,0) * s, M(2,1) * s, M(2,2) * s, M(2,3) * s,
	              M(3,0) * s, M(3,1) * s, M(3,2) * s, M(3,3) * s);
}

inline Matrix operator * (const Matrix & M1, const Matrix & M2) // Matrix multiply
{
#if 1

	Matrix result;
	asm volatile (
		"lqc2         vf1, 0x00(%1) \n\t"
		"lqc2         vf2, 0x10(%1) \n\t"
		"lqc2         vf3, 0x20(%1) \n\t"
		"lqc2         vf4, 0x30(%1) \n\t"
		"lqc2         vf5, 0x00(%2) \n\t"
		"lqc2         vf6, 0x10(%2) \n\t"
		"lqc2         vf7, 0x20(%2) \n\t"
		"lqc2         vf8, 0x30(%2) \n\t"
		"vmulax.xyzw  ACC, vf5, vf1 \n\t"
		"vmadday.xyzw ACC, vf6, vf1 \n\t"
		"vmaddaz.xyzw ACC, vf7, vf1 \n\t"
		"vmaddw.xyzw  vf1, vf8, vf1 \n\t"
		"vmulax.xyzw  ACC, vf5, vf2 \n\t"
		"vmadday.xyzw ACC, vf6, vf2 \n\t"
		"vmaddaz.xyzw ACC, vf7, vf2 \n\t"
		"vmaddw.xyzw  vf2, vf8, vf2 \n\t"
		"vmulax.xyzw  ACC, vf5, vf3 \n\t"
		"vmadday.xyzw ACC, vf6, vf3 \n\t"
		"vmaddaz.xyzw ACC, vf7, vf3 \n\t"
		"vmaddw.xyzw  vf3, vf8, vf3 \n\t"
		"vmulax.xyzw  ACC, vf5, vf4 \n\t"
		"vmadday.xyzw ACC, vf6, vf4 \n\t"
		"vmaddaz.xyzw ACC, vf7, vf4 \n\t"
		"vmaddw.xyzw  vf4, vf8, vf4 \n\t"
		"sqc2         vf1, 0x00(%0) \n\t"
		"sqc2         vf2, 0x10(%0) \n\t"
		"sqc2         vf3, 0x20(%0) \n\t"
		"sqc2         vf4, 0x30(%0) \n\t"
		: : "r" (&result), "r" (&M1), "r" (&M2)
	);
	return result;

#else

	Matrix result;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			register float value = 0;
			for (int k = 0; k < 4; ++k)
			{
				value += M1(i,k) * M2(k,j);
			}
			result(i,j) = value;
		}
	}
	return result;

#endif
}

inline Vector operator * (const Matrix & M, const Vector & V) // Transform point
{
#if 1

	Vector result;
	asm volatile (
		"lqc2         vf4, 0x0(%1)  \n\t"
		"lqc2         vf5, 0x10(%1) \n\t"
		"lqc2         vf6, 0x20(%1) \n\t"
		"lqc2         vf7, 0x30(%1) \n\t"
		"lqc2         vf8, 0x0(%2)  \n\t"
		"vmulax.xyzw  ACC, vf4, vf8 \n\t"
		"vmadday.xyzw ACC, vf5, vf8 \n\t"
		"vmaddaz.xyzw ACC, vf6, vf8 \n\t"
		"vmaddw.xyzw  vf9, vf7, vf8 \n\t"
		"sqc2         vf9, 0x0(%0)  \n\t"
		: : "r" (&result), "r" (&M), "r" (&V)
	);
	return result;

#else

    Vector result;
	result.x = M(0,0) * V.x + M(1,0) * V.y + M(2,0) * V.z + M(3,0) * V.w;
	result.y = M(0,1) * V.x + M(1,1) * V.y + M(2,1) * V.z + M(3,1) * V.w;
	result.z = M(0,2) * V.x + M(1,2) * V.y + M(2,2) * V.z + M(3,2) * V.w;
	result.w = M(0,3) * V.x + M(1,3) * V.y + M(2,3) * V.z + M(3,3) * V.w;
    return result;

#endif
}

inline Matrix transpose(const Matrix & M)
{
	Matrix result;
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < 4; ++i)
		{
			result(i,j) = M(j,i);
		}
	}
	return result;
}

inline float * toFloatPtr(Matrix & m)
{
	return reinterpret_cast<float *>(&m);
}

inline const float * toFloatPtr(const Matrix & m)
{
	return reinterpret_cast<const float *>(&m);
}

#endif // PS2MATH_MATRIX_HPP
