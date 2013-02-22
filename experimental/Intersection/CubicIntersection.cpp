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

#if ONE_OFF_DEBUG
static const double tLimits[2][2] = {{0.599274754, 0.599275135}, {0.599274754, 0.599275135}};
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

static double distanceFromEnd(double t) {
    return t > 0.5 ? 1 - t : t;
}

// OPTIMIZATION: this used to try to guess the value for delta, and that may still be worthwhile
static void bumpForRetry(double t1, double t2, double& s1, double& e1, double& s2, double& e2) {
    double dt1 = distanceFromEnd(t1);
    double dt2 = distanceFromEnd(t2);
    double delta = 1.0 / precisionUnit;
    if (dt1 < dt2) {
        if (t1 == dt1) {
            s1 = SkTMax(s1 - delta, 0.);
        } else {
            e1 = SkTMin(e1 + delta, 1.);
        }
    } else {
        if (t2 == dt2) {
            s2 = SkTMax(s2 - delta, 0.);
        } else {
            e2 = SkTMin(e2 + delta, 1.);
        }
    }
}

static bool doIntersect(const Cubic& cubic1, double t1s, double t1m, double t1e,
        const Cubic& cubic2, double t2s, double t2m, double t2e, Intersections& i) {
    bool result = false;
    i.upDepth();
    // divide the quadratics at the new t value and try again
    double p1s = t1s;
    double p1e = t1m;
    for (int p1 = 0; p1 < 2; ++p1) {
        Quadratic s1a;
        int o1a = quadPart(cubic1, p1s, p1e, s1a);
        double p2s = t2s;
        double p2e = t2m;
        for (int p2 = 0; p2 < 2; ++p2) {
            Quadratic s2a;
            int o2a = quadPart(cubic2, p2s, p2e, s2a);
            Intersections locals;
        #if ONE_OFF_DEBUG
            if (tLimits[0][0] >= p1s && tLimits[0][1] <= p1e
                            && tLimits[1][0] >= p2s && tLimits[1][1] <= p2e) {
                SkDebugf("t1=(%1.9g,%1.9g) o1=%d t2=(%1.9g,%1.9g) o2=%d\n",
                    p1s, p1e, o1a, p2s, p2e, o2a);
                if (o1a == 2) {
                    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n",
                            s1a[0].x, s1a[0].y, s1a[1].x, s1a[1].y);
                } else {
                    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n",
                            s1a[0].x, s1a[0].y, s1a[1].x, s1a[1].y, s1a[2].x, s1a[2].y);
                }
                if (o2a == 2) {
                    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n",
                            s2a[0].x, s2a[0].y, s2a[1].x, s2a[1].y);
                } else {
                    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n",
                            s2a[0].x, s2a[0].y, s2a[1].x, s2a[1].y, s2a[2].x, s2a[2].y);
                }
                Intersections xlocals;
                intersectWithOrder(s1a, o1a, s2a, o2a, xlocals);
                SkDebugf("xlocals.fUsed=%d depth=%d\n", xlocals.used(), i.depth());
            }
        #endif
            intersectWithOrder(s1a, o1a, s2a, o2a, locals);
            for (int tIdx = 0; tIdx < locals.used(); ++tIdx) {
                double to1 = p1s + (p1e - p1s) * locals.fT[0][tIdx];
                double to2 = p2s + (p2e - p2s) * locals.fT[1][tIdx];
    // if the computed t is not sufficiently precise, iterate
                _Point p1, p2;
                xy_at_t(cubic1, to1, p1.x, p1.y);
                xy_at_t(cubic2, to2, p2.x, p2.y);
        #if ONE_OFF_DEBUG
                SkDebugf("to1=%1.9g p1=(%1.9g,%1.9g) to2=%1.9g p2=(%1.9g,%1.9g) d=%1.9g\n",
                    to1, p1.x, p1.y, to2, p2.x, p2.y, p1.distance(p2));

        #endif
                if (p1.approximatelyEqualHalf(p2)) {
                    i.insertSwap(to1, to2, p1);
                    result = true;
                } else {
                    result = doIntersect(cubic1, p1s, to1, p1e, cubic2, p2s, to2, p2e, i);
                    if (!result && p1.approximatelyEqual(p2)) {
                        i.insertSwap(to1, to2, p1);
        #if SWAP_TOP_DEBUG
                        SkDebugf("!!!\n");
        #endif
                        result = true;
                    } else
                    // if both cubics curve in the same direction, the quadratic intersection
                    // may mark a range that does not contain the cubic intersection. If no
                    // intersection is found, look again including the t distance of the
                    // of the quadratic intersection nearest a quadratic end (which in turn is
                    // nearest the actual cubic)
                    if (!result) {
                        double b1s = p1s;
                        double b1e = p1e;
                        double b2s = p2s;
                        double b2e = p2e;
                        bumpForRetry(locals.fT[0][tIdx], locals.fT[1][tIdx], b1s, b1e, b2s, b2e);
                        result = doIntersect(cubic1, b1s, to1, b1e, cubic2, b2s, to2, b2e, i);
                    }
                }
            }
            p2s = p2e;
            p2e = t2e;
        }
        p1s = p1e;
        p1e = t1e;
    }
    i.downDepth();
    return result;
}

