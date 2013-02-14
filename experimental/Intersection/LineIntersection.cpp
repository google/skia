/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "Intersections.h"
#include "LineIntersection.h"
#include "LineUtilities.h"

/* Determine the intersection point of two lines. This assumes the lines are not parallel,
   and that that the lines are infinite.
   From http://en.wikipedia.org/wiki/Line-line_intersection
 */
void lineIntersect(const _Line& a, const _Line& b, _Point& p) {
    double axLen = a[1].x - a[0].x;
    double ayLen = a[1].y - a[0].y;
    double bxLen = b[1].x - b[0].x;
    double byLen = b[1].y - b[0].y;
    double denom = byLen * axLen - ayLen * bxLen;
    SkASSERT(denom);
    double term1 = a[1].x * a[0].y - a[1].y * a[0].x;
    double term2 = b[1].x * b[0].y - b[1].y * b[0].x;
    p.x = (term1 * bxLen - axLen * term2) / denom;
    p.y = (term1 * byLen - ayLen * term2) / denom;
}

static int computePoints(const _Line& a, int used, Intersections& i) {
    i.fPt[0] = xy_at_t(a, i.fT[0][0]);
    if ((i.fUsed = used) == 2) {
        i.fPt[1] = xy_at_t(a, i.fT[0][1]);
    }
    return i.fUsed;
}

/*
   Determine the intersection point of two line segments
   Return FALSE if the lines don't intersect
   from: http://paulbourke.net/geometry/lineline2d/
 */

int intersect(const _Line& a, const _Line& b, Intersections& i) {
    double axLen = a[1].x - a[0].x;
    double ayLen = a[1].y - a[0].y;
    double bxLen = b[1].x - b[0].x;
    double byLen = b[1].y - b[0].y;
    /* Slopes match when denom goes to zero:
                      axLen / ayLen ==                   bxLen / byLen
    (ayLen * byLen) * axLen / ayLen == (ayLen * byLen) * bxLen / byLen
             byLen  * axLen         ==  ayLen          * bxLen
             byLen  * axLen         -   ayLen          * bxLen == 0 ( == denom )
     */
    double denom = byLen * axLen - ayLen * bxLen;
    double ab0y = a[0].y - b[0].y;
    double ab0x = a[0].x - b[0].x;
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
        i.fT[0][0] = numerA;
        i.fT[1][0] = numerB;
        i.fPt[0] = xy_at_t(a, numerA);
        return computePoints(a, 1, i);
    }
   /* See if the axis intercepts match:
              ay - ax * ayLen / axLen  ==          by - bx * ayLen / axLen
     axLen * (ay - ax * ayLen / axLen) == axLen * (by - bx * ayLen / axLen)
     axLen *  ay - ax * ayLen          == axLen *  by - bx * ayLen
    */
    // FIXME: need to use AlmostEqualUlps variant instead
    if (!approximately_equal_squared(axLen * a[0].y - ayLen * a[0].x,
            axLen * b[0].y - ayLen * b[0].x)) {
        return 0;
    }
    const double* aPtr;
    const double* bPtr;
    if (fabs(axLen) > fabs(ayLen) || fabs(bxLen) > fabs(byLen)) {
        aPtr = &a[0].x;
        bPtr = &b[0].x;
    } else {
        aPtr = &a[0].y;
        bPtr = &b[0].y;
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
            return 0;
        }
        i.fT[0][0] = i.fT[0][1] = 0;
    } else {
        double at0 = (a0 - b0) / aDenom;
        double at1 = (a0 - b1) / aDenom;
        if ((at0 < 0 && at1 < 0) || (at0 > 1 && at1 > 1)) {
            return 0;
        }
        i.fT[0][0] = SkTMax(SkTMin(at0, 1.0), 0.0);
        i.fT[0][1] = SkTMax(SkTMin(at1, 1.0), 0.0);
    }
    double bDenom = b0 - b1;
    if (approximately_zero(bDenom)) {
        i.fT[1][0] = i.fT[1][1] = 0;
    } else {
        int bIn = aDenom * bDenom < 0;
        i.fT[1][bIn] = SkTMax(SkTMin((b0 - a0) / bDenom, 1.0), 0.0);
        i.fT[1][!bIn] = SkTMax(SkTMin((b0 - a1) / bDenom, 1.0), 0.0);
    }
    bool second = fabs(i.fT[0][0] - i.fT[0][1]) > FLT_EPSILON;
    SkASSERT((fabs(i.fT[1][0] - i.fT[1][1]) <= FLT_EPSILON) ^ second);
    return computePoints(a, 1 + second, i);
}

