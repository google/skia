#include "CurveIntersection.h"
#include "Extrema.h"
#include "IntersectionUtilities.h"
#include "LineParameters.h"

static double interp_cubic_coords(const double* src, double t)
{
    double ab = interp(src[0], src[2], t);
    double bc = interp(src[2], src[4], t);
    double cd = interp(src[4], src[6], t);
    double abc = interp(ab, bc, t);
    double bcd = interp(bc, cd, t);
    return interp(abc, bcd, t);
}

static int coincident_line(const Cubic& cubic, Cubic& reduction) {
    reduction[0] = reduction[1] = cubic[0];
    return 1;
}

static int vertical_line(const Cubic& cubic, Cubic& reduction) {
    double tValues[2];
    reduction[0] = cubic[0];
    reduction[1] = cubic[3];
    int smaller = reduction[1].y > reduction[0].y;
    int larger = smaller ^ 1;
    int roots = findExtrema(cubic[0].y, cubic[1].y, cubic[2].y, cubic[3].y, tValues);
    for (int index = 0; index < roots; ++index) {
        double yExtrema = interp_cubic_coords(&cubic[0].y, tValues[index]);
        if (reduction[smaller].y > yExtrema) {
            reduction[smaller].y = yExtrema;
            continue;
        } 
        if (reduction[larger].y < yExtrema) {
            reduction[larger].y = yExtrema;
        }
    }
    return 2;
}

static int horizontal_line(const Cubic& cubic, Cubic& reduction) {
    double tValues[2];
    reduction[0] = cubic[0];
    reduction[1] = cubic[3];
    int smaller = reduction[1].x > reduction[0].x;
    int larger = smaller ^ 1;
    int roots = findExtrema(cubic[0].x, cubic[1].x, cubic[2].x, cubic[3].x, tValues);
    for (int index = 0; index < roots; ++index) {
        double xExtrema = interp_cubic_coords(&cubic[0].x, tValues[index]);
        if (reduction[smaller].x > xExtrema) {
            reduction[smaller].x = xExtrema;
            continue;
        } 
        if (reduction[larger].x < xExtrema) {
            reduction[larger].x = xExtrema;
        }
    }
    return 2;
}

// check to see if it is a quadratic or a line
static int check_quadratic(const Cubic& cubic, Cubic& reduction) {
    double dx10 = cubic[1].x - cubic[0].x;
    double dx23 = cubic[2].x - cubic[3].x;
    double midX = cubic[0].x + dx10 * 3 / 2;
    if (!approximately_equal(midX - cubic[3].x, dx23 * 3 / 2)) {
        return 0;
    }
    double dy10 = cubic[1].y - cubic[0].y;
    double dy23 = cubic[2].y - cubic[3].y;
    double midY = cubic[0].y + dy10 * 3 / 2;
    if (!approximately_equal(midY - cubic[3].y, dy23 * 3 / 2)) {
        return 0;
    }
    reduction[0] = cubic[0];
    reduction[1].x = midX;
    reduction[1].y = midY;
    reduction[2] = cubic[3];
    return 3;
}

static int check_linear(const Cubic& cubic, Cubic& reduction,
        int minX, int maxX, int minY, int maxY) {
    int startIndex = 0;
    int endIndex = 3;
    while (cubic[startIndex].approximatelyEqual(cubic[endIndex])) {
        --endIndex;
        if (endIndex == 0) {
            printf("%s shouldn't get here if all four points are about equal", __FUNCTION__);
            assert(0);
        }
    }
    if (!isLinear(cubic, startIndex, endIndex)) {
        return 0;
    }
    // four are colinear: return line formed by outside
    reduction[0] = cubic[0];
    reduction[1] = cubic[3];
    int sameSide1;
    int sameSide2;
    bool useX = cubic[maxX].x - cubic[minX].x >= cubic[maxY].y - cubic[minY].y;
    if (useX) {
        sameSide1 = sign(cubic[0].x - cubic[1].x) + sign(cubic[3].x - cubic[1].x);
        sameSide2 = sign(cubic[0].x - cubic[2].x) + sign(cubic[3].x - cubic[2].x);
    } else {
        sameSide1 = sign(cubic[0].y - cubic[1].y) + sign(cubic[3].y - cubic[1].y);
        sameSide2 = sign(cubic[0].y - cubic[2].y) + sign(cubic[3].y - cubic[2].y);
    }
    if (sameSide1 == sameSide2 && (sameSide1 & 3) != 2) {
        return 2;
    }
    double tValues[2];
    int roots;
    if (useX) {
        roots = findExtrema(cubic[0].x, cubic[1].x, cubic[2].x, cubic[3].x, tValues);
    } else {
        roots = findExtrema(cubic[0].y, cubic[1].y, cubic[2].y, cubic[3].y, tValues);
    }
    for (int index = 0; index < roots; ++index) {
        _Point extrema;
        extrema.x = interp_cubic_coords(&cubic[0].x, tValues[index]);
        extrema.y = interp_cubic_coords(&cubic[0].y, tValues[index]);
        // sameSide > 0 means mid is smaller than either [0] or [3], so replace smaller
        int replace;
        if (useX) {
            if (extrema.x < cubic[0].x ^ extrema.x < cubic[3].x) {
                continue;
            }
            replace = (extrema.x < cubic[0].x | extrema.x < cubic[3].x)
                    ^ cubic[0].x < cubic[3].x;
        } else {
            if (extrema.y < cubic[0].y ^ extrema.y < cubic[3].y) {
                continue;
            }
            replace = (extrema.y < cubic[0].y | extrema.y < cubic[3].y)
                    ^ cubic[0].y < cubic[3].y;
        }
        reduction[replace] = extrema;
    }
    return 2;
}