// this flavor approximates the cubics with quads to find the intersecting ts
// OPTIMIZE: if this strategy proves successful, the quad approximations, or the ts used
// to create the approximations, could be stored in the cubic segment
// FIXME: this strategy needs to intersect the convex hull on either end with the opposite to
// account for inset quadratics that cause the endpoint intersection to avoid detection
// the segments can be very short -- the length of the maximum quadratic error (precision)
static bool intersect2(const Cubic& cubic1, double t1s, double t1e, const Cubic& cubic2,
        double t2s, double t2e, double precisionScale, Intersections& i) {
    Cubic c1, c2;
    sub_divide(cubic1, t1s, t1e, c1);
    sub_divide(cubic2, t2s, t2e, c2);
    SkTDArray<double> ts1;
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
            Quadratic s2;
            int o2 = quadPart(cubic2, t2Start, t2, s2);
        #if ONE_OFF_DEBUG
                if (tLimits[0][0] >= t1Start && tLimits[0][1] <= t1
                        && tLimits[1][0] >= t2Start && tLimits[1][1] <= t2) {
                Cubic cSub1, cSub2;
                sub_divide(cubic1, t1Start, tEnd1, cSub1);
                sub_divide(cubic2, t2Start, tEnd2, cSub2);
                SkDebugf("t1=(%1.9g,%1.9g) t2=(%1.9g,%1.9g)\n",
                        t1Start, t1, t2Start, t2);
                Intersections xlocals;
                intersectWithOrder(s1, o1, s2, o2, xlocals);
                SkDebugf("xlocals.fUsed=%d\n", xlocals.used());
            }
        #endif
            Intersections locals;
            intersectWithOrder(s1, o1, s2, o2, locals);

            for (int tIdx = 0; tIdx < locals.used(); ++tIdx) {
                double to1 = t1Start + (t1 - t1Start) * locals.fT[0][tIdx];
                double to2 = t2Start + (t2 - t2Start) * locals.fT[1][tIdx];
    // if the computed t is not sufficiently precise, iterate
                _Point p1, p2;
                xy_at_t(cubic1, to1, p1.x, p1.y);
                xy_at_t(cubic2, to2, p2.x, p2.y);
                if (p1.approximatelyEqual(p2)) {
                    i.insert(to1, to2, p1);
                } else {
                #if ONE_OFF_DEBUG
                    if (tLimits[0][0] >= t1Start && tLimits[0][1] <= t1
                            && tLimits[1][0] >= t2Start && tLimits[1][1] <= t2) {
                        SkDebugf("t1=(%1.9g,%1.9g) t2=(%1.9g,%1.9g)\n",
                                t1Start, t1, t2Start, t2);
                    }
                #endif
                    bool found = doIntersect(cubic1, t1Start, to1, t1, cubic2, t2Start, to2, t2, i);
                    if (!found) {
                        double b1s = t1Start;
                        double b1e = t1;
                        double b2s = t2Start;
                        double b2e = t2;
                        bumpForRetry(locals.fT[0][tIdx], locals.fT[1][tIdx], b1s, b1e, b2s, b2e);
                        doIntersect(cubic1, b1s, to1, b1e, cubic2, b2s, to2, b2e, i);
                    }
                }
            }
            int coincidentCount = locals.coincidentUsed();
            if (coincidentCount) {
                // FIXME: one day, we'll probably need to allow coincident + non-coincident pts
                SkASSERT(coincidentCount == locals.used());
                SkASSERT(coincidentCount == 2);
                double coTs[2][2];
                for (int tIdx = 0; tIdx < coincidentCount; ++tIdx) {
                    if (locals.fIsCoincident[0] & (1 << tIdx)) {
                        coTs[0][tIdx] = t1Start + (t1 - t1Start) * locals.fT[0][tIdx];
                    }
                    if (locals.fIsCoincident[1] & (1 << tIdx)) {
                        coTs[1][tIdx] = t2Start + (t2 - t2Start) * locals.fT[1][tIdx];
                    }
                }
                i.insertCoincidentPair(coTs[0][0], coTs[0][1], coTs[1][0], coTs[1][1],
                        locals.fPt[0], locals.fPt[1]);
            }
            t2Start = t2;
        }
        t1Start = t1;
    }
    return i.intersected();
}

