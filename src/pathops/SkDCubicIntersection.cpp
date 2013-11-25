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
static const double tLimits1[2][2] = {{0.3, 0.4}, {0.8, 0.9}};
static const double tLimits2[2][2] = {{-0.8, -0.9}, {-0.8, -0.9}};
#endif

#define DEBUG_QUAD_PART ONE_OFF_DEBUG && 1
#define DEBUG_QUAD_PART_SHOW_SIMPLE DEBUG_QUAD_PART && 0
#define SWAP_TOP_DEBUG 0

static const int kCubicToQuadSubdivisionDepth = 8; // slots reserved for cubic to quads subdivision

static int quadPart(const SkDCubic& cubic, double tStart, double tEnd, SkReduceOrder* reducer) {
    SkDCubic part = cubic.subDivide(tStart, tEnd);
    SkDQuad quad = part.toQuad();
    // FIXME: should reduceOrder be looser in this use case if quartic is going to blow up on an
    // extremely shallow quadratic?
    int order = reducer->reduce(quad);
#if DEBUG_QUAD_PART
    SkDebugf("%s cubic=(%1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g)"
            " t=(%1.9g,%1.9g)\n", __FUNCTION__, cubic[0].fX, cubic[0].fY,
            cubic[1].fX, cubic[1].fY, cubic[2].fX, cubic[2].fY,
            cubic[3].fX, cubic[3].fY, tStart, tEnd);
    SkDebugf("  {{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n"
             "  {{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n",
            part[0].fX, part[0].fY, part[1].fX, part[1].fY, part[2].fX, part[2].fY,
            part[3].fX, part[3].fY, quad[0].fX, quad[0].fY,
            quad[1].fX, quad[1].fY, quad[2].fX, quad[2].fY);
#if DEBUG_QUAD_PART_SHOW_SIMPLE
    SkDebugf("%s simple=(%1.9g,%1.9g", __FUNCTION__, reducer->fQuad[0].fX, reducer->fQuad[0].fY);
    if (order > 1) {
        SkDebugf(" %1.9g,%1.9g", reducer->fQuad[1].fX, reducer->fQuad[1].fY);
    }
    if (order > 2) {
        SkDebugf(" %1.9g,%1.9g", reducer->fQuad[2].fX, reducer->fQuad[2].fY);
    }
    SkDebugf(")\n");
    SkASSERT(order < 4 && order > 0);
#endif
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
                xlocals.allowNear(false);
                intersectWithOrder(s1.fQuad, o1, s2.fQuad, o2, xlocals);
                SkDebugf(" xlocals.fUsed=%d\n", xlocals.used());
            }
        #endif
            SkIntersections locals;
            locals.allowNear(false);
            intersectWithOrder(s1.fQuad, o1, s2.fQuad, o2, locals);
            int tCount = locals.used();
            for (int tIdx = 0; tIdx < tCount; ++tIdx) {
                double to1 = t1Start + (t1 - t1Start) * locals[0][tIdx];
                double to2 = t2Start + (t2 - t2Start) * locals[1][tIdx];
    // if the computed t is not sufficiently precise, iterate
                SkDPoint p1 = cubic1.ptAtT(to1);
                SkDPoint p2 = cubic2.ptAtT(to2);
                if (p1.approximatelyEqual(p2)) {
    // FIXME: local edge may be coincident -- experiment with not propagating coincidence to caller
//                    SkASSERT(!locals.isCoincident(tIdx));
                    if (&cubic1 != &cubic2 || !approximately_equal(to1, to2)) {
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
          //          intersect(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                    // FIXME: if no intersection is found, either quadratics intersected where
                    // cubics did not, or the intersection was missed. In the former case, expect
                    // the quadratics to be nearly parallel at the point of intersection, and check
                    // for that.
                }
            }
            t2Start = t2;
        }
        t1Start = t1;
    }
    i.downDepth();
}

    // if two ends intersect, check middle for coincidence
bool SkIntersections::cubicCheckCoincidence(const SkDCubic& c1, const SkDCubic& c2) {
    if (fUsed < 2) {
        return false;
    }
    int last = fUsed - 1;
    double tRange1 = fT[0][last] - fT[0][0];
    double tRange2 = fT[1][last] - fT[1][0];
    for (int index = 1; index < 5; ++index) {
        double testT1 = fT[0][0] + tRange1 * index / 5;
        double testT2 = fT[1][0] + tRange2 * index / 5;
        SkDPoint testPt1 = c1.ptAtT(testT1);
        SkDPoint testPt2 = c2.ptAtT(testT2);
        if (!testPt1.approximatelyEqual(testPt2)) {
            return false;
        }
    }
    if (fUsed > 2) {
        fPt[1] = fPt[last];
        fT[0][1] = fT[0][last];
        fT[1][1] = fT[1][last];
        fUsed = 2;
    }
    fIsCoincident[0] = fIsCoincident[1] = 0x03;
    return true;
}

#define LINE_FRACTION 0.1

