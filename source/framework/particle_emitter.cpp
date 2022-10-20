
// ================================================================================================
// -*- C++ -*-
// File: particle_emitter.cpp
// Author: Guilherme R. Lampert
// Created on: 05/04/15
// Brief: Simple particle emitter and renderer.
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

#include "particle_emitter.hpp"
#include "ps2_math/frustum.hpp"
#include "quick_sort.hpp"
#include "game_time.hpp"

// ========================================================
// ParticleEmitter shared static data:
// ========================================================

uint16     * ParticleEmitter::particleIndexBuffer  = nullptr;
DrawVertex * ParticleEmitter::particleVertexBuffer = nullptr;
static const uint MAX_PARTICLES_PER_EMITTER        = 1024;

// ========================================================
// ParticleEmitter::ParticleEmitter():
// ========================================================

ParticleEmitter::ParticleEmitter()
	: particles(nullptr)
	, spriteTexture(nullptr)
	, ibPtr(nullptr)
	, vbPtr(nullptr)
{
	// One time initialization of shared data:
	//
	if (particleIndexBuffer == nullptr)
	{
		particleIndexBuffer = memClearedAlloc<uint16>(MEM_TAG_GEOMETRY, (MAX_PARTICLES_PER_EMITTER * 6));
		logComment("Initialized PRT index buffer.");
	}
	if (particleVertexBuffer == nullptr)
	{
		particleVertexBuffer = memClearedAlloc<DrawVertex>(MEM_TAG_GEOMETRY, (MAX_PARTICLES_PER_EMITTER * 4));
		logComment("Initialized PRT vertex buffer.");
	}

	ibPtr = particleIndexBuffer;
	vbPtr = particleVertexBuffer;

	setDefaults();
}

// ========================================================
// ParticleEmitter::~ParticleEmitter():
// ========================================================

ParticleEmitter::~ParticleEmitter()
{
	memFree(MEM_TAG_PARTICLES, particles);
}

// ========================================================
// ParticleEmitter::allocateParticles():
// ========================================================

void ParticleEmitter::allocateParticles()
{
	ps2assert(particles == nullptr && "Deallocate first!");
	ps2assert(maxParticles != 0 && "Set a max particle amount!");
	ps2assert(maxParticles <= MAX_PARTICLES_PER_EMITTER);

	particles = memAlloc<Particle>(MEM_TAG_PARTICLES, maxParticles);
}

// ========================================================
// ParticleEmitter::freeParticles():
// ========================================================

void ParticleEmitter::freeParticles()
{
	activeParticleCount = 0;

	memFree(MEM_TAG_PARTICLES, particles);
	particles = nullptr;
}

// ========================================================
// ParticleEmitter::setDefaults():
// ========================================================

void ParticleEmitter::setDefaults()
{
	if (particles != nullptr)
	{
		freeParticles();
	}

	gravityVec                = Vector(0.0f, 0.0f, 0.0f, 0.0f);
	particleVelocityVec       = Vector(0.0f, 0.0f, 0.0f, 0.0f);
	windVelocityVec           = Vector(0.0f, 0.0f, 0.0f, 0.0f);
	emitterOrigin             = Vector(0.0f, 0.0f, 0.0f, 1.0f);
	velocityVar               = 1.0f;
	minParticleSize           = 0.5f;
	maxParticleSize           = 1.0f;
	baseColor.r               = 1.0f;
	baseColor.g               = 1.0f;
	baseColor.b               = 1.0f;
	baseColor.a               = 1.0f;
	particleLifeCycleMs       = 1000; // 1 sec
	particleReleaseIntervalMs = 1000; // 1 sec
	particleReleaseAmount     = 1;
	particlesEmitted          = 0;
	lastUpdateMs              = 0;
	minParticles              = 1;
	maxParticles              = 1;
	activeParticleCount       = 0;
	enabled                   = true;
	simulateAirResistance     = false;
	oneTimeEmission           = false;
}

// ========================================================
// ParticleEmitter::updateParticles():
// ========================================================

