
// ================================================================================================
// -*- C++ -*-
// File: md2_model.cpp
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

#include "md2_model.hpp"
#include "ingame_console.hpp"

// ========================================================
// Local constants/data:
// ========================================================

namespace
{

//
// Standard MD2 animation frames with frame-rate:
//
const struct
{
	ubyte start;
	ubyte end;
	ubyte fps;
}
stdMd2Anims[MD2ANIM_COUNT] ATTRIBUTE_ALIGNED(16) =
{
	{   0,    39,   9 }, // Stand
	{  40,    45,  10 }, // Run
	{  46,    53,  10 }, // Attack
	{  54,    57,   7 }, // Pain A
	{  58,    61,   7 }, // Pain B
	{  62,    65,   7 }, // Pain C
	{  66,    71,   7 }, // Jump
	{  72,    83,   7 }, // Flip
	{  84,    94,   7 }, // Salute
	{  95,   111,  10 }, // Fall back
	{ 112,   122,   7 }, // Wave
	{ 123,   134,   6 }, // Point
	{ 135,   153,  10 }, // Crouch stand
	{ 154,   159,   7 }, // Crouch walk
	{ 160,   168,  10 }, // Crouch attack
	{ 196,   172,   7 }, // Crouch pain
	{ 173,   177,   5 }, // Crouch death
	{ 178,   183,   7 }, // Death fall back
	{ 184,   189,   7 }, // Death fall forward
	{ 190,   197,   7 }, // Death fall back slow
	{ 198,   198,   5 }  // Boom
};

} // namespace {}

// ================================================================================================
// Md2AnimState implementation:
// ================================================================================================

// ========================================================
// Md2AnimState::update():
// ========================================================

void Md2AnimState::update(const float timeSeconds, const uint keyframeCount)
{
	currTime = timeSeconds;
	const uint maxFrame = keyframeCount - 1;

	// Calculate current and next frames:
	if ((currTime - oldTime) > (1.0f / fps))
	{
		currFrame = nextFrame;
		nextFrame++;

		if (nextFrame > endFrame)
		{
			nextFrame = startFrame;
		}

		oldTime = currTime;
	}

	// Prevent having a current/next frame greater than the total number of frames:
	if (currFrame > maxFrame)
	{
		currFrame = 0;
	}
	if (nextFrame > maxFrame)
	{
		nextFrame = 0;
	}

	interp = fps * (currTime - oldTime);
}

// ========================================================
// Md2AnimState::clear():
// ========================================================

void Md2AnimState::clear()
{
	startFrame = 0;
	endFrame   = 0;
	fps        = 0;
	currFrame  = 0;
	nextFrame  = 0;
	currTime   = 0;
	oldTime    = 0;
	interp     = 0;
}

// ================================================================================================
// Md2Model implementation:
// ================================================================================================

// ========================================================
// Md2Model::Md2Model:
// ========================================================

Md2Model::Md2Model()
	: md2Header(nullptr)
	, md2Skins(nullptr)
	, md2TexCoords(nullptr)
	, md2Triangles(nullptr)
	, md2Keyframes(nullptr)
	, md2TriangleCount(0)
	, md2VertsPerFrame(0)
	, md2FrameCount(0)
	, md2Scale(1.0f)
{ }

// ========================================================
// Md2Model::initFromMemory():
// ========================================================

