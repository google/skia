/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathOpsCubic_DEFINED
#define SkPathOpsCubic_DEFINED

#include "SkPath.h"
#include "SkPathOpsPoint.h"
#include "SkTArray.h"

struct SkDCubicPair {
    const SkDCubic& first() const { return (const SkDCubic&) pts[0]; }
    const SkDCubic& second() const { return (const SkDCubic&) pts[3]; }
    SkDPoint pts[7];
};

struct SkDCubic {
    SkDPoint fPts[4];

    void set(const SkPoint pts[4]) {
        fPts[0] = pts[0];
        fPts[1] = pts[1];
        fPts[2] = pts[2];
        fPts[3] = pts[3];
    }

    static const int gPrecisionUnit;

    const SkDPoint& operator[](int n) const { SkASSERT(n >= 0 && n < 4); return fPts[n]; }
    SkDPoint& operator[](int n) { SkASSERT(n >= 0 && n < 4); return fPts[n]; }

    void align(int endIndex, int ctrlIndex, SkDPoint* dstPt) const;
    double calcPrecision() const;
    SkDCubicPair chopAt(double t) const;
    bool clockwise() const;
    static void Coefficients(const double* cubic, double* A, double* B, double* C, double* D);
    bool controlsContainedByEnds() const;
    SkDVector dxdyAtT(double t) const;
    bool endsAreExtremaInXOrY() const;
    static int FindExtrema(double a, double b, double c, double d, double tValue[2]);
    int findInflections(double tValues[]) const;

    static int FindInflections(const SkPoint a[4], double tValues[]) {
        SkDCubic cubic;
        cubic.set(a);
        return cubic.findInflections(tValues);
    }

    int findMaxCurvature(double tValues[]) const;
    bool isLinear(int startIndex, int endIndex) const;
    bool monotonicInY() const;
    SkDPoint ptAtT(double t) const;
    static int RootsReal(double A, double B, double C, double D, double t[3]);
    static int RootsValidT(const double A, const double B, const double C, double D, double s[3]);
    bool serpentine() const;
    SkDCubic subDivide(double t1, double t2) const;

    static SkDCubic SubDivide(const SkPoint a[4], double t1, double t2) {
        SkDCubic cubic;
        cubic.set(a);
        return cubic.subDivide(t1, t2);
    }

    void subDivide(const SkDPoint& a, const SkDPoint& d, double t1, double t2, SkDPoint p[2]) const;

    static void SubDivide(const SkPoint pts[4], const SkDPoint& a, const SkDPoint& d, double t1,
                          double t2, SkDPoint p[2]) {
        SkDCubic cubic;
        cubic.set(pts);
        cubic.subDivide(a, d, t1, t2, p);
    }

    SkDPoint top(double startT, double endT) const;
    void toQuadraticTs(double precision, SkTArray<double, true>* ts) const;
    SkDQuad toQuad() const;

#ifdef SK_DEBUG
    void dump();
#endif
};

#endif
