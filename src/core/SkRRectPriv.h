/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRRectPriv_DEFINED
#define SkRRectPriv_DEFINED

#include "SkRRect.h"

class SkRBuffer;
class SkWBuffer;

class SkRRectPriv {
public:
    static bool IsCircle(const SkRRect& rr) {
        SkVector rad = rr.getSimpleRadii();
        return rr.isOval() && SkScalarNearlyEqual(rad.fX, rad.fY);
    }

    static SkVector GetSimpleRadii(const SkRRect& rr) {
        SkASSERT(!rr.isComplex());
        return rr.getSimpleRadii();
    }

    static bool IsSimpleCircular(const SkRRect& rr) {
        SkVector rad = rr.getSimpleRadii();
        return rr.isSimple() && SkScalarNearlyEqual(rad.fX, rad.fY);
    }

    static bool EqualRadii(const SkRRect& rr) {
        return rr.isRect() || SkRRectPriv::IsCircle(rr)  || SkRRectPriv::IsSimpleCircular(rr);
    }

    static SkVector* GetRadiiArray(const SkRRect& rr, SkVector radii[4]);

    static bool AllCornersCircular(const SkRRect& rr, SkScalar tolerance = SK_ScalarNearlyZero);

    static bool ReadFromBuffer(SkRBuffer* buffer, SkRRect* rr);

    static void WriteToBuffer(const SkRRect& rr, SkWBuffer* buffer);
};

#endif

