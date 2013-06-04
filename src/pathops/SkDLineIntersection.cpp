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

/*
   Determine the intersection point of two line segments
   Return FALSE if the lines don't intersect
   from: http://paulbourke.net/geometry/lineline2d/
 */

int SkIntersections::intersect(const SkDLine& a, const SkDLine& b) {
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
            return fUsed = 0;
        }
        fT[0][0] = numerA;
        fT[1][0] = numerB;
        fPt[0] = a.xyAtT(numerA);
        return computePoints(a, 1);
    }
   /* See if the axis intercepts match:
              ay - ax * ayLen / axLen  ==          by - bx * ayLen / axLen
     axLen * (ay - ax * ayLen / axLen) == axLen * (by - bx * ayLen / axLen)
     axLen *  ay - ax * ayLen          == axLen *  by - bx * ayLen
    */
    if (!AlmostEqualUlps(axLen * a[0].fY - ayLen * a[0].fX,
            axLen * b[0].fY - ayLen * b[0].fX)) {
        return fUsed = 0;
    }
    const double* aPtr;
    const double* bPtr;
    if (fabs(axLen) > fabs(ayLen) || fabs(bxLen) > fabs(byLen)) {
        aPtr = &a[0].fX;
        bPtr = &b[0].fX;
    } else {
        aPtr = &a[0].fY;
        bPtr = &b[0].fY;
    }
    double a0 = aPtr[0];
    double a1 = aPtr[2];
    double b0 = bPtr[0];
    double b1 = bPtr[2];
    // OPTIMIZATION: restructure to reject before the divide
    // e.g., if ((a0 - b0) * (a0 - a1) < 0 || abs(a0 - b0) > abs(a0 - a1))
    // (except efficient)
    double aDenom = a0 - a1;
    if (approximately_zero(aDenom)) {
        if (!between(b0, a0, b1)) {
            return fUsed = 0;
        }
        fT[0][0] = fT[0][1] = 0;
    } else {
        double at0 = (a0 - b0) / aDenom;
        double at1 = (a0 - b1) / aDenom;
        if ((at0 < 0 && at1 < 0) || (at0 > 1 && at1 > 1)) {
            return fUsed = 0;
        }
        fT[0][0] = SkTMax(SkTMin(at0, 1.0), 0.0);
        fT[0][1] = SkTMax(SkTMin(at1, 1.0), 0.0);
    }
    double bDenom = b0 - b1;
    if (approximately_zero(bDenom)) {
        fT[1][0] = fT[1][1] = 0;
    } else {
        int bIn = aDenom * bDenom < 0;
        fT[1][bIn] = SkTMax(SkTMin((b0 - a0) / bDenom, 1.0), 0.0);
        fT[1][!bIn] = SkTMax(SkTMin((b0 - a1) / bDenom, 1.0), 0.0);
    }
    bool second = fabs(fT[0][0] - fT[0][1]) > FLT_EPSILON;
    SkASSERT((fabs(fT[1][0] - fT[1][1]) <= FLT_EPSILON) ^ second);
    return computePoints(a, 1 + second);
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
    if (AlmostEqualUlps(min, max)) {
        fT[0][0] = 0;
        fT[0][1] = 1;
        return fUsed = 2;
    }
    fT[0][0] = (y - line[0].fY) / (line[1].fY - line[0].fY);
    return fUsed = 1;
}

int SkIntersections::horizontal(const SkDLine& line, double left, double right,
                                double y, bool flipped) {
    int result = horizontal(line, y);
    switch (result) {
        case 0:
            break;
        case 1: {
            double xIntercept = line[0].fX + fT[0][0] * (line[1].fX - line[0].fX);
            if (!precisely_between(left, xIntercept, right)) {
                return fUsed = 0;
            }
            fT[1][0] = (xIntercept - left) / (right - left);
            break;
        }
        case 2:
            double a0 = line[0].fX;
            double a1 = line[1].fX;
            double b0 = flipped ? right : left;
            double b1 = flipped ? left : right;
            // FIXME: share common code below
            double at0 = (a0 - b0) / (a0 - a1);
            double at1 = (a0 - b1) / (a0 - a1);
            if ((at0 < 0 && at1 < 0) || (at0 > 1 && at1 > 1)) {
                return fUsed = 0;
            }
            fT[0][0] = SkTMax(SkTMin(at0, 1.0), 0.0);
            fT[0][1] = SkTMax(SkTMin(at1, 1.0), 0.0);
            int bIn = (a0 - a1) * (b0 - b1) < 0;
            fT[1][bIn] = SkTMax(SkTMin((b0 - a0) / (b0 - b1), 1.0), 0.0);
            fT[1][!bIn] = SkTMax(SkTMin((b0 - a1) / (b0 - b1), 1.0), 0.0);
            bool second = fabs(fT[0][0] - fT[0][1]) > FLT_EPSILON;
            SkASSERT((fabs(fT[1][0] - fT[1][1]) <= FLT_EPSILON) ^ second);
            return computePoints(line, 1 + second);
    }
    if (flipped) {
        // OPTIMIZATION: instead of swapping, pass original line, use [1].fX - [0].fX
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

int SkIntersections::vertical(const SkDLine& line, double top, double bottom,
                              double x, bool flipped) {
    int result = vertical(line, x);
    switch (result) {
        case 0:
            break;
        case 1: {
            double yIntercept = line[0].fY + fT[0][0] * (line[1].fY - line[0].fY);
            if (!precisely_between(top, yIntercept, bottom)) {
                return fUsed = 0;
            }
            fT[1][0] = (yIntercept - top) / (bottom - top);
            break;
        }
        case 2:
            double a0 = line[0].fY;
            double a1 = line[1].fY;
            double b0 = flipped ? bottom : top;
            double b1 = flipped ? top : bottom;
            // FIXME: share common code above
            double at0 = (a0 - b0) / (a0 - a1);
            double at1 = (a0 - b1) / (a0 - a1);
            if ((at0 < 0 && at1 < 0) || (at0 > 1 && at1 > 1)) {
                return fUsed = 0;
            }
            fT[0][0] = SkTMax(SkTMin(at0, 1.0), 0.0);
            fT[0][1] = SkTMax(SkTMin(at1, 1.0), 0.0);
            int bIn = (a0 - a1) * (b0 - b1) < 0;
            fT[1][bIn] = SkTMax(SkTMin((b0 - a0) / (b0 - b1), 1.0), 0.0);
            fT[1][!bIn] = SkTMax(SkTMin((b0 - a1) / (b0 - b1), 1.0), 0.0);
            bool second = fabs(fT[0][0] - fT[0][1]) > FLT_EPSILON;
            SkASSERT((fabs(fT[1][0] - fT[1][1]) <= FLT_EPSILON) ^ second);
            return computePoints(line, 1 + second);
    }
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
