/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkReduceOrder.h"

int SkReduceOrder::reduce(const SkDLine& line) {
    fLine[0] = line[0];
    int different = line[0] != line[1];
    fLine[1] = line[different];
    return 1 + different;
}

static double interp_quad_coords(double a, double b, double c, double t) {
    double ab = SkDInterp(a, b, t);
    double bc = SkDInterp(b, c, t);
    return SkDInterp(ab, bc, t);
}

static int coincident_line(const SkDQuad& quad, SkDQuad& reduction) {
    reduction[0] = reduction[1] = quad[0];
    return 1;
}

static int reductionLineCount(const SkDQuad& reduction) {
    return 1 + !reduction[0].approximatelyEqual(reduction[1]);
}

static int vertical_line(const SkDQuad& quad, SkReduceOrder::Style reduceStyle,
        SkDQuad& reduction) {
    double tValue;
    reduction[0] = quad[0];
    reduction[1] = quad[2];
    if (reduceStyle == SkReduceOrder::kFill_Style) {
        return reductionLineCount(reduction);
    }
    int smaller = reduction[1].fY > reduction[0].fY;
    int larger = smaller ^ 1;
    if (SkDQuad::FindExtrema(quad[0].fY, quad[1].fY, quad[2].fY, &tValue)) {
        double yExtrema = interp_quad_coords(quad[0].fY, quad[1].fY, quad[2].fY, tValue);
        if (reduction[smaller].fY > yExtrema) {
            reduction[smaller].fY = yExtrema;
        } else if (reduction[larger].fY < yExtrema) {
            reduction[larger].fY = yExtrema;
        }
    }
    return reductionLineCount(reduction);
}

static int horizontal_line(const SkDQuad& quad, SkReduceOrder::Style reduceStyle,
        SkDQuad& reduction) {
    double tValue;
    reduction[0] = quad[0];
    reduction[1] = quad[2];
    if (reduceStyle == SkReduceOrder::kFill_Style) {
        return reductionLineCount(reduction);
    }
    int smaller = reduction[1].fX > reduction[0].fX;
    int larger = smaller ^ 1;
    if (SkDQuad::FindExtrema(quad[0].fX, quad[1].fX, quad[2].fX, &tValue)) {
        double xExtrema = interp_quad_coords(quad[0].fX, quad[1].fX, quad[2].fX, tValue);
        if (reduction[smaller].fX > xExtrema) {
            reduction[smaller].fX = xExtrema;
        }  else if (reduction[larger].fX < xExtrema) {
            reduction[larger].fX = xExtrema;
        }
    }
    return reductionLineCount(reduction);
}

static int check_linear(const SkDQuad& quad, SkReduceOrder::Style reduceStyle,
        int minX, int maxX, int minY, int maxY, SkDQuad& reduction) {
    int startIndex = 0;
    int endIndex = 2;
    while (quad[startIndex].approximatelyEqual(quad[endIndex])) {
        --endIndex;
        if (endIndex == 0) {
            SkDebugf("%s shouldn't get here if all four points are about equal", __FUNCTION__);
            SkASSERT(0);
        }
    }
    if (!quad.isLinear(startIndex, endIndex)) {
        return 0;
    }
    // four are colinear: return line formed by outside
    reduction[0] = quad[0];
    reduction[1] = quad[2];
    if (reduceStyle == SkReduceOrder::kFill_Style) {
        return reductionLineCount(reduction);
    }
    int sameSide;
    bool useX = quad[maxX].fX - quad[minX].fX >= quad[maxY].fY - quad[minY].fY;
    if (useX) {
        sameSide = SkDSign(quad[0].fX - quad[1].fX) + SkDSign(quad[2].fX - quad[1].fX);
    } else {
        sameSide = SkDSign(quad[0].fY - quad[1].fY) + SkDSign(quad[2].fY - quad[1].fY);
    }
    if ((sameSide & 3) != 2) {
        return reductionLineCount(reduction);
    }
    double tValue;
    int root;
    if (useX) {
        root = SkDQuad::FindExtrema(quad[0].fX, quad[1].fX, quad[2].fX, &tValue);
    } else {
        root = SkDQuad::FindExtrema(quad[0].fY, quad[1].fY, quad[2].fY, &tValue);
    }
    if (root) {
        SkDPoint extrema;
        extrema.fX = interp_quad_coords(quad[0].fX, quad[1].fX, quad[2].fX, tValue);
        extrema.fY = interp_quad_coords(quad[0].fY, quad[1].fY, quad[2].fY, tValue);
        // sameSide > 0 means mid is smaller than either [0] or [2], so replace smaller
        int replace;
        if (useX) {
            if ((extrema.fX < quad[0].fX) ^ (extrema.fX < quad[2].fX)) {
                return reductionLineCount(reduction);
            }
            replace = ((extrema.fX < quad[0].fX) | (extrema.fX < quad[2].fX))
                    ^ (quad[0].fX < quad[2].fX);
        } else {
            if ((extrema.fY < quad[0].fY) ^ (extrema.fY < quad[2].fY)) {
                return reductionLineCount(reduction);
            }
            replace = ((extrema.fY < quad[0].fY) | (extrema.fY < quad[2].fY))
                    ^ (quad[0].fY < quad[2].fY);
        }
        reduction[replace] = extrema;
    }
    return reductionLineCount(reduction);
}

