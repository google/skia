/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkCurve_DEFINED
#define SkCurve_DEFINED

#include "SkScalar.h"
#include "SkTArray.h"

class SkFieldVisitor;
class SkRandom;

struct SkCurveSegment {
    SkScalar eval(SkScalar x, SkRandom& random) const;
    void visitFields(SkFieldVisitor* v);

    void setConstant(SkScalar c) {
        fConstant = true;
        fRanged   = false;
        fMin[0] = c;
    }

    SkScalar fMin[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    SkScalar fMax[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    bool fConstant      = true;
    bool fRanged        = false;
    bool fBidirectional = false;
};

struct SkCurve {
    SkCurve(SkScalar c = 0.0f) {
        fSegments.push_back().setConstant(c);
    }

    SkScalar eval(SkScalar x, SkRandom& random) const;
    void visitFields(SkFieldVisitor* v);
    void getExtents(SkScalar extents[2]) const;

    SkTArray<SkScalar, true>       fXValues;
    SkTArray<SkCurveSegment, true> fSegments;
};

#endif // SkCurve_DEFINED
