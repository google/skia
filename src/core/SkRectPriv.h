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
    // Returns an irect that is very large, and can be safely round-trip with SkRect and still
    // be considered non-empty (i.e. width/height > 0) even if we round-out the SkRect.
    // SK_MaxS32 >> 1 seems better, but it does not survive round-trip with SkRect and rounding.
    static SkIRect MakeILarge() {
        const int32_t ihalf = 1 << 29;
        return { -ihalf, -ihalf, ihalf, ihalf };
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

    static SkRect MakeLargestInverted() {
        return { SK_ScalarMax, SK_ScalarMax, SK_ScalarMin, SK_ScalarMin };
    }

    static void GrowToInclude(SkRect* r, const SkPoint& pt) {
        r->fLeft  =  SkMinScalar(pt.fX, r->fLeft);
        r->fRight =  SkMaxScalar(pt.fX, r->fRight);
        r->fTop    = SkMinScalar(pt.fY, r->fTop);
        r->fBottom = SkMaxScalar(pt.fY, r->fBottom);
    }
};


#endif
