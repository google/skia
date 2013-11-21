/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkIntersections_DEFINE
#define SkIntersections_DEFINE

#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsPoint.h"
#include "SkPathOpsQuad.h"

class SkIntersections {
public:
    SkIntersections()
        : fSwap(0)
#ifdef SK_DEBUG
        , fDepth(0)
#endif
    {
        sk_bzero(fPt, sizeof(fPt));
        sk_bzero(fT, sizeof(fT));
        sk_bzero(fIsCoincident, sizeof(fIsCoincident));
        reset();
        fMax = 0;  // require that the caller set the max
    }

    class TArray {
    public:
        explicit TArray(const double ts[9]) : fTArray(ts) {}
        double operator[](int n) const {
            return fTArray[n];
        }
        const double* fTArray;
    };
    TArray operator[](int n) const { return TArray(fT[n]); }

    void set(const SkIntersections& i) {
        memcpy(fPt, i.fPt, sizeof(fPt));
        memcpy(fT, i.fT, sizeof(fT));
        memcpy(fIsCoincident, i.fIsCoincident, sizeof(fIsCoincident));
        fUsed = i.fUsed;
        fMax = i.fMax;
        fSwap = i.fSwap;
        SkDEBUGCODE(fDepth = i.fDepth);
    }

    void allowNear(bool nearAllowed) {
        fAllowNear = nearAllowed;
    }

    int cubic(const SkPoint a[4]) {
        SkDCubic cubic;
        cubic.set(a);
        fMax = 1;  // self intersect
        return intersect(cubic);
    }

    int cubicCubic(const SkPoint a[4], const SkPoint b[4]) {
        SkDCubic aCubic;
        aCubic.set(a);
        SkDCubic bCubic;
        bCubic.set(b);
        fMax = 9;
        return intersect(aCubic, bCubic);
    }

    int cubicHorizontal(const SkPoint a[4], SkScalar left, SkScalar right, SkScalar y,
                        bool flipped) {
        SkDCubic cubic;
        cubic.set(a);
        fMax = 3;
        return horizontal(cubic, left, right, y, flipped);
    }

    int cubicVertical(const SkPoint a[4], SkScalar top, SkScalar bottom, SkScalar x, bool flipped) {
        SkDCubic cubic;
        cubic.set(a);
        fMax = 3;
        return vertical(cubic, top, bottom, x, flipped);
    }

    int cubicLine(const SkPoint a[4], const SkPoint b[2]) {
        SkDCubic cubic;
        cubic.set(a);
        SkDLine line;
        line.set(b);
        fMax = 3;
        return intersect(cubic, line);
    }

    int cubicQuad(const SkPoint a[4], const SkPoint b[3]) {
        SkDCubic cubic;
        cubic.set(a);
        SkDQuad quad;
        quad.set(b);
        fMax = 6;
        return intersect(cubic, quad);
    }

    bool hasT(double t) const {
        SkASSERT(t == 0 || t == 1);
        return fUsed > 0 && (t == 0 ? fT[0][0] == 0 : fT[0][fUsed - 1] == 1);
    }

    int insertSwap(double one, double two, const SkDPoint& pt) {
        if (fSwap) {
            return insert(two, one, pt);
        } else {
            return insert(one, two, pt);
        }
    }

    bool isCoincident(int index) {
        return (fIsCoincident[0] & 1 << index) != 0;
    }

    int lineHorizontal(const SkPoint a[2], SkScalar left, SkScalar right, SkScalar y,
                       bool flipped) {
        SkDLine line;
        line.set(a);
        fMax = 2;
        return horizontal(line, left, right, y, flipped);
    }

    int lineVertical(const SkPoint a[2], SkScalar top, SkScalar bottom, SkScalar x, bool flipped) {
        SkDLine line;
        line.set(a);
        fMax = 2;
        return vertical(line, top, bottom, x, flipped);
    }

    int lineLine(const SkPoint a[2], const SkPoint b[2]) {
        SkDLine aLine, bLine;
        aLine.set(a);
        bLine.set(b);
        fMax = 2;
        return intersect(aLine, bLine);
    }

    const SkDPoint& pt(int index) const {
        return fPt[index];
    }

    int quadHorizontal(const SkPoint a[3], SkScalar left, SkScalar right, SkScalar y,
                       bool flipped) {
        SkDQuad quad;
        quad.set(a);
        fMax = 2;
        return horizontal(quad, left, right, y, flipped);
    }

    int quadVertical(const SkPoint a[3], SkScalar top, SkScalar bottom, SkScalar x, bool flipped) {
        SkDQuad quad;
        quad.set(a);
        fMax = 2;
        return vertical(quad, top, bottom, x, flipped);
    }

    int quadLine(const SkPoint a[3], const SkPoint b[2]) {
        SkDQuad quad;
        quad.set(a);
        SkDLine line;
        line.set(b);
        fMax = 2;
        return intersect(quad, line);
    }

