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

class SK_API SkPathBuilder {
public:
    SkPathBuilder();
    ~SkPathBuilder();

    SkPath snapshot();  // the builder is unchanged after returning this path
    SkPath detach();    // the builder is reset to empty after returning this path

    void setFillType(SkPathFillType ft) { fFillType = ft; }
    void setIsVolatile(bool isVolatile) { fIsVolatile = isVolatile; }

    SkPathBuilder& reset();

    SkPathBuilder& moveTo(SkPoint pt);
    SkPathBuilder& moveTo(SkScalar x, SkScalar y) { return this->moveTo(SkPoint::Make(x, y)); }

    SkPathBuilder& lineTo(SkPoint pt);
    SkPathBuilder& lineTo(SkScalar x, SkScalar y) { return this->lineTo(SkPoint::Make(x, y)); }

    SkPathBuilder& quadTo(SkPoint pt1, SkPoint pt2);
    SkPathBuilder& quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
        return this->quadTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2));
    }
    SkPathBuilder& quadTo(const SkPoint pts[2]) { return this->quadTo(pts[0], pts[1]); }

    SkPathBuilder& conicTo(SkPoint pt1, SkPoint pt2, SkScalar w);
    SkPathBuilder& conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar w) {
        return this->conicTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2), w);
    }
    SkPathBuilder& conicTo(const SkPoint pts[2], SkScalar w) {
        return this->conicTo(pts[0], pts[1], w);
    }

    SkPathBuilder& cubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3);
    SkPathBuilder& cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3) {
        return this->cubicTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2), SkPoint::Make(x3, y3));
    }
    SkPathBuilder& cubicTo(const SkPoint pts[3]) {
        return this->cubicTo(pts[0], pts[1], pts[2]);
    }

    SkPathBuilder& close();

    // Relative versions of segments, relative to the previous position.

    SkPathBuilder& rLineTo(SkPoint pt);
    SkPathBuilder& rLineTo(SkScalar x, SkScalar y) { return this->rLineTo({x, y}); }
    SkPathBuilder& rQuadTo(SkPoint pt1, SkPoint pt2);
    SkPathBuilder& rQuadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
        return this->rQuadTo({x1, y1}, {x2, y2});
    }
    SkPathBuilder& rConicTo(SkPoint p1, SkPoint p2, SkScalar w);
    SkPathBuilder& rConicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar w) {
        return this->rConicTo({x1, y1}, {x2, y2}, w);
    }
    SkPathBuilder& rCubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3);
    SkPathBuilder& rCubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3) {
        return this->rCubicTo({x1, y1}, {x2, y2}, {x3, y3});
    }

    // Add a closed contour

    SkPathBuilder& addRect(const SkRect&, SkPathDirection, unsigned startIndex);
    SkPathBuilder& addOval(const SkRect&, SkPathDirection, unsigned startIndex);
    SkPathBuilder& addRRect(const SkRRect&, SkPathDirection, unsigned startIndex);

    SkPathBuilder& addRect(const SkRect& rect, SkPathDirection dir = SkPathDirection::kCW) {
        return this->addRect(rect, dir, 0);
    }
    SkPathBuilder& addOval(const SkRect& rect, SkPathDirection dir = SkPathDirection::kCW) {
        // legacy start index: 1
        return this->addOval(rect, dir, 1);
    }
    SkPathBuilder& addRRect(const SkRRect& rrect, SkPathDirection dir = SkPathDirection::kCW) {
        // legacy start indices: 6 (CW) and 7 (CCW)
        return this->addRRect(rrect, dir, dir == SkPathDirection::kCW ? 6 : 7);
    }

    SkPathBuilder& addCircle(SkScalar center_x, SkScalar center_y, SkScalar radius,
                             SkPathDirection dir = SkPathDirection::kCW);
    SkPathBuilder& addPolygon(const SkPoint pts[], int count, bool isClosed);

    // Performance hint, to reserve extra storage for subsequent calls to lineTo, quadTo, etc.

    void incReserve(int extraPtCount, int extraVerbCount);
    void incReserve(int extraPtCount) {
        this->incReserve(extraPtCount, extraPtCount);
    }

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

