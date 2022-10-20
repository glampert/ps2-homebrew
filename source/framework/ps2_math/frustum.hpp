
// ================================================================================================
// -*- C++ -*-
// File: frustum.hpp
// Author: Guilherme R. Lampert
// Created on: 26/03/15
// Brief: Frustum culling.
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

#ifndef PS2MATH_FRUSTUM_HPP
#define PS2MATH_FRUSTUM_HPP

#include "vector.hpp"
#include "matrix.hpp"

// ========================================================
// struct Frustum:
// ========================================================

struct PS2MATH_ALIGNED(16) Frustum
{
	// Sets everything to zero / identity.
	Frustum();

	// Compute a fresh projection matrix for the frustum.
	void setProjection(float fovRadians, int width, int height, float zn, float zf);

	// Compute frustum planes from camera matrix. Also sets `clipMatrix` by multiplying view and projection.
	void update(const Matrix & view);

	//
	// Bounding geometry => frustum testing:
	//

	// Point:
	bool testPoint(float x, float y, float z) const;
	bool testPoint(const Vector & v) const;

	// Bounding sphere:
	bool testSphere(float x, float y, float z, float radius) const;
	bool testSphere(const Vector & center, float radius) const;

	// A cube:
	bool testCube(float x, float y, float z, float size) const;
	bool testCube(const Vector & center, float size) const;

	// Axis-aligned bounding box. True if box is partly intersecting or fully contained in the frustum.
	bool testAabb(const Vector & mins, const Vector & maxs) const;
	bool testAabb(const Aabb & aabb) const;

	// view * projection:
	Matrix clipMatrix;
	Matrix projection;

	// Frustum planes:
	enum { A, B, C, D };
	float p[6][4];
};

// ========================================================
// Frustum inline methods:
// ========================================================

inline void normalizePlane(float p[4])
{
	// plane *= 1/sqrt(p.a * p.a + p.b * p.b + p.c * p.c);
	const float invLen = ps2math::invSqrt((p[0] * p[0]) + (p[1] * p[1]) + (p[2] * p[2]));
	p[0] *= invLen;
	p[1] *= invLen;
	p[2] *= invLen;
	p[3] *= invLen;
}

inline Frustum::Frustum()
{
	clipMatrix.makeIdentity();
	projection.makeIdentity();

	for (int x = 0; x < 6; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			p[x][y] = 0.0f;
		}
	}
}

inline void Frustum::setProjection(const float fovRadians, const int width, const int height, const float zn, const float zf)
{
	float * matrix = toFloatPtr(projection);

	const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	const float yScale = 1.0f / tanf(fovRadians / 2.0f);
	const float xScale = yScale / aspectRatio;

	matrix[0] = xScale;
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = yScale;
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = zf / (zn - zf);
	matrix[11] = -1.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = zn * zf / (zn - zf);
	matrix[15] = 0.0f;
}

inline void Frustum::update(const Matrix & view)
{
	// Compute a clip matrix:
	clipMatrix = view * projection;

	// Compute and normalize the 6 frustum planes:
	const float * m = toFloatPtr(clipMatrix);
	p[0][A] = m[ 3] - m[ 0];
	p[0][B] = m[ 7] - m[ 4];
	p[0][C] = m[11] - m[ 8];
	p[0][D] = m[15] - m[12];
	normalizePlane(p[0]);
	p[1][A] = m[ 3] + m[ 0];
	p[1][B] = m[ 7] + m[ 4];
	p[1][C] = m[11] + m[ 8];
	p[1][D] = m[15] + m[12];
	normalizePlane(p[1]);
	p[2][A] = m[ 3] + m[ 1];
	p[2][B] = m[ 7] + m[ 5];
	p[2][C] = m[11] + m[ 9];
	p[2][D] = m[15] + m[13];
	normalizePlane(p[2]);
	p[3][A] = m[ 3] - m[ 1];
	p[3][B] = m[ 7] - m[ 5];
	p[3][C] = m[11] - m[ 9];
	p[3][D] = m[15] - m[13];
	normalizePlane(p[3]);
	p[4][A] = m[ 3] - m[ 2];
	p[4][B] = m[ 7] - m[ 6];
	p[4][C] = m[11] - m[10];
	p[4][D] = m[15] - m[14];
	normalizePlane(p[4]);
	p[5][A] = m[ 3] + m[ 2];
	p[5][B] = m[ 7] + m[ 6];
	p[5][C] = m[11] + m[10];
	p[5][D] = m[15] + m[14];
	normalizePlane(p[5]);
}

inline bool Frustum::testPoint(const float x, const float y, const float z) const
{
	for (int i = 0; i < 6; ++i)
	{
		if ((p[i][A] * x + p[i][B] * y + p[i][C] * z + p[i][D]) <= 0.0f)
		{
			return false;
		}
	}
	return true;
}

inline bool Frustum::testPoint(const Vector & v) const
{
	return testPoint(v.x, v.y, v.z);
}

inline bool Frustum::testSphere(const float x, const float y, const float z, const float radius) const
{
	for (int i = 0; i < 6; ++i)
	{
		if ((p[i][A] * x + p[i][B] * y + p[i][C] * z + p[i][D]) <= -radius)
		{
			return false;
		}
	}
	return true;
}

inline bool Frustum::testSphere(const Vector & center, const float radius) const
{
	return testSphere(center.x, center.y, center.z, radius);
}

inline bool Frustum::testCube(const float x, const float y, const float z, const float size) const
{
	for (int i = 0; i < 6; ++i)
	{
		if ((p[i][A] * (x - size) + p[i][B] * (y - size) + p[i][C] * (z - size) + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * (x + size) + p[i][B] * (y - size) + p[i][C] * (z - size) + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * (x - size) + p[i][B] * (y + size) + p[i][C] * (z - size) + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * (x + size) + p[i][B] * (y + size) + p[i][C] * (z - size) + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * (x - size) + p[i][B] * (y - size) + p[i][C] * (z + size) + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * (x + size) + p[i][B] * (y - size) + p[i][C] * (z + size) + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * (x - size) + p[i][B] * (y + size) + p[i][C] * (z + size) + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * (x + size) + p[i][B] * (y + size) + p[i][C] * (z + size) + p[i][D]) > 0.0f) { continue; }
		return false;
	}
	return true;
}

inline bool Frustum::testCube(const Vector & center, const float size) const
{
	return testCube(center.x, center.y, center.z, size);
}

inline bool Frustum::testAabb(const Vector & mins, const Vector & maxs) const
{
	for (int i = 0; i < 6; ++i)
	{
		if ((p[i][A] * mins.x + p[i][B] * mins.y + p[i][C] * mins.z + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * maxs.x + p[i][B] * mins.y + p[i][C] * mins.z + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * mins.x + p[i][B] * maxs.y + p[i][C] * mins.z + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * maxs.x + p[i][B] * maxs.y + p[i][C] * mins.z + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * mins.x + p[i][B] * mins.y + p[i][C] * maxs.z + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * maxs.x + p[i][B] * mins.y + p[i][C] * maxs.z + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * mins.x + p[i][B] * maxs.y + p[i][C] * maxs.z + p[i][D]) > 0.0f) { continue; }
		if ((p[i][A] * maxs.x + p[i][B] * maxs.y + p[i][C] * maxs.z + p[i][D]) > 0.0f) { continue; }
		return false;
	}
	return true;
}

inline bool Frustum::testAabb(const Aabb & aabb) const
{
	return testAabb(aabb.mins, aabb.maxs);
}

#endif // PS2MATH_FRUSTUM_HPP
