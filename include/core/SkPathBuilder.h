/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathBuilder_DEFINED
#define SkPathBuilder_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/private/SkTDArray.h"

class SkPathBuilder {
public:
    SkPathBuilder();
    ~SkPathBuilder();

    SkPath snapshot();  // the builder is unchanged after returning this path
    SkPath detach();    // the builder is reset to empty after returning this path

    void setFillType(SkPathFillType ft) { fFillType = ft; }
    void setIsVolatile(bool isVolatile) { fIsVolatile = isVolatile; }

    SkPathBuilder& reset();

    SkPathBuilder& moveTo(SkPoint pt);
    SkPathBuilder& lineTo(SkPoint pt);
    SkPathBuilder& quadTo(SkPoint pt1, SkPoint pt2);
    SkPathBuilder& conicTo(SkPoint pt1, SkPoint pt2, SkScalar w);
    SkPathBuilder& cubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3);
    SkPathBuilder& close();

    SkPathBuilder&  moveTo(SkScalar x, SkScalar y) { return this->moveTo(SkPoint::Make(x, y)); }
    SkPathBuilder&  lineTo(SkScalar x, SkScalar y) { return this->lineTo(SkPoint::Make(x, y)); }
    SkPathBuilder&  quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
        return this->quadTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2));
    }
    SkPathBuilder&  quadTo(const SkPoint pts[2]) { return this->quadTo(pts[0], pts[1]); }
    SkPathBuilder&  conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar w) {
        return this->conicTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2), w);
    }
    SkPathBuilder&  conicTo(const SkPoint pts[2], SkScalar w) {
        return this->conicTo(pts[0], pts[1], w);
    }
    SkPathBuilder&  cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3) {
        return this->cubicTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2), SkPoint::Make(x3, y3));
    }
    SkPathBuilder&  cubicTo(const SkPoint pts[3]) {
        return this->cubicTo(pts[0], pts[1], pts[2]);
    }

    SkPathBuilder& addRect(const SkRect&, SkPathDirection, unsigned startIndex);
    SkPathBuilder& addOval(const SkRect&, SkPathDirection, unsigned startIndex);
    SkPathBuilder& addRRect(const SkRRect&, SkPathDirection, unsigned startIndex);

    SkPathBuilder& addRect(const SkRect& rect, SkPathDirection dir = SkPathDirection::kCW) {
        return this->addRect(rect, dir, 0);
    }
    SkPathBuilder& addOval(const SkRect& rect, SkPathDirection dir = SkPathDirection::kCW) {
        return this->addOval(rect, dir, 0);
    }
    SkPathBuilder& addRRect(const SkRRect& rect, SkPathDirection dir = SkPathDirection::kCW) {
        return this->addRRect(rect, dir, 0);
    }

    void incReserve(int extraPtCount, int extraVerbCount);
    void incReserve(int extraPtCount) {
        this->incReserve(extraPtCount, extraPtCount);
    }

    static SkPath Make(const SkPoint[],  int pointCount,
                       const uint8_t[],  int verbCount,
                       const SkScalar[], int conicWeightCount,
                       SkPathFillType, bool isVolatile = false);

private:
    SkTDArray<SkPoint>  fPts;
    SkTDArray<uint8_t>  fVerbs;
    SkTDArray<SkScalar> fConicWeights;

    SkPathFillType      fFillType;
    bool                fIsVolatile;

    unsigned    fSegmentMask;
    SkPoint     fLastMovePoint;
    bool        fNeedsMoveVerb;

    int countVerbs() const { return fVerbs.count(); }

    void ensureMove() {
        if (fNeedsMoveVerb) {
            this->moveTo(fLastMovePoint);
        }
    }
};

#endif

