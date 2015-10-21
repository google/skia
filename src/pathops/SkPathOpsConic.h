/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathOpsConic_DEFINED
#define SkPathOpsConic_DEFINED

#include "SkPathOpsPoint.h"
#include "SkPathOpsQuad.h"

struct SkDConic {
    static const int kPointCount = 3;
    static const int kPointLast = kPointCount - 1;
    static const int kMaxIntersections = 4;

    SkDQuad fPts;
    SkScalar fWeight;

    bool collapsed() const {
        return fPts.collapsed();
    }

    bool controlsInside() const {
        return fPts.controlsInside();
    }

    void debugInit() {
        fPts.debugInit();
    }

    SkDConic flip() const {
        SkDConic result = {{{fPts[2], fPts[1], fPts[0]}}, fWeight};
        return result;
    }

    static bool IsCubic() { return false; }

    const SkDConic& set(const SkPoint pts[kPointCount], SkScalar weight) {
        fPts.set(pts);
        fWeight = weight;
        return *this;
    }

    const SkDPoint& operator[](int n) const { return fPts[n]; }
    SkDPoint& operator[](int n) { return fPts[n]; }

    static int AddValidTs(double s[], int realRoots, double* t) {
        return SkDQuad::AddValidTs(s, realRoots, t);
    }

    void align(int endIndex, SkDPoint* dstPt) const {
        fPts.align(endIndex, dstPt);
    }

    SkDVector dxdyAtT(double t) const;
    static int FindExtrema(const double src[], SkScalar weight, double tValue[1]);

    bool hullIntersects(const SkDQuad& quad, bool* isLinear) const {
        return fPts.hullIntersects(quad, isLinear);
    }

    bool hullIntersects(const SkDConic& conic, bool* isLinear) const {
        return fPts.hullIntersects(conic.fPts, isLinear);
    }

    bool hullIntersects(const SkDCubic& cubic, bool* isLinear) const;

    bool isLinear(int startIndex, int endIndex) const {
        return fPts.isLinear(startIndex, endIndex);
    }

    bool monotonicInX() const {
        return fPts.monotonicInX();
    }

    bool monotonicInY() const {
        return fPts.monotonicInY();
    }

    void otherPts(int oddMan, const SkDPoint* endPt[2]) const {
        fPts.otherPts(oddMan, endPt);
    }

    SkDPoint ptAtT(double t) const;

    static int RootsReal(double A, double B, double C, double t[2]) {
        return SkDQuad::RootsReal(A, B, C, t);
    }

    static int RootsValidT(const double A, const double B, const double C, double s[2]) {
        return SkDQuad::RootsValidT(A, B, C, s);
    }

    SkDConic subDivide(double t1, double t2) const;

    static SkDConic SubDivide(const SkPoint a[kPointCount], SkScalar weight, double t1, double t2) {
        SkDConic conic;
        conic.set(a, weight);
        return conic.subDivide(t1, t2);
    }

    SkDPoint subDivide(const SkDPoint& a, const SkDPoint& c, double t1, double t2,
            SkScalar* weight) const;

    static SkDPoint SubDivide(const SkPoint pts[kPointCount], SkScalar weight,
                              const SkDPoint& a, const SkDPoint& c,
                              double t1, double t2, SkScalar* newWeight) {
        SkDConic conic;
        conic.set(pts, weight);
        return conic.subDivide(a, c, t1, t2, newWeight);
    }

    // utilities callable by the user from the debugger when the implementation code is linked in
    void dump() const;
    void dumpID(int id) const;
    void dumpInner() const;
};


#endif