// reduce to a quadratic or smaller
// look for identical points
// look for all four points in a line
    // note that three points in a line doesn't simplify a cubic
// look for approximation with single quadratic
    // save approximation with multiple quadratics for later
int SkReduceOrder::reduce(const SkDQuad& quad, Style reduceStyle) {
    int index, minX, maxX, minY, maxY;
    int minXSet, minYSet;
    minX = maxX = minY = maxY = 0;
    minXSet = minYSet = 0;
    for (index = 1; index < 3; ++index) {
        if (quad[minX].fX > quad[index].fX) {
            minX = index;
        }
        if (quad[minY].fY > quad[index].fY) {
            minY = index;
        }
        if (quad[maxX].fX < quad[index].fX) {
            maxX = index;
        }
        if (quad[maxY].fY < quad[index].fY) {
            maxY = index;
        }
    }
    for (index = 0; index < 3; ++index) {
        if (AlmostEqualUlps(quad[index].fX, quad[minX].fX)) {
            minXSet |= 1 << index;
        }
        if (AlmostEqualUlps(quad[index].fY, quad[minY].fY)) {
            minYSet |= 1 << index;
        }
    }
    if (minXSet == 0x7) {  // test for vertical line
        if (minYSet == 0x7) {  // return 1 if all four are coincident
            return coincident_line(quad, fQuad);
        }
        return vertical_line(quad, reduceStyle, fQuad);
    }
    if (minYSet == 0xF) {  // test for horizontal line
        return horizontal_line(quad, reduceStyle, fQuad);
    }
    int result = check_linear(quad, reduceStyle, minX, maxX, minY, maxY, fQuad);
    if (result) {
        return result;
    }
    fQuad = quad;
    return 3;
}

////////////////////////////////////////////////////////////////////////////////////

static double interp_cubic_coords(const double* src, double t) {
    double ab = SkDInterp(src[0], src[2], t);
    double bc = SkDInterp(src[2], src[4], t);
    double cd = SkDInterp(src[4], src[6], t);
    double abc = SkDInterp(ab, bc, t);
    double bcd = SkDInterp(bc, cd, t);
    return SkDInterp(abc, bcd, t);
}

static int coincident_line(const SkDCubic& cubic, SkDCubic& reduction) {
    reduction[0] = reduction[1] = cubic[0];
    return 1;
}

static int reductionLineCount(const SkDCubic& reduction) {
    return 1 + !reduction[0].approximatelyEqual(reduction[1]);
}

static int vertical_line(const SkDCubic& cubic, SkReduceOrder::Style reduceStyle,
                         SkDCubic& reduction) {
    double tValues[2];
    reduction[0] = cubic[0];
    reduction[1] = cubic[3];
    if (reduceStyle == SkReduceOrder::kFill_Style) {
        return reductionLineCount(reduction);
    }
    int smaller = reduction[1].fY > reduction[0].fY;
    int larger = smaller ^ 1;
    int roots = SkDCubic::FindExtrema(cubic[0].fY, cubic[1].fY, cubic[2].fY, cubic[3].fY, tValues);
    for (int index = 0; index < roots; ++index) {
        double yExtrema = interp_cubic_coords(&cubic[0].fY, tValues[index]);
        if (reduction[smaller].fY > yExtrema) {
            reduction[smaller].fY = yExtrema;
            continue;
        }
        if (reduction[larger].fY < yExtrema) {
            reduction[larger].fY = yExtrema;
        }
    }
    return reductionLineCount(reduction);
}