static bool intersectEnd(const Cubic& cubic1, bool start, const Cubic& cubic2, const _Rect& bounds2,
        Intersections& i) {
    _Line line1;
    line1[1] = cubic1[start ? 0 : 3];
    if (line1[1].approximatelyEqual(cubic2[0]) || line1[1].approximatelyEqual(cubic2[3])) {
        return false;
    }
    line1[0] = line1[1];
    _Point dxy1 = line1[0] - cubic1[start ? 1 : 2];
    if (dxy1.approximatelyZero()) {
        dxy1 = line1[0] - cubic1[start ? 2 : 1];
    }
    dxy1 /= precisionUnit;
    line1[1] += dxy1;
    _Rect line1Bounds;
    line1Bounds.setBounds(line1);
    if (!bounds2.intersects(line1Bounds)) {
        return false;
    }
    _Line line2;
    line2[0] = line2[1] = line1[0];
    _Point dxy2 = line2[0] - cubic1[start ? 3 : 0];
    SkASSERT(!dxy2.approximatelyZero());
    dxy2 /= precisionUnit;
    line2[1] += dxy2;
#if 0 // this is so close to the first bounds test it isn't worth the short circuit test
    _Rect line2Bounds;
    line2Bounds.setBounds(line2);
    if (!bounds2.intersects(line2Bounds)) {
        return false;
    }
#endif
    Intersections local1;
    if (!intersect(cubic2, line1, local1)) {
        return false;
    }
    Intersections local2;
    if (!intersect(cubic2, line2, local2)) {
        return false;
    }
    double tMin, tMax;
    tMin = tMax = local1.fT[0][0];
    for (int index = 1; index < local1.fUsed; ++index) {
        tMin = SkTMin(tMin, local1.fT[0][index]);
        tMax = SkTMax(tMax, local1.fT[0][index]);
    }
    for (int index = 1; index < local2.fUsed; ++index) {
        tMin = SkTMin(tMin, local2.fT[0][index]);
        tMax = SkTMax(tMax, local2.fT[0][index]);
    }
    return intersect2(cubic1, start ? 0 : 1, start ? 1.0 / precisionUnit : 1 - 1.0 / precisionUnit,
            cubic2, tMin, tMax, 1, i);
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
            if (tLimits[0][0] >= t1Start && tLimits[0][1] <= t1
                    && tLimits[1][0] >= t2Start && tLimits[1][1] <= t2) {
                Cubic cSub1, cSub2;
                sub_divide(cubic1, t1Start, tEnd1, cSub1);
                sub_divide(cubic2, t2Start, tEnd2, cSub2);
                SkDebugf("t1=(%1.9g,%1.9g) t2=(%1.9g,%1.9g)\n",
                        t1Start, t1, t2Start, t2);
                Intersections xlocals;
                intersectWithOrder(s1, o1, s2, o2, xlocals);
                SkDebugf("xlocals.fUsed=%d\n", xlocals.used());
            }
        #endif
            Intersections locals;
            intersectWithOrder(s1, o1, s2, o2, locals);
            double coStart[2] = { -1 };
            _Point coPoint;
            for (int tIdx = 0; tIdx < locals.used(); ++tIdx) {
                double to1 = t1Start + (t1 - t1Start) * locals.fT[0][tIdx];
                double to2 = t2Start + (t2 - t2Start) * locals.fT[1][tIdx];
    // if the computed t is not sufficiently precise, iterate
                _Point p1, p2;
                xy_at_t(cubic1, to1, p1.x, p1.y);
                xy_at_t(cubic2, to2, p2.x, p2.y);
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
                        i.insert(to1, to2, p1);
                        result = true;
                    }
                } else {
                    double offset = precisionScale / 16; // FIME: const is arbitrary -- test & refine
                    double c1Min = SkTMax(0., to1 - offset);
                    double c1Max = SkTMin(1., to1 + offset);
                    double c2Min = SkTMax(0., to2 - offset);
                    double c2Max = SkTMin(1., to2 + offset);
                    bool found = intersect3(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                    if (false && !found) {
                        // either offset was overagressive or cubics didn't really intersect
                        // if they didn't intersect, then quad tangents ought to be nearly parallel
                        offset = precisionScale / 2; // try much less agressive offset
                        c1Min = SkTMax(0., to1 - offset);
                        c1Max = SkTMin(1., to1 + offset);
                        c2Min = SkTMax(0., to2 - offset);
                        c2Max = SkTMin(1., to2 + offset);
                        found = intersect3(cubic1, c1Min, c1Max, cubic2, c2Min, c2Max, offset, i);
                        if (found) {
                            SkDebugf("%s *** over-aggressive? offset=%1.9g depth=%d\n", __FUNCTION__,
                                    offset, i.depth());
                        }
                        // try parallel measure
                        _Point d1 = dxdy_at_t(cubic1, to1);
                        _Point d2 = dxdy_at_t(cubic2, to2);
                        double shallow = d1.cross(d2);
                    #if 1 || ONE_OFF_DEBUG // not sure this is worth debugging
                        if (!approximately_zero(shallow)) {
                            SkDebugf("%s *** near-miss? shallow=%1.9g depth=%d\n", __FUNCTION__,
                                    offset, i.depth());
                        }
                    #endif
                        if (i.depth() == 1 && shallow < 0.6) {
                            SkDebugf("%s !!! near-miss? shallow=%1.9g depth=%d\n", __FUNCTION__,
                                    offset, i.depth());
                        }
                    }
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

// FIXME: add intersection of convex hull on cubics' ends with the opposite cubic. The hull line
// segments can be constructed to be only as long as the calculated precision suggests. If the hull
// line segments intersect the cubic, then use the intersections to construct a subdivision for
// quadratic curve fitting.
bool intersect2(const Cubic& c1, const Cubic& c2, Intersections& i) {
    bool result = intersect2(c1, 0, 1, c2, 0, 1, 1, i);
    // FIXME: pass in cached bounds from caller
    _Rect c1Bounds, c2Bounds;
    c1Bounds.setBounds(c1); // OPTIMIZE use setRawBounds ?
    c2Bounds.setBounds(c2);
    result |= intersectEnd(c1, false, c2, c2Bounds, i);
    result |= intersectEnd(c1, true, c2, c2Bounds, i);
    i.swap();
    result |= intersectEnd(c2, false, c1, c1Bounds, i);
    result |= intersectEnd(c2, true, c1, c1Bounds, i);
    i.swap();
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
    i.swap();
    result |= intersectEnd(c2, false, c1, c1Bounds, i);
    result |= intersectEnd(c2, true, c1, c1Bounds, i);
    i.swap();
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
    if ((between(c[0].x, c[1].x, c[3].x) && between(c[0].x, c[2].x, c[3].x))
            || (between(c[0].y, c[1].y, c[3].y) && between(c[0].y, c[2].y, c[3].y))) {
        return false;
    }
    (void) intersect3(c, c, i);
    return i.used();
}
