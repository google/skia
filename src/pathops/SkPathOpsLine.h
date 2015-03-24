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

    const SkDPoint& operator[](int n) const { SkASSERT(n >= 0 && n < 2); return fPts[n]; }
    SkDPoint& operator[](int n) { SkASSERT(n >= 0 && n < 2); return fPts[n]; }

    void set(const SkPoint pts[2]) {
        fPts[0] = pts[0];
        fPts[1] = pts[1];
    }

    static SkDLine SubDivide(const SkPoint a[2], double t1, double t2) {
        SkDLine line;
        line.set(a);
        return line.subDivide(t1, t2);
    }

    double exactPoint(const SkDPoint& xy) const;
    static double ExactPointH(const SkDPoint& xy, double left, double right, double y);
    static double ExactPointV(const SkDPoint& xy, double top, double bottom, double x);
    double isLeft(const SkDPoint& pt) const;
    double nearPoint(const SkDPoint& xy, bool* unequal) const;
    bool nearRay(const SkDPoint& xy) const;
    static double NearPointH(const SkDPoint& xy, double left, double right, double y);
    static double NearPointV(const SkDPoint& xy, double top, double bottom, double x);
    static bool NearRay(double dx1, double dy1, double dx2, double dy2);
    SkDPoint ptAtT(double t) const;
    SkDLine subDivide(double t1, double t2) const;

    void dump() const;
private:
    SkDVector tangent() const { return fPts[0] - fPts[1]; }
};

#endif
