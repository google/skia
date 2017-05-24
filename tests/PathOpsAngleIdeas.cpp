/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkIntersections.h"
#include "SkOpContour.h"
#include "SkOpSegment.h"
#include "SkRandom.h"
#include "SkTArray.h"
#include "SkTSort.h"
#include "Test.h"

static bool gPathOpsAngleIdeasVerbose = false;
static bool gPathOpsAngleIdeasEnableBruteCheck = false;

class PathOpsAngleTester {
public:
    static int ConvexHullOverlaps(SkOpAngle& lh, SkOpAngle& rh) {
        return lh.convexHullOverlaps(&rh);
    }

    static int EndsIntersect(SkOpAngle& lh, SkOpAngle& rh) {
        return lh.endsIntersect(&rh);
    }
};

struct TRange {
    double tMin1;
    double tMin2;
    double t1;
    double t2;
    double tMin;
    double a1;
    double a2;
    bool ccw;
};

static double testArc(skiatest::Reporter* reporter, const SkDQuad& quad, const SkDQuad& arcRef,
        int octant) {
    SkDQuad arc = arcRef;
    SkDVector offset = {quad[0].fX, quad[0].fY};
    arc[0] += offset;
    arc[1] += offset;
    arc[2] += offset;
    SkIntersections i;
    i.intersect(arc, quad);
    if (i.used() == 0) {
        return -1;
    }
    int smallest = -1;
    double t = 2;
    for (int idx = 0; idx < i.used(); ++idx) {
        if (i[0][idx] > 1 || i[0][idx] < 0) {
            i.reset();
            i.intersect(arc, quad);
        }
        if (i[1][idx] > 1 || i[1][idx] < 0) {
            i.reset();
            i.intersect(arc, quad);
        }
        if (t > i[1][idx]) {
            smallest = idx;
            t = i[1][idx];
        }
    }
    REPORTER_ASSERT(reporter, smallest >= 0);
    REPORTER_ASSERT(reporter, t >= 0 && t <= 1);
    return i[1][smallest];
}

static void orderQuads(skiatest::Reporter* reporter, const SkDQuad& quad, double radius,
        SkTArray<double, false>* tArray) {
    double r = radius;
    double s = r * SK_ScalarTanPIOver8;
    double m = r * SK_ScalarRoot2Over2;
    // construct circle from quads
    const QuadPts circle[8] = {{{{ r,  0}, { r, -s}, { m, -m}}},
                                {{{ m, -m}, { s, -r}, { 0, -r}}},
                                {{{ 0, -r}, {-s, -r}, {-m, -m}}},
                                {{{-m, -m}, {-r, -s}, {-r,  0}}},
                                {{{-r,  0}, {-r,  s}, {-m,  m}}},
                                {{{-m,  m}, {-s,  r}, { 0,  r}}},
                                {{{ 0,  r}, { s,  r}, { m,  m}}},
                                {{{ m,  m}, { r,  s}, { r,  0}}}};
    for (int octant = 0; octant < 8; ++octant) {
        SkDQuad cQuad;
        cQuad.debugSet(circle[octant].fPts);
        double t = testArc(reporter, quad, cQuad, octant);
        if (t < 0) {
            continue;
        }
        for (int index = 0; index < tArray->count(); ++index) {
            double matchT = (*tArray)[index];
            if (approximately_equal(t, matchT)) {
                goto next;
            }
        }
        tArray->push_back(t);
next:   ;
    }
}

static double quadAngle(skiatest::Reporter* reporter, const SkDQuad& quad, double t) {
    const SkDVector& pt = quad.ptAtT(t) - quad[0];
    double angle = (atan2(pt.fY, pt.fX) + SK_ScalarPI) * 8 / (SK_ScalarPI * 2);
    REPORTER_ASSERT(reporter, angle >= 0 && angle <= 8);
    return angle;
}

static bool angleDirection(double a1, double a2) {
    double delta = a1 - a2;
    return (delta < 4 && delta > 0) || delta < -4;
}

static void setQuadHullSweep(const SkDQuad& quad, SkDVector sweep[2]) {
    sweep[0] = quad[1] - quad[0];
    sweep[1] = quad[2] - quad[0];
}

static double distEndRatio(double dist, const SkDQuad& quad) {
    SkDVector v[] = {quad[2] - quad[0], quad[1] - quad[0], quad[2] - quad[1]};
    double longest = SkTMax(v[0].length(), SkTMax(v[1].length(), v[2].length()));
    return longest / dist;
}

