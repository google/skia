/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsRect_DEFINED
#define SkPathOpsRect_DEFINED

#include "SkPathOpsPoint.h"

struct SkDRect {
    double fLeft, fTop, fRight, fBottom;

    void add(const SkDPoint& pt) {
        fLeft = SkTMin(fLeft, pt.fX);
        fTop = SkTMin(fTop, pt.fY);
        fRight = SkTMax(fRight, pt.fX);
        fBottom = SkTMax(fBottom, pt.fY);
    }

    bool contains(const SkDPoint& pt) const {
        return approximately_between(fLeft, pt.fX, fRight)
                && approximately_between(fTop, pt.fY, fBottom);
    }

    void debugInit();

    bool intersects(const SkDRect& r) const {
        SkASSERT(fLeft <= fRight);
        SkASSERT(fTop <= fBottom);
        SkASSERT(r.fLeft <= r.fRight);
        SkASSERT(r.fTop <= r.fBottom);
        return r.fLeft <= fRight && fLeft <= r.fRight && r.fTop <= fBottom && fTop <= r.fBottom;
    }

    void set(const SkDPoint& pt) {
        fLeft = fRight = pt.fX;
        fTop = fBottom = pt.fY;
    }

    double width() const {
        return fRight - fLeft;
    }

    double height() const {
        return fBottom - fTop;
    }

    void setBounds(const SkDConic& curve) {
        setBounds(curve, curve, 0, 1);
    }

    void setBounds(const SkDConic& curve, const SkDConic& sub, double tStart, double tEnd);

    void setBounds(const SkDCubic& curve) {
        setBounds(curve, curve, 0, 1);
    }

    void setBounds(const SkDCubic& curve, const SkDCubic& sub, double tStart, double tEnd);

    void setBounds(const SkDQuad& curve) {
        setBounds(curve, curve, 0, 1);
    }

    void setBounds(const SkDQuad& curve, const SkDQuad& sub, double tStart, double tEnd);

    bool valid() const {
        return fLeft <= fRight && fTop <= fBottom;
    }
};

#endif
