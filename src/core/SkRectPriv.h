/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRectPriv_DEFINED
#define SkRectPriv_DEFINED

#include "include/core/SkRect.h"
#include "src/core/SkMathPriv.h"

class SkRectPriv {
public:
    // Returns an irect that is very large, and can be safely round-trip with SkRect and still
    // be considered non-empty (i.e. width/height > 0) even if we round-out the SkRect.
    static SkIRect MakeILarge() {
        // SK_MaxS32 >> 1 seemed better, but it did not survive round-trip with SkRect and rounding.
        // Also, 1 << 29 can be perfectly represented in float, while SK_MaxS32 >> 1 cannot.
        const int32_t large = 1 << 29;
        return { -large, -large, large, large };
    }

    static SkIRect MakeILargestInverted() {
        return { SK_MaxS32, SK_MaxS32, SK_MinS32, SK_MinS32 };
    }

    static SkRect MakeLargeS32() {
        SkRect r;
        r.set(MakeILarge());
        return r;
    }

    static SkRect MakeLargest() {
        return { SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax };
    }

    static constexpr SkRect MakeLargestInverted() {
        return { SK_ScalarMax, SK_ScalarMax, SK_ScalarMin, SK_ScalarMin };
    }

    static void GrowToInclude(SkRect* r, const SkPoint& pt) {
        r->fLeft  =  SkMinScalar(pt.fX, r->fLeft);
        r->fRight =  SkMaxScalar(pt.fX, r->fRight);
        r->fTop    = SkMinScalar(pt.fY, r->fTop);
        r->fBottom = SkMaxScalar(pt.fY, r->fBottom);
    }

    // Conservative check if r can be expressed in fixed-point.
    // Will return false for very large values that might have fit
    static bool FitsInFixed(const SkRect& r) {
        return SkFitsInFixed(r.fLeft) && SkFitsInFixed(r.fTop) &&
               SkFitsInFixed(r.fRight) && SkFitsInFixed(r.fBottom);
    }

    static bool Is16Bit(const SkIRect& r) {
        return  SkTFitsIn<int16_t>(r.fLeft)  && SkTFitsIn<int16_t>(r.fTop) &&
                SkTFitsIn<int16_t>(r.fRight) && SkTFitsIn<int16_t>(r.fBottom);
    }
};


#endif
