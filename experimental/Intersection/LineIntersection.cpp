#include "DataTypes.h"
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
            if (bRange) {
                bRange[0] = aMin <= bMin ? 0 : (aMin - bMin) / (bMax - bMin);
                bRange[1] = aMax >= bMax ? 1 : (aMax - bMin) / (bMax - bMin);
            }
            return 2;
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
