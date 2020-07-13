/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTSort_DEFINED
#define SkTSort_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"
#include "src/core/SkMathPriv.h"

#include <utility>

/* A comparison functor which performs the comparison 'a < b'. */
template <typename T> struct SkTCompareLT {
    bool operator()(const T a, const T b) const { return a < b; }
};

///////////////////////////////////////////////////////////////////////////////

/*  Sifts a broken heap. The input array is a heap from root to bottom
 *  except that the root entry may be out of place.
 *
 *  Sinks a hole from array[root] to leaf and then sifts the original array[root] element
 *  from the leaf level up.
 *
 *  This version does extra work, in that it copies child to parent on the way down,
 *  then copies parent to child on the way back up. When copies are inexpensive,
 *  this is an optimization as this sift variant should only be used when
 *  the potentially out of place root entry value is expected to be small.
 *
 *  @param root the one based index into array of the out-of-place root of the heap.
 *  @param bottom the one based index in the array of the last entry in the heap.
 */
template <typename T, typename C>
void SkTHeapSort_SiftUp(T array[], size_t root, size_t bottom, C lessThan) {
    T x = array[root-1];
    size_t start = root;
    size_t j = root << 1;
    while (j <= bottom) {
        if (j < bottom && lessThan(array[j-1], array[j])) {
            ++j;
        }
        array[root-1] = array[j-1];
        root = j;
        j = root << 1;
    }
    j = root >> 1;
    while (j >= start) {
        if (lessThan(array[j-1], x)) {
            array[root-1] = array[j-1];
            root = j;
            j = root >> 1;
        } else {
            break;
        }
    }
    array[root-1] = x;
}

/*  Sifts a broken heap. The input array is a heap from root to bottom
 *  except that the root entry may be out of place.
 *
 *  Sifts the array[root] element from the root down.
 *
 *  @param root the one based index into array of the out-of-place root of the heap.
 *  @param bottom the one based index in the array of the last entry in the heap.
 */
template <typename T, typename C>
void SkTHeapSort_SiftDown(T array[], size_t root, size_t bottom, C lessThan) {
    T x = array[root-1];
    size_t child = root << 1;
    while (child <= bottom) {
        if (child < bottom && lessThan(array[child-1], array[child])) {
            ++child;
        }
        if (lessThan(x, array[child-1])) {
            array[root-1] = array[child-1];
            root = child;
            child = root << 1;
        } else {
            break;
        }
    }
    array[root-1] = x;
}

/** Sorts the array of size count using comparator lessThan using a Heap Sort algorithm. Be sure to
 *  specialize swap if T has an efficient swap operation.
 *
 *  @param array the array to be sorted.
 *  @param count the number of elements in the array.
 *  @param lessThan a functor with bool operator()(T a, T b) which returns true if a comes before b.
 */
template <typename T, typename C> void SkTHeapSort(T array[], size_t count, C lessThan) {
    for (size_t i = count >> 1; i > 0; --i) {
        SkTHeapSort_SiftDown(array, i, count, lessThan);
    }

    for (size_t i = count - 1; i > 0; --i) {
        using std::swap;
        swap(array[0], array[i]);
        SkTHeapSort_SiftUp(array, 1, i, lessThan);
    }
}

/** Sorts the array of size count using comparator '<' using a Heap Sort algorithm. */
template <typename T> void SkTHeapSort(T array[], size_t count) {
    SkTHeapSort(array, count, SkTCompareLT<T>());
}

///////////////////////////////////////////////////////////////////////////////

/** Sorts the array of size count using comparator lessThan using an Insertion Sort algorithm. */
template <typename T, typename C> static void SkTInsertionSort(T* left, T* right, C lessThan) {
    for (T* next = left + 1; next <= right; ++next) {
        if (!lessThan(*next, *(next - 1))) {
            continue;
        }
        T insert = std::move(*next);
        T* hole = next;
        do {
            *hole = std::move(*(hole - 1));
            --hole;
        } while (left < hole && lessThan(insert, *(hole - 1)));
        *hole = std::move(insert);
    }
}


#endif
