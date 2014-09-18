/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkIntersections.h"
#include "SkOpAngle.h"
#include "SkOpSegment.h"
#include "SkPathOpsCurve.h"
#include "SkTSort.h"

#if DEBUG_ANGLE
#include "SkString.h"
#endif

/* Angles are sorted counterclockwise. The smallest angle has a positive x and the smallest
   positive y. The largest angle has a positive x and a zero y. */

#if DEBUG_ANGLE
    static bool CompareResult(SkString* bugOut, int append, bool compare) {
        SkDebugf("%s %c %d\n", bugOut->c_str(), compare ? 'T' : 'F', append);
        return compare;
    }

    #define COMPARE_RESULT(append, compare) CompareResult(&bugOut, append, compare)
#else
    #define COMPARE_RESULT(append, compare) compare
#endif

/*             quarter angle values for sector

31   x > 0, y == 0              horizontal line (to the right)
0    x > 0, y == epsilon        quad/cubic horizontal tangent eventually going +y
1    x > 0, y > 0, x > y        nearer horizontal angle
2                  x + e == y   quad/cubic 45 going horiz
3    x > 0, y > 0, x == y       45 angle
4                  x == y + e   quad/cubic 45 going vert
5    x > 0, y > 0, x < y        nearer vertical angle
6    x == epsilon, y > 0        quad/cubic vertical tangent eventually going +x
7    x == 0, y > 0              vertical line (to the top)

                                      8  7  6
                                 9       |       5
                              10         |          4
                            11           |            3
                          12  \          |           / 2
                         13              |              1
                        14               |               0
                        15 --------------+------------- 31
                        16               |              30
                         17              |             29
                          18  /          |          \ 28
                            19           |           27
                              20         |         26
                                 21      |      25
                                     22 23 24
*/

// return true if lh < this < rh
bool SkOpAngle::after(const SkOpAngle* test) const {
    const SkOpAngle& lh = *test;
    const SkOpAngle& rh = *lh.fNext;
    SkASSERT(&lh != &rh);
#if DEBUG_ANGLE
    SkString bugOut;
    bugOut.printf("%s [%d/%d] %d/%d tStart=%1.9g tEnd=%1.9g"
                  " < [%d/%d] %d/%d tStart=%1.9g tEnd=%1.9g"
                  " < [%d/%d] %d/%d tStart=%1.9g tEnd=%1.9g ", __FUNCTION__,
            lh.fSegment->debugID(), lh.debugID(), lh.fSectorStart, lh.fSectorEnd,
            lh.fSegment->t(lh.fStart), lh.fSegment->t(lh.fEnd),
            fSegment->debugID(), debugID(), fSectorStart, fSectorEnd, fSegment->t(fStart),
            fSegment->t(fEnd),
            rh.fSegment->debugID(), rh.debugID(), rh.fSectorStart, rh.fSectorEnd,
            rh.fSegment->t(rh.fStart), rh.fSegment->t(rh.fEnd));
#endif
    if (lh.fComputeSector && !const_cast<SkOpAngle&>(lh).computeSector()) {
        return COMPARE_RESULT(1, true);
    }
    if (fComputeSector && !const_cast<SkOpAngle*>(this)->computeSector()) {
        return COMPARE_RESULT(2, true);
    }
    if (rh.fComputeSector && !const_cast<SkOpAngle&>(rh).computeSector()) {
        return COMPARE_RESULT(3, true);
    }
#if DEBUG_ANGLE  // reset bugOut with computed sectors
    bugOut.printf("%s [%d/%d] %d/%d tStart=%1.9g tEnd=%1.9g"
                  " < [%d/%d] %d/%d tStart=%1.9g tEnd=%1.9g"
                  " < [%d/%d] %d/%d tStart=%1.9g tEnd=%1.9g ", __FUNCTION__,
            lh.fSegment->debugID(), lh.debugID(), lh.fSectorStart, lh.fSectorEnd,
            lh.fSegment->t(lh.fStart), lh.fSegment->t(lh.fEnd),
            fSegment->debugID(), debugID(), fSectorStart, fSectorEnd, fSegment->t(fStart),
            fSegment->t(fEnd),
            rh.fSegment->debugID(), rh.debugID(), rh.fSectorStart, rh.fSectorEnd,
            rh.fSegment->t(rh.fStart), rh.fSegment->t(rh.fEnd));
#endif
    bool ltrOverlap = (lh.fSectorMask | rh.fSectorMask) & fSectorMask;
    bool lrOverlap = lh.fSectorMask & rh.fSectorMask;
    int lrOrder;  // set to -1 if either order works
    if (!lrOverlap) {  // no lh/rh sector overlap
        if (!ltrOverlap) {  // no lh/this/rh sector overlap
            return COMPARE_RESULT(4,  (lh.fSectorEnd > rh.fSectorStart)
                    ^ (fSectorStart > lh.fSectorEnd) ^ (fSectorStart > rh.fSectorStart));
        }
        int lrGap = (rh.fSectorStart - lh.fSectorStart + 32) & 0x1f;
        /* A tiny change can move the start +/- 4. The order can only be determined if
           lr gap is not 12 to 20 or -12 to -20.
               -31 ..-21      1
               -20 ..-12     -1
               -11 .. -1      0
                 0          shouldn't get here
                11 ..  1      1
                12 .. 20     -1
                21 .. 31      0
         */
        lrOrder = lrGap > 20 ? 0 : lrGap > 11 ? -1 : 1;
    } else {
        lrOrder = (int) lh.orderable(rh);
        if (!ltrOverlap) {
            return COMPARE_RESULT(5, !lrOrder);
        }
    }
    int ltOrder;
    SkASSERT((lh.fSectorMask & fSectorMask) || (rh.fSectorMask & fSectorMask));
    if (lh.fSectorMask & fSectorMask) {
        ltOrder = (int) lh.orderable(*this);
    } else {
        int ltGap = (fSectorStart - lh.fSectorStart + 32) & 0x1f;
        ltOrder = ltGap > 20 ? 0 : ltGap > 11 ? -1 : 1;
    }
    int trOrder;
    if (rh.fSectorMask & fSectorMask) {
        trOrder = (int) orderable(rh);
    } else {
        int trGap = (rh.fSectorStart - fSectorStart + 32) & 0x1f;
        trOrder = trGap > 20 ? 0 : trGap > 11 ? -1 : 1;
    }
    if (lrOrder >= 0 && ltOrder >= 0 && trOrder >= 0) {
        return COMPARE_RESULT(7, lrOrder ? (ltOrder & trOrder) : (ltOrder | trOrder));
    }
    SkASSERT(lrOrder >= 0 || ltOrder >= 0 || trOrder >= 0);
// There's not enough information to sort. Get the pairs of angles in opposite planes.
// If an order is < 0, the pair is already in an opposite plane. Check the remaining pairs.
    // FIXME : once all variants are understood, rewrite this more simply
    if (ltOrder == 0 && lrOrder == 0) {
        SkASSERT(trOrder < 0);
        // FIXME : once this is verified to work, remove one opposite angle call
        SkDEBUGCODE(bool lrOpposite = lh.oppositePlanes(rh));
        bool ltOpposite = lh.oppositePlanes(*this);
        SkASSERT(lrOpposite != ltOpposite);
        return COMPARE_RESULT(8, ltOpposite);
    } else if (ltOrder == 1 && trOrder == 0) {
        SkASSERT(lrOrder < 0);
        SkDEBUGCODE(bool ltOpposite = lh.oppositePlanes(*this));
        bool trOpposite = oppositePlanes(rh);
        SkASSERT(ltOpposite != trOpposite);
        return COMPARE_RESULT(9, trOpposite);
    } else if (lrOrder == 1 && trOrder == 1) {
        SkASSERT(ltOrder < 0);
        SkDEBUGCODE(bool trOpposite = oppositePlanes(rh));
        bool lrOpposite = lh.oppositePlanes(rh);
        SkASSERT(lrOpposite != trOpposite);
        return COMPARE_RESULT(10, lrOpposite);
    }
    if (lrOrder < 0) {
        if (ltOrder < 0) {
            return COMPARE_RESULT(11, trOrder);
        }
        return COMPARE_RESULT(12, ltOrder);
    }
    return COMPARE_RESULT(13, !lrOrder);
}

