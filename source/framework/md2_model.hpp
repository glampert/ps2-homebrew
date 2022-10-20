
// ================================================================================================
// -*- C++ -*-
// File: md2_model.hpp
// Author: Guilherme R. Lampert
// Created on: 10/01/15
// Brief: Quake MD2 model class and importer.
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

#ifndef MD2_MODEL_HPP
#define MD2_MODEL_HPP

#include "common.hpp"
#include "array.hpp"
#include "renderer.hpp"

// ========================================================
// MD2 constants:
// ========================================================

// Used for non-standard animations.
const uint MD2ANIM_DEFAULT_FPS = 7;

// Standard MD2 animations, with predefined frame numbers and FPS.
enum StdMd2AnimId
{
	MD2ANIM_STAND,
	MD2ANIM_RUN,
	MD2ANIM_ATTACK,
	MD2ANIM_PAIN_A,
	MD2ANIM_PAIN_B,
	MD2ANIM_PAIN_C,
	MD2ANIM_JUMP,
	MD2ANIM_FLIP,
	MD2ANIM_SALUTE,
	MD2ANIM_FALL_BACK,
	MD2ANIM_WAVE,
	MD2ANIM_POINT,
	MD2ANIM_CROUCH_STAND,
	MD2ANIM_CROUCH_WALK,
	MD2ANIM_CROUCH_ATTACK,
	MD2ANIM_CROUCH_PAIN,
	MD2ANIM_CROUCH_DEATH,
	MD2ANIM_DEATH_FALL_BACK,
	MD2ANIM_DEATH_FALL_FORWARD,
	MD2ANIM_DEATH_FALL_BACK_SLOW,
	MD2ANIM_BOOM,

	// Number of entries in this enum. Internal use.
	MD2ANIM_COUNT
};

// ========================================================
// class Md2AnimState:
// ========================================================

class Md2AnimState
{
public:

	uint  startFrame; // Fist frame of animation.
	uint  endFrame;   // Last frame of animation (inclusive).
	uint  fps;        // Animation frames-per-second.
	uint  currFrame;  // Current frame for this animation.
	uint  nextFrame;  // Next frame for this animation.
	float currTime;   // Current time in seconds.
	float oldTime;    // Old time in seconds.
	float interp;     // Percent of interpolation for this animation.

	// Sets all fields to zero on initialization.
	Md2AnimState() { clear(); }

	// Updates the animation state. Should be called every
	// frame with the current system time in seconds.
	void update(float timeSeconds, uint keyframeCount);

	// Sets all fields to zero.
	void clear();
};

// ========================================================
// class Md2Model:
// ========================================================

class Md2Model
{
public:

	Md2Model();

	// Initializes the model from a memory hunk containing the MD2 model data
	// (possibly originated from a file). The model class will keep references
	// to offsets into this buffer, so the memory should remain valid until
	// the class itself is disposed.
	bool initFromMemory(const ubyte * data, uint sizeBytes, float scale = 1.0f);

	// Generates a vertex array for a given animation frame.
	// `drawVerts[]` must be `triangleCount * 3` elements in size.
	void assembleFrame(uint frameIndex, uint skinWidth, uint skinHeight,
	                   const Color4f & vertColor, DrawVertex * drawVerts) const;

	// Generated a vertex array for a given frame, from the compressed MD2 frame data.
	// `drawVerts[]` must be `triangleCount * 3` elements in size.
	void assembleFrameInterpolated(uint frameA, uint frameB, float interp, uint skinWidth, uint skinHeight,
	                               const Color4f & vertColor, DrawVertex * drawVerts) const;

	// Prints a list of animations to the console.
	void printAnimList() const;

	// Find an animation by exact or partial name. Frames-per-second might be an estimate.
	bool findAnimByName(const char * name, uint & start, uint & end, uint & fps) const;
	bool findAnimByPartialName(const char * partialName, uint & start, uint & end, uint & fps) const;

	// Get an animation by its index in the animation list. Index ranges from 0 to getAnimCount()-1.
	bool findAnimByIndex(uint index, uint & start, uint & end, uint & fps) const;

	// Get the range and frame-rate of an standard MD2 animation.
	static void getStdAnim(StdMd2AnimId id, uint & start, uint & end, uint & fps);

	// Get the frame-rate of an animation if it is within a known range. A default value otherwise.
	static uint getAnimFpsForRange(uint start, uint end);

