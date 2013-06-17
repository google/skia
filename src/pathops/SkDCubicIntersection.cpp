/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkIntersections.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsPoint.h"
#include "SkPathOpsQuad.h"
#include "SkPathOpsRect.h"
#include "SkReduceOrder.h"
#include "SkTSort.h"

#if ONE_OFF_DEBUG
static const double tLimits1[2][2] = {{0.36, 0.37}, {0.63, 0.64}};
static const double tLimits2[2][2] = {{-0.865211397, -0.865215212}, {-0.865207696, -0.865208078}};
#endif

#define DEBUG_QUAD_PART 0
#define SWAP_TOP_DEBUG 0

static const int kCubicToQuadSubdivisionDepth = 8; // slots reserved for cubic to quads subdivision

static int quadPart(const SkDCubic& cubic, double tStart, double tEnd, SkReduceOrder* reducer) {
    SkDCubic part = cubic.subDivide(tStart, tEnd);
    SkDQuad quad = part.toQuad();
    // FIXME: should reduceOrder be looser in this use case if quartic is going to blow up on an
    // extremely shallow quadratic?
    int order = reducer->reduce(quad, SkReduceOrder::kFill_Style);
#if DEBUG_QUAD_PART
    SkDebugf("%s cubic=(%1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g)"
            " t=(%1.17g,%1.17g)\n", __FUNCTION__, cubic[0].fX, cubic[0].fY,
            cubic[1].fX, cubic[1].fY, cubic[2].fX, cubic[2].fY,
            cubic[3].fX, cubic[3].fY, tStart, tEnd);
    SkDebugf("%s part=(%1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g)"
            " quad=(%1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g)\n", __FUNCTION__,
            part[0].fX, part[0].fY, part[1].fX, part[1].fY, part[2].fX, part[2].fY,
            part[3].fX, part[3].fY, quad[0].fX, quad[0].fY,
            quad[1].fX, quad[1].fY, quad[2].fX, quad[2].fY);
    SkDebugf("%s simple=(%1.17g,%1.17g", __FUNCTION__, reducer->fQuad[0].fX, reducer->fQuad[0].fY);
    if (order > 1) {
        SkDebugf(" %1.17g,%1.17g", reducer->fQuad[1].fX, reducer->fQuad[1].fY);
    }
    if (order > 2) {
        SkDebugf(" %1.17g,%1.17g", reducer->fQuad[2].fX, reducer->fQuad[2].fY);
    }
    SkDebugf(")\n");
    SkASSERT(order < 4 && order > 0);
#endif
    return order;
}

static void intersectWithOrder(const SkDQuad& simple1, int order1, const SkDQuad& simple2,
        int order2, SkIntersections& i) {
    if (order1 == 3 && order2 == 3) {
        i.intersect(simple1, simple2);
    } else if (order1 <= 2 && order2 <= 2) {
        i.intersect((const SkDLine&) simple1, (const SkDLine&) simple2);
    } else if (order1 == 3 && order2 <= 2) {
        i.intersect(simple1, (const SkDLine&) simple2);
    } else {
        SkASSERT(order1 <= 2 && order2 == 3);
        i.intersect(simple2, (const SkDLine&) simple1);
        i.swapPts();
    }
}