// given a line, see if the opposite curve's convex hull is all on one side
// returns -1=not on one side    0=this CW of test   1=this CCW of test
int SkOpAngle::allOnOneSide(const SkOpAngle& test) const {
    SkASSERT(!fIsCurve);
    SkASSERT(test.fIsCurve);
    const SkDPoint& origin = test.fCurvePart[0];
    SkVector line;
    if (fSegment->verb() == SkPath::kLine_Verb) {
        const SkPoint* linePts = fSegment->pts();
        int lineStart = fStart < fEnd ? 0 : 1;
        line = linePts[lineStart ^ 1] - linePts[lineStart];
    } else {
        SkPoint shortPts[2] = { fCurvePart[0].asSkPoint(), fCurvePart[1].asSkPoint() };
        line = shortPts[1] - shortPts[0];
    }
    float crosses[3];
    SkPath::Verb testVerb = test.fSegment->verb();
    int iMax = SkPathOpsVerbToPoints(testVerb);
//    SkASSERT(origin == test.fCurveHalf[0]);
    const SkDCubic& testCurve = test.fCurvePart;
//    do {
        for (int index = 1; index <= iMax; ++index) {
            float xy1 = (float) (line.fX * (testCurve[index].fY - origin.fY));
            float xy2 = (float) (line.fY * (testCurve[index].fX - origin.fX));
            crosses[index - 1] = AlmostEqualUlps(xy1, xy2) ? 0 : xy1 - xy2;
        }
        if (crosses[0] * crosses[1] < 0) {
            return -1;
        }
        if (SkPath::kCubic_Verb == testVerb) {
            if (crosses[0] * crosses[2] < 0 || crosses[1] * crosses[2] < 0) {
                return -1;
            }
        }
        if (crosses[0]) {
            return crosses[0] < 0;
        }
        if (crosses[1]) {
            return crosses[1] < 0;
        }
        if (SkPath::kCubic_Verb == testVerb && crosses[2]) {
            return crosses[2] < 0;
        }
    fUnorderable = true;
    return -1;
}

bool SkOpAngle::calcSlop(double x, double y, double rx, double ry, bool* result) const {
    double absX = fabs(x);
    double absY = fabs(y);
    double length = absX < absY ? absX / 2 + absY : absX + absY / 2;
    int exponent;
    (void) frexp(length, &exponent);
    double epsilon = ldexp(FLT_EPSILON, exponent);
    SkPath::Verb verb = fSegment->verb();
    SkASSERT(verb == SkPath::kQuad_Verb || verb == SkPath::kCubic_Verb);
    // FIXME: the quad and cubic factors are made up ; determine actual values
    double slop = verb == SkPath::kQuad_Verb ? 4 * epsilon : 512 * epsilon;
    double xSlop = slop;
    double ySlop = x * y < 0 ? -xSlop : xSlop; // OPTIMIZATION: use copysign / _copysign ?
    double x1 = x - xSlop;
    double y1 = y + ySlop;
    double x_ry1 = x1 * ry;
    double rx_y1 = rx * y1;
    *result = x_ry1 < rx_y1;
    double x2 = x + xSlop;
    double y2 = y - ySlop;
    double x_ry2 = x2 * ry;
    double rx_y2 = rx * y2;
    bool less2 = x_ry2 < rx_y2;
    return *result == less2;
}

