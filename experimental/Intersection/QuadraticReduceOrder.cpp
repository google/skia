#include "CurveIntersection.h"
#include "Extrema.h"
#include "IntersectionUtilities.h"
#include "LineParameters.h"

static double interp_quad_coords(double a, double b, double c, double t)
{
    double ab = interp(a, b, t);
    double bc = interp(b, c, t);
    return interp(ab, bc, t);
}

static int coincident_line(const Quadratic& quad, Quadratic& reduction) {
    reduction[0] = reduction[1] = quad[0];
    return 1;
}

static int vertical_line(const Quadratic& quad, Quadratic& reduction) {
    double tValue;
    reduction[0] = quad[0];
    reduction[1] = quad[2];
    int smaller = reduction[1].y > reduction[0].y;
    int larger = smaller ^ 1;
    if (SkFindQuadExtrema(quad[0].y, quad[1].y, quad[2].y, &tValue)) {
        double yExtrema = interp_quad_coords(quad[0].y, quad[1].y, quad[2].y, tValue);
        if (reduction[smaller].y > yExtrema) {
            reduction[smaller].y = yExtrema;
        } else if (reduction[larger].y < yExtrema) {
            reduction[larger].y = yExtrema;
        }
    }
    return 2;
}

static int horizontal_line(const Quadratic& quad, Quadratic& reduction) {
    double tValue;
    reduction[0] = quad[0];
    reduction[1] = quad[2];
    int smaller = reduction[1].x > reduction[0].x;
    int larger = smaller ^ 1;
    if (SkFindQuadExtrema(quad[0].x, quad[1].x, quad[2].x, &tValue)) {
        double xExtrema = interp_quad_coords(quad[0].x, quad[1].x, quad[2].x, tValue);
        if (reduction[smaller].x > xExtrema) {
            reduction[smaller].x = xExtrema;
        }  else if (reduction[larger].x < xExtrema) {
            reduction[larger].x = xExtrema;
        }
    }
    return 2;
}

static int check_linear(const Quadratic& quad, Quadratic& reduction,
        int minX, int maxX, int minY, int maxY) {
    int startIndex = 0;
    int endIndex = 2;
    while (quad[startIndex].approximatelyEqual(quad[endIndex])) {
        --endIndex;
        if (endIndex == 0) {
            printf("%s shouldn't get here if all four points are about equal", __FUNCTION__);
            assert(0);
        }
    }
    LineParameters lineParameters;
    lineParameters.quadEndPoints(quad, startIndex, endIndex);
    double normalSquared = lineParameters.normalSquared();
    double distance = lineParameters.controlPtDistance(quad); // not normalized
    double limit = normalSquared * SquaredEpsilon;
    double distSq = distance * distance;
    if (distSq > limit) {
        return 0;
    }
    // four are colinear: return line formed by outside
    reduction[0] = quad[0];
    reduction[1] = quad[2];
    int sameSide;
    bool useX = quad[maxX].x - quad[minX].x >= quad[maxY].y - quad[minY].y;
    if (useX) {
        sameSide = sign(quad[0].x - quad[1].x) + sign(quad[2].x - quad[1].x);
    } else {
        sameSide = sign(quad[0].y - quad[1].y) + sign(quad[2].y - quad[1].y);
    }
    if ((sameSide & 3) != 2) {
        return 2;
    }
    double tValue;
    int root;
    if (useX) {
        root = SkFindQuadExtrema(quad[0].x, quad[1].x, quad[2].x, &tValue);
    } else {
        root = SkFindQuadExtrema(quad[0].y, quad[1].y, quad[2].y, &tValue);
    }
    if (root) {
        _Point extrema;
        extrema.x = interp_quad_coords(quad[0].x, quad[1].x, quad[2].x, tValue);
        extrema.y = interp_quad_coords(quad[0].x, quad[1].x, quad[2].x, tValue);
        // sameSide > 0 means mid is smaller than either [0] or [2], so replace smaller
        int replace;
        if (useX) {
            if (extrema.x < quad[0].x ^ extrema.x < quad[2].x) {
                return 2;
            }
            replace = (extrema.x < quad[0].x | extrema.x < quad[2].x)
                    ^ quad[0].x < quad[2].x;
        } else {
            if (extrema.y < quad[0].y ^ extrema.y < quad[2].y) {
                return 2;
            }
            replace = (extrema.y < quad[0].y | extrema.y < quad[2].y)
                    ^ quad[0].y < quad[2].y;
        }
        reduction[replace] = extrema;
    }
    return 2;
}

// reduce to a quadratic or smaller
// look for identical points
// look for all four points in a line 
    // note that three points in a line doesn't simplify a cubic
// look for approximation with single quadratic
    // save approximation with multiple quadratics for later
int reduceOrder(const Quadratic& quad, Quadratic& reduction) {
    int index, minX, maxX, minY, maxY;
    int minXSet, minYSet;
    minX = maxX = minY = maxY = 0;
    minXSet = minYSet = 0;
    for (index = 1; index < 3; ++index) {
        if (quad[minX].x > quad[index].x) {
            minX = index;
        }
        if (quad[minY].y > quad[index].y) {
            minY = index;
        }
        if (quad[maxX].x < quad[index].x) {
            maxX = index;
        }
        if (quad[maxY].y < quad[index].y) {
            maxY = index;
        }
    }
    for (index = 0; index < 3; ++index) {
        if (approximately_equal(quad[index].x, quad[minX].x)) {
            minXSet |= 1 << index;
        }
        if (approximately_equal(quad[index].y, quad[minY].y)) {
            minYSet |= 1 << index;
        }
    }
    if (minXSet == 0xF) { // test for vertical line
        if (minYSet == 0xF) { // return 1 if all four are coincident
            return coincident_line(quad, reduction);
        }
        return vertical_line(quad, reduction);
    }
    if (minYSet == 0xF) { // test for horizontal line
        return horizontal_line(quad, reduction);
    }
    int result = check_linear(quad, reduction, minX, maxX, minY, maxY);
    if (result) {
        return result;
    }
    memcpy(reduction, quad, sizeof(Quadratic));
    return 3;
}