static int horizontal_line(const SkDCubic& cubic, SkReduceOrder::Style reduceStyle,
                           SkDCubic& reduction) {
    double tValues[2];
    reduction[0] = cubic[0];
    reduction[1] = cubic[3];
    if (reduceStyle == SkReduceOrder::kFill_Style) {
        return reductionLineCount(reduction);
    }
    int smaller = reduction[1].fX > reduction[0].fX;
    int larger = smaller ^ 1;
    int roots = SkDCubic::FindExtrema(cubic[0].fX, cubic[1].fX, cubic[2].fX, cubic[3].fX, tValues);
    for (int index = 0; index < roots; ++index) {
        double xExtrema = interp_cubic_coords(&cubic[0].fX, tValues[index]);
        if (reduction[smaller].fX > xExtrema) {
            reduction[smaller].fX = xExtrema;
            continue;
        }
        if (reduction[larger].fX < xExtrema) {
            reduction[larger].fX = xExtrema;
        }
    }
    return reductionLineCount(reduction);
}

// check to see if it is a quadratic or a line
static int check_quadratic(const SkDCubic& cubic, SkDCubic& reduction) {
    double dx10 = cubic[1].fX - cubic[0].fX;
    double dx23 = cubic[2].fX - cubic[3].fX;
    double midX = cubic[0].fX + dx10 * 3 / 2;
    double sideAx = midX - cubic[3].fX;
    double sideBx = dx23 * 3 / 2;
    if (approximately_zero(sideAx) ? !approximately_equal(sideAx, sideBx)
            : !AlmostEqualUlps(sideAx, sideBx)) {
        return 0;
    }
    double dy10 = cubic[1].fY - cubic[0].fY;
    double dy23 = cubic[2].fY - cubic[3].fY;
    double midY = cubic[0].fY + dy10 * 3 / 2;
    double sideAy = midY - cubic[3].fY;
    double sideBy = dy23 * 3 / 2;
    if (approximately_zero(sideAy) ? !approximately_equal(sideAy, sideBy)
            : !AlmostEqualUlps(sideAy, sideBy)) {
        return 0;
    }
    reduction[0] = cubic[0];
    reduction[1].fX = midX;
    reduction[1].fY = midY;
    reduction[2] = cubic[3];
    return 3;
}