bool SkOpAngle::checkCrossesZero() const {
    int start = SkTMin(fSectorStart, fSectorEnd);
    int end = SkTMax(fSectorStart, fSectorEnd);
    bool crossesZero = end - start > 16;
    return crossesZero;
}

bool SkOpAngle::checkParallel(const SkOpAngle& rh) const {
    SkDVector scratch[2];
    const SkDVector* sweep, * tweep;
    if (!fUnorderedSweep) {
        sweep = fSweep;
    } else {
        scratch[0] = fCurvePart[1] - fCurvePart[0];
        sweep = &scratch[0];
    }
    if (!rh.fUnorderedSweep) {
        tweep = rh.fSweep;
    } else {
        scratch[1] = rh.fCurvePart[1] - rh.fCurvePart[0];
        tweep = &scratch[1];
    }
    double s0xt0 = sweep->crossCheck(*tweep);
    if (tangentsDiverge(rh, s0xt0)) {
        return s0xt0 < 0;
    }
    SkDVector m0 = fSegment->dPtAtT(midT()) - fCurvePart[0];
    SkDVector m1 = rh.fSegment->dPtAtT(rh.midT()) - rh.fCurvePart[0];
    double m0xm1 = m0.crossCheck(m1);
    if (m0xm1 == 0) {
        fUnorderable = true;
        rh.fUnorderable = true;
        return true;
    }
    return m0xm1 < 0;
}

// the original angle is too short to get meaningful sector information
// lengthen it until it is long enough to be meaningful or leave it unset if lengthening it
// would cause it to intersect one of the adjacent angles
bool SkOpAngle::computeSector() {
    if (fComputedSector) {
        return !fUnorderable;
    }
//    SkASSERT(fSegment->verb() != SkPath::kLine_Verb && small());
    fComputedSector = true;
    int step = fStart < fEnd ? 1 : -1;
    int limit = step > 0 ? fSegment->count() : -1;
    int checkEnd = fEnd;
    do {
// advance end
        const SkOpSpan& span = fSegment->span(checkEnd);
        const SkOpSegment* other = span.fOther;
        int oCount = other->count();
        for (int oIndex = 0; oIndex < oCount; ++oIndex) {
            const SkOpSpan& oSpan = other->span(oIndex);
            if (oSpan.fOther != fSegment) {
                continue;
            }
            if (oSpan.fOtherIndex == checkEnd) {
                continue;
            }
            if (!approximately_equal(oSpan.fOtherT, span.fT)) {
                continue;
            }
            goto recomputeSector;
        }
        checkEnd += step;
    } while (checkEnd != limit);
recomputeSector:
    if (checkEnd == fEnd || checkEnd - step == fEnd) {
        fUnorderable = true;
        return false;
    }
    int saveEnd = fEnd;
    fComputedEnd = fEnd = checkEnd - step;
    setSpans();
    setSector();
    fEnd = saveEnd;
    return !fUnorderable;
}

// returns -1 if overlaps   0 if no overlap cw    1 if no overlap ccw
int SkOpAngle::convexHullOverlaps(const SkOpAngle& rh) const {
    const SkDVector* sweep = fSweep;
    const SkDVector* tweep = rh.fSweep;
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
    SkDVector m0 = fSegment->dPtAtT(midT()) - fCurvePart[0];
    SkDVector m1 = rh.fSegment->dPtAtT(rh.midT()) - rh.fCurvePart[0];
    double m0xm1 = m0.crossCheck(m1);
    if (s0xt0 > 0 && m0xm1 > 0) {
        return 0;
    }
    if (s0xt0 < 0 && m0xm1 < 0) {
        return 1;
    }
    if (tangentsDiverge(rh, s0xt0)) {
        return s0xt0 < 0;
    }
    return m0xm1 < 0;
}

// OPTIMIZATION: longest can all be either lazily computed here or precomputed in setup
double SkOpAngle::distEndRatio(double dist) const {
    double longest = 0;
    const SkOpSegment& segment = *this->segment();
    int ptCount = SkPathOpsVerbToPoints(segment.verb());
    const SkPoint* pts = segment.pts();
    for (int idx1 = 0; idx1 <= ptCount - 1; ++idx1) {
        for (int idx2 = idx1 + 1; idx2 <= ptCount; ++idx2) {
            if (idx1 == idx2) {
                continue;
            }
            SkDVector v;
            v.set(pts[idx2] - pts[idx1]);
            double lenSq = v.lengthSquared();
            longest = SkTMax(longest, lenSq);
        }
    }
    return sqrt(longest) / dist;
}

