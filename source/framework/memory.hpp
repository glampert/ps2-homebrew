
// ================================================================================================
// -*- C++ -*-
// File: memory.hpp
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

#ifndef MEMORY_HPP
#define MEMORY_HPP

// ========================================================
// enum MemAllocTag:
// ========================================================

enum MemAllocTag
{
	MEM_TAG_GENERIC,   // Generic / untagged allocations.
	MEM_TAG_CPP_NEW,   // Allocated with operator new (generic allocation).
	MEM_TAG_GEOMETRY,  // Any geometry / vertexes / MD2.
	MEM_TAG_TEXTURE,   // Textures/images allocated on the heap.
	MEM_TAG_PARTICLES, // Particle emitters.
	MEM_TAG_RENDERER,  // Renderer misc & render packets.

	// Internal use.
	MEM_TAG_COUNT
};

// ========================================================
// Memory allocations functions:
// ========================================================

// All allocations are 16bytes aligned. This facilitates
// writing code that takes advantage of the PS2's SIMD capabilities.
const size_t DEFAULT_MEM_ALIGNMENT = 16;

//
// Low-level `void*` allocations:
//

void * memTagMalloc(MemAllocTag tag, size_t sizeBytes, size_t alignment);
void * memTagRealloc(MemAllocTag tag, void * oldPtr, size_t newSizeBytes);
void   memTagFree(MemAllocTag tag, void * ptr);

//
// Templated helper functions.
// Allocate in terms of `T` instances.
//

template<class T>
inline T * memAlloc(MemAllocTag tag, size_t elementCount, size_t alignment = DEFAULT_MEM_ALIGNMENT)
{
	return reinterpret_cast<T *>(memTagMalloc(tag, elementCount * sizeof(T), alignment));
}

template<class T>
inline T * memClearedAlloc(MemAllocTag tag, size_t elementCount, size_t alignment = DEFAULT_MEM_ALIGNMENT)
{
	void * memory = memTagMalloc(tag, elementCount * sizeof(T), alignment);
	memset(memory, 0, elementCount * sizeof(T));
	return reinterpret_cast<T *>(memory);
}

template<class T>
inline T * memRealloc(MemAllocTag tag, T * oldPtr, size_t newElementCount)
{
	return reinterpret_cast<T *>(memTagRealloc(tag, oldPtr, newElementCount * sizeof(T)));
}

inline void memFree(MemAllocTag tag, void * ptr)
{
	if (ptr != nullptr)
	{
		memTagFree(tag, ptr);
	}
}

// ========================================================
// Overrides for operator `new` and `delete`:
// ========================================================

//
// With implicit MEM_TAG_CPP_NEW:
//

inline void * operator new (size_t size)
{
	return memTagMalloc(MEM_TAG_CPP_NEW, size, DEFAULT_MEM_ALIGNMENT);
}

inline void operator delete (void * ptr)
{
	if (ptr != nullptr)
	{
		memTagFree(MEM_TAG_CPP_NEW, ptr);
	}
}

inline void * operator new[] (size_t size)
{
	return memTagMalloc(MEM_TAG_CPP_NEW, size, DEFAULT_MEM_ALIGNMENT);
}

inline void operator delete[] (void * ptr)
{
	if (ptr != nullptr)
	{
		memTagFree(MEM_TAG_CPP_NEW, ptr);
	}
}

//
// With explicit memory tag:
//

inline void * operator new (size_t size, MemAllocTag tag)
{
	return memTagMalloc(tag, size, DEFAULT_MEM_ALIGNMENT);
}

template<class T>
inline void deleteSingle(MemAllocTag tag, T * ptr)
{
	if (ptr != nullptr)
	{
		ptr->~T();
		memTagFree(tag, ptr);
	}
}

inline void * operator new[] (size_t size, MemAllocTag tag)
{
	return memTagMalloc(tag, size, DEFAULT_MEM_ALIGNMENT);
}

template<class T>
inline void deleteArray(MemAllocTag tag, T * ptr, size_t count)
{
	if (ptr != nullptr)
	{
		for (size_t i = 0; i < count; ++i)
		{
			ptr[i].~T();
		}
		memTagFree(tag, ptr);
	}
}

// ========================================================
// Misc helpers:
// ========================================================

// Both return statically allocated buffers that should not be freed!
const char * formatMemoryUnit(size_t memorySizeInBytes, bool abbreviated = true);
const char * getMemTagsStr();

#endif // MEMORY_HPP