static bool checkParallel(skiatest::Reporter* reporter, const SkDQuad& quad1, const SkDQuad& quad2) {
    SkDVector sweep[2], tweep[2];
    setQuadHullSweep(quad1, sweep);
    setQuadHullSweep(quad2, tweep);
    // if the ctrl tangents are not nearly parallel, use them
    // solve for opposite direction displacement scale factor == m
    // initial dir = v1.cross(v2) == v2.x * v1.y - v2.y * v1.x
    // displacement of q1[1] : dq1 = { -m * v1.y, m * v1.x } + q1[1]
    // straight angle when : v2.x * (dq1.y - q1[0].y) == v2.y * (dq1.x - q1[0].x)
    //                       v2.x * (m * v1.x + v1.y) == v2.y * (-m * v1.y + v1.x)
    // - m * (v2.x * v1.x + v2.y * v1.y) == v2.x * v1.y - v2.y * v1.x
    // m = (v2.y * v1.x - v2.x * v1.y) / (v2.x * v1.x + v2.y * v1.y)
    // m = v1.cross(v2) / v1.dot(v2)
    double s0dt0 = sweep[0].dot(tweep[0]);
    REPORTER_ASSERT(reporter, s0dt0 != 0);
    double s0xt0 = sweep[0].crossCheck(tweep[0]);
    double m = s0xt0 / s0dt0;
    double sDist = sweep[0].length() * m;
    double tDist = tweep[0].length() * m;
    bool useS = fabs(sDist) < fabs(tDist);
    double mFactor = fabs(useS ? distEndRatio(sDist, quad1) : distEndRatio(tDist, quad2));
    if (mFactor < 5000) {  // empirically found limit
        return s0xt0 < 0;
    }
    SkDVector m0 = quad1.ptAtT(0.5) - quad1[0];
    SkDVector m1 = quad2.ptAtT(0.5) - quad2[0];
    return m0.crossCheck(m1) < 0;
}

/* returns
   -1 if overlaps
    0 if no overlap cw
    1 if no overlap ccw
*/
static int quadHullsOverlap(skiatest::Reporter* reporter, const SkDQuad& quad1,
        const SkDQuad& quad2) {
    SkDVector sweep[2], tweep[2];
    setQuadHullSweep(quad1, sweep);
    setQuadHullSweep(quad2, tweep);
    double s0xs1 = sweep[0].crossCheck(sweep[1]);
    double s0xt0 = sweep[0].crossCheck(tweep[0]);
    double s1xt0 = sweep[1].crossCheck(tweep[0]);
    bool tBetweenS = s0xs1 > 0 ? s0xt0 > 0 && s1xt0 < 0 : s0xt0 < 0 && s1xt0 > 0;
    double s0xt1 = sweep[0].crossCheck(tweep[1]);
    double s1xt1 = sweep[1].crossCheck(tweep[1]);
    tBetweenS |= s0xs1 > 0 ? s0xt1 > 0 && s1xt1 < 0 : s0xt1 < 0 && s1xt1 > 0;
    double t0xt1 = tweep[0].crossCheck(tweep[1]);
    if (tBetweenS) {
        return -1;
    }
    if ((s0xt0 == 0 && s1xt1 == 0) || (s1xt0 == 0 && s0xt1 == 0)) {  // s0 to s1 equals t0 to t1
        return -1;
    }
    bool sBetweenT = t0xt1 > 0 ? s0xt0 < 0 && s0xt1 > 0 : s0xt0 > 0 && s0xt1 < 0;
    sBetweenT |= t0xt1 > 0 ? s1xt0 < 0 && s1xt1 > 0 : s1xt0 > 0 && s1xt1 < 0;
    if (sBetweenT) {
        return -1;
    }
    // if all of the sweeps are in the same half plane, then the order of any pair is enough
    if (s0xt0 >= 0 && s0xt1 >= 0 && s1xt0 >= 0 && s1xt1 >= 0) {
        return 0;
    }
    if (s0xt0 <= 0 && s0xt1 <= 0 && s1xt0 <= 0 && s1xt1 <= 0) {
        return 1;
    }
    // if the outside sweeps are greater than 180 degress:
        // first assume the inital tangents are the ordering
        // if the midpoint direction matches the inital order, that is enough
    SkDVector m0 = quad1.ptAtT(0.5) - quad1[0];
    SkDVector m1 = quad2.ptAtT(0.5) - quad2[0];
    double m0xm1 = m0.crossCheck(m1);
    if (s0xt0 > 0 && m0xm1 > 0) {
        return 0;
    }
    if (s0xt0 < 0 && m0xm1 < 0) {
        return 1;
    }
    REPORTER_ASSERT(reporter, s0xt0 != 0);
    return checkParallel(reporter, quad1, quad2);
}

static double radianSweep(double start, double end) {
    double sweep = end - start;
    if (sweep > SK_ScalarPI) {
        sweep -= 2 * SK_ScalarPI;
    } else if (sweep < -SK_ScalarPI) {
        sweep += 2 * SK_ScalarPI;
    }
    return sweep;
}

static bool radianBetween(double start, double test, double end) {
    double startToEnd = radianSweep(start, end);
    double startToTest = radianSweep(start, test);
    double testToEnd = radianSweep(test, end);
    return (startToTest <= 0 && testToEnd <= 0 && startToTest >= startToEnd) ||
        (startToTest >= 0 && testToEnd >= 0 && startToTest <= startToEnd);
}

static bool orderTRange(skiatest::Reporter* reporter, const SkDQuad& quad1, const SkDQuad& quad2,
        double r, TRange* result) {
    SkTArray<double, false> t1Array, t2Array;
    orderQuads(reporter, quad1, r, &t1Array);
    orderQuads(reporter,quad2, r, &t2Array);
    if (!t1Array.count() || !t2Array.count()) {
        return false;
    }
    SkTQSort<double>(t1Array.begin(), t1Array.end() - 1);
    SkTQSort<double>(t2Array.begin(), t2Array.end() - 1);
    double t1 = result->tMin1 = t1Array[0];
    double t2 = result->tMin2 = t2Array[0];
    double a1 = quadAngle(reporter,quad1, t1);
    double a2 = quadAngle(reporter,quad2, t2);
    if (approximately_equal(a1, a2)) {
        return false;
    }
    bool refCCW = angleDirection(a1, a2);
    result->t1 = t1;
    result->t2 = t2;
    result->tMin = SkTMin(t1, t2);
    result->a1 = a1;
    result->a2 = a2;
    result->ccw = refCCW;
    return true;
}