bool SkOpAngle::endsIntersect(const SkOpAngle& rh) const {
    SkPath::Verb lVerb = fSegment->verb();
    SkPath::Verb rVerb = rh.fSegment->verb();
    int lPts = SkPathOpsVerbToPoints(lVerb);
    int rPts = SkPathOpsVerbToPoints(rVerb);
    SkDLine rays[] = {{{fCurvePart[0], rh.fCurvePart[rPts]}},
            {{fCurvePart[0], fCurvePart[lPts]}}};
    if (rays[0][1] == rays[1][1]) {
        return checkParallel(rh);
    }
    double smallTs[2] = {-1, -1};
    bool limited[2] = {false, false};
    for (int index = 0; index < 2; ++index) {
        const SkOpSegment& segment = index ? *rh.fSegment : *fSegment;
        SkIntersections i;
        (*CurveIntersectRay[index ? rPts : lPts])(segment.pts(), rays[index], &i);
//      SkASSERT(i.used() >= 1);
//        if (i.used() <= 1) {
//            continue;
//        }
        double tStart = segment.t(index ? rh.fStart : fStart);
        double tEnd = segment.t(index ? rh.fComputedEnd : fComputedEnd);
        bool testAscends = index ? rh.fStart < rh.fComputedEnd : fStart < fComputedEnd;
        double t = testAscends ? 0 : 1;
        for (int idx2 = 0; idx2 < i.used(); ++idx2) {
            double testT = i[0][idx2];
            if (!approximately_between_orderable(tStart, testT, tEnd)) {
                continue;
            }
            if (approximately_equal_orderable(tStart, testT)) {
                continue;
            }
            smallTs[index] = t = testAscends ? SkTMax(t, testT) : SkTMin(t, testT);
            limited[index] = approximately_equal_orderable(t, tEnd);
        }
    }
#if 0
    if (smallTs[0] < 0 && smallTs[1] < 0) {  // if neither ray intersects, do endpoint sort
        double m0xm1 = 0;
        if (lVerb == SkPath::kLine_Verb) {
            SkASSERT(rVerb != SkPath::kLine_Verb);
            SkDVector m0 = rays[1][1] - fCurvePart[0];
            SkDPoint endPt;
            endPt.set(rh.fSegment->pts()[rh.fStart < rh.fEnd ? rPts : 0]);
            SkDVector m1 = endPt - fCurvePart[0];
            m0xm1 = m0.crossCheck(m1);
        }
        if (rVerb == SkPath::kLine_Verb) {
            SkDPoint endPt;
            endPt.set(fSegment->pts()[fStart < fEnd ? lPts : 0]);
            SkDVector m0 = endPt - fCurvePart[0];
            SkDVector m1 = rays[0][1] - fCurvePart[0];
            m0xm1 = m0.crossCheck(m1);
        }
        if (m0xm1 != 0) {
            return m0xm1 < 0;
        }
    }
#endif
    bool sRayLonger = false;
    SkDVector sCept = {0, 0};
    double sCeptT = -1;
    int sIndex = -1;
    bool useIntersect = false;
    for (int index = 0; index < 2; ++index) {
        if (smallTs[index] < 0) {
            continue;
        }
        const SkOpSegment& segment = index ? *rh.fSegment : *fSegment;
        const SkDPoint& dPt = segment.dPtAtT(smallTs[index]);
        SkDVector cept = dPt - rays[index][0];
        // If this point is on the curve, it should have been detected earlier by ordinary
        // curve intersection. This may be hard to determine in general, but for lines,
        // the point could be close to or equal to its end, but shouldn't be near the start.
        if ((index ? lPts : rPts) == 1) {
            SkDVector total = rays[index][1] - rays[index][0];
            if (cept.lengthSquared() * 2 < total.lengthSquared()) {
                continue;
            }
        }
        SkDVector end = rays[index][1] - rays[index][0];
        if (cept.fX * end.fX < 0 || cept.fY * end.fY < 0) {
            continue;
        }
        double rayDist = cept.length();
        double endDist = end.length();
        bool rayLonger = rayDist > endDist;
        if (limited[0] && limited[1] && rayLonger) {
            useIntersect = true;
            sRayLonger = rayLonger;
            sCept = cept;
            sCeptT = smallTs[index];
            sIndex = index;
            break;
        }
        double delta = fabs(rayDist - endDist);
        double minX, minY, maxX, maxY;
        minX = minY = SK_ScalarInfinity;
        maxX = maxY = -SK_ScalarInfinity;
        const SkDCubic& curve = index ? rh.fCurvePart : fCurvePart;
        int ptCount = index ? rPts : lPts;
        for (int idx2 = 0; idx2 <= ptCount; ++idx2) {
            minX = SkTMin(minX, curve[idx2].fX);
            minY = SkTMin(minY, curve[idx2].fY);
            maxX = SkTMax(maxX, curve[idx2].fX);
            maxY = SkTMax(maxY, curve[idx2].fY);
        }
        double maxWidth = SkTMax(maxX - minX, maxY - minY);
        delta /= maxWidth;
        if (delta > 1e-4 && (useIntersect ^= true)) {  // FIXME: move this magic number
            sRayLonger = rayLonger;
            sCept = cept;
            sCeptT = smallTs[index];
            sIndex = index;
        }
    }
    if (useIntersect) {
        const SkDCubic& curve = sIndex ? rh.fCurvePart : fCurvePart;
        const SkOpSegment& segment = sIndex ? *rh.fSegment : *fSegment;
        double tStart = segment.t(sIndex ? rh.fStart : fStart);
        SkDVector mid = segment.dPtAtT(tStart + (sCeptT - tStart) / 2) - curve[0];
        double septDir = mid.crossCheck(sCept);
        if (!septDir) {
            return checkParallel(rh);
        }
        return sRayLonger ^ (sIndex == 0) ^ (septDir < 0);
    } else {
        return checkParallel(rh);
    }
}