// this flavor centers potential intersections recursively. In contrast, '2' may inadvertently
// chase intersections near quadratic ends, requiring odd hacks to find them.
static void intersect(const SkDCubic& cubic1, double t1s, double t1e, const SkDCubic& cubic2,
        double t2s, double t2e, double precisionScale, SkIntersections& i) {
    i.upDepth();
    SkDCubic c1 = cubic1.subDivide(t1s, t1e);
    SkDCubic c2 = cubic2.subDivide(t2s, t2e);
    SkSTArray<kCubicToQuadSubdivisionDepth, double, true> ts1;
    // OPTIMIZE: if c1 == c2, call once (happens when detecting self-intersection)
    c1.toQuadraticTs(c1.calcPrecision() * precisionScale, &ts1);
    SkSTArray<kCubicToQuadSubdivisionDepth, double, true> ts2;
    c2.toQuadraticTs(c2.calcPrecision() * precisionScale, &ts2);
    double t1Start = t1s;
    int ts1Count = ts1.count();
    for (int i1 = 0; i1 <= ts1Count; ++i1) {
        const double tEnd1 = i1 < ts1Count ? ts1[i1] : 1;
        const double t1 = t1s + (t1e - t1s) * tEnd1;
        SkReduceOrder s1;
        int o1 = quadPart(cubic1, t1Start, t1, &s1);
        double t2Start = t2s;
        int ts2Count = ts2.count();
        for (int i2 = 0; i2 <= ts2Count; ++i2) {
            const double tEnd2 = i2 < ts2Count ? ts2[i2] : 1;
            const double t2 = t2s + (t2e - t2s) * tEnd2;
            if (&cubic1 == &cubic2 && t1Start >= t2Start) {
                t2Start = t2;
                continue;
            }
            SkReduceOrder s2;
            int o2 = quadPart(cubic2, t2Start, t2, &s2);
        #if ONE_OFF_DEBUG
            char tab[] = "                  ";
            if (tLimits1[0][0] >= t1Start && tLimits1[0][1] <= t1
                    && tLimits1[1][0] >= t2Start && tLimits1[1][1] <= t2) {
                SkDebugf("%.*s %s t1=(%1.9g,%1.9g) t2=(%1.9g,%1.9g)", i.depth()*2, tab,
                        __FUNCTION__, t1Start, t1, t2Start, t2);
                SkIntersections xlocals;
                intersectWithOrder(s1.fQuad, o1, s2.fQuad, o2, xlocals);
                SkDebugf(" xlocals.fUsed=%d\n", xlocals.used());
            }
        #endif
            SkIntersections locals;
            intersectWithOrder(s1.fQuad, o1, s2.fQuad, o2, locals);
            double coStart[2] = { -1 };
            SkDPoint coPoint;
            int tCount = locals.used();
            for (int tIdx = 0; tIdx < tCount; ++tIdx) {
                double to1 = t1Start + (t1 - t1Start) * locals[0][tIdx];
                double to2 = t2Start + (t2 - t2Start) * locals[1][tIdx];
    // if the computed t is not sufficiently precise, iterate
                SkDPoint p1 = cubic1.xyAtT(to1);
                SkDPoint p2 = cubic2.xyAtT(to2);
                if (p1.approximatelyEqual(p2)) {
                    if (locals.isCoincident(tIdx)) {
                        if (coStart[0] < 0) {
                            coStart[0] = to1;
                            coStart[1] = to2;
                            coPoint = p1;
                        } else {
                            i.insertCoincidentPair(coStart[0], to1, coStart[1], to2, coPoint, p1);
                            coStart[0] = -1;
                        }
                    } else if (&cubic1 != &cubic2 || !approximately_equal(to1, to2)) {
                        if (i.swapped()) {  //  FIXME: insert should respect swap
                            i.insert(to2, to1, p1);
                        } else {
                            i.insert(to1, to2, p1);
                        }
                    }
                } else {
                    double offset = precisionScale / 16;  // FIME: const is arbitrary: test, refine
                    double c1Bottom = tIdx == 0 ? 0 :
                            (t1Start + (t1 - t1Start) * locals[0][tIdx - 1] + to1) / 2;
                    double c1Min = SkTMax(c1Bottom, to1 - offset);
                    double c1Top = tIdx == tCount - 1 ? 1 :
                            (t1Start + (t1 - t1Start) * locals[0][tIdx + 1] + to1) / 2;
                    double c1Max = SkTMin(c1Top, to1 + offset);
                    double c2Min = SkTMax(0., to2 - offset);
                    double c2Max = SkTMin(1., to2 + offset);
                #if ONE_OFF_DEBUG
                    SkDebugf("%.*s %s 1 contains1=%d/%d contains2=%d/%d\n", i.depth()*2, tab,
                            __FUNCTION__,
                            c1Min <= tLimits1[0][1] && tLimits1[0][0] <= c1Max
                         && c2Min <= tLimits1[1][1] && tLimits1[1][0] <= c2Max,
                            to1 - offset <= tLimits1[0][1] && tLimits1[0][0] <= to1 + offset
                         && to2 - offset <= tLimits1[1][1] && tLimits1[1][0] <= to2 + offset,
                            c1Min <= tLimits2[0][1] && tLimits2[0][0] <= c1Max
                         && c2Min <= tLimits2[1][1] && tLimits2[1][0] <= c2Max,
                            to1 - offset <= tLimits2[0][1] && tLimits2[0][0] <= to1 + offset
                         && to2 - offset <= tLimits2[1][1] && tLimits2[1][0] <= to2 + offset);
                    SkDebugf("%.*s %s 1 c1Bottom=%1.9g c1Top=%1.9g c2Bottom=%1.9g c2Top=%1.9g"
                            " 1-o=%1.9g 1+o=%1.9g 2-o=%1.9g 2+o=%1.9g offset=%1.9g\n",
                            i.depth()*2, tab, __FUNCTION__, c1Bottom, c1Top, 0., 1.,
                            to1 - offset, to1 + offset, to2 - offset, to2 + offset, offset);
                    SkDebugf("%.*s %s 1 to1=%1.9g to2=%1.9g c1Min=%1.9g c1Max=%1.9g c2Min=%1.9g"
                            " c2Max=%1.9g\n", i.depth()*2, tab, __FUNCTION__, to1, to2, c1Min,
                            c1Max, c2Min, c2Max);
                #endif
                    intersect(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                #if ONE_OFF_DEBUG
                    SkDebugf("%.*s %s 1 i.used=%d t=%1.9g\n", i.depth()*2, tab, __FUNCTION__,
                            i.used(), i.used() > 0 ? i[0][i.used() - 1] : -1);
                #endif
                    if (tCount > 1) {
                        c1Min = SkTMax(0., to1 - offset);
                        c1Max = SkTMin(1., to1 + offset);
                        double c2Bottom = tIdx == 0 ? to2 :
                                (t2Start + (t2 - t2Start) * locals[1][tIdx - 1] + to2) / 2;
                        double c2Top = tIdx == tCount - 1 ? to2 :
                                (t2Start + (t2 - t2Start) * locals[1][tIdx + 1] + to2) / 2;
                        if (c2Bottom > c2Top) {
                            SkTSwap(c2Bottom, c2Top);
                        }
                        if (c2Bottom == to2) {
                            c2Bottom = 0;
                        }
                        if (c2Top == to2) {
                            c2Top = 1;
                        }
                        c2Min = SkTMax(c2Bottom, to2 - offset);
                        c2Max = SkTMin(c2Top, to2 + offset);
                    #if ONE_OFF_DEBUG
                        SkDebugf("%.*s %s 2 contains1=%d/%d contains2=%d/%d\n", i.depth()*2, tab,
                            __FUNCTION__,
                            c1Min <= tLimits1[0][1] && tLimits1[0][0] <= c1Max
                         && c2Min <= tLimits1[1][1] && tLimits1[1][0] <= c2Max,
                            to1 - offset <= tLimits1[0][1] && tLimits1[0][0] <= to1 + offset
                         && to2 - offset <= tLimits1[1][1] && tLimits1[1][0] <= to2 + offset,
                            c1Min <= tLimits2[0][1] && tLimits2[0][0] <= c1Max
                         && c2Min <= tLimits2[1][1] && tLimits2[1][0] <= c2Max,
                            to1 - offset <= tLimits2[0][1] && tLimits2[0][0] <= to1 + offset
                         && to2 - offset <= tLimits2[1][1] && tLimits2[1][0] <= to2 + offset);
                        SkDebugf("%.*s %s 2 c1Bottom=%1.9g c1Top=%1.9g c2Bottom=%1.9g c2Top=%1.9g"
                                " 1-o=%1.9g 1+o=%1.9g 2-o=%1.9g 2+o=%1.9g offset=%1.9g\n",
                                i.depth()*2, tab, __FUNCTION__, 0., 1., c2Bottom, c2Top,
                                to1 - offset, to1 + offset, to2 - offset, to2 + offset, offset);
                        SkDebugf("%.*s %s 2 to1=%1.9g to2=%1.9g c1Min=%1.9g c1Max=%1.9g c2Min=%1.9g"
                                " c2Max=%1.9g\n", i.depth()*2, tab, __FUNCTION__, to1, to2, c1Min,
                                c1Max, c2Min, c2Max);
                    #endif
                        intersect(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                #if ONE_OFF_DEBUG
                    SkDebugf("%.*s %s 2 i.used=%d t=%1.9g\n", i.depth()*2, tab, __FUNCTION__,
                            i.used(), i.used() > 0 ? i[0][i.used() - 1] : -1);
                #endif
                        c1Min = SkTMax(c1Bottom, to1 - offset);
                        c1Max = SkTMin(c1Top, to1 + offset);
                    #if ONE_OFF_DEBUG
                        SkDebugf("%.*s %s 3 contains1=%d/%d contains2=%d/%d\n", i.depth()*2, tab,
                        __FUNCTION__,
                            c1Min <= tLimits1[0][1] && tLimits1[0][0] <= c1Max
                         && c2Min <= tLimits1[1][1] && tLimits1[1][0] <= c2Max,
                            to1 - offset <= tLimits1[0][1] && tLimits1[0][0] <= to1 + offset
                         && to2 - offset <= tLimits1[1][1] && tLimits1[1][0] <= to2 + offset,
                            c1Min <= tLimits2[0][1] && tLimits2[0][0] <= c1Max
                         && c2Min <= tLimits2[1][1] && tLimits2[1][0] <= c2Max,
                            to1 - offset <= tLimits2[0][1] && tLimits2[0][0] <= to1 + offset
                         && to2 - offset <= tLimits2[1][1] && tLimits2[1][0] <= to2 + offset);
                        SkDebugf("%.*s %s 3 c1Bottom=%1.9g c1Top=%1.9g c2Bottom=%1.9g c2Top=%1.9g"
                                " 1-o=%1.9g 1+o=%1.9g 2-o=%1.9g 2+o=%1.9g offset=%1.9g\n",
                                i.depth()*2, tab, __FUNCTION__, 0., 1., c2Bottom, c2Top,
                                to1 - offset, to1 + offset, to2 - offset, to2 + offset, offset);
                        SkDebugf("%.*s %s 3 to1=%1.9g to2=%1.9g c1Min=%1.9g c1Max=%1.9g c2Min=%1.9g"
                                " c2Max=%1.9g\n", i.depth()*2, tab, __FUNCTION__, to1, to2, c1Min,
                                c1Max, c2Min, c2Max);
                    #endif
                        intersect(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                #if ONE_OFF_DEBUG
                    SkDebugf("%.*s %s 3 i.used=%d t=%1.9g\n", i.depth()*2, tab, __FUNCTION__,
                            i.used(), i.used() > 0 ? i[0][i.used() - 1] : -1);
                #endif
                    }
                    intersect(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                    // FIXME: if no intersection is found, either quadratics intersected where
                    // cubics did not, or the intersection was missed. In the former case, expect
                    // the quadratics to be nearly parallel at the point of intersection, and check
                    // for that.
                }
            }
            SkASSERT(coStart[0] == -1);
            t2Start = t2;
        }
        t1Start = t1;
    }
    i.downDepth();
}

#define LINE_FRACTION 0.1

// intersect the end of the cubic with the other. Try lines from the end to control and opposite
// end to determine range of t on opposite cubic.
static void intersectEnd(const SkDCubic& cubic1, bool start, const SkDCubic& cubic2,
                         const SkDRect& bounds2, SkIntersections& i) {
    SkDLine line;
    int t1Index = start ? 0 : 3;
    // don't bother if the two cubics are connnected
#if 1
    static const int kPointsInCubic = 4; // FIXME: move to DCubic, replace '4' with this
    static const int kMaxLineCubicIntersections = 3;
    SkSTArray<(kMaxLineCubicIntersections - 1) * kMaxLineCubicIntersections, double, true> tVals;
    line[0] = cubic1[t1Index];
    // this variant looks for intersections with the end point and lines parallel to other points
    for (int index = 0; index < kPointsInCubic; ++index) {
        if (index == t1Index) {
            continue;
        }
        SkDVector dxy1 = cubic1[index] - line[0];
        dxy1 /= SkDCubic::gPrecisionUnit;
        line[1] = line[0] + dxy1;
        SkDRect lineBounds;
        lineBounds.setBounds(line);
        if (!bounds2.intersects(&lineBounds)) {
            continue;
        }
        SkIntersections local;
        if (!local.intersect(cubic2, line)) {
            continue;
        }
        for (int idx2 = 0; idx2 < local.used(); ++idx2) {
            double foundT = local[0][idx2];
            if (approximately_less_than_zero(foundT)
                    || approximately_greater_than_one(foundT)) {
                continue;
            }
            if (local.pt(idx2).approximatelyEqual(line[0])) {
                if (i.swapped()) {  // FIXME: insert should respect swap
                    i.insert(foundT, start ? 0 : 1, line[0]);
                } else {
                    i.insert(start ? 0 : 1, foundT, line[0]);
                }
            } else {
                tVals.push_back(foundT);
            }
        }
    }
    if (tVals.count() == 0) {
        return;
    }
    SkTQSort<double>(tVals.begin(), tVals.end() - 1);
    double tMin1 = start ? 0 : 1 - LINE_FRACTION;
    double tMax1 = start ? LINE_FRACTION : 1;
    int tIdx = 0;
    do {
        int tLast = tIdx;
        while (tLast + 1 < tVals.count() && roughly_equal(tVals[tLast + 1], tVals[tIdx])) {
            ++tLast;
        }
        double tMin2 = SkTMax(tVals[tIdx] - LINE_FRACTION, 0.0);
        double tMax2 = SkTMin(tVals[tLast] + LINE_FRACTION, 1.0);
        int lastUsed = i.used();
        intersect(cubic1, tMin1, tMax1, cubic2, tMin2, tMax2, 1, i);
        if (lastUsed == i.used()) {
            tMin2 = SkTMax(tVals[tIdx] - (1.0 / SkDCubic::gPrecisionUnit), 0.0);
            tMax2 = SkTMin(tVals[tLast] + (1.0 / SkDCubic::gPrecisionUnit), 1.0);
            intersect(cubic1, tMin1, tMax1, cubic2, tMin2, tMax2, 1, i);
        }
        tIdx = tLast + 1;
    } while (tIdx < tVals.count());
#else
    const SkDPoint& endPt = cubic1[t1Index];
    if (!bounds2.contains(endPt)) {
        return;
    }
    // this variant looks for intersections within an 'x' of the endpoint
    double delta = SkTMax(bounds2.width(), bounds2.height());
    for (int index = 0; index < 2; ++index) {
        if (index == 0) {
            line[0].fY = line[1].fY = endPt.fY;
            line[0].fX = endPt.fX - delta;
            line[1].fX = endPt.fX + delta;
        } else {
            line[0].fX = line[1].fX = cubic1[t1Index].fX;
            line[0].fY = endPt.fY - delta;
            line[1].fY = endPt.fY + delta;
        }
        SkIntersections local;
        local.intersectRay(cubic2, line); // OPTIMIZE: special for horizontal/vertical lines
        int used = local.used();
        for (int index = 0; index < used; ++index) {
            double foundT = local[0][index];
            if (approximately_less_than_zero(foundT) || approximately_greater_than_one(foundT)) {
                continue;
            }
            if (!local.pt(index).approximatelyEqual(endPt)) {
                continue;
            }
            if (i.swapped()) {  // FIXME: insert should respect swap
                i.insert(foundT, start ? 0 : 1, endPt);
            } else {
                i.insert(start ? 0 : 1, foundT, endPt);
            }
            return;
        }
    }
// the above doesn't catch when the end of the cubic missed the other cubic because the quad
// approximation moved too far away, so something like the below is still needed. The enabled
// code above tries to avoid this heavy lifting unless the convex hull intersected the cubic.
    double tMin1 = start ? 0 : 1 - LINE_FRACTION;
    double tMax1 = start ? LINE_FRACTION : 1;
    double tMin2 = SkTMax(foundT - LINE_FRACTION, 0.0);
    double tMax2 = SkTMin(foundT + LINE_FRACTION, 1.0);
    int lastUsed = i.used();
    intersect(cubic1, tMin1, tMax1, cubic2, tMin2, tMax2, 1, i);
    if (lastUsed == i.used()) {
        tMin2 = SkTMax(foundT - (1.0 / SkDCubic::gPrecisionUnit), 0.0);
        tMax2 = SkTMin(foundT + (1.0 / SkDCubic::gPrecisionUnit), 1.0);
        intersect(cubic1, tMin1, tMax1, cubic2, tMin2, tMax2, 1, i);
    }
#endif
    return;
}

const double CLOSE_ENOUGH = 0.001;

static bool closeStart(const SkDCubic& cubic, int cubicIndex, SkIntersections& i, SkDPoint& pt) {
    if (i[cubicIndex][0] != 0 || i[cubicIndex][1] > CLOSE_ENOUGH) {
        return false;
    }
    pt = cubic.xyAtT((i[cubicIndex][0] + i[cubicIndex][1]) / 2);
    return true;
}

static bool closeEnd(const SkDCubic& cubic, int cubicIndex, SkIntersections& i, SkDPoint& pt) {
    int last = i.used() - 1;
    if (i[cubicIndex][last] != 1 || i[cubicIndex][last - 1] < 1 - CLOSE_ENOUGH) {
        return false;
    }
    pt = cubic.xyAtT((i[cubicIndex][last] + i[cubicIndex][last - 1]) / 2);
    return true;
}

int SkIntersections::intersect(const SkDCubic& c1, const SkDCubic& c2) {
    ::intersect(c1, 0, 1, c2, 0, 1, 1, *this);
    // FIXME: pass in cached bounds from caller
    SkDRect c1Bounds, c2Bounds;
    c1Bounds.setBounds(c1);  // OPTIMIZE use setRawBounds ?
    c2Bounds.setBounds(c2);
    intersectEnd(c1, false, c2, c2Bounds, *this);
    intersectEnd(c1, true, c2, c2Bounds, *this);
    bool selfIntersect = &c1 == &c2;
    if (!selfIntersect) {
        swap();
        intersectEnd(c2, false, c1, c1Bounds, *this);
        intersectEnd(c2, true, c1, c1Bounds, *this);
        swap();
    }
    // If an end point and a second point very close to the end is returned, the second
    // point may have been detected because the approximate quads
    // intersected at the end and close to it. Verify that the second point is valid.
    if (fUsed <= 1 || coincidentUsed()) {
        return fUsed;
    }
    SkDPoint pt[2];
    if (closeStart(c1, 0, *this, pt[0]) && closeStart(c2, 1, *this, pt[1])
            && pt[0].approximatelyEqual(pt[1])) {
        removeOne(1);
    }
    if (closeEnd(c1, 0, *this, pt[0]) && closeEnd(c2, 1, *this, pt[1])
            && pt[0].approximatelyEqual(pt[1])) {
        removeOne(used() - 2);
    }
    // vet the pairs of t values to see if the mid value is also on the curve. If so, mark
    // the span as coincident
    if (fUsed >= 2 && !coincidentUsed()) {
        int last = fUsed - 1;
        int match = 0;
        for (int index = 0; index < last; ++index) {
            double mid1 = (fT[0][index] + fT[0][index + 1]) / 2;
            double mid2 = (fT[1][index] + fT[1][index + 1]) / 2;
            pt[0] = c1.xyAtT(mid1);
            pt[1] = c2.xyAtT(mid2);
            if (pt[0].approximatelyEqual(pt[1])) {
                match |= 1 << index;
            }
        }
        if (match) {
            if (((match + 1) & match) != 0) {
                SkDebugf("%s coincident hole\n", __FUNCTION__);
            }
            // for now, assume that everything from start to finish is coincident
            if (fUsed > 2) {
                  fPt[1] = fPt[last];
                  fT[0][1] = fT[0][last];
                  fT[1][1] = fT[1][last];
                  fIsCoincident[0] = 0x03;
                  fIsCoincident[1] = 0x03;
                  fUsed = 2;
            }
        }
    }
    return fUsed;
}

// Up promote the quad to a cubic.
// OPTIMIZATION If this is a common use case, optimize by duplicating
// the intersect 3 loop to avoid the promotion  / demotion code
int SkIntersections::intersect(const SkDCubic& cubic, const SkDQuad& quad) {
    SkDCubic up = quad.toCubic();
    (void) intersect(cubic, up);
    return used();
}

/* http://www.ag.jku.at/compass/compasssample.pdf
( Self-Intersection Problems and Approximate Implicitization by Jan B. Thomassen
Centre of Mathematics for Applications, University of Oslo http://www.cma.uio.no janbth@math.uio.no
SINTEF Applied Mathematics http://www.sintef.no )
describes a method to find the self intersection of a cubic by taking the gradient of the implicit
form dotted with the normal, and solving for the roots. My math foo is too poor to implement this.*/

int SkIntersections::intersect(const SkDCubic& c) {
    // check to see if x or y end points are the extrema. Are other quick rejects possible?
    if (c.endsAreExtremaInXOrY()) {
        return false;
    }
    (void) intersect(c, c);
    if (used() > 0) {
        SkASSERT(used() == 1);
        if (fT[0][0] > fT[1][0]) {
            swapPts();
        }
    }
    return used();
}
