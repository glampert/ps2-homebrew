
// ================================================================================================
// -*- C++ -*-
// File: particle_emitter.hpp
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

#ifndef PARTICLE_EMITTER_HPP
#define PARTICLE_EMITTER_HPP

#include "renderer.hpp"
class Frustum;

// ========================================================
// struct Particle:
// ========================================================

struct Particle
{
	Vector position;   // Current world position of this particle.
	Vector velocity;   // Current velocity of this particle.
	float  size;       // Size/scale of this particle.
	uint   durationMs; // Time this particle will go inactive, in milliseconds.
};

// ========================================================
// class ParticleEmitter:
// ========================================================

class ParticleEmitter
{
public:

	// Constructor create a default particle emitter.
	 ParticleEmitter();
	~ParticleEmitter();

	// Allocate/free the underlaying array of particles:
	void allocateParticles();
	void freeParticles();

	// Set particle parameters to defaults. This will also reset the emitter.
	void setDefaults();

	// Update the particle emitter and particles alive.
	// Should be called every frame for particles in the view.
	void updateParticles();

	// Effectively renders all batched particles. Depth writing should be already disabled.
	void drawParticles(const Matrix & viewMatrix, const Vector & eyePos);

	// Check if there are pending particles to be rendered.
	bool hasParticlesToDraw() const;

	// Approximate test for the emitter bounds against a frustum.
	bool isVisible(const Frustum & frustum) const;

	// Reset the particle emitter to its initial state.
	void resetEmitter();

	// Get/set min/max number of particles in the emitter:
	void setMinMaxParticles(uint min, uint max);
	void getMinMaxParticles(uint & min, uint & max) const;

	// Get/set base particle color that is modulated with the texture color:
	void setParticleBaseColor(const Color4f & color) { baseColor = color; }
	const Color4f & getParticleBaseColor() const { return baseColor; }

	// Get/set max particle sizes:
	void setParticleMinSize(const float size) { minParticleSize = size; }
	float getParticleMinSize() const { return minParticleSize; }

	// Get/set min particle sizes:
	void setParticleMaxSize(const float size) { maxParticleSize = size; }
	float getParticleMaxSize() const { return maxParticleSize; }

	// Get/set particle lifetime milliseconds:
	void setParticleLifeCycle(const uint ms) { particleLifeCycleMs = ms; }
	uint getParticleLifeCycle() const { return particleLifeCycleMs; }

	// Get/set new particle release interval milliseconds (flow rate):
	void setParticleReleaseInterval(const uint ms) { particleReleaseIntervalMs = ms; }
	uint getParticleReleaseInterval() const { return particleReleaseIntervalMs; }

	// Get/set number of new particles to release at every release interval:
	void setParticleReleaseAmount(const uint amount) { particleReleaseAmount = amount; }
	uint getParticleReleaseAmount() const { return particleReleaseAmount; }

	// Get/set velocity vector of particles:
	void setParticleVelocity(const Vector & velocity) { particleVelocityVec = velocity; }
	const Vector & getParticleVelocity() const { return particleVelocityVec; }

	// Get/set particle emitter origin (in world coordinates):
	void setEmitterOrigin(const Vector & origin) { emitterOrigin = origin; }
	const Vector & getEmitterOrigin() const { return emitterOrigin; }

	// Get/set gravity force applied to particles:
	void setEmitterGravity(const Vector & gravity) { gravityVec = gravity; }
	const Vector & getEmitterGravity() const { return gravityVec; }

	// Get/set simulated wind velocity for this emitter:
	void setWindVelocity(const Vector & wind) { windVelocityVec = wind; }
	const Vector & getWindVelocity() const { return windVelocityVec; }

	// Get/set variable velocity. Zero is constant:
	void setVelocityVar(const float vel) { velocityVar = vel; }
	float getVelocityVar() const { return velocityVar; }

	// Enable/disable simulated air resistance:
	void setAirResistance(const bool airResistance) { simulateAirResistance = airResistance; }
	bool hasAirResistance() const { return simulateAirResistance; }

	// Get/set enabled flag:
	void setEnabled(const bool enable) { enabled = enable; }
	bool isEnabled() const { return enabled; }

	// Set the sprite used by all particles for this emitter.
	void setParticleSpriteTexture(const Texture * tex) { spriteTexture = tex; }
	const Texture * getParticleSpriteTexture() const { return spriteTexture;  }

	// Make particle emitter run once, emitting 'particleReleaseAmount' particles and then stopping.
	void setOneTimeEmission(const bool oneTime) { oneTimeEmission = oneTime; }
	bool isOneTimeEmission() const { return oneTimeEmission; }