// Most of the time, the first one can be found trivially by detecting the smallest sector value.
// If all angles have the same sector value, actual sorting is required.
const SkOpAngle* SkOpAngle::findFirst() const {
    const SkOpAngle* best = this;
    int bestStart = SkTMin(fSectorStart, fSectorEnd);
    const SkOpAngle* angle = this;
    while ((angle = angle->fNext) != this) {
        int angleEnd = SkTMax(angle->fSectorStart, angle->fSectorEnd);
        if (angleEnd < bestStart) {
            return angle;    // we wrapped around
        }
        int angleStart = SkTMin(angle->fSectorStart, angle->fSectorEnd);
        if (bestStart > angleStart) {
            best = angle;
            bestStart = angleStart;
        }
    }
    // back up to the first possible angle
    const SkOpAngle* firstBest = best;
    angle = best;
    int bestEnd = SkTMax(best->fSectorStart, best->fSectorEnd);
    while ((angle = angle->previous()) != firstBest) {
        if (angle->fStop) {
            break;
        }
        int angleStart = SkTMin(angle->fSectorStart, angle->fSectorEnd);
        // angles that are smaller by one aren't necessary better, since the larger may be a line
        // and the smaller may be a curve that curls to the other side of the line.
        if (bestEnd + 1 < angleStart) {
            return best;
        }
        best = angle;
        bestEnd = SkTMax(angle->fSectorStart, angle->fSectorEnd);
    }
    // in the case where all angles are nearly in the same sector, check the order to find the best
    firstBest = best;
    angle = best;
    do {
        angle = angle->fNext;
        if (angle->fStop) {
            return firstBest;
        }
        bool orderable = best->orderable(*angle);  // note: may return an unorderable angle
        if (orderable == 0) {
            return angle;
        }
        best = angle;
    } while (angle != firstBest);
    // if the angles are equally ordered, fall back on the initial tangent
    bool foundBelow = false;
    while ((angle = angle->fNext)) {
        SkDVector scratch[2];
        const SkDVector* sweep;
        if (!angle->fUnorderedSweep) {
            sweep = angle->fSweep;
        } else {
            scratch[0] = angle->fCurvePart[1] - angle->fCurvePart[0];
            sweep = &scratch[0];
        }
        bool isAbove = sweep->fY <= 0;
        if (isAbove && foundBelow) {
            return angle;
        }
        foundBelow |= !isAbove;
        if (angle == firstBest) {
            return NULL; // should not loop around
        }
    }
    SkASSERT(0);  // should never get here
    return NULL;
}

/*      y<0 y==0 y>0  x<0 x==0 x>0 xy<0 xy==0 xy>0
    0    x                      x               x
    1    x                      x          x
    2    x                      x    x
    3    x                  x        x
    4    x             x             x
    5    x             x                   x
    6    x             x                        x
    7         x        x                        x
    8             x    x                        x
    9             x    x                   x
    10            x    x             x
    11            x         x        x
    12            x             x    x
    13            x             x          x
    14            x             x               x
    15        x                 x               x
*/
int SkOpAngle::findSector(SkPath::Verb verb, double x, double y) const {
    double absX = fabs(x);
    double absY = fabs(y);
    double xy = SkPath::kLine_Verb == verb || !AlmostEqualUlps(absX, absY) ? absX - absY : 0;
    // If there are four quadrants and eight octants, and since the Latin for sixteen is sedecim,
    // one could coin the term sedecimant for a space divided into 16 sections.
   // http://english.stackexchange.com/questions/133688/word-for-something-partitioned-into-16-parts
    static const int sedecimant[3][3][3] = {
    //       y<0           y==0           y>0
    //   x<0 x==0 x>0  x<0 x==0 x>0  x<0 x==0 x>0
        {{ 4,  3,  2}, { 7, -1, 15}, {10, 11, 12}},  // abs(x) <  abs(y)
        {{ 5, -1,  1}, {-1, -1, -1}, { 9, -1, 13}},  // abs(x) == abs(y)
        {{ 6,  3,  0}, { 7, -1, 15}, { 8, 11, 14}},  // abs(x) >  abs(y)
    };
    int sector = sedecimant[(xy >= 0) + (xy > 0)][(y >= 0) + (y > 0)][(x >= 0) + (x > 0)] * 2 + 1;
//    SkASSERT(SkPath::kLine_Verb == verb || sector >= 0);
    return sector;
}

// OPTIMIZE: if this loops to only one other angle, after first compare fails, insert on other side
// OPTIMIZE: return where insertion succeeded. Then, start next insertion on opposite side
void SkOpAngle::insert(SkOpAngle* angle) {
    if (angle->fNext) {
        if (loopCount() >= angle->loopCount()) {
            if (!merge(angle)) {
                return;
            }
        } else if (fNext) {
            if (!angle->merge(this)) {
                return;
            }
        } else {
            angle->insert(this);
        }
        return;
    }
    bool singleton = NULL == fNext;
    if (singleton) {
        fNext = this;
    }
    SkOpAngle* next = fNext;
    if (next->fNext == this) {
        if (angle->overlap(*this)) {
            return;
        }
        if (singleton || angle->after(this)) {
            this->fNext = angle;
            angle->fNext = next;
        } else {
            next->fNext = angle;
            angle->fNext = this;
        }
        debugValidateNext();
        return;
    }
    SkOpAngle* last = this;
    do {
        SkASSERT(last->fNext == next);
        if (angle->overlap(*last) || angle->overlap(*next)) {
            return;
        }
        if (angle->after(last)) {
            last->fNext = angle;
            angle->fNext = next;
            debugValidateNext();
            return;
        }
        last = next;
        next = next->fNext;
        if (last == this && next->fUnorderable) {
            fUnorderable = true;
            return;
        }
        SkASSERT(last != this);
    } while (true);
}

