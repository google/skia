/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRectPriv_DEFINED
#define SkRectPriv_DEFINED

#include "SkRect.h"

class SkRectPriv {
public:
    // Returns true iff width and height are positive. Catches inverted, empty, and overflowing
    // (way too big) rects. This is used by clients that want a non-empty rect that they can also
    // actually use its computed width/height.
    //
    static bool PositiveDimensions(const SkIRect& r) {
        return r.width() > 0 && r.height() > 0;
    }

    static SkRect MakeLargestS32() {
        const int32_t ihalf = SK_MaxS32 >> 1;
        const SkScalar half = SkIntToScalar(ihalf);

        return { -half, -half, half, half };
    }

    static SkRect MakeLargest() {
        return { SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax };
    }

    static SkIRect MakeILargest() {
        return { SK_MinS32, SK_MinS32, SK_MaxS32, SK_MaxS32 };
    }

    static SkRect MakeLargestInverted() {
        return { SK_ScalarMax, SK_ScalarMax, SK_ScalarMin, SK_ScalarMin };
    }

    static SkIRect MakeILargestInverted() {
        return { SK_MaxS32, SK_MaxS32, SK_MinS32, SK_MinS32 };
    }
};


#endif
