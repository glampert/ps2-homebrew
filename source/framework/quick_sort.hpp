
// ================================================================================================
// -*- C++ -*-
// File: quick_sort.hpp
// Author: Guilherme R. Lampert
// Created on: 10/04/15
// Brief: Custom templated Quick Sort function. Slightly faster than C's `qsort` on my profiling.
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

#ifndef QUICK_SORT_HPP
#define QUICK_SORT_HPP

#include "common.hpp"

// ========================================================
// quickSort<T, PRED>():
// ========================================================

// Standard quick-sort function for arrays.
// Predicate takes the form: `int pred(const T & a, const T & b)`, returning -1,0,+1.
template<class T, class PRED>
inline void quickSort(T * array, const uint elementCount, const PRED & pred)
{
	ps2assert(array != nullptr);
	if (elementCount == 0)
	{
		return; // Empty array
	}

	// This should be defined to the fastest integer type available
	// in the platform. Should not be smaller in size than `uint`.
	typedef int32 quickInt;

	// Artificial stacks. Avoids recursion.
	static const quickInt MAX_LEVELS = 128;
	quickInt lo[MAX_LEVELS] ATTRIBUTE_ALIGNED(16);
	quickInt hi[MAX_LEVELS] ATTRIBUTE_ALIGNED(16);

	// `lo` is the lower index, `hi` is the upper index
	// of the region of the array that is being sorted.
	lo[0] = 0;
	hi[0] = elementCount - 1;

	for (quickInt level = 0; level >= 0;)
	{
		quickInt i = lo[level];
		quickInt j = hi[level];

		// Only use quick-sort when there are 4 or more elements in this region
		// and we are below `MAX_LEVELS`. Otherwise fall back to an insertion-sort.
		if ((j - i) >= 4 && level < (MAX_LEVELS - 1))
		{
			// Use the center element as the pivot.
			// The median of a multi point sample could be used
			// but simply taking the center works quite well.
			const quickInt p = (i + j) / 2;

			// Move the pivot element to the end of the region.
			swap(array[j], array[p]);

			// Get a reference to the pivot element.
			T & pivot = array[j--];

			// Partition the region:
			do
			{
				while (pred(array[i], pivot) < 0) { if (++i >= j) break; }
				while (pred(array[j], pivot) > 0) { if (--j <= i) break; }
				if (i >= j) { break; }
				swap(array[i], array[j]);
			}
			while (++i < --j);

			// Without these iterations sorting of arrays with many duplicates may
			// become really slow because the partitioning can be very unbalanced.
			// However, these iterations are unnecessary if all elements are unique.
			while (pred(array[i], pivot) <= 0 && i < hi[level]) { ++i; }
			while (pred(array[j], pivot) >= 0 && lo[level] < j) { --j; }

			// Move the pivot element in place:
			swap(pivot, array[i]);

			// Adjust the recursion stack:
			lo[level + 1] = i;
			hi[level + 1] = hi[level];
			hi[level] = j;
			++level;
		}
		else
		{
			// Insertion-sort of the remaining elements.
			for (; i < j; --j)
			{
				quickInt m = i;
				for (quickInt k = i + 1; k <= j; ++k)
				{
					if (pred(array[k], array[m]) > 0)
					{
						m = k;
					}
				}
				swap(array[m], array[j]);
			}
			--level;
		}
	}
}

#endif // QUICK_SORT_HPP
