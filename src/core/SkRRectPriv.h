/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRRectPriv_DEFINED
#define SkRRectPriv_DEFINED

#include "SkRRect.h"

static bool nearly_square(const SkRect& r) {
    return SkScalarNearlyEqual(r.width(), r.height());
}

class SkRRectPriv {
public:
    static SkVector GetSimpleRadii(const SkRRect& rr) { return rr.getSimpleRadii(); }

    static bool IsCircle(const SkRRect& rr) {
        return rr.isOval() && nearly_square(rr.fRect);
    }

    static bool IsSimpleCircular(const SkRRect& rr) {
        return rr.isSimple() && nearly_square(rr.fRect);
    }

    static bool EqualRadii(const SkRRect& rr) {
        return rr.isRect() || SkRRectPriv::IsCircle(rr)  || SkRRectPriv::IsSimpleCircular(rr);
    }

    static bool AllCornersCircular(const SkRRect& rr, SkScalar tolerance = SK_ScalarNearlyZero);
};

#endif

