
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTSort_DEFINED
#define SkTSort_DEFINED

#include "SkTypes.h"
/**
 *  Sifts a broken heap. The input array is a heap from root to bottom
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
template <typename T> void SkTHeapSort_SiftUp(T array[], size_t root, size_t bottom) {
    T x = array[root-1];
    size_t start = root;
    size_t j = root << 1;
    while (j <= bottom) {
        if (j < bottom && array[j-1] < array[j]) {
            ++j;
        }
        array[root-1] = array[j-1];
        root = j;
        j = root << 1;
    }
    j = root >> 1;
    while (j >= start) {
        if (array[j-1] < x) {
            array[root-1] = array[j-1];
            root = j;
            j = root >> 1;
        } else {
            break;
        }
    }
    array[root-1] = x;
}

/**
 *  Sifts a broken heap. The input array is a heap from root to bottom
 *  except that the root entry may be out of place.
 *
 *  Sifts the array[root] element from the root down.
 *
 *  @param root the one based index into array of the out-of-place root of the heap.
 *  @param bottom the one based index in the array of the last entry in the heap.
 */
template <typename T> void SkTHeapSort_SiftDown(T array[], size_t root, size_t bottom) {
    T x = array[root-1];
    size_t child = root << 1;
    while (child <= bottom) {
        if (child < bottom && array[child-1] < array[child]) {
            ++child;
        }
        if (x < array[child-1]) {
            array[root-1] = array[child-1];
            root = child;
            child = root << 1;
        } else {
            break;
        }
    }
    array[root-1] = x;
}

template <typename T> void SkTHeapSort(T array[], size_t count) {
    for (size_t i = count >> 1; i > 0; --i) {
        SkTHeapSort_SiftDown<T>(array, i, count);
    }

    for (size_t i = count - 1; i > 0; --i) {
        SkTSwap<T>(array[0], array[i]);
        SkTHeapSort_SiftUp(array, 1, i);
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
static T** SkTQSort_Partition(T** left, T** right, T** pivot) {
    T* pivotValue = *pivot;
    SkTSwap(*pivot, *right);
    T** newPivot = left;
    while (left < right) {
        if (**left < *pivotValue) {
            SkTSwap(*left, *newPivot);
            newPivot += 1;
        }
        left += 1;
    }
    SkTSwap(*newPivot, *right);
    return newPivot;
}

template <typename T> void SkTQSort(T** left, T** right) {
    while (left < right) {
        T** pivot = left + ((right - left) >> 1);
        pivot = SkTQSort_Partition(left, right, pivot);

        if (right - pivot > pivot - left) {
            SkTQSort(left, pivot - 1);
            left = pivot + 1;
        } else {
            SkTQSort(pivot + 1, right);
            right = pivot - 1;
        }
    }
}

template <typename T>
static T* SkTQSort_Partition(T* left, T* right, T* pivot) {
    T pivotValue = *pivot;
    SkTSwap(*pivot, *right);
    T* newPivot = left;
    while (left < right) {
        if (*left < pivotValue) {
            SkTSwap(*left, *newPivot);
            newPivot += 1;
        }
        left += 1;
    }
    SkTSwap(*newPivot, *right);
    return newPivot;
}

template <typename T> void SkTQSort(T* left, T* right) {
    if (left >= right) {
        return;
    }
    T* pivot = left + ((right - left) >> 1);
    pivot = SkTQSort_Partition(left, right, pivot);
    SkTQSort(left, pivot - 1);
    SkTQSort(pivot + 1, right);
}

template <typename S, typename T>
static T* SkTQSort_Partition(S& context, T* left, T* right, T* pivot,
                             bool (*lessThan)(S&, const T, const T)) {
    T pivotValue = *pivot;
    SkTSwap(*pivot, *right);
    T* newPivot = left;
    while (left < right) {
        if (lessThan(context, *left, pivotValue)) {
            SkTSwap(*left, *newPivot);
            newPivot += 1;
        }
        left += 1;
    }
    SkTSwap(*newPivot, *right);
    return newPivot;
}

template <typename S, typename T>
void SkQSort(S& context, T* left, T* right,
             bool (*lessThan)(S& , const T, const T)) {
    if (left >= right) {
        return;
    }
    T* pivot = left + ((right - left) >> 1);
    pivot = SkTQSort_Partition(context, left, right, pivot, lessThan);
    SkQSort(context, left, pivot - 1, lessThan);
    SkQSort(context, pivot + 1, right, lessThan);
}

#endif