// intersect the end of the cubic with the other. Try lines from the end to control and opposite
// end to determine range of t on opposite cubic.
bool SkIntersections::cubicExactEnd(const SkDCubic& cubic1, bool start, const SkDCubic& cubic2) {
    int t1Index = start ? 0 : 3;
    double testT = (double) !start;
    bool swap = swapped();
    // quad/quad at this point checks to see if exact matches have already been found
    // cubic/cubic can't reject so easily since cubics can intersect same point more than once
    SkDLine tmpLine;
    tmpLine[0] = tmpLine[1] = cubic2[t1Index];
    tmpLine[1].fX += cubic2[2 - start].fY - cubic2[t1Index].fY;
    tmpLine[1].fY -= cubic2[2 - start].fX - cubic2[t1Index].fX;
    SkIntersections impTs;
    impTs.allowNear(false);
    impTs.intersectRay(cubic1, tmpLine);
    for (int index = 0; index < impTs.used(); ++index) {
        SkDPoint realPt = impTs.pt(index);
        if (!tmpLine[0].approximatelyEqual(realPt)) {
            continue;
        }
        if (swap) {
            insert(testT, impTs[0][index], tmpLine[0]);
        } else {
            insert(impTs[0][index], testT, tmpLine[0]);
        }
        return true;
    }
    return false;
}

void SkIntersections::cubicNearEnd(const SkDCubic& cubic1, bool start, const SkDCubic& cubic2,
                         const SkDRect& bounds2) {
    SkDLine line;
    int t1Index = start ? 0 : 3;
    double testT = (double) !start;
   // don't bother if the two cubics are connnected
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
                if (swapped()) {  // FIXME: insert should respect swap
                    insert(foundT, testT, line[0]);
                } else {
                    insert(testT, foundT, line[0]);
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
        int lastUsed = used();
        ::intersect(cubic1, tMin1, tMax1, cubic2, tMin2, tMax2, 1, *this);
        if (lastUsed == used()) {
            tMin2 = SkTMax(tVals[tIdx] - (1.0 / SkDCubic::gPrecisionUnit), 0.0);
            tMax2 = SkTMin(tVals[tLast] + (1.0 / SkDCubic::gPrecisionUnit), 1.0);
            ::intersect(cubic1, tMin1, tMax1, cubic2, tMin2, tMax2, 1, *this);
        }
        tIdx = tLast + 1;
    } while (tIdx < tVals.count());
    return;
}

const double CLOSE_ENOUGH = 0.001;

static bool closeStart(const SkDCubic& cubic, int cubicIndex, SkIntersections& i, SkDPoint& pt) {
    if (i[cubicIndex][0] != 0 || i[cubicIndex][1] > CLOSE_ENOUGH) {
        return false;
    }
    pt = cubic.ptAtT((i[cubicIndex][0] + i[cubicIndex][1]) / 2);
    return true;
}

static bool closeEnd(const SkDCubic& cubic, int cubicIndex, SkIntersections& i, SkDPoint& pt) {
    int last = i.used() - 1;
    if (i[cubicIndex][last] != 1 || i[cubicIndex][last - 1] < 1 - CLOSE_ENOUGH) {
        return false;
    }
    pt = cubic.ptAtT((i[cubicIndex][last] + i[cubicIndex][last - 1]) / 2);
    return true;
}

static bool only_end_pts_in_common(const SkDCubic& c1, const SkDCubic& c2) {
// the idea here is to see at minimum do a quick reject by rotating all points
// to either side of the line formed by connecting the endpoints
// if the opposite curves points are on the line or on the other side, the
// curves at most intersect at the endpoints
    for (int oddMan = 0; oddMan < 4; ++oddMan) {
        const SkDPoint* endPt[3];
        for (int opp = 1; opp < 4; ++opp) {
            int end = oddMan ^ opp;  // choose a value not equal to oddMan
            endPt[opp - 1] = &c1[end];
        }
        for (int triTest = 0; triTest < 3; ++triTest) {
            double origX = endPt[triTest]->fX;
            double origY = endPt[triTest]->fY;
            int oppTest = triTest + 1;
            if (3 == oppTest) {
                oppTest = 0;
            }
            double adj = endPt[oppTest]->fX - origX;
            double opp = endPt[oppTest]->fY - origY;
            double sign = (c1[oddMan].fY - origY) * adj - (c1[oddMan].fX - origX) * opp;
            if (approximately_zero(sign)) {
                goto tryNextHalfPlane;
            }
            for (int n = 0; n < 4; ++n) {
                double test = (c2[n].fY - origY) * adj - (c2[n].fX - origX) * opp;
                if (test * sign > 0 && !precisely_zero(test)) {
                    goto tryNextHalfPlane;
                }
            }
        }
        return true;
tryNextHalfPlane:
        ;
    }
    return false;
}

