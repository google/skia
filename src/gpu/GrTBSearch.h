
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrTBSearch_DEFINED
#define GrTBSearch_DEFINED

template <typename ELEM, typename KEY>
int GrTBSearch(const ELEM array[], int count, KEY target) {
    GrAssert(count >= 0);
    if (0 == count) {
        // we should insert it at 0
        return ~0;
    }

    int high = count - 1;
    int low = 0;
    while (high > low) {
        int index = (low + high) >> 1;
        if (LT(array[index], target)) {
            low = index + 1;
        } else {
            high = index;
        }
    }

    // check if we found it
    if (EQ(array[high], target)) {
        return high;
    }

    // now return the ~ of where we should insert it
    if (LT(array[high], target)) {
        high += 1;
    }
    return ~high;
}

#endif
