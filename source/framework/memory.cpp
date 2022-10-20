
// ================================================================================================
// -*- C++ -*-
// File: memory.cpp
// Author: Guilherme R. Lampert
// Created on: 10/04/15
// Brief: Memory allocation helpers and memory accounting.
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

#include "common.hpp"
#include "memory.hpp"

// ========================================================
// Tag counters:
// ========================================================

struct MemTagCounters
{
	uint bytesAllocated;
	uint mallocCount;
	uint reallocCount;
	uint freeCount;
};

static MemTagCounters memoryTags[MEM_TAG_COUNT] ATTRIBUTE_ALIGNED(16);

// ========================================================
// memTagMalloc():
// ========================================================

void * memTagMalloc(const MemAllocTag tag, const size_t sizeBytes, const size_t alignment)
{
	memoryTags[tag].mallocCount++;
	memoryTags[tag].bytesAllocated += (sizeBytes + alignment);

	void * memory = memalign(alignment, sizeBytes);
	if (memory == nullptr)
	{
		fatalError("Failed to allocate %u bytes!", sizeBytes);
	}

	return memory;
}

// ========================================================
// memTagRealloc():
// ========================================================

void * memTagRealloc(const MemAllocTag tag, void * oldPtr, const size_t newSizeBytes)
{
	memoryTags[tag].reallocCount++;

	void * memory = realloc(oldPtr, newSizeBytes);
	if (memory == nullptr)
	{
		fatalError("Failed to re-allocate %u bytes!", newSizeBytes);
	}

	return memory;
}

// ========================================================
// memTagFree():
// ========================================================

void memTagFree(const MemAllocTag tag, void * ptr)
{
	memoryTags[tag].freeCount++;

	free(ptr);
}

// ========================================================
// formatMemoryUnit():
// ========================================================

const char * formatMemoryUnit(const size_t memorySizeInBytes, const bool abbreviated)
{
	static const size_t KILOBYTE = 1024;
	static const size_t MEGABYTE = 1024 * KILOBYTE;
	static const size_t GIGABYTE = 1024 * MEGABYTE;

	const char * memUnitStr;
	float adjustedSize;

	if (memorySizeInBytes < KILOBYTE)
	{
		memUnitStr   = abbreviated ? "B" : "Bytes";
		adjustedSize = scast<float>(memorySizeInBytes);
	}
	else if (memorySizeInBytes < MEGABYTE)
	{
		memUnitStr   = abbreviated ? "KB" : "Kilobytes";
		adjustedSize = memorySizeInBytes / 1024.0f;
	}
	else if (memorySizeInBytes < GIGABYTE)
	{
		memUnitStr   = abbreviated ? "MB" : "Megabytes";
		adjustedSize = memorySizeInBytes / 1024.0f / 1024.0f;
	}
	else
	{
		memUnitStr   = abbreviated ? "GB" : "Gigabytes";
		adjustedSize = memorySizeInBytes / 1024.0f / 1024.0f / 1024.0f;
	}

	char * fmtBuf;
	int  charsWritten;
	char numStrBuf[100];

	// Max chars reserved for the output string, max `numStrBuf + 28` chars for the unit str:
	enum { MEM_UNIT_STR_MAXLEN = sizeof(numStrBuf) + 28 };
	static int bufNum = 0;
	static char localStrBuf[8][MEM_UNIT_STR_MAXLEN];

	fmtBuf = localStrBuf[bufNum];
	bufNum = (bufNum + 1) & 7;

	// We only care about the first 2 decimal digs
	charsWritten = snprintf(numStrBuf, 100, "%.2f", adjustedSize);
	if (charsWritten <= 0)
	{
		fmtBuf[0] = '?';
		fmtBuf[1] = '?';
		fmtBuf[2] = '?';
		fmtBuf[3] = '\0';
		return fmtBuf; // Error return
	}

	// Remove trailing zeros if no significant decimal digits:
	removeTrailingFloatZeros(numStrBuf, nullptr);

	// Consolidate the string:
	charsWritten = snprintf(fmtBuf, MEM_UNIT_STR_MAXLEN, "%s %s", numStrBuf, memUnitStr);
	if (charsWritten <= 0)
	{
		fmtBuf[0] = '?';
		fmtBuf[1] = '?';
		fmtBuf[2] = '?';
		fmtBuf[3] = '\0';
		return fmtBuf; // Error return
	}

	return fmtBuf;
}

// ========================================================
// getMemTagsStr():
// ========================================================

const char * getMemTagsStr()
{
	// NOTE: New tags added to `MemAllocTag` should be updated here !!!
	static const char * tagNames[MEM_TAG_COUNT] =
	{
		"GENERIC",
		"CPP_NEW",
		"GEOMETRY",
		"TEXTURE",
		"PARTICLE",
		"RENDERER"
	};

	static char buffer[2048];
	int charsLeft = 2048, written = 0;
	char * pStr = buffer;

	written = snprintf(pStr, charsLeft,
		"| tag name | total mem | mallocs | reallocs | frees\n");
	charsLeft -= written, pStr += written;

	size_t usedRam = 0;
	for (uint t = 0; t < MEM_TAG_COUNT; ++t)
	{
		written = snprintf(pStr, charsLeft,
			"| %-8s | %-9s | %-7u | %-8u | %-5u\n",
			tagNames[t],
			formatMemoryUnit(memoryTags[t].bytesAllocated),
			memoryTags[t].mallocCount,
			memoryTags[t].reallocCount,
			memoryTags[t].freeCount);
		charsLeft -= written, pStr += written;

		usedRam += memoryTags[t].bytesAllocated;
	}

	written = snprintf(pStr, charsLeft, "RAM  used: ~%s\n",
			formatMemoryUnit(usedRam, false));
	charsLeft -= written, pStr += written;

	extern uint gVRamUsedBytes;
	written = snprintf(pStr, charsLeft, "VRAM used: ~%s\n",
			formatMemoryUnit(gVRamUsedBytes, false));

	return buffer;
}