static bool equalPoints(const SkDPoint& pt1, const SkDPoint& pt2, double max) {
    return approximately_zero_when_compared_to(pt1.fX - pt2.fX, max)
            && approximately_zero_when_compared_to(pt1.fY - pt2.fY, max);
}

static double maxDist(const SkDQuad& quad) {
    SkDRect bounds;
    bounds.setBounds(quad);
    SkDVector corner[4] = {
        { bounds.fLeft - quad[0].fX, bounds.fTop - quad[0].fY },
        { bounds.fRight - quad[0].fX, bounds.fTop - quad[0].fY },
        { bounds.fLeft - quad[0].fX, bounds.fBottom - quad[0].fY },
        { bounds.fRight - quad[0].fX, bounds.fBottom - quad[0].fY }
    };
    double max = 0;
    for (unsigned index = 0; index < SK_ARRAY_COUNT(corner); ++index) {
        max = SkTMax(max, corner[index].length());
    }
    return max;
}

static double maxQuad(const SkDQuad& quad) {
    double max = 0;
    for (int index = 0; index < 2; ++index) {
        max = SkTMax(max, fabs(quad[index].fX));
        max = SkTMax(max, fabs(quad[index].fY));
    }
    return max;
}

static bool bruteMinT(skiatest::Reporter* reporter, const SkDQuad& quad1, const SkDQuad& quad2,
        TRange* lowerRange, TRange* upperRange) {
    double maxRadius = SkTMin(maxDist(quad1), maxDist(quad2));
    double maxQuads = SkTMax(maxQuad(quad1), maxQuad(quad2));
    double r = maxRadius / 2;
    double rStep = r / 2;
    SkDPoint best1 = {SK_ScalarInfinity, SK_ScalarInfinity};
    SkDPoint best2 = {SK_ScalarInfinity, SK_ScalarInfinity};
    int bestCCW = -1;
    double bestR = maxRadius;
    upperRange->tMin = 0;
    lowerRange->tMin = 1;
    do {
        do {  // find upper bounds of single result
            TRange tRange;
            bool stepUp = orderTRange(reporter, quad1, quad2, r, &tRange);
            if (stepUp) {
                SkDPoint pt1 = quad1.ptAtT(tRange.t1);
                if (equalPoints(pt1, best1, maxQuads)) {
                    break;
                }
                best1 = pt1;
                SkDPoint pt2 = quad2.ptAtT(tRange.t2);
                if (equalPoints(pt2, best2, maxQuads)) {
                    break;
                }
                best2 = pt2;
                if (gPathOpsAngleIdeasVerbose) {
                    SkDebugf("u bestCCW=%d ccw=%d bestMin=%1.9g:%1.9g r=%1.9g tMin=%1.9g\n",
                            bestCCW, tRange.ccw, lowerRange->tMin, upperRange->tMin, r,
                            tRange.tMin);
                }
                if (bestCCW >= 0 && bestCCW != (int) tRange.ccw) {
                    if (tRange.tMin < upperRange->tMin) {
                        upperRange->tMin = 0;
                    } else {
                        stepUp = false;
                    }
                }
                if (upperRange->tMin < tRange.tMin) {
                    bestCCW = tRange.ccw;
                    bestR = r;
                    *upperRange = tRange;
                }
                if (lowerRange->tMin > tRange.tMin) {
                    *lowerRange = tRange;
                }
            }
            r += stepUp ? rStep : -rStep;
            rStep /= 2;
        } while (rStep > FLT_EPSILON);
        if (bestCCW < 0) {
            if (bestR >= maxRadius) {
                SkDebugf("");
            }
            REPORTER_ASSERT(reporter, bestR < maxRadius);
            return false;
        }
        double lastHighR = bestR;
        r = bestR / 2;
        rStep = r / 2;
        do {  // find lower bounds of single result
            TRange tRange;
            bool success = orderTRange(reporter, quad1, quad2, r, &tRange);
            if (success) {
                if (gPathOpsAngleIdeasVerbose) {
                    SkDebugf("l bestCCW=%d ccw=%d bestMin=%1.9g:%1.9g r=%1.9g tMin=%1.9g\n",
                            bestCCW, tRange.ccw, lowerRange->tMin, upperRange->tMin, r,
                            tRange.tMin);
                }
                if (bestCCW != (int) tRange.ccw || upperRange->tMin < tRange.tMin) {
                    bestCCW = tRange.ccw;
                    *upperRange = tRange;
                    bestR = lastHighR;
                    break;  // need to establish a new upper bounds
                }
                SkDPoint pt1 = quad1.ptAtT(tRange.t1);
                SkDPoint pt2 = quad2.ptAtT(tRange.t2);
                if (equalPoints(pt1, best1, maxQuads)) {
                    goto breakOut;
                }
                best1 = pt1;
                if (equalPoints(pt2, best2, maxQuads)) {
                    goto breakOut;
                }
                best2 = pt2;
                if (equalPoints(pt1, pt2, maxQuads)) {
                    success = false;
                } else {
                    if (upperRange->tMin < tRange.tMin) {
                        *upperRange = tRange;
                    }
                    if (lowerRange->tMin > tRange.tMin) {
                        *lowerRange = tRange;
                    }
                }
                lastHighR = SkTMin(r, lastHighR);
            }
            r += success ? -rStep : rStep;
            rStep /= 2;
        } while (rStep > FLT_EPSILON);
    } while (rStep > FLT_EPSILON);
breakOut:
    if (gPathOpsAngleIdeasVerbose) {
        SkDebugf("l a2-a1==%1.9g\n", lowerRange->a2 - lowerRange->a1);
    }
    return true;
}

