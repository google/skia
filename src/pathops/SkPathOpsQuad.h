/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathOpsQuad_DEFINED
#define SkPathOpsQuad_DEFINED

#include "SkPathOpsPoint.h"

struct SkDQuadPair {
    const SkDQuad& first() const { return (const SkDQuad&) pts[0]; }
    const SkDQuad& second() const { return (const SkDQuad&) pts[2]; }
    SkDPoint pts[5];
};

struct SkDQuad {
    SkDPoint fPts[3];

    SkDQuad flip() const {
        SkDQuad result = {{fPts[2], fPts[1], fPts[0]}};
        return result;
    }

    void set(const SkPoint pts[3]) {
        fPts[0] = pts[0];
        fPts[1] = pts[1];
        fPts[2] = pts[2];
    }

    const SkDPoint& operator[](int n) const { SkASSERT(n >= 0 && n < 3); return fPts[n]; }
    SkDPoint& operator[](int n) { SkASSERT(n >= 0 && n < 3); return fPts[n]; }

    static int AddValidTs(double s[], int realRoots, double* t);
    void align(int endIndex, SkDPoint* dstPt) const;
    SkDQuadPair chopAt(double t) const;
    SkDVector dxdyAtT(double t) const;
    static int FindExtrema(double a, double b, double c, double tValue[1]);
    bool isLinear(int startIndex, int endIndex) const;
    bool monotonicInY() const;
    double nearestT(const SkDPoint&) const;
    bool pointInHull(const SkDPoint&) const;
    SkDPoint ptAtT(double t) const;
    static int RootsReal(double A, double B, double C, double t[2]);
    static int RootsValidT(const double A, const double B, const double C, double s[2]);
    static void SetABC(const double* quad, double* a, double* b, double* c);
    SkDQuad subDivide(double t1, double t2) const;
    static SkDQuad SubDivide(const SkPoint a[3], double t1, double t2) {
        SkDQuad quad;
        quad.set(a);
        return quad.subDivide(t1, t2);
    }
    SkDPoint subDivide(const SkDPoint& a, const SkDPoint& c, double t1, double t2) const;
    static SkDPoint SubDivide(const SkPoint pts[3], const SkDPoint& a, const SkDPoint& c,
                              double t1, double t2) {
        SkDQuad quad;
        quad.set(pts);
        return quad.subDivide(a, c, t1, t2);
    }
    SkDCubic toCubic() const;
    SkDPoint top(double startT, double endT) const;

#ifdef SK_DEBUG
    void dump();
#endif
private:
//  static double Tangent(const double* quadratic, double t);  // uncalled
};

#endif
