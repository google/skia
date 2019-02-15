/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkCurve_DEFINED
#define SkCurve_DEFINED

#include "SkColor.h"
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

struct SkColorCurveSegment {
    SkColor4f eval(SkScalar x, SkRandom& random) const;
    void visitFields(SkFieldVisitor* v);

    void setConstant(SkColor4f c) {
        fConstant = true;
        fRanged = false;
        fMin[0] = c;
    }

    SkColor4f fMin[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    SkColor4f fMax[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    bool fConstant = true;
    bool fRanged = false;
};

struct SkColorCurve {
    SkColorCurve(SkColor4f c = { 1.0f, 1.0f, 1.0f, 1.0f }) {
        fSegments.push_back().setConstant(c);
    }

    SkColor4f eval(SkScalar x, SkRandom& random) const;
    void visitFields(SkFieldVisitor* v);

    SkTArray<SkScalar, true>            fXValues;
    SkTArray<SkColorCurveSegment, true> fSegments;
};

#endif // SkCurve_DEFINED