static void bruteForce(skiatest::Reporter* reporter, const SkDQuad& quad1, const SkDQuad& quad2,
        bool ccw) {
    if (!gPathOpsAngleIdeasEnableBruteCheck) {
        return;
    }
    TRange lowerRange, upperRange;
    bool result = bruteMinT(reporter, quad1, quad2, &lowerRange, &upperRange);
    REPORTER_ASSERT(reporter, result);
    double angle = fabs(lowerRange.a2 - lowerRange.a1);
    REPORTER_ASSERT(reporter, angle > 3.998 || ccw == upperRange.ccw);
}

static bool bruteForceCheck(skiatest::Reporter* reporter, const SkDQuad& quad1,
        const SkDQuad& quad2, bool ccw) {
    TRange lowerRange, upperRange;
    bool result = bruteMinT(reporter, quad1, quad2, &lowerRange, &upperRange);
    REPORTER_ASSERT(reporter, result);
    return ccw == upperRange.ccw;
}

static void makeSegment(SkOpContour* contour, const SkDQuad& quad, SkPoint shortQuad[3]) {
    shortQuad[0] = quad[0].asSkPoint();
    shortQuad[1] = quad[1].asSkPoint();
    shortQuad[2] = quad[2].asSkPoint();
    contour->addQuad(shortQuad);
}

