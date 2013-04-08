/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsLine_DEFINED
#define SkPathOpsLine_DEFINED

#include "SkPathOpsPoint.h"

struct SkDLine {
    SkDPoint fPts[2];

    void set(const SkPoint pts[2]) {
        fPts[0] = pts[0];
        fPts[1] = pts[1];
    }

    const SkDPoint& operator[](int n) const { SkASSERT(n >= 0 && n < 2); return fPts[n]; }
    SkDPoint& operator[](int n) { SkASSERT(n >= 0 && n < 2); return fPts[n]; }

    double isLeft(const SkDPoint& pt) const;
    SkDLine subDivide(double t1, double t2) const;
    static SkDLine SubDivide(const SkPoint a[2], double t1, double t2) {
        SkDLine line;
        line.set(a);
        return line.subDivide(t1, t2);
    }
    SkDPoint xyAtT(double t) const;
private:
    SkDVector tangent() const { return fPts[0] - fPts[1]; }
};

#endif