bool Md2Model::initFromMemory(const ubyte * data, const uint sizeBytes, const float scale)
{
	if (data == nullptr || sizeBytes == 0)
	{
		logError("Invalid data buffer!");
		return false;
	}

	logComment("Beginning MD2 import from data buffer...");

	// Fetch the header (first few bytes of data):
	md2Header = rcast<const Header *>(data);

	// Data validation section:
	if (md2Header->magic != make4CC('I', 'D', 'P', '2'))
	{
		logWarning("MD2 identifier doesn't match! Continuing anyway...");
		// Allow it to continue...
	}
	if (md2Header->version != MD2_GOOD_VERSION)
	{
		logWarning("MD2 version is %u! Only version %u is fully supported...", md2Header->version, MD2_GOOD_VERSION);
		// Allow it to continue...
	}

	// Validate num keyframes and file end offset:
	if (md2Header->frameCount == 0)
	{
		logError("Number of keyframes for MD2 is zero!");
		return false;
	}
	if (md2Header->offsetEnd > sizeBytes)
	{
		logError("MD2 buffer is too small! / corrupted file offset!");
		return false;
	}

	// Gotta have triangles and texCoords:
	if (md2Header->texCoordCount == 0)
	{
		logError("MD2 has no texture coordinates!");
		return false;
	}
	if (md2Header->triangleCount == 0)
	{
		logError("MD2 has no triangles!");
		return false;
	}

	// Validate the remaining offsets:
	if ((md2Header->offsetSkins     + md2Header->skinCount     * sizeof(Skin))     >= sizeBytes ||
	    (md2Header->offsetTexCoords + md2Header->texCoordCount * sizeof(TexCoord)) >= sizeBytes ||
	    (md2Header->offsetTris      + md2Header->triangleCount * sizeof(Triangle)) >= sizeBytes)
	{
		logError("MD2 header has invalid offsets!");
		return false;
	}

	// Save the vertex scaling factor:
	md2Scale = scale;

	// These are accessed frequently, so it is best to cache
	// them to avoid reading into `md2Header` memory all the time.
	md2TriangleCount = md2Header->triangleCount;
	md2VertsPerFrame = md2Header->vertsPerFrame;
	md2FrameCount    = md2Header->frameCount;

	// Check max values but only warn if they are invalid:
	if (md2Header->skinCount > MD2_MAX_SKINS)
	{
		logWarning("MD2 model data contains more skins than Quake II supports...");
	}
	if (md2Header->frameCount > MD2_MAX_KEYFRAMES)
	{
		logWarning("MD2 model data contains more keyframes than Quake II supports...");
	}
	if (md2Header->vertsPerFrame > MD2_MAX_VERTS)
	{
		logWarning("MD2 model data contains more vertexes than Quake II supports...");
	}
	if (md2Header->triangleCount > MD2_MAX_TRIS)
	{
		logWarning("MD2 model data contains more triangles than Quake II supports...");
	}

	logComment("Importing MD2 skins, texCoords and tris...");

	// Read skin names (optional):
	if (md2Header->skinCount != 0)
	{
		md2Skins = rcast<const Skin *>(data + md2Header->offsetSkins);
	}

	// Read texture coordinates and triangles (mandatory):
	md2TexCoords = rcast<const TexCoord *>(data + md2Header->offsetTexCoords);
	md2Triangles = rcast<const Triangle *>(data + md2Header->offsetTris);

	logComment("Importing MD2 keyframes...");

	// NOTE: Each keyframe is followed by a Vertex packet!
	md2Keyframes = rcast<const Keyframe *>(data + md2Header->offsetFrames);

	// Pre-allocate one bounding box for each frame:
	md2FrameBounds.resize(md2FrameCount);
	for (uint f = 0; f < md2FrameCount; ++f)
	{
		// Validate the offset:
		if ((f * md2VertsPerFrame) >= (md2FrameCount * md2VertsPerFrame))
		{
			logError("Bad vertex offset in MD2 frame #%u!", f);
			return false;
		}
		computeAabbForFrame(f);
	}

	// Finish by setting up animation frame numbers:
	setUpAminations();
	logComment("MD2 import completed!");
	return true;
}

// ========================================================
// Md2Model::assembleFrame():
// ========================================================