static void testQuadAngles(skiatest::Reporter* reporter, const SkDQuad& quad1, const SkDQuad& quad2,
        int testNo, SkArenaAlloc* allocator) {
    SkPoint shortQuads[2][3];

    SkOpContourHead contour;
    SkOpGlobalState state(&contour, allocator  SkDEBUGPARAMS(false) SkDEBUGPARAMS(nullptr));
    contour.init(&state, false, false);
    makeSegment(&contour, quad1, shortQuads[0]);
    makeSegment(&contour, quad1, shortQuads[1]);
    SkOpSegment* seg1 = contour.first();
    seg1->debugAddAngle(0, 1);
    SkOpSegment* seg2 = seg1->next();
    seg2->debugAddAngle(0, 1);
    int realOverlap = PathOpsAngleTester::ConvexHullOverlaps(*seg1->debugLastAngle(),
            *seg2->debugLastAngle());
    const SkDPoint& origin = quad1[0];
    REPORTER_ASSERT(reporter, origin == quad2[0]);
    double a1s = atan2(origin.fY - quad1[1].fY, quad1[1].fX - origin.fX);
    double a1e = atan2(origin.fY - quad1[2].fY, quad1[2].fX - origin.fX);
    double a2s = atan2(origin.fY - quad2[1].fY, quad2[1].fX - origin.fX);
    double a2e = atan2(origin.fY - quad2[2].fY, quad2[2].fX - origin.fX);
    bool oldSchoolOverlap = radianBetween(a1s, a2s, a1e)
        || radianBetween(a1s, a2e, a1e) || radianBetween(a2s, a1s, a2e)
        || radianBetween(a2s, a1e, a2e);
    int overlap = quadHullsOverlap(reporter, quad1, quad2);
    bool realMatchesOverlap = realOverlap == overlap || SK_ScalarPI - fabs(a2s - a1s) < 0.002;
    if (realOverlap != overlap) {
        SkDebugf("\nSK_ScalarPI - fabs(a2s - a1s) = %1.9g\n", SK_ScalarPI - fabs(a2s - a1s));
    }
    if (!realMatchesOverlap) {
        DumpQ(quad1, quad2, testNo);
    }
    REPORTER_ASSERT(reporter, realMatchesOverlap);
    if (oldSchoolOverlap != (overlap < 0)) {
        overlap = quadHullsOverlap(reporter, quad1, quad2);  // set a breakpoint and debug if assert fires
        REPORTER_ASSERT(reporter, oldSchoolOverlap == (overlap < 0));
    }
    SkDVector v1s = quad1[1] - quad1[0];
    SkDVector v1e = quad1[2] - quad1[0];
    SkDVector v2s = quad2[1] - quad2[0];
    SkDVector v2e = quad2[2] - quad2[0];
    double vDir[2] = { v1s.cross(v1e), v2s.cross(v2e) };
    bool ray1In2 = v1s.cross(v2s) * vDir[1] <= 0 && v1s.cross(v2e) * vDir[1] >= 0;
    bool ray2In1 = v2s.cross(v1s) * vDir[0] <= 0 && v2s.cross(v1e) * vDir[0] >= 0;
    if (overlap >= 0) {
        // verify that hulls really don't overlap
        REPORTER_ASSERT(reporter, !ray1In2);
        REPORTER_ASSERT(reporter, !ray2In1);
        bool ctrl1In2 = v1e.cross(v2s) * vDir[1] <= 0 && v1e.cross(v2e) * vDir[1] >= 0;
        REPORTER_ASSERT(reporter, !ctrl1In2);
        bool ctrl2In1 = v2e.cross(v1s) * vDir[0] <= 0 && v2e.cross(v1e) * vDir[0] >= 0;
        REPORTER_ASSERT(reporter, !ctrl2In1);
        // check answer against reference
        bruteForce(reporter, quad1, quad2, overlap > 0);
    }
    // continue end point rays and see if they intersect the opposite curve
    SkDLine rays[] = {{{origin, quad2[2]}}, {{origin, quad1[2]}}};
    const SkDQuad* quads[] = {&quad1, &quad2};
    SkDVector midSpokes[2];
    SkIntersections intersect[2];
    double minX, minY, maxX, maxY;
    minX = minY = SK_ScalarInfinity;
    maxX = maxY = -SK_ScalarInfinity;
    double maxWidth = 0;
    bool useIntersect = false;
    double smallestTs[] = {1, 1};
    for (unsigned index = 0; index < SK_ARRAY_COUNT(quads); ++index) {
        const SkDQuad& q = *quads[index];
        midSpokes[index] = q.ptAtT(0.5) - origin;
        minX = SkTMin(SkTMin(SkTMin(minX, origin.fX), q[1].fX), q[2].fX);
        minY = SkTMin(SkTMin(SkTMin(minY, origin.fY), q[1].fY), q[2].fY);
        maxX = SkTMax(SkTMax(SkTMax(maxX, origin.fX), q[1].fX), q[2].fX);
        maxY = SkTMax(SkTMax(SkTMax(maxY, origin.fY), q[1].fY), q[2].fY);
        maxWidth = SkTMax(maxWidth, SkTMax(maxX - minX, maxY - minY));
        intersect[index].intersectRay(q, rays[index]);
        const SkIntersections& i = intersect[index];
        REPORTER_ASSERT(reporter, i.used() >= 1);
        bool foundZero = false;
        double smallT = 1;
        for (int idx2 = 0; idx2 < i.used(); ++idx2) {
            double t = i[0][idx2];
            if (t == 0) {
                foundZero = true;
                continue;
            }
            if (smallT > t) {
                smallT = t;
            }
        }
        REPORTER_ASSERT(reporter, foundZero == true);
        if (smallT == 1) {
            continue;
        }
        SkDVector ray = q.ptAtT(smallT) - origin;
        SkDVector end = rays[index][1] - origin;
        if (ray.fX * end.fX < 0 || ray.fY * end.fY < 0) {
            continue;
        }
        double rayDist = ray.length();
        double endDist = end.length();
        double delta = fabs(rayDist - endDist) / maxWidth;
        if (delta > 1e-4) {
            useIntersect ^= true;
        }
        smallestTs[index] = smallT;
    }
    bool firstInside;
    if (useIntersect) {
        int sIndex = (int) (smallestTs[1] < 1);
        REPORTER_ASSERT(reporter, smallestTs[sIndex ^ 1] == 1);
        double t = smallestTs[sIndex];
        const SkDQuad& q = *quads[sIndex];
        SkDVector ray = q.ptAtT(t) - origin;
        SkDVector end = rays[sIndex][1] - origin;
        double rayDist = ray.length();
        double endDist = end.length();
        SkDVector mid = q.ptAtT(t / 2) - origin;
        double midXray = mid.crossCheck(ray);
        if (gPathOpsAngleIdeasVerbose) {
            SkDebugf("rayDist>endDist:%d sIndex==0:%d vDir[sIndex]<0:%d midXray<0:%d\n",
                    rayDist > endDist, sIndex == 0, vDir[sIndex] < 0, midXray < 0);
        }
        SkASSERT(SkScalarSignAsInt(SkDoubleToScalar(midXray))
            == SkScalarSignAsInt(SkDoubleToScalar(vDir[sIndex])));
        firstInside = (rayDist > endDist) ^ (sIndex == 0) ^ (vDir[sIndex] < 0);
    } else if (overlap >= 0) {
        return;  // answer has already been determined
    } else {
        firstInside = checkParallel(reporter, quad1, quad2);
    }
    if (overlap < 0) {
        SkDEBUGCODE(int realEnds =)
                PathOpsAngleTester::EndsIntersect(*seg1->debugLastAngle(),
                *seg2->debugLastAngle());
        SkASSERT(realEnds == (firstInside ? 1 : 0));
    }
    bruteForce(reporter, quad1, quad2, firstInside);
}

