/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkIntersections.h"
#include "SkPathOpsLine.h"

/* Determine the intersection point of two lines. This assumes the lines are not parallel,
   and that that the lines are infinite.
   From http://en.wikipedia.org/wiki/Line-line_intersection
 */
SkDPoint SkIntersections::Line(const SkDLine& a, const SkDLine& b) {
    double axLen = a[1].fX - a[0].fX;
    double ayLen = a[1].fY - a[0].fY;
    double bxLen = b[1].fX - b[0].fX;
    double byLen = b[1].fY - b[0].fY;
    double denom = byLen * axLen - ayLen * bxLen;
    SkASSERT(denom);
    double term1 = a[1].fX * a[0].fY - a[1].fY * a[0].fX;
    double term2 = b[1].fX * b[0].fY - b[1].fY * b[0].fX;
    SkDPoint p;
    p.fX = (term1 * bxLen - axLen * term2) / denom;
    p.fY = (term1 * byLen - ayLen * term2) / denom;
    return p;
}

int SkIntersections::computePoints(const SkDLine& line, int used) {
    fPt[0] = line.xyAtT(fT[0][0]);
    if ((fUsed = used) == 2) {
        fPt[1] = line.xyAtT(fT[0][1]);
    }
    return fUsed;
}

int SkIntersections::intersectRay(const SkDLine& a, const SkDLine& b) {
    double axLen = a[1].fX - a[0].fX;
    double ayLen = a[1].fY - a[0].fY;
    double bxLen = b[1].fX - b[0].fX;
    double byLen = b[1].fY - b[0].fY;
    /* Slopes match when denom goes to zero:
                      axLen / ayLen ==                   bxLen / byLen
    (ayLen * byLen) * axLen / ayLen == (ayLen * byLen) * bxLen / byLen
             byLen  * axLen         ==  ayLen          * bxLen
             byLen  * axLen         -   ayLen          * bxLen == 0 ( == denom )
     */
    double denom = byLen * axLen - ayLen * bxLen;
    double ab0y = a[0].fY - b[0].fY;
    double ab0x = a[0].fX - b[0].fX;
    double numerA = ab0y * bxLen - byLen * ab0x;
    double numerB = ab0y * axLen - ayLen * ab0x;
    numerA /= denom;
    numerB /= denom;
    int used;
    if (!approximately_zero(denom)) {
        fT[0][0] = numerA;
        fT[1][0] = numerB;
        used = 1;
    } else {
       /* See if the axis intercepts match:
                  ay - ax * ayLen / axLen  ==          by - bx * ayLen / axLen
         axLen * (ay - ax * ayLen / axLen) == axLen * (by - bx * ayLen / axLen)
         axLen *  ay - ax * ayLen          == axLen *  by - bx * ayLen
        */
        if (!AlmostEqualUlps(axLen * a[0].fY - ayLen * a[0].fX,
                axLen * b[0].fY - ayLen * b[0].fX)) {
            return fUsed = 0;
        }
        // there's no great answer for intersection points for coincident rays, but return something
        fT[0][0] = fT[1][0] = 0;
        fT[1][0] = fT[1][1] = 1;
        used = 2;
    }
    return computePoints(a, used);
}

static bool checkEndPoint(double x, double y, const SkDLine& l, double* tPtr, int useX) {
    if (!between(l[0].fX, x, l[1].fX) || !between(l[0].fY, y, l[1].fY)) {
        return false;
    }
    double xLen = l[1].fX - l[0].fX;
    double yLen = l[1].fY - l[0].fY;
    if (useX < 0) {
        useX = SkTAbs(xLen) > SkTAbs(yLen);
    }
    // OPTIMIZATION: do between test before divide
    double t = useX ? (x - l[0].fX) / xLen : (y - l[0].fY) / yLen;
    if (!between(0, t, 1)) {
        return false;
    }
    double opp = useX ? (1 - t) * l[0].fY + t * l[1].fY : (1 - t) * l[0].fX + t * l[1].fX;
    if (!AlmostEqualUlps(opp, useX ? y : x)) {
        return false;
    }
    *tPtr = t;
    return true;
}