bool SkOpAngle::isHorizontal() const {
    return !fIsCurve && fSweep[0].fY == 0;
}

SkOpSpan* SkOpAngle::lastMarked() const {
    if (fLastMarked) {
        if (fLastMarked->fChased) {
            return NULL;
        }
        fLastMarked->fChased = true;
    }
    return fLastMarked;
}

bool SkOpAngle::loopContains(const SkOpAngle& test) const {
    if (!fNext) {
        return false;
    }
    const SkOpAngle* first = this;
    const SkOpAngle* loop = this;
    const SkOpSegment* tSegment = test.fSegment;
    double tStart = tSegment->span(test.fStart).fT;
    double tEnd = tSegment->span(test.fEnd).fT;
    do {
        const SkOpSegment* lSegment = loop->fSegment;
        // FIXME : use precisely_equal ? or compare points exactly ?
        if (lSegment != tSegment) {
            continue;
        }
        double lStart = lSegment->span(loop->fStart).fT;
        if (lStart != tEnd) {
            continue;
        }
        double lEnd = lSegment->span(loop->fEnd).fT;
        if (lEnd == tStart) {
            return true;
        }
    } while ((loop = loop->fNext) != first);
    return false;
}

int SkOpAngle::loopCount() const {
    int count = 0;
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    do {
        next = next->fNext;
        ++count;
    } while (next && next != first);
    return count;
}

// OPTIMIZATION: can this be done better in after when angles are sorted?
void SkOpAngle::markStops() {
    SkOpAngle* angle = this;
    int lastEnd = SkTMax(fSectorStart, fSectorEnd);
    do {
        angle = angle->fNext;
        int angleStart = SkTMin(angle->fSectorStart, angle->fSectorEnd);
        // angles that are smaller by one aren't necessary better, since the larger may be a line
        // and the smaller may be a curve that curls to the other side of the line.
        if (lastEnd + 1 < angleStart) {
            angle->fStop = true;
        }
        lastEnd = SkTMax(angle->fSectorStart, angle->fSectorEnd);
    } while (angle != this);
}

bool SkOpAngle::merge(SkOpAngle* angle) {
    SkASSERT(fNext);
    SkASSERT(angle->fNext);
    SkOpAngle* working = angle;
    do {
        if (this == working) {
            return false;
        }
        working = working->fNext;
    } while (working != angle);
    do {
        SkOpAngle* next = working->fNext;
        working->fNext = NULL;
        insert(working);
        working = next;
    } while (working != angle);
    // it's likely that a pair of the angles are unorderable
#if DEBUG_ANGLE
    SkOpAngle* last = angle;
    working = angle->fNext;
    do {
        SkASSERT(last->fNext == working);
        last->fNext = working->fNext;
        SkASSERT(working->after(last));
        last->fNext = working;
        last = working;
        working = working->fNext;
    } while (last != angle);
#endif
    debugValidateNext();
    return true;
}

double SkOpAngle::midT() const {
    return (fSegment->t(fStart) + fSegment->t(fEnd)) / 2;
}

bool SkOpAngle::oppositePlanes(const SkOpAngle& rh) const {
    int startSpan = abs(rh.fSectorStart - fSectorStart);
    return startSpan >= 8;
}

bool SkOpAngle::orderable(const SkOpAngle& rh) const {
    int result;
    if (!fIsCurve) {
        if (!rh.fIsCurve) {
            double leftX = fTangentHalf.dx();
            double leftY = fTangentHalf.dy();
            double rightX = rh.fTangentHalf.dx();
            double rightY = rh.fTangentHalf.dy();
            double x_ry = leftX * rightY;
            double rx_y = rightX * leftY;
            if (x_ry == rx_y) {
                if (leftX * rightX < 0 || leftY * rightY < 0) {
                    return true;  // exactly 180 degrees apart
                }
                goto unorderable;
            }
            SkASSERT(x_ry != rx_y); // indicates an undetected coincidence -- worth finding earlier
            return x_ry < rx_y;
        }
        if ((result = allOnOneSide(rh)) >= 0) {
            return result;
        }
        if (fUnorderable || approximately_zero(rh.fSide)) {
            goto unorderable;
        }
    } else if (!rh.fIsCurve) {
        if ((result = rh.allOnOneSide(*this)) >= 0) {
            return !result;
        }
        if (rh.fUnorderable || approximately_zero(fSide)) {
            goto unorderable;
        }
    }
    if ((result = convexHullOverlaps(rh)) >= 0) {
        return result;
    }
    return endsIntersect(rh);
unorderable:
    fUnorderable = true;
    rh.fUnorderable = true;
    return true;
}

bool SkOpAngle::overlap(const SkOpAngle& other) const {
    int min = SkTMin(fStart, fEnd);
    const SkOpSpan& span = fSegment->span(min);
    const SkOpSegment* oSeg = other.fSegment;
    int oMin = SkTMin(other.fStart, other.fEnd);
    const SkOpSpan& oSpan = oSeg->span(oMin);
    if (!span.fSmall && !oSpan.fSmall) {
        return false;
    }
    if (fSegment->span(fStart).fPt != oSeg->span(other.fStart).fPt) {
        return false;
    }
    // see if small span is contained by opposite span
    return span.fSmall ? oSeg->containsPt(fSegment->span(fEnd).fPt, other.fEnd, other.fStart)
            : fSegment->containsPt(oSeg->span(other.fEnd).fPt, fEnd, fStart);
}