    int quadQuad(const SkPoint a[3], const SkPoint b[3]) {
        SkDQuad aQuad;
        aQuad.set(a);
        SkDQuad bQuad;
        bQuad.set(b);
        fMax = 4;
        return intersect(aQuad, bQuad);
    }

    // leaves flip, swap, max alone
    void reset() {
        fAllowNear = true;
        fUsed = 0;
    }

    void setMax(int max) {
        fMax = max;
    }

    void swap() {
        fSwap ^= true;
    }

    void swapPts();

    bool swapped() const {
        return fSwap;
    }

    int used() const {
        return fUsed;
    }

    void downDepth() {
        SkASSERT(--fDepth >= 0);
    }

    void upDepth() {
        SkASSERT(++fDepth < 16);
    }

    void append(const SkIntersections& );
    static double Axial(const SkDQuad& , const SkDPoint& , bool vertical);
    void cleanUpCoincidence();
    int coincidentUsed() const;
    int cubicRay(const SkPoint pts[4], const SkDLine& line);
    void flip();
    int horizontal(const SkDLine&, double y);
    int horizontal(const SkDLine&, double left, double right, double y, bool flipped);
    int horizontal(const SkDQuad&, double left, double right, double y, bool flipped);
    int horizontal(const SkDQuad&, double left, double right, double y, double tRange[2]);
    int horizontal(const SkDCubic&, double y, double tRange[3]);
    int horizontal(const SkDCubic&, double left, double right, double y, bool flipped);
    int horizontal(const SkDCubic&, double left, double right, double y, double tRange[3]);
    // FIXME : does not respect swap
    int insert(double one, double two, const SkDPoint& pt);
    void insertNear(double one, double two, const SkDPoint& pt);
    // start if index == 0 : end if index == 1
    void insertCoincident(double one, double two, const SkDPoint& pt);
    int intersect(const SkDLine&, const SkDLine&);
    int intersect(const SkDQuad&, const SkDLine&);
    int intersect(const SkDQuad&, const SkDQuad&);
    int intersect(const SkDCubic&);  // return true if cubic self-intersects
    int intersect(const SkDCubic&, const SkDLine&);
    int intersect(const SkDCubic&, const SkDQuad&);
    int intersect(const SkDCubic&, const SkDCubic&);
    int intersectRay(const SkDLine&, const SkDLine&);
    int intersectRay(const SkDQuad&, const SkDLine&);
    int intersectRay(const SkDCubic&, const SkDLine&);
    static SkDPoint Line(const SkDLine&, const SkDLine&);
    int lineRay(const SkPoint pts[2], const SkDLine& line);
    void offset(int base, double start, double end);
    void quickRemoveOne(int index, int replace);
    int quadRay(const SkPoint pts[3], const SkDLine& line);
    void removeOne(int index);
    static bool Test(const SkDLine& , const SkDLine&);
    int vertical(const SkDLine&, double x);
    int vertical(const SkDLine&, double top, double bottom, double x, bool flipped);
    int vertical(const SkDQuad&, double top, double bottom, double x, bool flipped);
    int vertical(const SkDCubic&, double top, double bottom, double x, bool flipped);
    int verticalCubic(const SkPoint a[4], SkScalar top, SkScalar bottom, SkScalar x, bool flipped);
    int verticalLine(const SkPoint a[2], SkScalar top, SkScalar bottom, SkScalar x, bool flipped);
    int verticalQuad(const SkPoint a[3], SkScalar top, SkScalar bottom, SkScalar x, bool flipped);

    int depth() const {
#ifdef SK_DEBUG
        return fDepth;
#else
        return 0;
#endif
    }

private:
    bool cubicCheckCoincidence(const SkDCubic& c1, const SkDCubic& c2);
    bool cubicExactEnd(const SkDCubic& cubic1, bool start, const SkDCubic& cubic2);
    void cubicNearEnd(const SkDCubic& cubic1, bool start, const SkDCubic& cubic2, const SkDRect& );
    void cleanUpParallelLines(bool parallel);
    void computePoints(const SkDLine& line, int used);
    // used by addCoincident to remove ordinary intersections in range
 //   void remove(double one, double two, const SkDPoint& startPt, const SkDPoint& endPt);

    SkDPoint fPt[9];  // FIXME: since scans store points as SkPoint, this should also
    double fT[2][9];
    uint16_t fIsCoincident[2];  // bit set for each curve's coincident T
    unsigned char fUsed;
    unsigned char fMax;
    bool fAllowNear;
    bool fSwap;
#ifdef SK_DEBUG
    int fDepth;
#endif
};

extern int (SkIntersections::*CurveRay[])(const SkPoint[], const SkDLine& );
extern int (SkIntersections::*CurveVertical[])(const SkPoint[], SkScalar top, SkScalar bottom,
            SkScalar x, bool flipped);

#endif