void Md2Model::assembleFrame(const uint frameIndex, const uint skinWidth, const uint skinHeight,
                             const Color4f & vertColor, DrawVertex * restrict drawVerts) const
{
	// Validation:
	ps2assert(md2Header    != nullptr);
	ps2assert(md2TexCoords != nullptr);
	ps2assert(md2Triangles != nullptr);
	ps2assert(md2Keyframes != nullptr);
	ps2assert(drawVerts    != nullptr);

	const Keyframe & frame = getKeyframe(frameIndex);
	const Vector vColor(vertColor.r, vertColor.g, vertColor.b, vertColor.a);

	// For each triangle:
	for (uint t = 0; t < md2TriangleCount; ++t)
	{
		// For each vertex of this triangle:
		for (uint v = 0; v < 3; ++v)
		{
			// Fetch correct vertex and texture coords:
			const Vertex   & vert = getFrameVertex(frameIndex, md2Triangles[t].vertex[v]);
			const TexCoord & texc = md2TexCoords[md2Triangles[t].uv[v]];

			// Compute final texture coords:
			const float uvX = scast<float>(texc.u) / skinWidth;
			const float uvY = scast<float>(texc.v) / skinHeight;

			// Uncompress vertex position and transform it.
			//
			// NOTE: We swap y-z because Quake's coordinate system is
			// different from the standard setup we are used to.
			//
			Vector xyzPos;
			xyzPos.x = ((frame.scale[0] * vert.v[0]) + frame.translate[0]) * md2Scale;
			xyzPos.z = ((frame.scale[1] * vert.v[1]) + frame.translate[1]) * md2Scale;
			xyzPos.y = ((frame.scale[2] * vert.v[2]) + frame.translate[2]) * md2Scale;

			// Store the new DrawVertex:
			packDrawVertex(drawVerts[v], xyzPos, Vector(uvX, uvY, 0.0f, 1.0f), vColor);
		}

		// Advance 3 vertexes (one triangle).
		drawVerts += 3;
	}
}

// ========================================================
// Md2Model::assembleFrameInterpolated():
// ========================================================

void Md2Model::assembleFrameInterpolated(const uint frameA, const uint frameB, const float interp, const uint skinWidth,
                                         const uint skinHeight, const Color4f & vertColor, DrawVertex * restrict drawVerts) const
{
	// Validation:
	ps2assert(md2Header    != nullptr);
	ps2assert(md2TexCoords != nullptr);
	ps2assert(md2Triangles != nullptr);
	ps2assert(md2Keyframes != nullptr);
	ps2assert(drawVerts    != nullptr);

	const Keyframe & keyFrameA = getKeyframe(frameA);
	const Keyframe & keyFrameB = getKeyframe(frameB);
	const Vector vColor(vertColor.r, vertColor.g, vertColor.b, vertColor.a);

	// For each triangle:
	for (uint t = 0; t < md2TriangleCount; ++t)
	{
		// For each vertex of this triangle:
		for (uint v = 0; v < 3; ++v)
		{
			const Vertex   & vertA = getFrameVertex(frameA, md2Triangles[t].vertex[v]);
			const Vertex   & vertB = getFrameVertex(frameB, md2Triangles[t].vertex[v]);
			const TexCoord & texc  = md2TexCoords[md2Triangles[t].uv[v]];

			// Compute final texture coords:
			const float uvX = scast<float>(texc.u) / skinWidth;
			const float uvY = scast<float>(texc.v) / skinHeight;

			// Compute final vertex position:
			//
			// NOTE: We swap y-z because Quake's coordinate system is
			// different from the standard setup we are used to.
			//
			Vector vecA, vecB;
			vecA.x = (keyFrameA.scale[0] * vertA.v[0]) + keyFrameA.translate[0];
			vecA.z = (keyFrameA.scale[1] * vertA.v[1]) + keyFrameA.translate[1];
			vecA.y = (keyFrameA.scale[2] * vertA.v[2]) + keyFrameA.translate[2];
			vecB.x = (keyFrameB.scale[0] * vertB.v[0]) + keyFrameB.translate[0];
			vecB.z = (keyFrameB.scale[1] * vertB.v[1]) + keyFrameB.translate[1];
			vecB.y = (keyFrameB.scale[2] * vertB.v[2]) + keyFrameB.translate[2];

			// Linearly interpolate final position and scale:
			Vector xyzPos;
			lerpScale(xyzPos, vecA, vecB, interp, md2Scale);

			// Store the new DrawVertex:
			packDrawVertex(drawVerts[v], xyzPos, Vector(uvX, uvY, 0.0f, 1.0f), vColor);
		}

		// Advance 3 vertexes (one triangle).
		drawVerts += 3;
	}
}

// ========================================================
// Md2Model::setUpAminations():
// ========================================================

