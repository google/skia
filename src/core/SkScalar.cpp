
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkMath.h"
#include "SkScalar.h"

SkScalar SkScalarInterpFunc(SkScalar searchKey, const SkScalar keys[],
                            const SkScalar values[], int length) {
    SkASSERT(length > 0);
    SkASSERT(keys != nullptr);
    SkASSERT(values != nullptr);
#ifdef SK_DEBUG
    for (int i = 1; i < length; i++)
        SkASSERT(keys[i] >= keys[i-1]);
#endif
    int right = 0;
    while (right < length && searchKey > keys[right])
        right++;
    // Could use sentinel values to eliminate conditionals, but since the
    // tables are taken as input, a simpler format is better.
    if (length == right)
        return values[length-1];
    if (0 == right)
        return values[0];
    // Otherwise, interpolate between right - 1 and right.
    SkScalar rightKey = keys[right];
    SkScalar leftKey = keys[right-1];
    SkScalar fract = (searchKey - leftKey) / (rightKey - leftKey);
    return SkScalarInterp(values[right-1], values[right], fract);
}
