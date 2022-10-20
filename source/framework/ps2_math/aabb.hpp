
// ================================================================================================
// -*- C++ -*-
// File: aabb.hpp
// Author: Guilherme R. Lampert
// Created on: 25/03/15
// Brief: Axis Aligned Bounding Box (AABB).
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

#ifndef PS2MATH_AABB_HPP
#define PS2MATH_AABB_HPP

#include "vector.hpp"

// ========================================================
// struct Aabb:
// ========================================================

// Simple Axis Aligned Bounding Box (AABB).
// Has a min point and a max point for the two extents of a box.
struct PS2MATH_ALIGNED(16) Aabb
{
	Aabb(); // Constructs at (0,0,0) origin.
	Aabb(const float min[], const float max[]);
	Aabb(const Vector & min, const Vector & max);
	Aabb(const void * vertexes, unsigned int vertexCount, unsigned int vertexStide);

	// Inside-out bounds.
	void clear();

	// Set as a point at the origin.
	void setZero();

	// Returns true if bounds are inside-out.
	bool isCleared() const;

	// Returns the volume of the bounds.
	float getVolume() const;

	// Returns the center point of the bounds as a Vector.
	Vector getCenter() const;

	// Test if a point is touching or is inside the box bounds.
	bool containsPoint(const Vector & boxPos, const Vector & p) const;

	// Returns true if the line intersects the bounds between the start and end points.
	bool lineIntersection(const Vector & start, const Vector & end) const;

	// Build tightest bounds for a set of mesh vertexes. First element of the vertex must be a Vec3 (the vertex position).
	Aabb & fromMeshVertexes(const void * vertexes, unsigned int vertexCount, unsigned int vertexStide);

	// Scale the vertexes of this AABB.
	Aabb & scale(float s);

	// Transform the points defining this bounds by a given matrix.
	Aabb & transform(const Matrix & mat);

	// Get a transformed copy of this Aabb.
	Aabb transformed(const Matrix & mat) const;

	// Turn the AABB into a set of points defining a box. Useful for debug visualization.
	void toPoints(Vector points[8]) const;

	// Linearly interpolate two AABBs.
	static Aabb lerp(const Aabb & a, const Aabb & b, float t);

	// Test if two AABBs collide/intersect.
	static bool collision(const Aabb & box1, const Vector & box1Pos, const Aabb & box2, const Vector & box2Pos);

	// Minimum and maximum extents of the box.
	Vector mins;
	Vector maxs;
};

// ========================================================
// Aabb inline methods:
// ========================================================

inline Aabb::Aabb()
	: mins(0.0f, 0.0f, 0.0f, 1.0f)
	, maxs(0.0f, 0.0f, 0.0f, 1.0f)
{ }

inline Aabb::Aabb(const float min[], const float max[])
	: mins(min[0], min[1], min[2], 1.0f)
	, maxs(max[0], max[1], max[2], 1.0f)
{ }

inline Aabb::Aabb(const Vector & min, const Vector & max)
	: mins(min)
	, maxs(max)
{ }

inline Aabb::Aabb(const void * vertexes, const unsigned int vertexCount, const unsigned int vertexStide)
{
	fromMeshVertexes(vertexes, vertexCount, vertexStide);
}

inline void Aabb::clear()
{
	const float INF = 1e30f; // A large float (HUGE_VAL is not available)
	mins = Vector( INF,  INF,  INF, 1.0f);
	maxs = Vector(-INF, -INF, -INF, 1.0f);
}

inline void Aabb::setZero()
{
	mins = Vector(0.0f, 0.0f, 0.0f, 1.0f);
	maxs = Vector(0.0f, 0.0f, 0.0f, 1.0f);
}

inline bool Aabb::isCleared() const
{
	return mins.x > maxs.x;
}

inline float Aabb::getVolume() const
{
	if ((mins.x >= maxs.x) || (mins.y >= maxs.y) || (mins.z >= maxs.z))
	{
		return 0.0f;
	}
	return (maxs.x - mins.x) * (maxs.y - mins.y) * (maxs.z - mins.z);
}

inline Vector Aabb::getCenter() const
{
	return (maxs + mins) * 0.5f;
}

inline bool Aabb::containsPoint(const Vector & boxPos, const Vector & p) const
{
	if ((p.x < mins.x + boxPos.x) || (p.y < mins.y + boxPos.y) || (p.z < mins.z + boxPos.z) ||
	    (p.x > maxs.x + boxPos.x) || (p.y > maxs.y + boxPos.y) || (p.z > maxs.z + boxPos.z))
	{
		return false;
	}
	return true;
}