void Md2Model::setUpAminations()
{
	Anim animInfo;
	char frameAnim[MD2_MAX_NAME_CHARS * 2];
	char currentAnim[MD2_MAX_NAME_CHARS * 2];

	memset(&animInfo,   0, sizeof(animInfo));
	memset(currentAnim, 0, sizeof(currentAnim));

	md2Anims.reserve(md2FrameCount);
	for (uint i = 0; i < md2FrameCount; ++i)
	{
		memset(frameAnim, 0, sizeof(frameAnim));
		const char * frameName = getKeyframe(i).name;

		// Extract animation name from frame name:
		// MD2 anim names are in the form <animName><frameNum>, such as "death001".
		//
		// `strcspn()` will find the index of the first occurrence
		// of any number in the frame name.
		//
		size_t len = strcspn(frameName, "0123456789");
		if ((len == strlen(frameName) - 3) && (frameName[len] != '0'))
		{
			++len;
		}

		strncpy(frameAnim, frameName, len);

		if (strcmp(currentAnim, frameAnim) != 0)
		{
			if (i > 0)
			{
				// Previous animation is finished, store it and start new animation.
				strncpy(animInfo.name, currentAnim, sizeof(animInfo.name));
				md2Anims.pushBack(animInfo);
			}

			// Initialize new anim info:
			animInfo.start = i;
			animInfo.end   = i;
			strncpy(currentAnim, frameAnim, sizeof(currentAnim));
		}
		else
		{
			animInfo.end = i;
		}
	}

	// Insert last animation:
	strncpy(animInfo.name, currentAnim, sizeof(animInfo.name));
	md2Anims.pushBack(animInfo);
}

// ========================================================
// Md2Model::computeAabbForFrame():
// ========================================================

void Md2Model::computeAabbForFrame(const uint frameIndex)
{
	Aabb & frameAabb = md2FrameBounds[frameIndex];
	frameAabb.clear();

	const Keyframe & keyframe = getKeyframe(frameIndex);

	for (uint t = 0; t < md2TriangleCount; ++t)
	{
		for (uint v = 0; v < 3; ++v)
		{
			const Vertex & vert = getFrameVertex(frameIndex, md2Triangles[t].vertex[v]);

			// Uncompress vertex position and transform it (swizzle y/z):
			Vector xyzPos;
			xyzPos.x = (keyframe.scale[0] * vert.v[0] + keyframe.translate[0]) * md2Scale;
			xyzPos.z = (keyframe.scale[1] * vert.v[1] + keyframe.translate[1]) * md2Scale;
			xyzPos.y = (keyframe.scale[2] * vert.v[2] + keyframe.translate[2]) * md2Scale;
			xyzPos.w = 1.0f;

			// Add to bounds if new min|max:
			frameAabb.mins = min3PerElement(xyzPos, frameAabb.mins);
			frameAabb.maxs = max3PerElement(xyzPos, frameAabb.maxs);
		}
	}
}

// ========================================================
// Md2Model::printAnimList():
// ========================================================

void Md2Model::printAnimList() const
{
	gConsole.print("|  md2-anin-name  | start |  end  |\n");
	for (uint i = 0; i < md2Anims.size(); ++i)
	{
		gConsole.print(format("| %-15s | %-5u | %-5u |\n",
			md2Anims[i].name, md2Anims[i].start, md2Anims[i].end));
	}
	gConsole.print(format("listed %u animations\n", md2Anims.size()));
}

// ========================================================
// Md2Model::findAnimByName():
// ========================================================

bool Md2Model::findAnimByName(const char * name, uint & start, uint & end, uint & fps) const
{
	ps2assert(name != nullptr);
	for (uint i = 0; i < md2Anims.size(); ++i)
	{
		if (strcmp(md2Anims[i].name, name) == 0)
		{
			start = md2Anims[i].start;
			end   = md2Anims[i].end;
			fps   = getAnimFpsForRange(start, end);
			return true;
		}
	}
	// Not fount.
	start = end = fps = 0;
	return false;
}

// ========================================================
// Md2Model::findAnimByPartialName():
// ========================================================

bool Md2Model::findAnimByPartialName(const char * partialName, uint & start, uint & end, uint & fps) const
{
	ps2assert(partialName != nullptr);
	for (uint i = 0; i < md2Anims.size(); ++i)
	{
		if (strstr(md2Anims[i].name, partialName) != nullptr)
		{
			start = md2Anims[i].start;
			end   = md2Anims[i].end;
			fps   = getAnimFpsForRange(start, end);
			return true;
		}
	}
	// Not fount.
	start = end = fps = 0;
	return false;
}