// OPTIMIZE: if this shows up in a profile, add a previous pointer
// as is, this should be rarely called
SkOpAngle* SkOpAngle::previous() const {
    SkOpAngle* last = fNext;
    do {
        SkOpAngle* next = last->fNext;
        if (next == this) {
            return last;
        }
        last = next;
    } while (true);
}

void SkOpAngle::set(const SkOpSegment* segment, int start, int end) {
    fSegment = segment;
    fStart = start;
    fComputedEnd = fEnd = end;
    fNext = NULL;
    fComputeSector = fComputedSector = false;
    fStop = false;
    setSpans();
    setSector();
}

void SkOpAngle::setCurveHullSweep() {
    fUnorderedSweep = false;
    fSweep[0] = fCurvePart[1] - fCurvePart[0];
    if (SkPath::kLine_Verb == fSegment->verb()) {
        fSweep[1] = fSweep[0];
        return;
    }
    fSweep[1] = fCurvePart[2] - fCurvePart[0];
    if (SkPath::kCubic_Verb != fSegment->verb()) {
        if (!fSweep[0].fX && !fSweep[0].fY) {
            fSweep[0] = fSweep[1];
        }
        return;
    }
    SkDVector thirdSweep = fCurvePart[3] - fCurvePart[0];
    if (fSweep[0].fX == 0 && fSweep[0].fY == 0) {
        fSweep[0] = fSweep[1];
        fSweep[1] = thirdSweep;
        if (fSweep[0].fX == 0 && fSweep[0].fY == 0) {
            fSweep[0] = fSweep[1];
            fCurvePart[1] = fCurvePart[3];
            fIsCurve = false;
        }
        return;
    }
    double s1x3 = fSweep[0].crossCheck(thirdSweep);
    double s3x2 = thirdSweep.crossCheck(fSweep[1]);
    if (s1x3 * s3x2 >= 0) {  // if third vector is on or between first two vectors
        return;
    }
    double s2x1 = fSweep[1].crossCheck(fSweep[0]);
    // FIXME: If the sweep of the cubic is greater than 180 degrees, we're in trouble
    // probably such wide sweeps should be artificially subdivided earlier so that never happens
    SkASSERT(s1x3 * s2x1 < 0 || s1x3 * s3x2 < 0);
    if (s3x2 * s2x1 < 0) {
        SkASSERT(s2x1 * s1x3 > 0);
        fSweep[0] = fSweep[1];
        fUnorderedSweep = true;
    }
    fSweep[1] = thirdSweep;
}

void SkOpAngle::setSector() {
    SkPath::Verb verb = fSegment->verb();
    if (SkPath::kLine_Verb != verb && small()) {
        goto deferTilLater;
    }
    fSectorStart = findSector(verb, fSweep[0].fX, fSweep[0].fY);
    if (fSectorStart < 0) {
        goto deferTilLater;
    }
    if (!fIsCurve) {  // if it's a line or line-like, note that both sectors are the same
        SkASSERT(fSectorStart >= 0);
        fSectorEnd = fSectorStart;
        fSectorMask = 1 << fSectorStart;
        return;
    }
    SkASSERT(SkPath::kLine_Verb != verb);
    fSectorEnd = findSector(verb, fSweep[1].fX, fSweep[1].fY);
    if (fSectorEnd < 0) {
deferTilLater:
        fSectorStart = fSectorEnd = -1;
        fSectorMask = 0;
        fComputeSector = true;  // can't determine sector until segment length can be found
        return;
    }
    if (fSectorEnd == fSectorStart) {
        SkASSERT((fSectorStart & 3) != 3);  // if the sector has no span, it can't be an exact angle
        fSectorMask = 1 << fSectorStart;
        return;
    }
    bool crossesZero = checkCrossesZero();
    int start = SkTMin(fSectorStart, fSectorEnd);
    bool curveBendsCCW = (fSectorStart == start) ^ crossesZero;
    // bump the start and end of the sector span if they are on exact compass points
    if ((fSectorStart & 3) == 3) {
        fSectorStart = (fSectorStart + (curveBendsCCW ? 1 : 31)) & 0x1f;
    }
    if ((fSectorEnd & 3) == 3) {
        fSectorEnd = (fSectorEnd + (curveBendsCCW ? 31 : 1)) & 0x1f;
    }
    crossesZero = checkCrossesZero();
    start = SkTMin(fSectorStart, fSectorEnd);
    int end = SkTMax(fSectorStart, fSectorEnd);
    if (!crossesZero) {
        fSectorMask = (unsigned) -1 >> (31 - end + start) << start;
    } else {
        fSectorMask = (unsigned) -1 >> (31 - start) | (-1 << end);
    }
}