int SkIntersections::intersect(const SkDCubic& c1, const SkDCubic& c2) {
    if (fMax == 0) {
        fMax = 9;
    }
    bool selfIntersect = &c1 == &c2;
    if (selfIntersect) {
        if (c1[0].approximatelyEqual(c1[3])) {
            insert(0, 1, c1[0]);
            return fUsed;
        }
    } else {
        // OPTIMIZATION: set exact end bits here to avoid cubic exact end later
        for (int i1 = 0; i1 < 4; i1 += 3) {
            for (int i2 = 0; i2 < 4; i2 += 3) {
                if (c1[i1].approximatelyEqual(c2[i2])) {
                    insert(i1 >> 1, i2 >> 1, c1[i1]);
                }
            }
        }
    }
    SkASSERT(fUsed < 4);
    if (!selfIntersect) {
        if (only_end_pts_in_common(c1, c2)) {
            return fUsed;
        }
        if (only_end_pts_in_common(c2, c1)) {
            return fUsed;
        }
    }
    // quad/quad does linear test here -- cubic does not
    // cubics which are really lines should have been detected in reduce step earlier
    int exactEndBits = 0;
    if (selfIntersect) {
        if (fUsed) {
            return fUsed;
        }
    } else {
        exactEndBits |= cubicExactEnd(c1, false, c2) << 0;
        exactEndBits |= cubicExactEnd(c1, true, c2) << 1;
        swap();
        exactEndBits |= cubicExactEnd(c2, false, c1) << 2;
        exactEndBits |= cubicExactEnd(c2, true, c1) << 3;
        swap();
    }
    if (cubicCheckCoincidence(c1, c2)) {
        SkASSERT(!selfIntersect);
        return fUsed;
    }
    // FIXME: pass in cached bounds from caller
    SkDRect c2Bounds;
    c2Bounds.setBounds(c2);
    if (!(exactEndBits & 4)) {
        cubicNearEnd(c1, false, c2, c2Bounds);
    }
    if (!(exactEndBits & 8)) {
        cubicNearEnd(c1, true, c2, c2Bounds);
    }
    if (!selfIntersect) {
        SkDRect c1Bounds;
        c1Bounds.setBounds(c1);  // OPTIMIZE use setRawBounds ?
        swap();
        if (!(exactEndBits & 1)) {
            cubicNearEnd(c2, false, c1, c1Bounds);
        }
        if (!(exactEndBits & 2)) {
            cubicNearEnd(c2, true, c1, c1Bounds);
        }
        swap();
    }
    if (cubicCheckCoincidence(c1, c2)) {
        SkASSERT(!selfIntersect);
        return fUsed;
    }
    SkIntersections i;
    i.fAllowNear = false;
    i.fMax = 9;
    ::intersect(c1, 0, 1, c2, 0, 1, 1, i);
    int compCount = i.used();
    if (compCount) {
        int exactCount = used();
        if (exactCount == 0) {
            set(i);
        } else {
            // at least one is exact or near, and at least one was computed. Eliminate duplicates
            for (int exIdx = 0; exIdx < exactCount; ++exIdx) {
                for (int cpIdx = 0; cpIdx < compCount; ) {
                    if (fT[0][0] == i[0][0] && fT[1][0] == i[1][0]) {
                        i.removeOne(cpIdx);
                        --compCount;
                        continue;
                    }
                    double tAvg = (fT[0][exIdx] + i[0][cpIdx]) / 2;
                    SkDPoint pt = c1.ptAtT(tAvg);
                    if (!pt.approximatelyEqual(fPt[exIdx])) {
                        ++cpIdx;
                        continue;
                    }
                    tAvg = (fT[1][exIdx] + i[1][cpIdx]) / 2;
                    pt = c2.ptAtT(tAvg);
                    if (!pt.approximatelyEqual(fPt[exIdx])) {
                        ++cpIdx;
                        continue;
                    }
                    i.removeOne(cpIdx);
                    --compCount;
                }
            }
            // if mid t evaluates to nearly the same point, skip the t
            for (int cpIdx = 0; cpIdx < compCount - 1; ) {
                double tAvg = (fT[0][cpIdx] + i[0][cpIdx + 1]) / 2;
                SkDPoint pt = c1.ptAtT(tAvg);
                if (!pt.approximatelyEqual(fPt[cpIdx])) {
                    ++cpIdx;
                    continue;
                }
                tAvg = (fT[1][cpIdx] + i[1][cpIdx + 1]) / 2;
                pt = c2.ptAtT(tAvg);
                if (!pt.approximatelyEqual(fPt[cpIdx])) {
                    ++cpIdx;
                    continue;
                }
                i.removeOne(cpIdx);
                --compCount;
            }
            // in addition to adding below missing function, think about how to say
            append(i);
        }
    }
    // If an end point and a second point very close to the end is returned, the second
    // point may have been detected because the approximate quads
    // intersected at the end and close to it. Verify that the second point is valid.
    if (fUsed <= 1) {
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
            pt[0] = c1.ptAtT(mid1);
            pt[1] = c2.ptAtT(mid2);
            if (pt[0].approximatelyEqual(pt[1])) {
                match |= 1 << index;
            }
        }
        if (match) {
#if DEBUG_CONCIDENT
            if (((match + 1) & match) != 0) {
                SkDebugf("%s coincident hole\n", __FUNCTION__);
            }
#endif
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
    fMax = 6;
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
    fMax = 1;
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