DEF_TEST(PathOpsAngleOverlapHullsOne, reporter) {
    SkSTArenaAlloc<4096> allocator;
//    gPathOpsAngleIdeasVerbose = true;
    const QuadPts quads[] = {
{{{939.4808349609375, 914.355224609375}, {-357.7921142578125, 590.842529296875}, {736.8936767578125, -350.717529296875}}},
{{{939.4808349609375, 914.355224609375}, {-182.85418701171875, 634.4552001953125}, {-509.62615966796875, 576.1182861328125}}}
    };
    for (int index = 0; index < (int) SK_ARRAY_COUNT(quads); index += 2) {
        SkDQuad quad0, quad1;
        quad0.debugSet(quads[index].fPts);
        quad1.debugSet(quads[index + 1].fPts);
        testQuadAngles(reporter, quad0, quad1, 0, &allocator);
    }
}

DEF_TEST(PathOpsAngleOverlapHulls, reporter) {
    SkSTArenaAlloc<4096> allocator;
    if (!gPathOpsAngleIdeasVerbose) {  // takes a while to run -- so exclude it by default
        return;
    }
    SkRandom ran;
    for (int index = 0; index < 100000; ++index) {
        if (index % 1000 == 999) SkDebugf(".");
        SkDPoint origin = {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)};
        QuadPts quad1 = {{origin, {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)},
            {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)}}};
        if (quad1.fPts[0] == quad1.fPts[2]) {
            continue;
        }
        QuadPts quad2 = {{origin, {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)},
            {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)}}};
        if (quad2.fPts[0] == quad2.fPts[2]) {
            continue;
        }
        SkIntersections i;
        SkDQuad q1, q2;
        q1.debugSet(quad1.fPts);
        q2.debugSet(quad2.fPts);
        i.intersect(q1, q2);
        REPORTER_ASSERT(reporter, i.used() >= 1);
        if (i.used() > 1) {
            continue;
        }
        testQuadAngles(reporter, q1, q2, index, &allocator);
    }
}

DEF_TEST(PathOpsAngleBruteT, reporter) {
    if (!gPathOpsAngleIdeasVerbose) {  // takes a while to run -- so exclude it by default
        return;
    }
    SkRandom ran;
    double smaller = SK_Scalar1;
    SkDQuad small[2];
    SkDEBUGCODE(int smallIndex);
    for (int index = 0; index < 100000; ++index) {
        SkDPoint origin = {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)};
        QuadPts quad1 = {{origin, {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)},
            {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)}}};
        if (quad1.fPts[0] == quad1.fPts[2]) {
            continue;
        }
        QuadPts quad2 = {{origin, {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)},
            {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)}}};
        if (quad2.fPts[0] == quad2.fPts[2]) {
            continue;
        }
        SkDQuad q1, q2;
        q1.debugSet(quad1.fPts);
        q2.debugSet(quad2.fPts);
        SkIntersections i;
        i.intersect(q1, q2);
        REPORTER_ASSERT(reporter, i.used() >= 1);
        if (i.used() > 1) {
            continue;
        }
        TRange lowerRange, upperRange;
        bool result = bruteMinT(reporter, q1, q2, &lowerRange, &upperRange);
        REPORTER_ASSERT(reporter, result);
        double min = SkTMin(upperRange.t1, upperRange.t2);
        if (smaller > min) {
            small[0] = q1;
            small[1] = q2;
            SkDEBUGCODE(smallIndex = index);
            smaller = min;
        }
    }
#ifdef SK_DEBUG
    DumpQ(small[0], small[1], smallIndex);
#endif
}

DEF_TEST(PathOpsAngleBruteTOne, reporter) {
//    gPathOpsAngleIdeasVerbose = true;
    const QuadPts qPts[] = {
{{{-770.8492431640625, 948.2369384765625}, {-853.37066650390625, 972.0301513671875}, {-200.62042236328125, -26.7174072265625}}},
{{{-770.8492431640625, 948.2369384765625}, {513.602783203125, 578.8681640625}, {960.641357421875, -813.69757080078125}}},
{{{563.8267822265625, -107.4566650390625}, {-44.67724609375, -136.57452392578125}, {492.3856201171875, -268.79644775390625}}},
{{{563.8267822265625, -107.4566650390625}, {708.049072265625, -100.77789306640625}, {-48.88226318359375, 967.9022216796875}}},
{{{598.857421875, 846.345458984375}, {-644.095703125, -316.12921142578125}, {-97.64599609375, 20.6158447265625}}},
{{{598.857421875, 846.345458984375}, {715.7142333984375, 955.3599853515625}, {-919.9478759765625, 691.611328125}}},
    };
    TRange lowerRange, upperRange;
    SkDQuad quads[SK_ARRAY_COUNT(qPts)];
    for (int index = 0; index < (int) SK_ARRAY_COUNT(qPts); ++index) {
        quads[index].debugSet(qPts[index].fPts);
    }
    bruteMinT(reporter, quads[0], quads[1], &lowerRange, &upperRange);
    bruteMinT(reporter, quads[2], quads[3], &lowerRange, &upperRange);
    bruteMinT(reporter, quads[4], quads[5], &lowerRange, &upperRange);
}

/*
The sorting problem happens when the inital tangents are not a true indicator of the curve direction
Nearly always, the initial tangents do give the right answer,
so the trick is to figure out when the initial tangent cannot be trusted.
If the convex hulls of both curves are in the same half plane, and not overlapping, sorting the
hulls is enough.
If the hulls overlap, and have the same general direction, then intersect the shorter end point ray
with the opposing curve, and see on which side of the shorter curve the opposing intersection lies.
Otherwise, if the control vector is extremely short, likely the point on curve must be computed
If moving the control point slightly can change the sign of the cross product, either answer could
be "right".
We need to determine how short is extremely short. Move the control point a set percentage of
the largest length to determine how stable the curve is vis-a-vis the initial tangent.
*/

