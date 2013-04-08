/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CubicUtilities.h"
#include "CurveIntersection.h"
#include "Intersections.h"
#include "IntersectionUtilities.h"
#include "LineIntersection.h"
#include "LineUtilities.h"
#include "QuadraticUtilities.h"
#include "TSearch.h"

#if 0
#undef ONE_OFF_DEBUG
#define ONE_OFF_DEBUG 0
#endif

#if ONE_OFF_DEBUG
static const double tLimits1[2][2] = {{0.36, 0.37}, {0.63, 0.64}};
static const double tLimits2[2][2] = {{-0.865211397, -0.865215212}, {-0.865207696, -0.865208078}};
#endif

#define DEBUG_QUAD_PART 0
#define SWAP_TOP_DEBUG 0

static int quadPart(const Cubic& cubic, double tStart, double tEnd, Quadratic& simple) {
    Cubic part;
    sub_divide(cubic, tStart, tEnd, part);
    Quadratic quad;
    demote_cubic_to_quad(part, quad);
    // FIXME: should reduceOrder be looser in this use case if quartic is going to blow up on an
    // extremely shallow quadratic?
    int order = reduceOrder(quad, simple, kReduceOrder_TreatAsFill);
#if DEBUG_QUAD_PART
    SkDebugf("%s cubic=(%1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g) t=(%1.17g,%1.17g)\n",
            __FUNCTION__, cubic[0].x, cubic[0].y, cubic[1].x, cubic[1].y, cubic[2].x, cubic[2].y,
            cubic[3].x, cubic[3].y, tStart, tEnd);
    SkDebugf("%s part=(%1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g)"
            " quad=(%1.17g,%1.17g %1.17g,%1.17g %1.17g,%1.17g)\n", __FUNCTION__, part[0].x, part[0].y,
            part[1].x, part[1].y, part[2].x, part[2].y, part[3].x, part[3].y, quad[0].x, quad[0].y,
            quad[1].x, quad[1].y, quad[2].x, quad[2].y);
    SkDebugf("%s simple=(%1.17g,%1.17g", __FUNCTION__, simple[0].x, simple[0].y);
    if (order > 1) {
        SkDebugf(" %1.17g,%1.17g", simple[1].x, simple[1].y);
    }
    if (order > 2) {
        SkDebugf(" %1.17g,%1.17g", simple[2].x, simple[2].y);
    }
    SkDebugf(")\n");
    SkASSERT(order < 4 && order > 0);
#endif
    return order;
}

static void intersectWithOrder(const Quadratic& simple1, int order1, const Quadratic& simple2,
        int order2, Intersections& i) {
    if (order1 == 3 && order2 == 3) {
        intersect2(simple1, simple2, i);
    } else if (order1 <= 2 && order2 <= 2) {
        intersect((const _Line&) simple1, (const _Line&) simple2, i);
    } else if (order1 == 3 && order2 <= 2) {
        intersect(simple1, (const _Line&) simple2, i);
    } else {
        SkASSERT(order1 <= 2 && order2 == 3);
        intersect(simple2, (const _Line&) simple1, i);
        for (int s = 0; s < i.fUsed; ++s) {
            SkTSwap(i.fT[0][s], i.fT[1][s]);
        }
    }
}