void ParticleEmitter::updateParticles()
{
	if (particles == nullptr)
	{
		allocateParticles();
	}

	// Self consistency check:
	ps2assert(activeParticleCount <= maxParticles);

	// Shortcut variables:
	const float deltaTimeSec  = gTime.deltaTimeSeconds;
	const uint  currentTimeMs = gTime.currentTimeMillis;

	// First move the ones that are no longer active to the end
	// and mark those that will start to fade away:
	uint num = 0;
	Particle * particle = particles;
	for (uint i = 0; i < activeParticleCount; ++i, ++particle)
	{
		if (particle->durationMs > currentTimeMs)
		{
			if (num != i)
			{
				ps2assert(num < maxParticles);
				particles[num] = *particle;
			}
			++num;
		}
	}

	// Update active count:
	activeParticleCount = num;
	ps2assert(activeParticleCount <= maxParticles);

	// Update the ones still active:
	for (uint i = 0; i < activeParticleCount; ++i)
	{
		particle = &particles[i];

		// Update velocity with respect to gravity (constant acceleration):
		particle->velocity += gravityVec * deltaTimeSec;

		// Update velocity with respect to wind
		// (acceleration based on difference of vectors):
		if (simulateAirResistance)
		{
			particle->velocity += (windVelocityVec - particle->velocity) * deltaTimeSec;
		}

		// Finally, update position with respect to velocity:
		particle->position += particle->velocity * deltaTimeSec;
	}

	// Return early if this is a one-time emitter and
	// it has already burned its particle budget.
	if (isOneTimeEmitterFinished())
	{
		return;
	}

	// Emit new particles in accordance to the flow rate.
	// Also emit new ones if we have less particles active than the minimum required.
	if ((currentTimeMs - lastUpdateMs) > particleReleaseIntervalMs || activeParticleCount < minParticles)
	{
		lastUpdateMs = currentTimeMs;

		// Emit new particles at specified flow rate:
		for (uint i = 0; i < particleReleaseAmount; ++i)
		{
			// Do we have any free particles to put back to work?
			if (activeParticleCount < maxParticles)
			{
				particle = &particles[activeParticleCount];
				particle->position   = emitterOrigin;
				particle->velocity   = particleVelocityVec;
				particle->size       = randomFloat(minParticleSize, maxParticleSize);
				particle->durationMs = currentTimeMs + particleLifeCycleMs + randomInt(0, 500);

				if (velocityVar != 0.0f)
				{
					particle->velocity += getRandVector() * velocityVar;
				}

				++activeParticleCount;
				++particlesEmitted;
			}
			else
			{
				// No more free particles, can't spawn new ones this frame.
				break;
			}
		}
	}
}

// ========================================================
// ParticleEmitter::drawParticles():
// ========================================================

void ParticleEmitter::drawParticles(const Matrix & viewMatrix, const Vector & eyePos)
{
	if (!hasParticlesToDraw())
	{
		return;
	}

	// Sort the particles according to the distance of each from the viewer.
	// This is necessary to ensure proper transparency rendering and
	// also because particles are rendered with depth-writing disabled.
	//
	sortActiveParticles(eyePos);

	// Orient particle billboards to face the viewer:
	const float * view = toFloatPtr(viewMatrix);
	const Vector rightVec(view[0], view[4], view[8], 1.0f);
	const Vector upVec   (view[1], view[5], view[9], 1.0f);

	uint16     * restrict indexPtr  = ibPtr;
	DrawVertex * restrict vertexPtr = vbPtr;

	const uint16 baseIndexes[6] ATTRIBUTE_ALIGNED(16) = { 0, 1, 2, 2, 3, 0 };
	uint vertexesUsed = 0;
	uint indexesUsed  = 0;

	for (uint i = 0; i < activeParticleCount; ++i)
	{
		const Particle & particle = particles[i];

		// Make a billboard for the particle:
		//
		Billboard billboard;
		makeBillboardFacingCameraPlane(billboard, upVec, rightVec,
			particle.position, baseColor, particle.size, particle.size);

		// Add its vertexes & indexes to the draw batch:
		//
		ps2assert((indexesUsed  + 6) <= (MAX_PARTICLES_PER_EMITTER * 6));
		ps2assert((vertexesUsed + 4) <= (MAX_PARTICLES_PER_EMITTER * 4));

		// Copy the 6 indexes (two triangles):
		for (uint i = 0; i < 6; ++i)
		{
			indexPtr[i] = baseIndexes[i] + vertexesUsed;
		}
		indexesUsed += 6;
		indexPtr    += 6;

		// Expand the four edges of the billboard quadrilateral:
		for (uint v = 0; v < 4; ++v)
		{
			packDrawVertex(vertexPtr[v], billboard.points[v], billboard.texCoords[v], billboard.color);
		}
		vertexesUsed += 4;
		vertexPtr    += 4;
	}

	if (spriteTexture != nullptr)
	{
		gRenderer.setTexture(*spriteTexture);
	}

	// Finally, issue the draw call for all particles in this emitter.
	// Particles don't need to be back-face culled, since they consist
	// of camera-facing billboarded quadrilaterals.
	gRenderer.drawIndexedTrianglesUnculled(ibPtr, indexesUsed, vbPtr, vertexesUsed);
}