static const QuadPts extremeTests[][2] = {
    {
        {{{-708.0077926931004,-154.61669472244046},
            {-707.9234268635319,-154.30459999551294},
            {505.58447265625,-504.9130859375}}},
        {{{-708.0077926931004,-154.61669472244046},
            {-711.127526325141,-163.9446090624656},
            {-32.39227294921875,-906.3277587890625}}},
    }, {
        {{{-708.0077926931004,-154.61669472244046},
            {-708.2875337527566,-154.36676458635623},
            {505.58447265625,-504.9130859375}}},
        {{{-708.0077926931004,-154.61669472244046},
            {-708.4111557216864,-154.5366642875255},
            {-32.39227294921875,-906.3277587890625}}},
    }, {
        {{{-609.0230951752058,-267.5435593490574},
            {-594.1120809906336,-136.08492475411555},
            {505.58447265625,-504.9130859375}}},
        {{{-609.0230951752058,-267.5435593490574},
            {-693.7467719138988,-341.3259237831895},
            {-32.39227294921875,-906.3277587890625}}}
    }, {
        {{{-708.0077926931004,-154.61669472244046},
            {-707.9994640658723,-154.58588461064852},
            {505.58447265625,-504.9130859375}}},
        {{{-708.0077926931004,-154.61669472244046},
            {-708.0239418990758,-154.6403553507124},
            {-32.39227294921875,-906.3277587890625}}}
    }, {
        {{{-708.0077926931004,-154.61669472244046},
            {-707.9993222215099,-154.55999389855003},
            {68.88981098017803,296.9273945411635}}},
        {{{-708.0077926931004,-154.61669472244046},
            {-708.0509091919608,-154.64675214697067},
            {-777.4871194247767,-995.1470120113145}}}
    }, {
        {{{-708.0077926931004,-154.61669472244046},
            {-708.0060491116379,-154.60889321524968},
            {229.97088707895057,-430.0569357467175}}},
        {{{-708.0077926931004,-154.61669472244046},
            {-708.013911296257,-154.6219143988058},
            {138.13162892614037,-573.3689311737394}}}
    }, {
        {{{-543.2570545751013,-237.29243831075053},
            {-452.4119186056987,-143.47223056267802},
            {229.97088707895057,-430.0569357467175}}},
        {{{-543.2570545751013,-237.29243831075053},
            {-660.5330371214436,-362.0016148388},
            {138.13162892614037,-573.3689311737394}}},
    },
};

static double endCtrlRatio(const SkDQuad quad) {
    SkDVector longEdge = quad[2] - quad[0];
    double longLen = longEdge.length();
    SkDVector shortEdge = quad[1] - quad[0];
    double shortLen = shortEdge.length();
    return longLen / shortLen;
}

static void computeMV(const SkDQuad& quad, const SkDVector& v, double m, SkDVector mV[2]) {
        SkDPoint mPta = {quad[1].fX - m * v.fY, quad[1].fY + m * v.fX};
        SkDPoint mPtb = {quad[1].fX + m * v.fY, quad[1].fY - m * v.fX};
        mV[0] = mPta - quad[0];
        mV[1] = mPtb - quad[0];
}

static double mDistance(skiatest::Reporter* reporter, bool agrees, const SkDQuad& q1,
        const SkDQuad& q2) {
    if (1 && agrees) {
        return SK_ScalarMax;
    }
    // how close is the angle from inflecting in the opposite direction?
    SkDVector v1 = q1[1] - q1[0];
    SkDVector v2 = q2[1] - q2[0];
    double dir = v1.crossCheck(v2);
    REPORTER_ASSERT(reporter, dir != 0);
    // solve for opposite direction displacement scale factor == m
    // initial dir = v1.cross(v2) == v2.x * v1.y - v2.y * v1.x
    // displacement of q1[1] : dq1 = { -m * v1.y, m * v1.x } + q1[1]
    // straight angle when : v2.x * (dq1.y - q1[0].y) == v2.y * (dq1.x - q1[0].x)
    //                       v2.x * (m * v1.x + v1.y) == v2.y * (-m * v1.y + v1.x)
    // - m * (v2.x * v1.x + v2.y * v1.y) == v2.x * v1.y - v2.y * v1.x
    // m = (v2.y * v1.x - v2.x * v1.y) / (v2.x * v1.x + v2.y * v1.y)
    // m = v1.cross(v2) / v1.dot(v2)
    double div = v1.dot(v2);
    REPORTER_ASSERT(reporter, div != 0);
    double m = dir / div;
    SkDVector mV1[2], mV2[2];
    computeMV(q1, v1, m, mV1);
    computeMV(q2, v2, m, mV2);
    double dist1 = v1.length() * m;
    double dist2 = v2.length() * m;
    if (gPathOpsAngleIdeasVerbose) {
        SkDebugf("%c r1=%1.9g r2=%1.9g m=%1.9g dist1=%1.9g dist2=%1.9g "
                " dir%c 1a=%1.9g 1b=%1.9g 2a=%1.9g 2b=%1.9g\n", agrees ? 'T' : 'F',
                endCtrlRatio(q1), endCtrlRatio(q2), m, dist1, dist2, dir > 0 ? '+' : '-',
                mV1[0].crossCheck(v2), mV1[1].crossCheck(v2),
                mV2[0].crossCheck(v1), mV2[1].crossCheck(v1));
    }
    if (1) {
        bool use1 = fabs(dist1) < fabs(dist2);
        if (gPathOpsAngleIdeasVerbose) {
            SkDebugf("%c dist=%1.9g r=%1.9g\n", agrees ? 'T' : 'F', use1 ? dist1 : dist2,
                use1 ? distEndRatio(dist1, q1) : distEndRatio(dist2, q2));
        }
        return fabs(use1 ? distEndRatio(dist1, q1) : distEndRatio(dist2, q2));
    }
    return SK_ScalarMax;
}