int horizontalIntersect(const _Line& line, double y, double tRange[2]) {
    double min = line[0].y;
    double max = line[1].y;
    if (min > max) {
        SkTSwap(min, max);
    }
    if (min > y || max < y) {
        return 0;
    }
    if (AlmostEqualUlps(min, max)) {
        tRange[0] = 0;
        tRange[1] = 1;
        return 2;
    }
    tRange[0] = (y - line[0].y) / (line[1].y - line[0].y);
    return 1;
}

// OPTIMIZATION  Given: dy = line[1].y - line[0].y
// and: xIntercept / (y - line[0].y) == (line[1].x - line[0].x) / dy
// then: xIntercept * dy == (line[1].x - line[0].x) * (y - line[0].y)
// Assuming that dy is always > 0, the line segment intercepts if:
//   left * dy <= xIntercept * dy <= right * dy
// thus: left * dy <= (line[1].x - line[0].x) * (y - line[0].y) <= right * dy
// (clever as this is, it does not give us the t value, so may be useful only
// as a quick reject -- and maybe not then; it takes 3 muls, 3 adds, 2 cmps)
int horizontalLineIntersect(const _Line& line, double left, double right,
        double y, double tRange[2]) {
    int result = horizontalIntersect(line, y, tRange);
    if (result != 1) {
        // FIXME: this is incorrect if result == 2
        return result;
    }
    double xIntercept = line[0].x + tRange[0] * (line[1].x - line[0].x);
    if (xIntercept > right || xIntercept < left) {
        return 0;
    }
    return result;
}

int horizontalIntersect(const _Line& line, double left, double right,
        double y, bool flipped, Intersections& intersections) {
    int result = horizontalIntersect(line, y, intersections.fT[0]);
    switch (result) {
        case 0:
            break;
        case 1: {
            double xIntercept = line[0].x + intersections.fT[0][0]
                    * (line[1].x - line[0].x);
            if (xIntercept > right || xIntercept < left) {
                return 0;
            }
            intersections.fT[1][0] = (xIntercept - left) / (right - left);
            break;
        }
        case 2:
        #if 0 // sorting edges fails to preserve original direction
            double lineL = line[0].x;
            double lineR = line[1].x;
            if (lineL > lineR) {
                SkTSwap(lineL, lineR);
            }
            double overlapL = SkTMax(left, lineL);
            double overlapR = SkTMin(right, lineR);
            if (overlapL > overlapR) {
                return 0;
            }
            if (overlapL == overlapR) {
                result = 1;
            }
            intersections.fT[0][0] = (overlapL - line[0].x) / (line[1].x - line[0].x);
            intersections.fT[1][0] = (overlapL - left) / (right - left);
            if (result > 1) {
                intersections.fT[0][1] = (overlapR - line[0].x) / (line[1].x - line[0].x);
                intersections.fT[1][1] = (overlapR - left) / (right - left);
            }
        #else
            double a0 = line[0].x;
            double a1 = line[1].x;
            double b0 = flipped ? right : left;
            double b1 = flipped ? left : right;
            // FIXME: share common code below
            double at0 = (a0 - b0) / (a0 - a1);
            double at1 = (a0 - b1) / (a0 - a1);
            if ((at0 < 0 && at1 < 0) || (at0 > 1 && at1 > 1)) {
                return 0;
            }
            intersections.fT[0][0] = SkTMax(SkTMin(at0, 1.0), 0.0);
            intersections.fT[0][1] = SkTMax(SkTMin(at1, 1.0), 0.0);
            int bIn = (a0 - a1) * (b0 - b1) < 0;
            intersections.fT[1][bIn] = SkTMax(SkTMin((b0 - a0) / (b0 - b1),
                    1.0), 0.0);
            intersections.fT[1][!bIn] = SkTMax(SkTMin((b0 - a1) / (b0 - b1),
                    1.0), 0.0);
            bool second = fabs(intersections.fT[0][0] - intersections.fT[0][1])
                    > FLT_EPSILON;
            SkASSERT((fabs(intersections.fT[1][0] - intersections.fT[1][1])
                    <= FLT_EPSILON) ^ second);
            return computePoints(line, 1 + second, intersections);
        #endif
            break;
    }
    if (flipped) {
        // OPTIMIZATION: instead of swapping, pass original line, use [1].x - [0].x
        for (int index = 0; index < result; ++index) {
            intersections.fT[1][index] = 1 - intersections.fT[1][index];
        }
    }
    return computePoints(line, result, intersections);
}