void SkOpAngle::setSpans() {
    fUnorderable = fSegment->isTiny(this);
    fLastMarked = NULL;
    const SkPoint* pts = fSegment->pts();
    SkDEBUGCODE(fCurvePart[2].fX = fCurvePart[2].fY = fCurvePart[3].fX = fCurvePart[3].fY
            = SK_ScalarNaN);
    fSegment->subDivide(fStart, fEnd, &fCurvePart);
    setCurveHullSweep();
    const SkPath::Verb verb = fSegment->verb();
    if (verb != SkPath::kLine_Verb
            && !(fIsCurve = fSweep[0].crossCheck(fSweep[1]) != 0)) {
        SkDLine lineHalf;
        lineHalf[0].set(fCurvePart[0].asSkPoint());
        lineHalf[1].set(fCurvePart[SkPathOpsVerbToPoints(verb)].asSkPoint());
        fTangentHalf.lineEndPoints(lineHalf);
        fSide = 0;
    }
    switch (verb) {
    case SkPath::kLine_Verb: {
        SkASSERT(fStart != fEnd);
        const SkPoint& cP1 = pts[fStart < fEnd];
        SkDLine lineHalf;
        lineHalf[0].set(fSegment->span(fStart).fPt);
        lineHalf[1].set(cP1);
        fTangentHalf.lineEndPoints(lineHalf);
        fSide = 0;
        fIsCurve = false;
        } return;
    case SkPath::kQuad_Verb: {
        SkLineParameters tangentPart;
        SkDQuad& quad2 = *SkTCast<SkDQuad*>(&fCurvePart);
        (void) tangentPart.quadEndPoints(quad2);
        fSide = -tangentPart.pointDistance(fCurvePart[2]);  // not normalized -- compare sign only
        } break;
    case SkPath::kCubic_Verb: {
        SkLineParameters tangentPart;
        (void) tangentPart.cubicPart(fCurvePart);
        fSide = -tangentPart.pointDistance(fCurvePart[3]);
        double testTs[4];
        // OPTIMIZATION: keep inflections precomputed with cubic segment?
        int testCount = SkDCubic::FindInflections(pts, testTs);
        double startT = fSegment->t(fStart);
        double endT = fSegment->t(fEnd);
        double limitT = endT;
        int index;
        for (index = 0; index < testCount; ++index) {
            if (!::between(startT, testTs[index], limitT)) {
                testTs[index] = -1;
            }
        }
        testTs[testCount++] = startT;
        testTs[testCount++] = endT;
        SkTQSort<double>(testTs, &testTs[testCount - 1]);
        double bestSide = 0;
        int testCases = (testCount << 1) - 1;
        index = 0;
        while (testTs[index] < 0) {
            ++index;
        }
        index <<= 1;
        for (; index < testCases; ++index) {
            int testIndex = index >> 1;
            double testT = testTs[testIndex];
            if (index & 1) {
                testT = (testT + testTs[testIndex + 1]) / 2;
            }
            // OPTIMIZE: could avoid call for t == startT, endT
            SkDPoint pt = dcubic_xy_at_t(pts, testT);
            SkLineParameters tangentPart;
            tangentPart.cubicEndPoints(fCurvePart);
            double testSide = tangentPart.pointDistance(pt);
            if (fabs(bestSide) < fabs(testSide)) {
                bestSide = testSide;
            }
        }
        fSide = -bestSide;  // compare sign only
        } break;
    default:
        SkASSERT(0);
    }
}

bool SkOpAngle::small() const {
    int min = SkMin32(fStart, fEnd);
    int max = SkMax32(fStart, fEnd);
    for (int index = min; index < max; ++index) {
        const SkOpSpan& mSpan = fSegment->span(index);
        if (!mSpan.fSmall) {
            return false;
        }
    }
    return true;
}

bool SkOpAngle::tangentsDiverge(const SkOpAngle& rh, double s0xt0) const {
    if (s0xt0 == 0) {
        return false;
    }
    // if the ctrl tangents are not nearly parallel, use them
    // solve for opposite direction displacement scale factor == m
    // initial dir = v1.cross(v2) == v2.x * v1.y - v2.y * v1.x
    // displacement of q1[1] : dq1 = { -m * v1.y, m * v1.x } + q1[1]
    // straight angle when : v2.x * (dq1.y - q1[0].y) == v2.y * (dq1.x - q1[0].x)
    //                       v2.x * (m * v1.x + v1.y) == v2.y * (-m * v1.y + v1.x)
    // - m * (v2.x * v1.x + v2.y * v1.y) == v2.x * v1.y - v2.y * v1.x
    // m = (v2.y * v1.x - v2.x * v1.y) / (v2.x * v1.x + v2.y * v1.y)
    // m = v1.cross(v2) / v1.dot(v2)
    const SkDVector* sweep = fSweep;
    const SkDVector* tweep = rh.fSweep;
    double s0dt0 = sweep[0].dot(tweep[0]);
    if (!s0dt0) {
        return true;
    }
    SkASSERT(s0dt0 != 0);
    double m = s0xt0 / s0dt0;
    double sDist = sweep[0].length() * m;
    double tDist = tweep[0].length() * m;
    bool useS = fabs(sDist) < fabs(tDist);
    double mFactor = fabs(useS ? distEndRatio(sDist) : rh.distEndRatio(tDist));
    return mFactor < 5000;  // empirically found limit
}

SkOpAngleSet::SkOpAngleSet() 
    : fAngles(NULL)
#if DEBUG_ANGLE
    , fCount(0)
#endif
{
}

SkOpAngleSet::~SkOpAngleSet() {
    SkDELETE(fAngles);
}

SkOpAngle& SkOpAngleSet::push_back() {
    if (!fAngles) {
        fAngles = SkNEW_ARGS(SkChunkAlloc, (2));
    }
    void* ptr = fAngles->allocThrow(sizeof(SkOpAngle));
    SkOpAngle* angle = (SkOpAngle*) ptr;
#if DEBUG_ANGLE
    angle->setID(++fCount);
#endif
    return *angle;
}

void SkOpAngleSet::reset() {
    if (fAngles) {
        fAngles->reset();
    }
}