// note that this only works if both lines are neither horizontal nor vertical
int SkIntersections::intersect(const SkDLine& a, const SkDLine& b) {
    // see if end points intersect the opposite line
    double t;
    for (int iA = 0; iA < 2; ++iA) {
        if (!checkEndPoint(a[iA].fX, a[iA].fY, b, &t, -1)) {
            continue;
        }
        insert(iA, t, a[iA]);
    }
    for (int iB = 0; iB < 2; ++iB) {
        if (!checkEndPoint(b[iB].fX, b[iB].fY, a, &t, -1)) {
            continue;
        }
        insert(t, iB, b[iB]);
    }
    if (used() > 0) {
        SkASSERT(fUsed <= 2);
        return used(); // coincident lines are returned here
    }
    /* Determine the intersection point of two line segments
       Return FALSE if the lines don't intersect
       from: http://paulbourke.net/geometry/lineline2d/ */
    double axLen = a[1].fX - a[0].fX;
    double ayLen = a[1].fY - a[0].fY;
    double bxLen = b[1].fX - b[0].fX;
    double byLen = b[1].fY - b[0].fY;
    /* Slopes match when denom goes to zero:
                      axLen / ayLen ==                   bxLen / byLen
    (ayLen * byLen) * axLen / ayLen == (ayLen * byLen) * bxLen / byLen
             byLen  * axLen         ==  ayLen          * bxLen
             byLen  * axLen         -   ayLen          * bxLen == 0 ( == denom )
     */
    double denom = byLen * axLen - ayLen * bxLen;
    double ab0y = a[0].fY - b[0].fY;
    double ab0x = a[0].fX - b[0].fX;
    double numerA = ab0y * bxLen - byLen * ab0x;
    double numerB = ab0y * axLen - ayLen * ab0x;
    bool mayNotOverlap = (numerA < 0 && denom > numerA) || (numerA > 0 && denom < numerA)
            || (numerB < 0 && denom > numerB) || (numerB > 0 && denom < numerB);
    numerA /= denom;
    numerB /= denom;
    if ((!approximately_zero(denom) || (!approximately_zero_inverse(numerA)
            && !approximately_zero_inverse(numerB))) && !sk_double_isnan(numerA)
            && !sk_double_isnan(numerB)) {
        if (mayNotOverlap) {
            return 0;
        }
        fT[0][0] = numerA;
        fT[1][0] = numerB;
        fPt[0] = a.xyAtT(numerA);
        return computePoints(a, 1);
    }
    return 0;
}

int SkIntersections::horizontal(const SkDLine& line, double y) {
    double min = line[0].fY;
    double max = line[1].fY;
    if (min > max) {
        SkTSwap(min, max);
    }
    if (min > y || max < y) {
        return fUsed = 0;
    }
    if (AlmostEqualUlps(min, max) && max - min < fabs(line[0].fX - line[1].fX)) {
        fT[0][0] = 0;
        fT[0][1] = 1;
        return fUsed = 2;
    }
    fT[0][0] = (y - line[0].fY) / (line[1].fY - line[0].fY);
    return fUsed = 1;
}

static bool checkEndPointH(const SkDPoint& end, double left, double right,
                           double y, bool flipped, double* tPtr) {
    if (!between(left, end.fX, right) || !AlmostEqualUlps(y, end.fY)) {
        return false;
    }
    double t = (end.fX - left) / (right - left);
    SkASSERT(between(0, t, 1));
    *tPtr = flipped ? 1 - t : t;
    return true;
}