// this flavor centers potential intersections recursively. In contrast, '2' may inadvertently
// chase intersections near quadratic ends, requiring odd hacks to find them.
static bool intersect3(const Cubic& cubic1, double t1s, double t1e, const Cubic& cubic2,
        double t2s, double t2e, double precisionScale, Intersections& i) {
    i.upDepth();
    bool result = false;
    Cubic c1, c2;
    sub_divide(cubic1, t1s, t1e, c1);
    sub_divide(cubic2, t2s, t2e, c2);
    SkTDArray<double> ts1;
    // OPTIMIZE: if c1 == c2, call once (happens when detecting self-intersection)
    cubic_to_quadratics(c1, calcPrecision(c1) * precisionScale, ts1);
    SkTDArray<double> ts2;
    cubic_to_quadratics(c2, calcPrecision(c2) * precisionScale, ts2);
    double t1Start = t1s;
    int ts1Count = ts1.count();
    for (int i1 = 0; i1 <= ts1Count; ++i1) {
        const double tEnd1 = i1 < ts1Count ? ts1[i1] : 1;
        const double t1 = t1s + (t1e - t1s) * tEnd1;
        Quadratic s1;
        int o1 = quadPart(cubic1, t1Start, t1, s1);
        double t2Start = t2s;
        int ts2Count = ts2.count();
        for (int i2 = 0; i2 <= ts2Count; ++i2) {
            const double tEnd2 = i2 < ts2Count ? ts2[i2] : 1;
            const double t2 = t2s + (t2e - t2s) * tEnd2;
            if (cubic1 == cubic2 && t1Start >= t2Start) {
                t2Start = t2;
                continue;
            }
            Quadratic s2;
            int o2 = quadPart(cubic2, t2Start, t2, s2);
        #if ONE_OFF_DEBUG
            char tab[] = "                  ";
            if (tLimits1[0][0] >= t1Start && tLimits1[0][1] <= t1
                    && tLimits1[1][0] >= t2Start && tLimits1[1][1] <= t2) {
                Cubic cSub1, cSub2;
                sub_divide(cubic1, t1Start, t1, cSub1);
                sub_divide(cubic2, t2Start, t2, cSub2);
                SkDebugf("%.*s %s t1=(%1.9g,%1.9g) t2=(%1.9g,%1.9g)", i.depth()*2, tab, __FUNCTION__,
                        t1Start, t1, t2Start, t2);
                Intersections xlocals;
                intersectWithOrder(s1, o1, s2, o2, xlocals);
                SkDebugf(" xlocals.fUsed=%d\n", xlocals.used());
            }
        #endif
            Intersections locals;
            intersectWithOrder(s1, o1, s2, o2, locals);
            double coStart[2] = { -1 };
            _Point coPoint;
            int tCount = locals.used();
            for (int tIdx = 0; tIdx < tCount; ++tIdx) {
                double to1 = t1Start + (t1 - t1Start) * locals.fT[0][tIdx];
                double to2 = t2Start + (t2 - t2Start) * locals.fT[1][tIdx];
    // if the computed t is not sufficiently precise, iterate
                _Point p1 = xy_at_t(cubic1, to1);
                _Point p2 = xy_at_t(cubic2, to2);
                if (p1.approximatelyEqual(p2)) {
                    if (locals.fIsCoincident[0] & 1 << tIdx) {
                        if (coStart[0] < 0) {
                            coStart[0] = to1;
                            coStart[1] = to2;
                            coPoint = p1;
                        } else {
                            i.insertCoincidentPair(coStart[0], to1, coStart[1], to2, coPoint, p1);
                            coStart[0] = -1;
                        }
                        result = true;
                    } else if (cubic1 != cubic2 || !approximately_equal(to1, to2)) {
                        if (i.swapped()) { // FIXME: insert should respect swap
                            i.insert(to2, to1, p1);
                        } else {
                            i.insert(to1, to2, p1);
                        }
                        result = true;
                    }
                } else {
                    double offset = precisionScale / 16; // FIME: const is arbitrary -- test & refine
#if 1
                    double c1Bottom = tIdx == 0 ? 0 :
                            (t1Start + (t1 - t1Start) * locals.fT[0][tIdx - 1] + to1) / 2;
                    double c1Min = SkTMax(c1Bottom, to1 - offset);
                    double c1Top = tIdx == tCount - 1 ? 1 :
                            (t1Start + (t1 - t1Start) * locals.fT[0][tIdx + 1] + to1) / 2;
                    double c1Max = SkTMin(c1Top, to1 + offset);
                    double c2Min = SkTMax(0., to2 - offset);
                    double c2Max = SkTMin(1., to2 + offset);
                #if ONE_OFF_DEBUG
                    SkDebugf("%.*s %s 1 contains1=%d/%d contains2=%d/%d\n", i.depth()*2, tab, __FUNCTION__,
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
                            " c2Max=%1.9g\n", i.depth()*2, tab, __FUNCTION__, to1, to2, c1Min, c1Max, c2Min, c2Max);
                #endif
                    intersect3(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                #if ONE_OFF_DEBUG
                    SkDebugf("%.*s %s 1 i.used=%d t=%1.9g\n", i.depth()*2, tab, __FUNCTION__, i.used(),
                            i.used() > 0 ? i.fT[0][i.used() - 1] : -1);
                #endif
                    if (tCount > 1) {
                        c1Min = SkTMax(0., to1 - offset);
                        c1Max = SkTMin(1., to1 + offset);
                        double c2Bottom = tIdx == 0 ? to2 :
                                (t2Start + (t2 - t2Start) * locals.fT[1][tIdx - 1] + to2) / 2;
                        double c2Top = tIdx == tCount - 1 ? to2 :
                                (t2Start + (t2 - t2Start) * locals.fT[1][tIdx + 1] + to2) / 2;
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
                        SkDebugf("%.*s %s 2 contains1=%d/%d contains2=%d/%d\n", i.depth()*2, tab, __FUNCTION__,
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
                                " c2Max=%1.9g\n", i.depth()*2, tab, __FUNCTION__, to1, to2, c1Min, c1Max, c2Min, c2Max);
                    #endif
                        intersect3(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                #if ONE_OFF_DEBUG
                    SkDebugf("%.*s %s 2 i.used=%d t=%1.9g\n", i.depth()*2, tab, __FUNCTION__, i.used(),
                            i.used() > 0 ? i.fT[0][i.used() - 1] : -1);
                #endif
                        c1Min = SkTMax(c1Bottom, to1 - offset);
                        c1Max = SkTMin(c1Top, to1 + offset);
                    #if ONE_OFF_DEBUG
                        SkDebugf("%.*s %s 3 contains1=%d/%d contains2=%d/%d\n", i.depth()*2, tab, __FUNCTION__,
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
                                " c2Max=%1.9g\n", i.depth()*2, tab, __FUNCTION__, to1, to2, c1Min, c1Max, c2Min, c2Max);
                    #endif
                        intersect3(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                #if ONE_OFF_DEBUG
                    SkDebugf("%.*s %s 3 i.used=%d t=%1.9g\n", i.depth()*2, tab, __FUNCTION__, i.used(),
                            i.used() > 0 ? i.fT[0][i.used() - 1] : -1);
                #endif
                    }
#else
                    double c1Bottom = tIdx == 0 ? 0 :
                            (t1Start + (t1 - t1Start) * locals.fT[0][tIdx - 1] + to1) / 2;
                    double c1Min = SkTMax(c1Bottom, to1 - offset);
                    double c1Top = tIdx == tCount - 1 ? 1 :
                            (t1Start + (t1 - t1Start) * locals.fT[0][tIdx + 1] + to1) / 2;
                    double c1Max = SkTMin(c1Top, to1 + offset);
                    double c2Bottom = tIdx == 0 ? to2 :
                            (t2Start + (t2 - t2Start) * locals.fT[1][tIdx - 1] + to2) / 2;
                    double c2Top = tIdx == tCount - 1 ? to2 :
                            (t2Start + (t2 - t2Start) * locals.fT[1][tIdx + 1] + to2) / 2;
                    if (c2Bottom > c2Top) {
                        SkTSwap(c2Bottom, c2Top);
                    }
                    if (c2Bottom == to2) {
                        c2Bottom = 0;
                    }
                    if (c2Top == to2) {
                        c2Top = 1;
                    }
                    double c2Min = SkTMax(c2Bottom, to2 - offset);
                    double c2Max = SkTMin(c2Top, to2 + offset);
                #if ONE_OFF_DEBUG
                    SkDebugf("%s contains1=%d/%d contains2=%d/%d\n", __FUNCTION__,
                            c1Min <= 0.210357794 && 0.210357794 <= c1Max
                         && c2Min <= 0.223476406 && 0.223476406 <= c2Max,
                            to1 - offset <= 0.210357794 && 0.210357794 <= to1 + offset
                         && to2 - offset <= 0.223476406 && 0.223476406 <= to2 + offset,
                            c1Min <= 0.211324707 && 0.211324707 <= c1Max
                         && c2Min <= 0.211327209 && 0.211327209 <= c2Max,
                            to1 - offset <= 0.211324707 && 0.211324707 <= to1 + offset
                         && to2 - offset <= 0.211327209 && 0.211327209 <= to2 + offset);
                    SkDebugf("%s c1Bottom=%1.9g c1Top=%1.9g c2Bottom=%1.9g c2Top=%1.9g"
                            " 1-o=%1.9g 1+o=%1.9g 2-o=%1.9g 2+o=%1.9g offset=%1.9g\n",
                            __FUNCTION__, c1Bottom, c1Top, c2Bottom, c2Top,
                            to1 - offset, to1 + offset, to2 - offset, to2 + offset, offset);
                    SkDebugf("%s to1=%1.9g to2=%1.9g c1Min=%1.9g c1Max=%1.9g c2Min=%1.9g"
                            " c2Max=%1.9g\n", __FUNCTION__, to1, to2, c1Min, c1Max, c2Min, c2Max);
                #endif
#endif
                    intersect3(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                    // TODO: if no intersection is found, either quadratics intersected where
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
    return result;
}

#if 0
#define LINE_FRACTION (1.0 / gPrecisionUnit)
#else
#define LINE_FRACTION 0.1
#endif

// intersect the end of the cubic with the other. Try lines from the end to control and opposite
// end to determine range of t on opposite cubic.
static bool intersectEnd(const Cubic& cubic1, bool start, const Cubic& cubic2, const _Rect& bounds2,
        Intersections& i) {
 //   bool selfIntersect = cubic1 == cubic2;
    _Line line;
    int t1Index = start ? 0 : 3;
    line[0] = cubic1[t1Index];
    // don't bother if the two cubics are connnected
#if 0
    if (!selfIntersect && (line[0].approximatelyEqual(cubic2[0])
            || line[0].approximatelyEqual(cubic2[3]))) {
        return false;
    }
#endif
    bool result = false;
    SkTDArray<double> tVals; // OPTIMIZE: replace with hard-sized array
    for (int index = 0; index < 4; ++index) {
        if (index == t1Index) {
            continue;
        }
        _Vector dxy1 = cubic1[index] - line[0];
        dxy1 /= gPrecisionUnit;
        line[1] = line[0] + dxy1;
        _Rect lineBounds;
        lineBounds.setBounds(line);
        if (!bounds2.intersects(lineBounds)) {
            continue;
        }
        Intersections local;
        if (!intersect(cubic2, line, local)) {
            continue;
        }
        for (int idx2 = 0; idx2 < local.used(); ++idx2) {
            double foundT = local.fT[0][idx2];
            if (approximately_less_than_zero(foundT)
                    || approximately_greater_than_one(foundT)) {
                continue;
            }
            if (local.fPt[idx2].approximatelyEqual(line[0])) {
                if (i.swapped()) { // FIXME: insert should respect swap
                    i.insert(foundT, start ? 0 : 1, line[0]);
                } else {
                    i.insert(start ? 0 : 1, foundT, line[0]);
                }
                result = true;
            } else {
                *tVals.append() = local.fT[0][idx2];
            }
        }
    }
    if (tVals.count() == 0) {
        return result;
    }
    QSort<double>(tVals.begin(), tVals.end() - 1);
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
        result |= intersect3(cubic1, tMin1, tMax1, cubic2, tMin2, tMax2, 1, i);
        if (lastUsed == i.used()) {
            tMin2 = SkTMax(tVals[tIdx] - (1.0 / gPrecisionUnit), 0.0);
            tMax2 = SkTMin(tVals[tLast] + (1.0 / gPrecisionUnit), 1.0);
            result |= intersect3(cubic1, tMin1, tMax1, cubic2, tMin2, tMax2, 1, i);
        }
        tIdx = tLast + 1;
    } while (tIdx < tVals.count());
    return result;
}

const double CLOSE_ENOUGH = 0.001;

static bool closeStart(const Cubic& cubic, int cubicIndex, Intersections& i, _Point& pt) {
    if (i.fT[cubicIndex][0] != 0 || i.fT[cubicIndex][1] > CLOSE_ENOUGH) {
        return false;
    }
    pt = xy_at_t(cubic, (i.fT[cubicIndex][0] + i.fT[cubicIndex][1]) / 2);
    return true;
}

static bool closeEnd(const Cubic& cubic, int cubicIndex, Intersections& i, _Point& pt) {
    int last = i.used() - 1;
    if (i.fT[cubicIndex][last] != 1 || i.fT[cubicIndex][last - 1] < 1 - CLOSE_ENOUGH) {
        return false;
    }
    pt = xy_at_t(cubic, (i.fT[cubicIndex][last] + i.fT[cubicIndex][last - 1]) / 2);
    return true;
}

bool intersect3(const Cubic& c1, const Cubic& c2, Intersections& i) {
    bool result = intersect3(c1, 0, 1, c2, 0, 1, 1, i);
    // FIXME: pass in cached bounds from caller
    _Rect c1Bounds, c2Bounds;
    c1Bounds.setBounds(c1); // OPTIMIZE use setRawBounds ?
    c2Bounds.setBounds(c2);
    result |= intersectEnd(c1, false, c2, c2Bounds, i);
    result |= intersectEnd(c1, true, c2, c2Bounds, i);
    bool selfIntersect = c1 == c2;
    if (!selfIntersect) {
        i.swap();
        result |= intersectEnd(c2, false, c1, c1Bounds, i);
        result |= intersectEnd(c2, true, c1, c1Bounds, i);
        i.swap();
    }
    // If an end point and a second point very close to the end is returned, the second
    // point may have been detected because the approximate quads
    // intersected at the end and close to it. Verify that the second point is valid.
    if (i.used() <= 1 || i.coincidentUsed()) {
        return result;
    }
    _Point pt[2];
    if (closeStart(c1, 0, i, pt[0]) && closeStart(c2, 1, i, pt[1])
            && pt[0].approximatelyEqual(pt[1])) {
        i.removeOne(1);
    }
    if (closeEnd(c1, 0, i, pt[0]) && closeEnd(c2, 1, i, pt[1])
            && pt[0].approximatelyEqual(pt[1])) {
        i.removeOne(i.used() - 2);
    }
    return result;
}

// Up promote the quad to a cubic.
// OPTIMIZATION If this is a common use case, optimize by duplicating
// the intersect 3 loop to avoid the promotion  / demotion code
int intersect(const Cubic& cubic, const Quadratic& quad, Intersections& i) {
    Cubic up;
    toCubic(quad, up);
    (void) intersect3(cubic, up, i);
    return i.used();
}

/* http://www.ag.jku.at/compass/compasssample.pdf
( Self-Intersection Problems and Approximate Implicitization by Jan B. Thomassen
Centre of Mathematics for Applications, University of Oslo http://www.cma.uio.no janbth@math.uio.no
SINTEF Applied Mathematics http://www.sintef.no )
describes a method to find the self intersection of a cubic by taking the gradient of the implicit
form dotted with the normal, and solving for the roots. My math foo is too poor to implement this.*/

int intersect(const Cubic& c, Intersections& i) {
    // check to see if x or y end points are the extrema. Are other quick rejects possible?
    if (ends_are_extrema_in_x_or_y(c)) {
        return false;
    }
    (void) intersect3(c, c, i);
    if (i.used() > 0) {
        SkASSERT(i.used() == 1);
        if (i.fT[0][0] > i.fT[1][0]) {
            SkTSwap(i.fT[0][0], i.fT[1][0]);
        }
    }
    return i.used();
}
