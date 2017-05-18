/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathOpsQuad_DEFINED
#define SkPathOpsQuad_DEFINED

#include "SkPathOpsPoint.h"

struct SkOpCurve;

struct SkDQuadPair {
    const SkDQuad& first() const { return (const SkDQuad&) pts[0]; }
    const SkDQuad& second() const { return (const SkDQuad&) pts[2]; }
    SkDPoint pts[5];
};

struct SkDQuad {
    static const int kPointCount = 3;
    static const int kPointLast = kPointCount - 1;
    static const int kMaxIntersections = 4;

    SkDPoint fPts[kPointCount];

    bool collapsed() const {
        return fPts[0].approximatelyEqual(fPts[1]) && fPts[0].approximatelyEqual(fPts[2]);
    }

    bool controlsInside() const {
        SkDVector v01 = fPts[0] - fPts[1];
        SkDVector v02 = fPts[0] - fPts[2];
        SkDVector v12 = fPts[1] - fPts[2];
        return v02.dot(v01) > 0 && v02.dot(v12) > 0;
    }

    void debugInit() {
        sk_bzero(fPts, sizeof(fPts));
    }

    void debugSet(const SkDPoint* pts);

    SkDQuad flip() const {
        SkDQuad result = {{fPts[2], fPts[1], fPts[0]}  SkDEBUGPARAMS(fDebugGlobalState) };
        return result;
    }

    static bool IsConic() { return false; }

    const SkDQuad& set(const SkPoint pts[kPointCount]
            SkDEBUGPARAMS(SkOpGlobalState* state = nullptr)) {
        fPts[0] = pts[0];
        fPts[1] = pts[1];
        fPts[2] = pts[2];
        SkDEBUGCODE(fDebugGlobalState = state);
        return *this;
    }

    const SkDPoint& operator[](int n) const { SkASSERT(n >= 0 && n < kPointCount); return fPts[n]; }
    SkDPoint& operator[](int n) { SkASSERT(n >= 0 && n < kPointCount); return fPts[n]; }

    static int AddValidTs(double s[], int realRoots, double* t);
    void align(int endIndex, SkDPoint* dstPt) const;
    SkDQuadPair chopAt(double t) const;
    SkDVector dxdyAtT(double t) const;
    static int FindExtrema(const double src[], double tValue[1]);

#ifdef SK_DEBUG
    SkOpGlobalState* globalState() const { return fDebugGlobalState; }
#endif

    /**
     *  Return the number of valid roots (0 < root < 1) for this cubic intersecting the
     *  specified horizontal line.
     */
    int horizontalIntersect(double yIntercept, double roots[2]) const;

    bool hullIntersects(const SkDQuad& , bool* isLinear) const;
    bool hullIntersects(const SkDConic& , bool* isLinear) const;
    bool hullIntersects(const SkDCubic& , bool* isLinear) const;
    bool isLinear(int startIndex, int endIndex) const;
    bool monotonicInX() const;
    bool monotonicInY() const;
    void otherPts(int oddMan, const SkDPoint* endPt[2]) const;
    SkDPoint ptAtT(double t) const;
    static int RootsReal(double A, double B, double C, double t[2]);
    static int RootsValidT(const double A, const double B, const double C, double s[2]);
    static void SetABC(const double* quad, double* a, double* b, double* c);
    SkDQuad subDivide(double t1, double t2) const;
    static SkDQuad SubDivide(const SkPoint a[kPointCount], double t1, double t2) {
        SkDQuad quad;
        quad.set(a);
        return quad.subDivide(t1, t2);
    }
    SkDPoint subDivide(const SkDPoint& a, const SkDPoint& c, double t1, double t2) const;
    static SkDPoint SubDivide(const SkPoint pts[kPointCount], const SkDPoint& a, const SkDPoint& c,
                              double t1, double t2) {
        SkDQuad quad;
        quad.set(pts);
        return quad.subDivide(a, c, t1, t2);
    }

    /**
     *  Return the number of valid roots (0 < root < 1) for this cubic intersecting the
     *  specified vertical line.
     */
    int verticalIntersect(double xIntercept, double roots[2]) const;

    SkDCubic debugToCubic() const;
    // utilities callable by the user from the debugger when the implementation code is linked in
    void dump() const;
    void dumpID(int id) const;
    void dumpInner() const;

    SkDEBUGCODE(SkOpGlobalState* fDebugGlobalState);
};

#endif