static void midPointAgrees(skiatest::Reporter* reporter, const SkDQuad& q1, const SkDQuad& q2,
        bool ccw) {
    SkDPoint mid1 = q1.ptAtT(0.5);
    SkDVector m1 = mid1 - q1[0];
    SkDPoint mid2 = q2.ptAtT(0.5);
    SkDVector m2 = mid2 - q2[0];
    REPORTER_ASSERT(reporter, ccw ? m1.crossCheck(m2) < 0 : m1.crossCheck(m2) > 0);
}

DEF_TEST(PathOpsAngleExtreme, reporter) {
    if (!gPathOpsAngleIdeasVerbose) {  // takes a while to run -- so exclude it by default
        return;
    }
    double maxR = SK_ScalarMax;
    for (int index = 0; index < (int) SK_ARRAY_COUNT(extremeTests); ++index) {
        const QuadPts& qu1 = extremeTests[index][0];
        const QuadPts& qu2 = extremeTests[index][1];
        SkDQuad quad1, quad2;
        quad1.debugSet(qu1.fPts);
        quad2.debugSet(qu2.fPts);
        if (gPathOpsAngleIdeasVerbose) {
            SkDebugf("%s %d\n", __FUNCTION__, index);
        }
        REPORTER_ASSERT(reporter, quad1[0] == quad2[0]);
        SkIntersections i;
        i.intersect(quad1, quad2);
        REPORTER_ASSERT(reporter, i.used() == 1);
        REPORTER_ASSERT(reporter, i.pt(0) == quad1[0]);
        int overlap = quadHullsOverlap(reporter, quad1, quad2);
        REPORTER_ASSERT(reporter, overlap >= 0);
        SkDVector sweep[2], tweep[2];
        setQuadHullSweep(quad1, sweep);
        setQuadHullSweep(quad2, tweep);
        double s0xt0 = sweep[0].crossCheck(tweep[0]);
        REPORTER_ASSERT(reporter, s0xt0 != 0);
        bool ccw = s0xt0 < 0;
        bool agrees = bruteForceCheck(reporter, quad1, quad2, ccw);
        maxR = SkTMin(maxR, mDistance(reporter, agrees, quad1, quad2));
        if (agrees) {
            continue;
        }
        midPointAgrees(reporter, quad1, quad2, !ccw);
        SkDQuad q1 = quad1;
        SkDQuad q2 = quad2;
        double loFail = 1;
        double hiPass = 2;
        // double vectors until t passes
        do {
            q1[1].fX = quad1[0].fX * (1 - hiPass) + quad1[1].fX * hiPass;
            q1[1].fY = quad1[0].fY * (1 - hiPass) + quad1[1].fY * hiPass;
            q2[1].fX = quad2[0].fX * (1 - hiPass) + quad2[1].fX * hiPass;
            q2[1].fY = quad2[0].fY * (1 - hiPass) + quad2[1].fY * hiPass;
            agrees = bruteForceCheck(reporter, q1, q2, ccw);
            maxR = SkTMin(maxR, mDistance(reporter, agrees, q1, q2));
            if (agrees) {
                break;
            }
            midPointAgrees(reporter, quad1, quad2, !ccw);
            loFail = hiPass;
            hiPass *= 2;
        } while (true);
        // binary search to find minimum pass
        double midTest = (loFail + hiPass) / 2;
        double step = (hiPass - loFail) / 4;
        while (step > FLT_EPSILON) {
            q1[1].fX = quad1[0].fX * (1 - midTest) + quad1[1].fX * midTest;
            q1[1].fY = quad1[0].fY * (1 - midTest) + quad1[1].fY * midTest;
            q2[1].fX = quad2[0].fX * (1 - midTest) + quad2[1].fX * midTest;
            q2[1].fY = quad2[0].fY * (1 - midTest) + quad2[1].fY * midTest;
            agrees = bruteForceCheck(reporter, q1, q2, ccw);
            maxR = SkTMin(maxR, mDistance(reporter, agrees, q1, q2));
            if (!agrees) {
                midPointAgrees(reporter, quad1, quad2, !ccw);
            }
            midTest += agrees ? -step : step;
            step /= 2;
        }
#ifdef SK_DEBUG
//        DumpQ(q1, q2, 999);
#endif
    }
    if (gPathOpsAngleIdeasVerbose) {
        SkDebugf("maxR=%1.9g\n", maxR);
    }
}
