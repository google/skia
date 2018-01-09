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

    static void GrowToInclude(SkRect* r, const SkPoint& pt) {
        r->fLeft  =  SkMinScalar(pt.fX, r->fLeft);
        r->fRight =  SkMaxScalar(pt.fX, r->fRight);
        r->fTop    = SkMinScalar(pt.fY, r->fTop);
        r->fBottom = SkMaxScalar(pt.fY, r->fBottom);
    }
};


#endif