	// Bounding box for a given frame.
	const Aabb & getBoundsForFrame(uint frameIndex) const;

	// Data accessors:
	uint  getKeyframeCount() const;
	uint  getVertexCount()   const;
	uint  getTriangleCount() const;
	uint  getAnimCount()     const;
	float getModelScale()    const;

private:

	// Copy/assign disallowed.
	Md2Model(const Md2Model &);
	Md2Model & operator = (const Md2Model &);

	// MD2 constants:
	static const uint MD2_GOOD_VERSION   = 8;    // Version used on Quake II.
	static const uint MD2_MAX_VERTS      = 2048; // Per keyframe.
	static const uint MD2_MAX_TEXCOORDS  = 2048; // For the whole model.
	static const uint MD2_MAX_TRIS       = 4096; // For the whole model.
	static const uint MD2_MAX_KEYFRAMES  = 512;  // For the whole model.
	static const uint MD2_MAX_SKINS      = 32;   // For the whole model.
	static const uint MD2_MAX_NAME_CHARS = 64;   // Skins/anims.

	// MD2 header:
	struct ATTRIBUTE_PACKED Header
	{
		uint32 magic;           // Magic number, "IDP2".
		uint32 version;         // MD2 format version, should be 8.
		uint32 skinWidth;       // Texture width.
		uint32 skinHeight;      // Texture height.
		uint32 frameSize;       // Size of a frame, in bytes.
		uint32 skinCount;       // Number of skins.
		uint32 vertsPerFrame;   // Number of vertexes per frame.
		uint32 texCoordCount;   // Number of texture coords.
		uint32 triangleCount;   // Number of triangles.
		uint32 glCmdCount;      // Number of OpenGL commands.
		uint32 frameCount;      // Number of frames.
		uint32 offsetSkins;     // Offset to skin data.
		uint32 offsetTexCoords; // Offset to texture coords.
		uint32 offsetTris;      // Offset to triangle data.
		uint32 offsetFrames;    // Offset to frame data.
		uint32 offsetGLCmds;    // Offset to OpenGL commands.
		uint32 offsetEnd;       // Offset to the end of the file.
	};

	// Skin (texture) filename:
	struct ATTRIBUTE_PACKED Skin
	{
		char name[MD2_MAX_NAME_CHARS]; // Null terminated filename string.
	};

	// Texture coordinates (UVs):
	struct ATTRIBUTE_PACKED TexCoord
	{
		uint16 u, v;
	};

	// Triangle data:
	struct ATTRIBUTE_PACKED Triangle
	{
		uint16 vertex[3]; // Triangle's vertex indexes.
		uint16 uv[3];     // Texture coordinate indexes.
	};

	// Vertex data:
	struct ATTRIBUTE_PACKED Vertex
	{
		uint8 v[3];        // Compressed vertex position.
		uint8 normalIndex; // Normal vector index into the emblematic MD2 normals table.
	};

	// Keyframe data:
	struct ATTRIBUTE_PACKED Keyframe
	{
		float scale[3];     // Scale factors.
		float translate[3]; // Translation vector.
		char  name[16];     // Frame name.

		// Followed by a vertex packet:
		// Vertex frameVerts[header.vertsPerFrame];
	};

	// Animation info (frame indexes and anim name):
	struct ATTRIBUTE_PACKED Anim
	{
		uint start, end;
		char name[MD2_MAX_NAME_CHARS];
	};

	// Internal helpers:
	void setUpAminations();
	void computeAabbForFrame(uint frameIndex);
	const Keyframe & getKeyframe(uint frameIndex) const;
	const Vertex & getFrameVertex(uint frameIndex, uint vertexIndex) const;

private:

	// Pointers to extensional data:
	const Header   * md2Header;
	const Skin     * md2Skins;
	const TexCoord * md2TexCoords;
	const Triangle * md2Triangles;
	const Keyframe * md2Keyframes;

	// Cached for quick access.
	uint md2TriangleCount;
	uint md2VertsPerFrame;
	uint md2FrameCount;

	// Scaling applied to model vertexes; (1.0) by default.
	float md2Scale;

	// Data owned by Md2Model:
	Array<Anim> md2Anims;
	Array<Aabb> md2FrameBounds;
};

#endif // MD2_MODEL_HPP
