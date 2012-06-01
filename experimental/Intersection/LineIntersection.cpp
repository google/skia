#include "CurveIntersection.h"
#include "Intersections.h"
#include "LineIntersection.h"
#include <algorithm> // used for std::swap


/*
   Determine the intersection point of two line segments
   Return FALSE if the lines don't intersect
   from: http://paulbourke.net/geometry/lineline2d/
*/

int intersect(const _Line& a, const _Line& b, double aRange[2], double bRange[2]) {
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
    double denom  = byLen * axLen - ayLen * bxLen;
    if (approximately_zero_squared(denom)) {
       /* See if the axis intercepts match:
                  ay - ax * ayLen / axLen  ==          by - bx * ayLen / axLen
         axLen * (ay - ax * ayLen / axLen) == axLen * (by - bx * ayLen / axLen)
         axLen *  ay - ax * ayLen          == axLen *  by - bx * ayLen
        */
        if (approximately_equal_squared(axLen * a[0].y - ayLen * a[0].x,
                axLen * b[0].y - ayLen * b[0].x)) {
            const double* aPtr;
            const double* bPtr;
            if (fabs(axLen) > fabs(ayLen) || fabs(bxLen) > fabs(byLen)) {
                aPtr = &a[0].x;
                bPtr = &b[0].x;
            } else {
                aPtr = &a[0].y;
                bPtr = &b[0].y;
            }
            double aMin = aPtr[0];
            double aMax = aPtr[2];
            double bMin = bPtr[0];
            double bMax = bPtr[2];
            if (aMin > aMax) {
                std::swap(aMin, aMax);
            }
            if (bMin > bMax) {
                std::swap(bMin, bMax);
            }
            if (aMax < bMin || bMax < aMin) {
                return 0;
            }
            if (aRange) {
                aRange[0] = bMin <= aMin ? 0 : (bMin - aMin) / (aMax - aMin);
                aRange[1] = bMax >= aMax ? 1 : (bMax - aMin) / (aMax - aMin);
            }
            int bIn = (aPtr[0] - aPtr[2]) * (bPtr[0] - bPtr[2]) < 0;
            if (bRange) {
                bRange[bIn] = aMin <= bMin ? 0 : (aMin - bMin) / (bMax - bMin);
                bRange[!bIn] = aMax >= bMax ? 1 : (aMax - bMin) / (bMax - bMin);
            }
            return 1 + ((aRange[0] != aRange[1]) || (bRange[0] != bRange[1]));
        }
    }
    double ab0y = a[0].y - b[0].y;
    double ab0x = a[0].x - b[0].x;
    double numerA = ab0y * bxLen - byLen * ab0x;
    if (numerA < 0 && denom > numerA || numerA > 0 && denom < numerA) {
        return 0;
    }
    double numerB = ab0y * axLen - ayLen * ab0x;
    if (numerB < 0 && denom > numerB || numerB > 0 && denom < numerB) {
        return 0;
    }
    /* Is the intersection along the the segments */
    if (aRange) {
        aRange[0] = numerA / denom;
    }
    if (bRange) {
        bRange[0] = numerB / denom;
    }
    return 1;
}

int horizontalIntersect(const _Line& line, double y, double tRange[2]) {
    double min = line[0].y;
    double max = line[1].y;
    if (min > max) {
        std::swap(min, max);
    }
    if (min > y || max < y) {
        return 0;
    }
    if (approximately_equal(min, max)) {
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
            double lineL = line[0].x;
            double lineR = line[1].x;
            if (lineL > lineR) {
                std::swap(lineL, lineR);
            }
            double overlapL = std::max(left, lineL);
            double overlapR = std::min(right, lineR);
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
            break;
    }
    if (flipped) {
        // OPTIMIZATION: instead of swapping, pass original line, use [1].x - [0].x
        for (int index = 0; index < result; ++index) {
            intersections.fT[1][index] = 1 - intersections.fT[1][index];
        }
    }
    return result;
}

static int verticalIntersect(const _Line& line, double x, double tRange[2]) {
    double min = line[0].x;
    double max = line[1].x;
    if (min > max) {
        std::swap(min, max);
    }
    if (min > x || max < x) {
        return 0;
    }
    if (approximately_equal(min, max)) {
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
            double lineT = line[0].y;
            double lineB = line[1].y;
            if (lineT > lineB) {
                std::swap(lineT, lineB);
            }
            double overlapT = std::max(top, lineT);
            double overlapB = std::min(bottom, lineB);
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
            break;
    }
    if (flipped) {
        // OPTIMIZATION: instead of swapping, pass original line, use [1].y - [0].y
        for (int index = 0; index < result; ++index) {
            intersections.fT[1][index] = 1 - intersections.fT[1][index];
        }
    }
    return result;
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