static int verticalIntersect(const _Line& line, double x, double tRange[2]) {
    double min = line[0].x;
    double max = line[1].x;
    if (min > max) {
        SkTSwap(min, max);
    }
    if (min > x || max < x) {
        return 0;
    }
    if (AlmostEqualUlps(min, max)) {
        tRange[0] = 0;
        tRange[1] = 1;
        return 2;
    }
    tRange[0] = (x - line[0].x) / (line[1].x - line[0].x);
    return 1;
}

int verticalIntersect(const _Line& line, double top, double bottom,
        double x, bool flipped, Intersections& intersections) {
    int result = verticalIntersect(line, x, intersections.fT[0]);
    switch (result) {
        case 0:
            break;
        case 1: {
            double yIntercept = line[0].y + intersections.fT[0][0]
                    * (line[1].y - line[0].y);
            if (yIntercept > bottom || yIntercept < top) {
                return 0;
            }
            intersections.fT[1][0] = (yIntercept - top) / (bottom - top);
            break;
        }
        case 2:
        #if 0 // sorting edges fails to preserve original direction
            double lineT = line[0].y;
            double lineB = line[1].y;
            if (lineT > lineB) {
                SkTSwap(lineT, lineB);
            }
            double overlapT = SkTMax(top, lineT);
            double overlapB = SkTMin(bottom, lineB);
            if (overlapT > overlapB) {
                return 0;
            }
            if (overlapT == overlapB) {
                result = 1;
            }
            intersections.fT[0][0] = (overlapT - line[0].y) / (line[1].y - line[0].y);
            intersections.fT[1][0] = (overlapT - top) / (bottom - top);
            if (result > 1) {
                intersections.fT[0][1] = (overlapB - line[0].y) / (line[1].y - line[0].y);
                intersections.fT[1][1] = (overlapB - top) / (bottom - top);
            }
        #else
            double a0 = line[0].y;
            double a1 = line[1].y;
            double b0 = flipped ? bottom : top;
            double b1 = flipped ? top : bottom;
            // FIXME: share common code above
            double at0 = (a0 - b0) / (a0 - a1);
            double at1 = (a0 - b1) / (a0 - a1);
            if ((at0 < 0 && at1 < 0) || (at0 > 1 && at1 > 1)) {
                return 0;
            }
            intersections.fT[0][0] = SkTMax(SkTMin(at0, 1.0), 0.0);
            intersections.fT[0][1] = SkTMax(SkTMin(at1, 1.0), 0.0);
            int bIn = (a0 - a1) * (b0 - b1) < 0;
            intersections.fT[1][bIn] = SkTMax(SkTMin((b0 - a0) / (b0 - b1),
                    1.0), 0.0);
            intersections.fT[1][!bIn] = SkTMax(SkTMin((b0 - a1) / (b0 - b1),
                    1.0), 0.0);
            bool second = fabs(intersections.fT[0][0] - intersections.fT[0][1])
                    > FLT_EPSILON;
            SkASSERT((fabs(intersections.fT[1][0] - intersections.fT[1][1])
                    <= FLT_EPSILON) ^ second);
            return computePoints(line, 1 + second, intersections);
        #endif
            break;
    }
    if (flipped) {
        // OPTIMIZATION: instead of swapping, pass original line, use [1].y - [0].y
        for (int index = 0; index < result; ++index) {
            intersections.fT[1][index] = 1 - intersections.fT[1][index];
        }
    }
    return computePoints(line, result, intersections);
}

// from http://www.bryceboe.com/wordpress/wp-content/uploads/2006/10/intersect.py
// 4 subs, 2 muls, 1 cmp
static bool ccw(const _Point& A, const _Point& B, const _Point& C) {
    return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
}

// 16 subs, 8 muls, 6 cmps
bool testIntersect(const _Line& a, const _Line& b) {
    return ccw(a[0], b[0], b[1]) != ccw(a[1], b[0], b[1])
            && ccw(a[0], a[1], b[0]) != ccw(a[0], a[1], b[1]);
}
