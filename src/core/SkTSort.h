
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

#endif
