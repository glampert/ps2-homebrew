
// ================================================================================================
// -*- C++ -*-
// File: array.hpp
// Author: Guilherme R. Lampert
// Created on: 12/01/15
// Brief: Dynamic array template class, a minimal replacement for std::vector<T> (POD types only).
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

#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "common.hpp"

// ========================================================
// template class Array<T>:
// ========================================================

//
// NOTE: This class will only work with Plain Old Data (POD) types!
// No constructors or destructor are called for type T.
// Copies on reallocation are done with `memcpy()/memmove()`.
//
// Only allocates memory on first insertion, if constructed
// with default `Array<T>()` empty constructor.
//
template<class T>
class Array
{
public:

	typedef unsigned int size_type;
	typedef T value_type;

	typedef T& reference;
	typedef const T& const_reference;

	typedef T* pointer;
	typedef const T* const_pointer;

public:

	 Array();
	~Array();

	explicit Array(size_type newSize);
	explicit Array(size_type newSize, const value_type & fillVal);

	void reserve(size_type newCapacity);
	void resize(size_type newSize, const value_type & fillVal = value_type());
	void clear(); // Also frees the memory!

	void pushBack(const value_type & val);
	void popBack();

	void insert(size_type index, const value_type & val);
	int addUnique(const value_type & val);
	int contains(const value_type & val) const;

	void fill(const value_type & val);
	void remove(const value_type & val);
	void erase(size_type index);
	void sort(int (*predicate)(const void * a, const void * b));

	bool isEmpty() const;
	size_type size() const;
	size_type capacity() const;

	const_reference at(const size_type index) const;
	reference at(const size_type index);

	const_reference operator [] (const size_type index) const;
	reference operator [] (const size_type index);

	const_pointer data() const;
	pointer data();

private:

	// Copy/assign disallowed.
	Array(const Array &);
	Array & operator = (const Array &);

	// Member data:
	value_type * ptr;   // Array with `total` elements.
	size_type    total; // Total elements allocated.
	size_type    used;  // Elements committed.

	// Extra elements per allocation (Powers of 2):
	enum { ALLOCEXTRA = ( (sizeof(T) <= 1) ? 64
	                    : (sizeof(T) <= 2) ? 32
	                    : (sizeof(T) <= 4) ? 16
	                    : (sizeof(T) <= 8) ? 8 : 4 ) };
};

// ========================================================
// Inline methods of Array<T>:
// ========================================================

template<class T>
Array<T>::Array()
	: ptr(nullptr)
	, total(0)
	, used(0)
{
	// Construct empty.
}

template<class T>
Array<T>::Array(const size_type newSize)
	: ptr(nullptr)
	, total(0)
	, used(0)
{
	resize(newSize);
}

template<class T>
Array<T>::Array(const size_type newSize, const value_type & fillVal)
	: ptr(nullptr)
	, total(0)
	, used(0)
{
	resize(newSize, fillVal);
}

template<class T>
Array<T>::~Array()
{
	if (ptr != nullptr)
	{
		memFree(MEM_TAG_GENERIC, ptr);
	}
}

template<class T>
void Array<T>::reserve(const size_type newCapacity)
{
	if (newCapacity <= total)
	{
		return;
	}

	value_type * oldPtr = ptr;

	total = newCapacity + ALLOCEXTRA;
	ptr   = memAlloc<T>(MEM_TAG_GENERIC, total);

	if (ptr == nullptr)
	{
		fatalError("Out-of-memory on Array<T> reallocation!");
	}

	if (oldPtr != nullptr)
	{
		memcpy(ptr, oldPtr, used * sizeof(T));
		memFree(MEM_TAG_GENERIC, oldPtr);
	}
}

template<class T>
void Array<T>::resize(const size_type newSize, const value_type & fillVal)
{
	reserve(newSize);
	ps2assert(total >= newSize);
	used = newSize;
	fill(fillVal);
}

template<class T>
void Array<T>::clear()
{
	if (ptr != nullptr)
	{
		memFree(MEM_TAG_GENERIC, ptr);
		ptr   = nullptr;
		total = 0;
		used  = 0;
	}
}

template<class T>
void Array<T>::pushBack(const value_type & val)
{
	if (used == total)
	{
		reserve(total + 1);
	}
	ptr[used++] = val;
}

template<class T>
void Array<T>::popBack()
{
	if (used != 0)
	{
		--used;
	}
}

template<class T>
void Array<T>::insert(const size_type index, const value_type & val)
{
	ps2assert(ptr != nullptr);
	ps2assert(index <= size());

	if (total <= used)
	{
		reserve(total + 1);
	}
	if (index < used)
	{
		ubyte * bytes = rcast<ubyte *>(ptr);
		memmove(
			bytes + (index + 1) * sizeof(T),
			bytes + (index + 0) * sizeof(T),
			(used - index) * sizeof(T));
	}
	++used;
	ptr[index] = val;
}

template<class T>
int Array<T>::addUnique(const value_type & val)
{
	const int res = contains(val);
	if (res == 0)
	{
		pushBack(val);
	}
	return res;
}

template<class T>
int Array<T>::contains(const value_type & val) const
{
	int count = 0;
	for (size_type i = 0; i < used; i++)
	{
		if (ptr[i] == val)
		{
			++count;
		}
	}
	return count;
}

template<class T>
void Array<T>::fill(const value_type & val)
{
	for (size_type i = 0; i < used; i++)
	{
		ptr[i] = val;
	}
}

template<class T>
void Array<T>::remove(const value_type & val)
{
	for (size_type i = 0; i < used; i++)
	{
		if (ptr[i] == val)
		{
			erase(i);
		}
	}
}

template<class T>
void Array<T>::erase(const size_type index)
{
	if (isEmpty() || index >= size())
	{
		return;
	}
	if (--used == 0)
	{
		return;
	}
	memmove(&ptr[index], &ptr[index + 1], used * sizeof(T));
}

template<class T>
void Array<T>::sort(int (*predicate)(const void * a, const void * b))
{
	qsort(ptr, used, sizeof(T), predicate);
}

template<class T>
bool Array<T>::isEmpty() const
{
	return used == 0;
}

template<class T>
typename Array<T>::size_type Array<T>::size() const
{
	return used;
}

template<class T>
typename Array<T>::size_type Array<T>::capacity() const
{
	return total;
}

template<class T>
typename Array<T>::const_reference Array<T>::at(const size_type index) const
{
	ps2assert(index < size());
	return ptr[index];
}

template<class T>
typename Array<T>::reference Array<T>::at(const size_type index)
{
	ps2assert(index < size());
	return ptr[index];
}

template<class T>
typename Array<T>::const_reference Array<T>::operator [] (const size_type index) const
{
	ps2assert(index < size());
	return ptr[index];
}

template<class T>
typename Array<T>::reference Array<T>::operator [] (const size_type index)
{
	ps2assert(index < size());
	return ptr[index];
}

template<class T>
typename Array<T>::const_pointer Array<T>::data() const
{
	return ptr;
}

template<class T>
typename Array<T>::pointer Array<T>::data()
{
	return ptr;
}

#endif // ARRAY_HPP
