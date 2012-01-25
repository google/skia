#include "DataTypes.h"
#include "LineIntersection.h"
#include "LineParameters.h"


static bool no_intersection(_Point* result) {
    result->x = 0;
    result->y = 0;
    return false;
}

/*
   Determine the intersection point of two line segments
   Return FALSE if the lines don't intersect
   from: http://paulbourke.net/geometry/lineline2d/
   min: 8 subs, 4 muls, 4 cmps
*/

bool lineIntersect(const _Line& a, const _Line& b, _Point* result) {
    LineParameters paramsA, paramsB;
    double axLen = a[1].x - a[0].x;
    double ayLen = a[1].y - a[0].y;
    double bxLen = b[1].x - b[0].x;
    double byLen = b[1].y - b[0].y;
    double denom  = byLen * axLen - ayLen * bxLen;
    if (approximately_zero_squared(denom)) { // is either 'a' or 'b' a point?
        bool aIsPoint = approximately_zero(axLen) && approximately_zero(ayLen);
        bool bIsPoint = approximately_zero(bxLen) && approximately_zero(byLen);
        if (aIsPoint & bIsPoint) {
            if (!approximately_equal(a[0].x, b[0].x)
                    || !approximately_equal(a[0].y, b[0].y)) {
                return no_intersection(result);
            }
        } else {
            double ptToLineDistance;
            if (aIsPoint) {
                paramsB.lineEndPoints(b);
                ptToLineDistance = paramsB.pointDistance(a[0]);
            } else {
                paramsA.lineEndPoints(a);
                ptToLineDistance = paramsA.pointDistance(b[0]);
            }
            if (!approximately_zero(ptToLineDistance)) {
                return no_intersection(result);
            }
        }
        if (aIsPoint) {
            result->x = a[0].x;
            result->y = a[0].y;
        } else {
            result->x = b[0].x;
            result->y = b[0].y;
        }
        return true;
    }
    double ab0y = a[0].y - b[0].y;
    double ab0x = a[0].x - b[0].x;
    double numerA = ab0y * bxLen - byLen * ab0x;
    double numerB;
    if (numerA < 0 && denom > numerA || numerA > 0 && denom < numerA) {
        return no_intersection(result);
    }
    numerB = ab0y * axLen - ayLen * ab0x;
    if (numerB < 0 && denom > numerB || numerB > 0 && denom < numerB) {
        return no_intersection(result);
    }
    /* Are the line coincident? See if they overlap */
    // FIXME: allow returning range of coincidence, instead of or in
    // addition to midpoint
    paramsA.lineEndPoints(a);
    double b0dist = paramsA.pointDistance(b[0]);
    bool b0on = approximately_zero_squared(b0dist);
    double b1dist = paramsA.pointDistance(b[1]);
    bool b1on = approximately_zero_squared(b1dist);
    if (b0on | b1on) {
        if (b0on & b1on) {
            result->x = (b[0].x + b[1].x) / 2;
            result->y = (b[0].y + b[1].y) / 2;
            return true;
        }
        paramsB.lineEndPoints(b);
        double a0dist = paramsB.pointDistance(a[0]);
        bool a0on = approximately_zero_squared(a0dist);
        double a1dist = paramsB.pointDistance(a[1]);
        bool a1on = approximately_zero_squared(a1dist);
        if (a0on | a1on) {
            if (a0on & a1on) {
                result->x = (a[0].x + a[1].x) / 2;
                result->y = (a[0].y + a[1].y) / 2;
                return true;
            }
            const _Point& aPt = a0on ? a[0] : a[1];
            const _Point& bPt = b0on ? b[0] : b[1];
            if (aPt.approximatelyEqual(bPt)) {
                *result = aPt;
                return true;
            }
            result->x = (aPt.x + bPt.x) / 2;
            result->y = (aPt.y + bPt.y) / 2;
            return true;
        }
    }

    /* Is the intersection along the the segments */
    double mua = numerA / denom;
    result->x = a[0].x + mua * (a[1].x - a[0].x);
    result->y = a[0].y + mua * (a[1].y - a[0].y);
    return true;
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
