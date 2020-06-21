/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathBuilder_DEFINED
#define SkPathBuilder_DEFINED

#include "SkMatrix.h"
#include "SkTDArray.h"
#include "SkPathTypes.h"
#include "SkPath.h"

class SkPathBuilder {
public:
    SkPathBuilder();
    ~SkPathBuilder();

    SkPath snapshot();  // the builder is unchanged after returning this path
    SkPath detach();    // the builder is reset to empty after returning this path

    void incReserve(int extraPtCount);

    SkPathBuilder& moveTo(SkPoint pt);
    SkPathBuilder& lineTo(SkPoint pt);
    SkPathBuilder& quadTo(SkPoint pt1, SkPoint pt2);
    SkPathBuilder& conicTo(SkPoint pt1, SkPoint pt2, SkScalar w);
    SkPathBuilder& cubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3);
    SkPathBuilder& close();

    void moveTo(SkScalar x, SkScalar y) { this->moveTo(SkPoint::Make(x, y)); }
    void lineTo(SkScalar x, SkScalar y) { this->lineTo(SkPoint::Make(x, y)); }
    void quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
        this->quadTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2));
    }
    void quadTo(const SkPoint pts[2]) { this->quadTo(pts[0], pts[1]); }
    void conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar w) {
        this->conicTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2), w);
    }
    void conicTo(const SkPoint pts[2], SkScalar w) {
        this->conicTo(pts[0], pts[1], w);
    }
    void cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3) {
        this->cubicTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2), SkPoint::Make(x3, y3));
    }
    void cubicTo(const SkPoint pts[3]) {
        this->cubicTo(pts[0], pts[1], pts[2]);
    }

    SkPathBuilder& addRect(const SkRect&, SkPathDirection, unsigned startIndex);

    SkPathBuilder& addRect(const SkRect& rect, SkPathDirection dir = kCW_SkPathDirection) {
        return this->addRect(rect, dir, 0);
    }
    SkPathBuilder& addOval(const SkRect&, SkPathDirection dir = kCW_SkPathDirection);
    SkPathBuilder& addRRect(const SkRRect&, SkPathDirection dir = kCW_SkPathDirection);

private:
    SkTDArray<SkPoint>  fPts;
    SkTDArray<char>     fVerbs;
    SkTDArray<SkScalar> fConicWeights;

    unsigned    fSegmentMask = 0;


    int countVerbs() const { return fVerbs.count(); }
};

#endif

