/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkIntersections.h"

void SkIntersections::append(const SkIntersections& i) {
    for (int index = 0; index < i.fUsed; ++index) {
        insert(i[0][index], i[1][index], i.pt(index));
    }
}

int (SkIntersections::*CurveVertical[])(const SkPoint[], SkScalar, SkScalar, SkScalar, bool) = {
    NULL,
    &SkIntersections::verticalLine,
    &SkIntersections::verticalQuad,
    &SkIntersections::verticalCubic
};

int (SkIntersections::*CurveRay[])(const SkPoint[], const SkDLine&) = {
    NULL,
    &SkIntersections::lineRay,
    &SkIntersections::quadRay,
    &SkIntersections::cubicRay
};

int SkIntersections::coincidentUsed() const {
    if (!fIsCoincident[0]) {
        SkASSERT(!fIsCoincident[1]);
        return 0;
    }
    int count = 0;
    SkDEBUGCODE(int count2 = 0;)
    for (int index = 0; index < fUsed; ++index) {
        if (fIsCoincident[0] & (1 << index)) {
            ++count;
        }
#ifdef SK_DEBUG
        if (fIsCoincident[1] & (1 << index)) {
            ++count2;
        }
#endif
    }
    SkASSERT(count == count2);
    return count;
}

int SkIntersections::cubicRay(const SkPoint pts[4], const SkDLine& line) {
    SkDCubic cubic;
    cubic.set(pts);
    fMax = 3;
    return intersectRay(cubic, line);
}

void SkIntersections::flip() {
    for (int index = 0; index < fUsed; ++index) {
        fT[1][index] = 1 - fT[1][index];
    }
}

int SkIntersections::insert(double one, double two, const SkDPoint& pt) {
    if (fIsCoincident[0] == 3 && between(fT[0][0], one, fT[0][1])) {
        // For now, don't allow a mix of coincident and non-coincident intersections
        return -1;
    }
    SkASSERT(fUsed <= 1 || fT[0][0] <= fT[0][1]);
    int index;
    for (index = 0; index < fUsed; ++index) {
        double oldOne = fT[0][index];
        double oldTwo = fT[1][index];
        if (one == oldOne && two == oldTwo) {
            return -1;
        }
        if (more_roughly_equal(oldOne, one) && more_roughly_equal(oldTwo, two)) {
            if ((precisely_zero(one) && !precisely_zero(oldOne))
                    || (precisely_equal(one, 1) && !precisely_equal(oldOne, 1))
                    || (precisely_zero(two) && !precisely_zero(oldTwo))
                    || (precisely_equal(two, 1) && !precisely_equal(oldTwo, 1))) {
                fT[0][index] = one;
                fT[1][index] = two;
                fPt[index] = pt;
            }
            return -1;
        }
    #if ONE_OFF_DEBUG
        if (pt.roughlyEqual(fPt[index])) {
            SkDebugf("%s t=%1.9g pts roughly equal\n", __FUNCTION__, one);
        }
    #endif
        if (fT[0][index] > one) {
            break;
        }
    }
    if (fUsed >= fMax) {
        SkASSERT(0);  // FIXME : this error, if it is to be handled at runtime in release, must
                      // be propagated all the way back down to the caller, and return failure.
        fUsed = 0;
        return 0;
    }
    int remaining = fUsed - index;
    if (remaining > 0) {
        memmove(&fPt[index + 1], &fPt[index], sizeof(fPt[0]) * remaining);
        memmove(&fPt2[index + 1], &fPt2[index], sizeof(fPt2[0]) * remaining);
        memmove(&fT[0][index + 1], &fT[0][index], sizeof(fT[0][0]) * remaining);
        memmove(&fT[1][index + 1], &fT[1][index], sizeof(fT[1][0]) * remaining);
        int clearMask = ~((1 << index) - 1);
        fIsCoincident[0] += fIsCoincident[0] & clearMask;
        fIsCoincident[1] += fIsCoincident[1] & clearMask;
    }
    fPt[index] = pt;
    fT[0][index] = one;
    fT[1][index] = two;
    ++fUsed;
    return index;
}

void SkIntersections::insertNear(double one, double two, const SkDPoint& pt1, const SkDPoint& pt2) {
    SkASSERT(one == 0 || one == 1);
    SkASSERT(two == 0 || two == 1);
    SkASSERT(pt1 != pt2);
    SkASSERT(fNearlySame[(int) one]);
    (void) insert(one, two, pt1);
    fPt2[one ? fUsed - 1 : 0] = pt2;
}

void SkIntersections::insertCoincident(double one, double two, const SkDPoint& pt) {
    int index = insertSwap(one, two, pt);
    int bit = 1 << index;
    fIsCoincident[0] |= bit;
    fIsCoincident[1] |= bit;
}

int SkIntersections::lineRay(const SkPoint pts[2], const SkDLine& line) {
    SkDLine l;
    l.set(pts);
    fMax = 2;
    return intersectRay(l, line);
}

void SkIntersections::offset(int base, double start, double end) {
    for (int index = base; index < fUsed; ++index) {
        double val = fT[fSwap][index];
        val *= end - start;
        val += start;
        fT[fSwap][index] = val;
    }
}

int SkIntersections::quadRay(const SkPoint pts[3], const SkDLine& line) {
    SkDQuad quad;
    quad.set(pts);
    fMax = 2;
    return intersectRay(quad, line);
}

void SkIntersections::quickRemoveOne(int index, int replace) {
    if (index < replace) {
        fT[0][index] = fT[0][replace];
    }
}

void SkIntersections::removeOne(int index) {
    int remaining = --fUsed - index;
    if (remaining <= 0) {
        return;
    }
    memmove(&fPt[index], &fPt[index + 1], sizeof(fPt[0]) * remaining);
    memmove(&fPt2[index], &fPt2[index + 1], sizeof(fPt2[0]) * remaining);
    memmove(&fT[0][index], &fT[0][index + 1], sizeof(fT[0][0]) * remaining);
    memmove(&fT[1][index], &fT[1][index + 1], sizeof(fT[1][0]) * remaining);
    SkASSERT(fIsCoincident[0] == 0);
    int coBit = fIsCoincident[0] & (1 << index);
    fIsCoincident[0] -= ((fIsCoincident[0] >> 1) & ~((1 << index) - 1)) + coBit;
    SkASSERT(!(coBit ^ (fIsCoincident[1] & (1 << index))));
    fIsCoincident[1] -= ((fIsCoincident[1] >> 1) & ~((1 << index) - 1)) + coBit;
}

void SkIntersections::swapPts() {
    int index;
    for (index = 0; index < fUsed; ++index) {
        SkTSwap(fT[0][index], fT[1][index]);
    }
}

int SkIntersections::verticalLine(const SkPoint a[2], SkScalar top, SkScalar bottom,
        SkScalar x, bool flipped) {
    SkDLine line;
    line.set(a);
    return vertical(line, top, bottom, x, flipped);
}

int SkIntersections::verticalQuad(const SkPoint a[3], SkScalar top, SkScalar bottom,
        SkScalar x, bool flipped) {
    SkDQuad quad;
    quad.set(a);
    return vertical(quad, top, bottom, x, flipped);
}

int SkIntersections::verticalCubic(const SkPoint a[4], SkScalar top, SkScalar bottom,
        SkScalar x, bool flipped) {
    SkDCubic cubic;
    cubic.set(a);
    return vertical(cubic, top, bottom, x, flipped);
}