inline bool Aabb::lineIntersection(const Vector & start, const Vector & end) const
{
	const Vector center     = (mins + maxs) * 0.5f;
	const Vector extents    = maxs - center;
	const Vector lineDir    = (end - start) * 0.5f;
	const Vector lineCenter = start + lineDir;
	const Vector dir        = lineCenter - center;

	const float ld0 = ps2math::abs(lineDir.x);
	if (ps2math::abs(dir.x) > (extents.x + ld0))
	{
		return false;
	}

	const float ld1 = ps2math::abs(lineDir.y);
	if (ps2math::abs(dir.y) > (extents.y + ld1))
	{
		return false;
	}

	const float ld2 = ps2math::abs(lineDir.z);
	if (ps2math::abs(dir.z) > (extents.z + ld2))
	{
		return false;
	}

	const Vector vCross = crossProduct(lineDir, dir);
	if (ps2math::abs(vCross.x) > (extents.y * ld2 + extents.z * ld1))
	{
		return false;
	}
	if (ps2math::abs(vCross.y) > (extents.x * ld2 + extents.z * ld0))
	{
		return false;
	}
	if (ps2math::abs(vCross.z) > (extents.x * ld1 + extents.y * ld0))
	{
		return false;
	}

	return true;
}

inline Aabb & Aabb::fromMeshVertexes(const void * restrict vertexes, const unsigned int vertexCount, const unsigned int vertexStide)
{
	ps2assert(vertexes != nullptr);
	ps2assert(vertexCount != 0 && vertexStide != 0);

	clear();
	const uint8 * restrict ptr = rcast<const uint8 *>(vertexes);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		const Vector * v = rcast<const Vector *>(ptr + i * vertexStide);
		const Vector xyz(v->x, v->y, v->z, 1.0f);

		mins = min3PerElement(xyz, mins);
		maxs = max3PerElement(xyz, maxs);
	}

	return *this;
}

inline Aabb & Aabb::scale(const float s)
{
	mins *= s;
	maxs *= s;
	return *this;
}

inline Aabb & Aabb::transform(const Matrix & mat)
{
	Vector tmp;

	tmp = mat * mins;
	mins = tmp;

	tmp = mat * maxs;
	maxs = tmp;

	return *this;
}

inline Aabb Aabb::transformed(const Matrix & mat) const
{
	Aabb copy(*this);
	copy.transform(mat);
	return copy;
}

inline void Aabb::toPoints(Vector points[8]) const
{
	const Vector b[2] = { mins, maxs };
	for (int i = 0; i < 8; ++i)
	{
		points[i].x = b[(i ^ (i >> 1)) & 1].x;
		points[i].y = b[(i >> 1) & 1].y;
		points[i].z = b[(i >> 2) & 1].z;
		points[i].w = 1.0f;
	}
}

inline Aabb Aabb::lerp(const Aabb & a, const Aabb & b, const float t)
{
	Vector mins;
	mins.x = a.mins.x + t * (b.mins.x - a.mins.x);
	mins.y = a.mins.y + t * (b.mins.y - a.mins.y);
	mins.z = a.mins.z + t * (b.mins.z - a.mins.z);
	mins.w = 1.0f;

	Vector maxs;
	maxs.x = a.maxs.x + t * (b.maxs.x - a.maxs.x);
	maxs.y = a.maxs.y + t * (b.maxs.y - a.maxs.y);
	maxs.z = a.maxs.z + t * (b.maxs.z - a.maxs.z);
	maxs.w = 1.0f;

	return Aabb(mins, maxs);
}

inline bool Aabb::collision(const Aabb & box1, const Vector & box1Pos, const Aabb & box2, const Vector & box2Pos)
{
	if ((box1.mins.x + box1Pos.x) > (box2.maxs.x + box2Pos.x) ||
	    (box1.maxs.x + box1Pos.x) < (box2.mins.x + box2Pos.x))
	{
		return false;
	}
	if ((box1.mins.y + box1Pos.y) > (box2.maxs.y + box2Pos.y) ||
	    (box1.maxs.y + box1Pos.y) < (box2.mins.y + box2Pos.y))
	{
		return false;
	}
	if ((box1.mins.z + box1Pos.z) > (box2.maxs.z + box2Pos.z) ||
	    (box1.maxs.z + box1Pos.z) < (box2.mins.z + box2Pos.z))
	{
		return false;
	}

	// If we get here, there is an intersection.
	return true;
}

#endif // PS2MATH_AABB_HPP