// ========================================================
// Md2Model::findAnimByIndex():
// ========================================================

bool Md2Model::findAnimByIndex(const uint index, uint & start, uint & end, uint & fps) const
{
	if (index >= md2Anims.size())
	{
		start = end = fps = 0;
		return false;
	}

	start = md2Anims[index].start;
	end   = md2Anims[index].end;
	fps   = getAnimFpsForRange(start, end);
	return true;
}

// ========================================================
// Md2Model::getStdAnim():
// ========================================================

void Md2Model::getStdAnim(const StdMd2AnimId id, uint & start, uint & end, uint & fps)
{
	ps2assert(id >= 0 && id < MD2ANIM_COUNT);
	start = stdMd2Anims[id].start;
	end   = stdMd2Anims[id].end;
	fps   = stdMd2Anims[id].fps;
}

// ========================================================
// Md2Model::getAnimFpsForRange():
// ========================================================

uint Md2Model::getAnimFpsForRange(const uint start, const uint end)
{
	// Return the FPS if we find an exact match.
	// The default value otherwise.
	//
	for (uint i = 0; i < arrayLength(stdMd2Anims); ++i)
	{
		if (start == stdMd2Anims[i].start && end == stdMd2Anims[i].end)
		{
			return stdMd2Anims[i].fps;
		}
	}
	return MD2ANIM_DEFAULT_FPS;
}

// ========================================================
// Md2Model::getKeyframe():
// ========================================================

const Md2Model::Keyframe & Md2Model::getKeyframe(const uint frameIndex) const
{
	ps2assert(frameIndex < md2FrameCount);

	// Start of the keyframe data:
	const ubyte * framesPtr = rcast<const ubyte *>(md2Keyframes);

	// Each keyframe is a single `Keyframe` struct + a vertex packet with `md2VertsPerFrame` vertexes:
	const uint keyframePacketSize = sizeof(Keyframe) + (sizeof(Vertex) * md2VertsPerFrame);

	// Skip to the frame we want:
	return *rcast<const Keyframe *>(framesPtr + (frameIndex * keyframePacketSize));
}

// ========================================================
// Md2Model::getFrameVertex():
// ========================================================

const Md2Model::Vertex & Md2Model::getFrameVertex(const uint frameIndex, const uint vertexIndex) const
{
	ps2assert(frameIndex < md2FrameCount);

	// Start of the keyframe data:
	const ubyte * vertsPtr = rcast<const ubyte *>(md2Keyframes);

	// Each keyframe is a single `Keyframe` struct + a vertex packet with `md2VertsPerFrame` vertexes:
	const uint keyframePacketSize = sizeof(Keyframe) + (sizeof(Vertex) * md2VertsPerFrame);

	// Skip to the frame we want:
	vertsPtr += (frameIndex * keyframePacketSize);

	// Add the size of a `Keyframe` instance to get to the vertexes:
	vertsPtr += sizeof(Keyframe);

	// Cast the pointer to `Vertex` and index the vertex we want:
	return rcast<const Vertex *>(vertsPtr)[vertexIndex];
}

// ========================================================
// Md2Model::getBoundsForFrame():
// ========================================================

const Aabb & Md2Model::getBoundsForFrame(const uint frameIndex) const
{
	return md2FrameBounds[frameIndex];
}

// ========================================================
// Md2Model::getKeyframeCount():
// ========================================================

uint Md2Model::getKeyframeCount() const
{
	return md2FrameCount;
}

// ========================================================
// Md2Model::getVertexCount():
// ========================================================

uint Md2Model::getVertexCount() const
{
	return md2FrameCount * md2VertsPerFrame;
}

// ========================================================
// Md2Model::getTriangleCount():
// ========================================================

uint Md2Model::getTriangleCount() const
{
	return md2TriangleCount;
}

// ========================================================
// Md2Model::getAnimCount():
// ========================================================

uint Md2Model::getAnimCount() const
{
	return md2Anims.size();
}

// ========================================================
// Md2Model::getModelScale():
// ========================================================

float Md2Model::getModelScale() const
{
	return md2Scale;
}
