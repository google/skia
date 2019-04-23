/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/utils/SkRandom.h"
#include "src/pathops/SkIntersections.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "src/pathops/SkPathOpsLine.h"
#include "src/pathops/SkPathOpsQuad.h"
#include "src/pathops/SkReduceOrder.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

static bool gPathOpsCubicLineIntersectionIdeasVerbose = false;

static struct CubicLineFailures {
    CubicPts c;
    double t;
    SkDPoint p;
} cubicLineFailures[] = {
    {{{{-164.3726806640625, 36.826904296875}, {-189.045166015625, -953.2220458984375},
        {926.505859375, -897.36175537109375}, {-139.33489990234375, 204.40771484375}}},
        0.37329583, {107.54935269006289, -632.13736293162208}},
    {{{{784.056884765625, -554.8350830078125}, {67.5489501953125, 509.0224609375},
        {-447.713134765625, 751.375}, {415.7784423828125, 172.22265625}}},
        0.660005242, {-32.973148967736151, 478.01341797403569}},
    {{{{-580.6834716796875, -127.044921875}, {-872.8983154296875, -945.54302978515625},
        {260.8092041015625, -909.34991455078125}, {-976.2125244140625, -18.46551513671875}}},
        0.578826774, {-390.17910153915489, -687.21144412296007}},
};

int cubicLineFailuresCount = (int) SK_ARRAY_COUNT(cubicLineFailures);

double measuredSteps[] = {
    9.15910731e-007, 8.6600277e-007, 7.4122059e-007, 6.92087618e-007, 8.35290245e-007,
    3.29763199e-007, 5.07547773e-007, 4.41294224e-007, 0, 0,
    3.76879167e-006, 1.06126249e-006, 2.36873967e-006, 1.62421134e-005, 3.09103599e-005,
    4.38917976e-005, 0.000112348938, 0.000243149242, 0.000433174114, 0.00170880232,
    0.00272619724, 0.00518844604, 0.000352621078, 0.00175960064, 0.027875185,
    0.0351329803, 0.103964925,
};

/* last output : errors=3121
    9.1796875e-007 8.59375e-007 7.5e-007 6.875e-007 8.4375e-007
    3.125e-007 5e-007 4.375e-007 0 0
    3.75e-006 1.09375e-006 2.1875e-006 1.640625e-005 3.0859375e-005
    4.38964844e-005 0.000112304687 0.000243164063 0.000433181763 0.00170898437
    0.00272619247 0.00518844604 0.000352621078 0.00175960064 0.027875185
    0.0351329803 0.103964925
*/

static double binary_search(const SkDCubic& cubic, double step, const SkDPoint& pt, double t,
        int* iters) {
    double firstStep = step;
    do {
        *iters += 1;
        SkDPoint cubicAtT = cubic.ptAtT(t);
        if (cubicAtT.approximatelyEqual(pt)) {
            break;
        }
        double calcX = cubicAtT.fX - pt.fX;
        double calcY = cubicAtT.fY - pt.fY;
        double calcDist = calcX * calcX + calcY * calcY;
        if (step == 0) {
            SkDebugf("binary search failed: step=%1.9g cubic=", firstStep);
            cubic.dump();
            SkDebugf(" t=%1.9g ", t);
            pt.dump();
            SkDebugf("\n");
            return -1;
        }
        double lastStep = step;
        step /= 2;
        SkDPoint lessPt = cubic.ptAtT(t - lastStep);
        double lessX = lessPt.fX - pt.fX;
        double lessY = lessPt.fY - pt.fY;
        double lessDist = lessX * lessX + lessY * lessY;
        // use larger x/y difference to choose step
        if (calcDist > lessDist) {
            t -= step;
            t = SkTMax(0., t);
        } else {
            SkDPoint morePt = cubic.ptAtT(t + lastStep);
            double moreX = morePt.fX - pt.fX;
            double moreY = morePt.fY - pt.fY;
            double moreDist = moreX * moreX + moreY * moreY;
            if (calcDist <= moreDist) {
                continue;
            }
            t += step;
            t = SkTMin(1., t);
        }
    } while (true);
    return t;
}

#if 0
static bool r2check(double A, double B, double C, double D, double* R2MinusQ3Ptr) {
    if (approximately_zero(A)
            && approximately_zero_when_compared_to(A, B)
            && approximately_zero_when_compared_to(A, C)
            && approximately_zero_when_compared_to(A, D)) {  // we're just a quadratic
        return false;
    }
    if (approximately_zero_when_compared_to(D, A)
            && approximately_zero_when_compared_to(D, B)
            && approximately_zero_when_compared_to(D, C)) {  // 0 is one root
        return false;
    }
    if (approximately_zero(A + B + C + D)) {  // 1 is one root
        return false;
    }
    double a, b, c;
    {
        double invA = 1 / A;
        a = B * invA;
        b = C * invA;
        c = D * invA;
    }
    double a2 = a * a;
    double Q = (a2 - b * 3) / 9;
    double R = (2 * a2 * a - 9 * a * b + 27 * c) / 54;
    double R2 = R * R;
    double Q3 = Q * Q * Q;
    double R2MinusQ3 = R2 - Q3;
    *R2MinusQ3Ptr = R2MinusQ3;
    return true;
}
#endif

/* What is the relationship between the accuracy of the root in range and the magnitude of all
   roots? To find out, create a bunch of cubics, and measure */

