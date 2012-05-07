
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTSort_DEFINED
#define SkTSort_DEFINED

#include "SkTypes.h"

template <typename T>
void SkTHeapSort_SiftDown(T array[], int root, int bottom) {
    while (root*2 + 1 <= bottom) {
        int child = root * 2 + 1;
        if (child+1 <= bottom && array[child] < array[child+1]) {
            child += 1;
        }
        if (array[root] < array[child]) {
            SkTSwap<T>(array[root], array[child]);
            root = child;
        } else {
            break;
        }
    }
}

template <typename T> void SkTHeapSort(T array[], int count) {
    int i;
    for (i = count/2 - 1; i >= 0; --i) {
        SkTHeapSort_SiftDown<T>(array, i, count-1);
    }
    for (i = count - 1; i > 0; --i) {
        SkTSwap<T>(array[0], array[i]);
        SkTHeapSort_SiftDown<T>(array, 0, i-1);
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
    if (left >= right) {
        return;
    }
    T** pivot = left + (right - left >> 1);
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
    T* pivot = left + (right - left >> 1);
    pivot = SkTQSort_Partition(context, left, right, pivot, lessThan);
    SkQSort(context, left, pivot - 1, lessThan);
    SkQSort(context, pivot + 1, right, lessThan);
}

#endif