	// If this is a one-time emitter, check if it has already emitted all particles.
	bool isOneTimeEmitterFinished() const { return oneTimeEmission && (particlesEmitted >= particleReleaseAmount); }

private:

	// Copy/assign disallowed.
	ParticleEmitter(const ParticleEmitter &);
	ParticleEmitter & operator = (const ParticleEmitter &);

	// Sort particle based on distance from viewer. Called pre rendering.
	void sortActiveParticles(const Vector & eyePos);

	// Generates a random vector where X, Y, and Z components are between -1 and 1.
	static Vector getRandVector();

private:

	// Shared state for all emitters:
	static uint16     * particleIndexBuffer;  // Two triangles per particle quadrilateral
	static DrawVertex * particleVertexBuffer; // Four vertexes per particle quadrilateral

	// Array of particles. Size equal to `maxParticles`.
	// Active particles are always at the front, from index 0 to `activeParticleCount-1`
	Particle * particles;

	// Reference to an external texture. Not owned by ParticleEmitter.
	const Texture * spriteTexture;

	// To avoid accessing the globals, keep a local copy
	// of each pointer in the hopes of better data locality.
	uint16     * ibPtr;
	DrawVertex * vbPtr;

	// Maximum number of active particles at any given time.
	uint maxParticles;

	// Minimum number of particles that should exist at any given time.
	// If this number is greater than zero, new particles are spawned when
	// old ones expire to keep at least this many in activity.
	uint minParticles;

	// Active particles are always kept in sequence,
	// from `particles[0]` to `particles[activeParticleCount - 1]`.
	uint activeParticleCount;

	// Number of particles emitted since creation or reset.
	uint particlesEmitted;

	// Amount of time each particle lives, in milliseconds:
	uint particleLifeCycleMs;

	// Particle release interval (flow rate) in milliseconds:
	uint particleReleaseIntervalMs;

	// Number of new particles to release at every release interval:
	uint particleReleaseAmount;

	// Last time the emitter was updated:
	uint lastUpdateMs;

	// Emitter origin (point) in world space:
	Vector emitterOrigin;

	// Particle velocity, gravity and wind vectors:
	Vector gravityVec;
	Vector particleVelocityVec;
	Vector windVelocityVec;

	// Velocity variation added to particles, combined with
	// a random direction vector. See `getRandVector()`.
	float velocityVar;

	// Minimum and maximum sizes of the particle quadrilaterals.
	// Sizes are randomized between these values.
	float minParticleSize;
	float maxParticleSize;

	// Base color / tint that modulates the sprite texture.
	Color4f baseColor;

	// True by default.
	bool enabled;

	// If set, simulate some air resistance using `windVelocityVec`:
	bool simulateAirResistance;

	// Will release `particleReleaseAmount` particles and stop. False by default.
	bool oneTimeEmission;
};

// ========================================================
// struct Billboard:
// ========================================================

struct Billboard
{
	Vector color;
	Vector origin;
	Vector points[4];
	Vector texCoords[4];
};

// ========================================================
// makeBillboardFacingCameraPlane():
// ========================================================

inline void makeBillboardFacingCameraPlane(Billboard & billboard,
		const Vector & up, const Vector & right, const Vector & origin,
		const Color4f & color, const float width, const float height)
{
	const Vector halfWidth = (width  * 0.5f) * right; // X
	const Vector halfHeigh = (height * 0.5f) * up;    // Y

	// Save the origin:
	billboard.origin = origin;

	// Set vertex color:
	billboard.color.x = color.r;
	billboard.color.y = color.g;
	billboard.color.z = color.b;
	billboard.color.w = color.a;

	// Create the points:
	billboard.points[0] = origin + halfWidth + halfHeigh;
	billboard.points[1] = origin - halfWidth + halfHeigh;
	billboard.points[2] = origin - halfWidth - halfHeigh;
	billboard.points[3] = origin + halfWidth - halfHeigh;

	// Set standard texture coords for a quadrilateral:
	billboard.texCoords[0] = Vector(0.0f, 0.0f, 0.0f, 1.0f);
	billboard.texCoords[1] = Vector(1.0f, 0.0f, 0.0f, 1.0f);
	billboard.texCoords[2] = Vector(1.0f, 1.0f, 0.0f, 1.0f);
	billboard.texCoords[3] = Vector(0.0f, 1.0f, 0.0f, 1.0f);
}

#endif // PARTICLE_EMITTER_HPP