DEF_TEST(PathOpsCubicLineRoots, reporter) {
    if (!gPathOpsCubicLineIntersectionIdeasVerbose) {  // slow; exclude it by default
        return;
    }
    SkRandom ran;
    double worstStep[256] = {0};
    int errors = 0;
    int iters = 0;
    double smallestR2 = 0;
    double largestR2 = 0;
    for (int index = 0; index < 1000000000; ++index) {
        SkDPoint origin = {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)};
        CubicPts cuPts = {{origin,
                {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)},
                {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)},
                {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)}
        }};
        // construct a line at a known intersection
        double t = ran.nextRangeF(0, 1);
        SkDCubic cubic;
        cubic.debugSet(cuPts.fPts);
        SkDPoint pt = cubic.ptAtT(t);
        // skip answers with no intersections (although note the bug!) or two, or more
        // see if the line / cubic has a fun range of roots
        double A, B, C, D;
        SkDCubic::Coefficients(&cubic[0].fY, &A, &B, &C, &D);
        D -= pt.fY;
        double allRoots[3] = {0}, validRoots[3] = {0};
        int realRoots = SkDCubic::RootsReal(A, B, C, D, allRoots);
        int valid = SkDQuad::AddValidTs(allRoots, realRoots, validRoots);
        if (valid != 1) {
            continue;
        }
        if (realRoots == 1) {
            continue;
        }
        t = validRoots[0];
        SkDPoint calcPt = cubic.ptAtT(t);
        if (calcPt.approximatelyEqual(pt)) {
            continue;
        }
#if 0
        double R2MinusQ3;
        if (r2check(A, B, C, D, &R2MinusQ3)) {
            smallestR2 = SkTMin(smallestR2, R2MinusQ3);
            largestR2 = SkTMax(largestR2, R2MinusQ3);
        }
#endif
        double largest = SkTMax(fabs(allRoots[0]), fabs(allRoots[1]));
        if (realRoots == 3) {
            largest = SkTMax(largest, fabs(allRoots[2]));
        }
        int largeBits;
        if (largest <= 1) {
#if 0
            SkDebugf("realRoots=%d (%1.9g, %1.9g, %1.9g) valid=%d (%1.9g, %1.9g, %1.9g)\n",
                realRoots, allRoots[0], allRoots[1], allRoots[2], valid, validRoots[0],
                validRoots[1], validRoots[2]);
#endif
            double smallest = SkTMin(allRoots[0], allRoots[1]);
            if (realRoots == 3) {
                smallest = SkTMin(smallest, allRoots[2]);
            }
            SkASSERT_RELEASE(smallest < 0);
            SkASSERT_RELEASE(smallest >= -1);
            largeBits = 0;
        } else {
            frexp(largest, &largeBits);
            SkASSERT_RELEASE(largeBits >= 0);
            SkASSERT_RELEASE(largeBits < 256);
        }
        double step = 1e-6;
        if (largeBits > 21) {
            step = 1e-1;
        } else if (largeBits > 18) {
            step = 1e-2;
        } else if (largeBits > 15) {
            step = 1e-3;
        } else if (largeBits > 12) {
            step = 1e-4;
        } else if (largeBits > 9) {
            step = 1e-5;
        }
        double diff;
        do {
            double newT = binary_search(cubic, step, pt, t, &iters);
            if (newT >= 0) {
                diff = fabs(t - newT);
                break;
            }
            step *= 1.5;
            SkASSERT_RELEASE(step < 1);
        } while (true);
        worstStep[largeBits] = SkTMax(worstStep[largeBits], diff);
#if 0
        {
            cubic.dump();
            SkDebugf("\n");
            SkDLine line = {{{pt.fX - 1, pt.fY}, {pt.fX + 1, pt.fY}}};
            line.dump();
            SkDebugf("\n");
        }
#endif
        ++errors;
    }
    SkDebugf("errors=%d avgIter=%1.9g", errors, (double) iters / errors);
    SkDebugf(" steps: ");
    int worstLimit = SK_ARRAY_COUNT(worstStep);
    while (worstStep[--worstLimit] == 0) ;
    for (int idx2 = 0; idx2 <= worstLimit; ++idx2) {
        SkDebugf("%1.9g ", worstStep[idx2]);
    }
    SkDebugf("\n");
    SkDebugf("smallestR2=%1.9g largestR2=%1.9g\n", smallestR2, largestR2);
}

static double testOneFailure(const CubicLineFailures& failure) {
    const CubicPts& c = failure.c;
    SkDCubic cubic;
    cubic.debugSet(c.fPts);
    const SkDPoint& pt = failure.p;
    double A, B, C, D;
    SkDCubic::Coefficients(&cubic[0].fY, &A, &B, &C, &D);
    D -= pt.fY;
    double allRoots[3] = {0}, validRoots[3] = {0};
    int realRoots = SkDCubic::RootsReal(A, B, C, D, allRoots);
    int valid = SkDQuad::AddValidTs(allRoots, realRoots, validRoots);
    SkASSERT_RELEASE(valid == 1);
    SkASSERT_RELEASE(realRoots != 1);
    double t = validRoots[0];
    SkDPoint calcPt = cubic.ptAtT(t);
    SkASSERT_RELEASE(!calcPt.approximatelyEqual(pt));
    int iters = 0;
    double newT = binary_search(cubic, 0.1, pt, t, &iters);
    return newT;
}

DEF_TEST(PathOpsCubicLineFailures, reporter) {
    return;  // disable for now
    for (int index = 0; index < cubicLineFailuresCount; ++index) {
        const CubicLineFailures& failure = cubicLineFailures[index];
        double newT = testOneFailure(failure);
        SkASSERT_RELEASE(newT >= 0);
    }
}

DEF_TEST(PathOpsCubicLineOneFailure, reporter) {
    return;  // disable for now
    const CubicLineFailures& failure = cubicLineFailures[1];
    double newT = testOneFailure(failure);
    SkASSERT_RELEASE(newT >= 0);
}
