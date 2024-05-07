/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkScalar.h"
#include "include/private/base/SkDebug.h"

SkScalar SkScalarInterpFunc(SkScalar searchKey, const SkScalar keys[],
                            const SkScalar values[], int length) {
    SkASSERT(length > 0);
    SkASSERT(keys != nullptr);
    SkASSERT(values != nullptr);
#ifdef SK_DEBUG
    for (int i = 1; i < length; i++) {
        SkASSERT(keys[i-1] <= keys[i]);
    }
#endif
    int right = 0;
    while (right < length && keys[right] < searchKey) {
        ++right;
    }
    // Could use sentinel values to eliminate conditionals, but since the
    // tables are taken as input, a simpler format is better.
    if (right == length) {
        return values[length-1];
    }
    if (right == 0) {
        return values[0];
    }
    // Otherwise, interpolate between right - 1 and right.
    SkScalar leftKey = keys[right-1];
    SkScalar rightKey = keys[right];
    SkScalar fract = (searchKey - leftKey) / (rightKey - leftKey);
    return SkScalarInterp(values[right-1], values[right], fract);
}