static int check_linear(const SkDCubic& cubic, SkReduceOrder::Style reduceStyle,
        int minX, int maxX, int minY, int maxY, SkDCubic& reduction) {
    int startIndex = 0;
    int endIndex = 3;
    while (cubic[startIndex].approximatelyEqual(cubic[endIndex])) {
        --endIndex;
        if (endIndex == 0) {
            SkDebugf("%s shouldn't get here if all four points are about equal\n", __FUNCTION__);
            SkASSERT(0);
        }
    }
    if (!cubic.isLinear(startIndex, endIndex)) {
        return 0;
    }
    // four are colinear: return line formed by outside
    reduction[0] = cubic[0];
    reduction[1] = cubic[3];
    if (reduceStyle == SkReduceOrder::kFill_Style) {
        return reductionLineCount(reduction);
    }
    int sameSide1;
    int sameSide2;
    bool useX = cubic[maxX].fX - cubic[minX].fX >= cubic[maxY].fY - cubic[minY].fY;
    if (useX) {
        sameSide1 = SkDSign(cubic[0].fX - cubic[1].fX) + SkDSign(cubic[3].fX - cubic[1].fX);
        sameSide2 = SkDSign(cubic[0].fX - cubic[2].fX) + SkDSign(cubic[3].fX - cubic[2].fX);
    } else {
        sameSide1 = SkDSign(cubic[0].fY - cubic[1].fY) + SkDSign(cubic[3].fY - cubic[1].fY);
        sameSide2 = SkDSign(cubic[0].fY - cubic[2].fY) + SkDSign(cubic[3].fY - cubic[2].fY);
    }
    if (sameSide1 == sameSide2 && (sameSide1 & 3) != 2) {
        return reductionLineCount(reduction);
    }
    double tValues[2];
    int roots;
    if (useX) {
        roots = SkDCubic::FindExtrema(cubic[0].fX, cubic[1].fX, cubic[2].fX, cubic[3].fX, tValues);
    } else {
        roots = SkDCubic::FindExtrema(cubic[0].fY, cubic[1].fY, cubic[2].fY, cubic[3].fY, tValues);
    }
    for (int index = 0; index < roots; ++index) {
        SkDPoint extrema;
        extrema.fX = interp_cubic_coords(&cubic[0].fX, tValues[index]);
        extrema.fY = interp_cubic_coords(&cubic[0].fY, tValues[index]);
        // sameSide > 0 means mid is smaller than either [0] or [3], so replace smaller
        int replace;
        if (useX) {
            if ((extrema.fX < cubic[0].fX) ^ (extrema.fX < cubic[3].fX)) {
                continue;
            }
            replace = ((extrema.fX < cubic[0].fX) | (extrema.fX < cubic[3].fX))
                    ^ (cubic[0].fX < cubic[3].fX);
        } else {
            if ((extrema.fY < cubic[0].fY) ^ (extrema.fY < cubic[3].fY)) {
                continue;
            }
            replace = ((extrema.fY < cubic[0].fY) | (extrema.fY < cubic[3].fY))
                    ^ (cubic[0].fY < cubic[3].fY);
        }
        reduction[replace] = extrema;
    }
    return reductionLineCount(reduction);
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
int SkReduceOrder::reduce(const SkDCubic& cubic, Quadratics allowQuadratics,
        Style reduceStyle) {
    int index, minX, maxX, minY, maxY;
    int minXSet, minYSet;
    minX = maxX = minY = maxY = 0;
    minXSet = minYSet = 0;
    for (index = 1; index < 4; ++index) {
        if (cubic[minX].fX > cubic[index].fX) {
            minX = index;
        }
        if (cubic[minY].fY > cubic[index].fY) {
            minY = index;
        }
        if (cubic[maxX].fX < cubic[index].fX) {
            maxX = index;
        }
        if (cubic[maxY].fY < cubic[index].fY) {
            maxY = index;
        }
    }
    for (index = 0; index < 4; ++index) {
        double cx = cubic[index].fX;
        double cy = cubic[index].fY;
        double denom = SkTMax(fabs(cx), SkTMax(fabs(cy),
                SkTMax(fabs(cubic[minX].fX), fabs(cubic[minY].fY))));
        if (denom == 0) {
            minXSet |= 1 << index;
            minYSet |= 1 << index;
            continue;
        }
        double inv = 1 / denom;
        if (approximately_equal_half(cx * inv, cubic[minX].fX * inv)) {
            minXSet |= 1 << index;
        }
        if (approximately_equal_half(cy * inv, cubic[minY].fY * inv)) {
            minYSet |= 1 << index;
        }
    }
    if (minXSet == 0xF) {  // test for vertical line
        if (minYSet == 0xF) {  // return 1 if all four are coincident
            return coincident_line(cubic, fCubic);
        }
        return vertical_line(cubic, reduceStyle, fCubic);
    }
    if (minYSet == 0xF) {  // test for horizontal line
        return horizontal_line(cubic, reduceStyle, fCubic);
    }
    int result = check_linear(cubic, reduceStyle, minX, maxX, minY, maxY, fCubic);
    if (result) {
        return result;
    }
    if (allowQuadratics == SkReduceOrder::kAllow_Quadratics
            && (result = check_quadratic(cubic, fCubic))) {
        return result;
    }
    fCubic = cubic;
    return 4;
}

SkPath::Verb SkReduceOrder::Quad(const SkPoint a[3], SkPoint* reducePts) {
    SkDQuad quad;
    quad.set(a);
    SkReduceOrder reducer;
    int order = reducer.reduce(quad, kFill_Style);
    if (order == 2) {  // quad became line
        for (int index = 0; index < order; ++index) {
            *reducePts++ = reducer.fLine[index].asSkPoint();
        }
    }
    return SkPathOpsPointsToVerb(order - 1);
}

SkPath::Verb SkReduceOrder::Cubic(const SkPoint a[4], SkPoint* reducePts) {
    SkDCubic cubic;
    cubic.set(a);
    SkReduceOrder reducer;
    int order = reducer.reduce(cubic, kAllow_Quadratics, kFill_Style);
    if (order == 2 || order == 3) {  // cubic became line or quad
        for (int index = 0; index < order; ++index) {
            *reducePts++ = reducer.fQuad[index].asSkPoint();
        }
    }
    return SkPathOpsPointsToVerb(order - 1);
}