bool isLinear(const Cubic& cubic, int startIndex, int endIndex) {
    LineParameters lineParameters;
    lineParameters.cubicEndPoints(cubic, startIndex, endIndex);
    double normalSquared = lineParameters.normalSquared();
    double distance[2]; // distance is not normalized
    int mask = other_two(startIndex, endIndex);
    int inner1 = startIndex ^ mask;
    int inner2 = endIndex ^ mask;
    lineParameters.controlPtDistance(cubic, inner1, inner2, distance);
    double limit = normalSquared;
    int index;
    for (index = 0; index < 2; ++index) {
        double distSq = distance[index];
        distSq *= distSq;
        if (approximately_greater(distSq, limit)) {
            return false;
        }
    }
    return true;
}

/* food for thought:
http://objectmix.com/graphics/132906-fast-precision-driven-cubic-quadratic-piecewise-degree-reduction-algos-2-a.html

Given points c1, c2, c3 and c4 of a cubic Bezier, the points of the
corresponding quadratic Bezier are (given in convex combinations of
points):

q1 = (11/13)c1 + (3/13)c2 -(3/13)c3 + (2/13)c4
q2 = -c1 + (3/2)c2 + (3/2)c3 - c4
q3 = (2/13)c1 - (3/13)c2 + (3/13)c3 + (11/13)c4

Of course, this curve does not interpolate the end-points, but it would
be interesting to see the behaviour of such a curve in an applet.

--
Kalle Rutanen
http://kaba.hilvi.org

*/

// reduce to a quadratic or smaller
// look for identical points
// look for all four points in a line 
    // note that three points in a line doesn't simplify a cubic
// look for approximation with single quadratic
    // save approximation with multiple quadratics for later
int reduceOrder(const Cubic& cubic, Cubic& reduction, ReduceOrder_Flags allowQuadratics) {
    int index, minX, maxX, minY, maxY;
    int minXSet, minYSet;
    minX = maxX = minY = maxY = 0;
    minXSet = minYSet = 0;
    for (index = 1; index < 4; ++index) {
        if (cubic[minX].x > cubic[index].x) {
            minX = index;
        }
        if (cubic[minY].y > cubic[index].y) {
            minY = index;
        }
        if (cubic[maxX].x < cubic[index].x) {
            maxX = index;
        }
        if (cubic[maxY].y < cubic[index].y) {
            maxY = index;
        }
    }
    for (index = 0; index < 4; ++index) {
        if (approximately_equal(cubic[index].x, cubic[minX].x)) {
            minXSet |= 1 << index;
        }
        if (approximately_equal(cubic[index].y, cubic[minY].y)) {
            minYSet |= 1 << index;
        }
    }
    if (minXSet == 0xF) { // test for vertical line
        if (minYSet == 0xF) { // return 1 if all four are coincident
            return coincident_line(cubic, reduction);
        }
        return vertical_line(cubic, reduction);
    }
    if (minYSet == 0xF) { // test for horizontal line
        return horizontal_line(cubic, reduction);
    }
    int result = check_linear(cubic, reduction, minX, maxX, minY, maxY);
    if (result) {
        return result;
    }
    if (allowQuadratics && (result = check_quadratic(cubic, reduction))) {
        return result;
    }
    memcpy(reduction, cubic, sizeof(Cubic));
    return 4;
}