// ========================================================
// struct PrtQSortPredicate:
// ========================================================

// I use this as predicate for `quickSort()` in `sortActiveParticles`.
// No idea why GCC doesn't allow it to be nested inside the function, but it doesn't!
struct PrtQSortPredicate
{
	Vector cameraPosition;
	int operator() (const Particle & prtA, const Particle & prtB) const
	{
		const float aDistSqr = distanceSqr(prtA.position, cameraPosition);
		const float bDistSqr = distanceSqr(prtB.position, cameraPosition);
		if (aDistSqr < bDistSqr) { return  1; }
		if (aDistSqr > bDistSqr) { return -1; }
		return 0;
	}
};

// ========================================================
// ParticleEmitter::sortActiveParticles():
// ========================================================

void ParticleEmitter::sortActiveParticles(const Vector & eyePos)
{
#if 1

	// My custom `quickSort` outperformed `qsort` in the tests
	// I've ran. Will stick to it until proved otherwise.
	//
	PrtQSortPredicate pred;
	pred.cameraPosition = eyePos;
	quickSort(particles, activeParticleCount, pred);

#else

	// This is a hack to allow passing additional data to `qsort`.
	// This static var is set before the call, just below.
	static Vector cameraPosition;

	// Nested sort predicate for `qsort`:
	struct ParticleSortHelper
	{
		static int sortByCameraDist(const void * a, const void * b)
		{
			const Particle * prtA = rcast<const Particle *>(a);
			const Particle * prtB = rcast<const Particle *>(b);

			const float aDistSqr = distanceSqr(prtA->position, cameraPosition);
			const float bDistSqr = distanceSqr(prtB->position, cameraPosition);

			if (aDistSqr < bDistSqr) { return  1; }
			if (aDistSqr > bDistSqr) { return -1; }
			return 0;
		}
	};

	cameraPosition = eyePos;
	qsort(particles, activeParticleCount, sizeof(Particle), &ParticleSortHelper::sortByCameraDist);

#endif
}

// ========================================================
// ParticleEmitter::hasParticlesToDraw():
// ========================================================

bool ParticleEmitter::hasParticlesToDraw() const
{
	return particles != nullptr && activeParticleCount != 0;
}

// ========================================================
// ParticleEmitter::isVisible():
// ========================================================

bool ParticleEmitter::isVisible(const Frustum & frustum) const
{
	return frustum.testPoint(emitterOrigin);
}

// ========================================================
// ParticleEmitter::resetEmitter():
// ========================================================

void ParticleEmitter::resetEmitter()
{
	activeParticleCount = 0;
	particlesEmitted    = 0;
	lastUpdateMs        = 0;
	enabled             = true;
}

// ========================================================
// ParticleEmitter::setMinMaxParticles():
// ========================================================

void ParticleEmitter::setMinMaxParticles(uint min, uint max)
{
	ps2assert(max != 0);
	ps2assert(min <= max);

	if (max > MAX_PARTICLES_PER_EMITTER)
	{
		logWarning("Max amount of particles per emitter is %u!", MAX_PARTICLES_PER_EMITTER);
		max = MAX_PARTICLES_PER_EMITTER;
	}

	minParticles = min;
	maxParticles = max;
}

// ========================================================
// ParticleEmitter::getMinMaxParticles():
// ========================================================

void ParticleEmitter::getMinMaxParticles(uint & min, uint & max) const
{
	min = minParticles;
	max = maxParticles;
}

// ========================================================
// ParticleEmitter::getRandVector():
// ========================================================

Vector ParticleEmitter::getRandVector()
{
	// Pick a random Z between -1 and 1:
	const float z = randomFloat(-1.0f, 1.0f);

	// Get radius of this circle:
	const float r = ps2math::sqrt(1.0f - (z * z));

	// Pick a random point on the circle:
	const float t = randomFloat(-PS2MATH_PI, PS2MATH_PI);

	// Compute matching X and Y for Z:
	return Vector(ps2math::cos(t) * r, ps2math::sin(t) * r, z, 1.0f);
}