int SkIntersections::horizontal(const SkDLine& line, double left, double right,
                                double y, bool flipped) {
    // see if end points intersect the opposite line
    double t;
    if (checkEndPoint(left, y, line, &t, true)) {
        insert(t, flipped, left, y);
    }
    if (left != right) {
        if (checkEndPoint(right, y, line, &t, true)) {
            insert(t, !flipped, right, y);
        }
        for (int index = 0; index < 2; ++index) {
            if (!checkEndPointH(line[index], left, right, y, flipped, &t)) {
                continue;
            }
            insert(index, t, line[index]);
        }
    }
    if (used() > 0) {
        SkASSERT(fUsed <= 2);
        return used(); // coincident lines are returned here
    }
    int result = horizontal(line, y);
    if (!result) {
        return 0;
    }
    SkASSERT(result == 1);
    double xIntercept = line[0].fX + fT[0][0] * (line[1].fX - line[0].fX);
    if (!precisely_between(left, xIntercept, right)) {
        return fUsed = 0;
    }
    fT[1][0] = (xIntercept - left) / (right - left);
    if (flipped) {
        // OPTIMIZATION: ? instead of swapping, pass original line, use [1].fX - [0].fX
        for (int index = 0; index < result; ++index) {
            fT[1][index] = 1 - fT[1][index];
        }
    }
    return computePoints(line, result);
}

int SkIntersections::vertical(const SkDLine& line, double x) {
    double min = line[0].fX;
    double max = line[1].fX;
    if (min > max) {
        SkTSwap(min, max);
    }
    if (!precisely_between(min, x, max)) {
        return fUsed = 0;
    }
    if (AlmostEqualUlps(min, max)) {
        fT[0][0] = 0;
        fT[0][1] = 1;
        return fUsed = 2;
    }
    fT[0][0] = (x - line[0].fX) / (line[1].fX - line[0].fX);
    return fUsed = 1;
}

static bool checkEndPointV(const SkDPoint& end, double top, double bottom,
                           double x, bool flipped, double* tPtr) {
    if (!between(top, end.fY, bottom) || !AlmostEqualUlps(x, end.fX)) {
        return false;
    }
    double t = (end.fY - top) / (bottom - top);
    SkASSERT(between(0, t, 1));
    *tPtr = flipped ? 1 - t : t;
    return true;
}

int SkIntersections::vertical(const SkDLine& line, double top, double bottom,
                                double x, bool flipped) {
    // see if end points intersect the opposite line
    double t;
    if (checkEndPoint(x, top, line, &t, false)) {
        insert(t, flipped, x, top);
    }
    if (top != bottom) {
        if (checkEndPoint(x, bottom,line, &t, false)) {
            insert(t, !flipped, x, bottom);
        }
        for (int index = 0; index < 2; ++index) {
            if (!checkEndPointV(line[index], top, bottom, x, flipped, &t)) {
                continue;
            }
            insert( index, t, line[index]);
        }
    }
    if (used() > 0) {
        SkASSERT(fUsed <= 2);
        return used(); // coincident lines are returned here
    }
    int result = vertical(line, x);
    if (!result) {
        return 0;
    }
    SkASSERT(result == 1);
    double yIntercept = line[0].fY + fT[0][0] * (line[1].fY - line[0].fY);
    if (!precisely_between(top, yIntercept, bottom)) {
        return fUsed = 0;
    }
    fT[1][0] = (yIntercept - top) / (bottom - top);
    if (flipped) {
        // OPTIMIZATION: instead of swapping, pass original line, use [1].fY - [0].fY
        for (int index = 0; index < result; ++index) {
            fT[1][index] = 1 - fT[1][index];
        }
    }
    return computePoints(line, result);
}

// from http://www.bryceboe.com/wordpress/wp-content/uploads/2006/10/intersect.py
// 4 subs, 2 muls, 1 cmp
static bool ccw(const SkDPoint& A, const SkDPoint& B, const SkDPoint& C) {
    return (C.fY - A.fY) * (B.fX - A.fX) > (B.fY - A.fY) * (C.fX - A.fX);
}

// 16 subs, 8 muls, 6 cmps
bool SkIntersections::Test(const SkDLine& a, const SkDLine& b) {
    return ccw(a[0], b[0], b[1]) != ccw(a[1], b[0], b[1])
            && ccw(a[0], a[1], b[0]) != ccw(a[0], a[1], b[1]);
}
