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

#define DEBUG_COMPUTE_DELTA 1
#define COMPUTE_DELTA 0

static const double tClipLimit = 0.8; // http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf see Multiple intersections

class CubicIntersections : public Intersections {
public:

CubicIntersections(const Cubic& c1, const Cubic& c2, Intersections& i)
    : cubic1(c1)
    , cubic2(c2)
    , intersections(i)
    , depth(0)
    , splits(0) {
}

bool intersect() {
    double minT1, minT2, maxT1, maxT2;
    if (!bezier_clip(cubic2, cubic1, minT1, maxT1)) {
        return false;
    }
    if (!bezier_clip(cubic1, cubic2, minT2, maxT2)) {
        return false;
    }
    int split;
    if (maxT1 - minT1 < maxT2 - minT2) {
        intersections.swap();
        minT2 = 0;
        maxT2 = 1;
        split = maxT1 - minT1 > tClipLimit;
    } else {
        minT1 = 0;
        maxT1 = 1;
        split = (maxT2 - minT2 > tClipLimit) << 1;
    }
    return chop(minT1, maxT1, minT2, maxT2, split);
}

protected:

bool intersect(double minT1, double maxT1, double minT2, double maxT2) {
    Cubic smaller, larger;
    // FIXME: carry last subdivide and reduceOrder result with cubic
    sub_divide(cubic1, minT1, maxT1, intersections.swapped() ? larger : smaller);
    sub_divide(cubic2, minT2, maxT2, intersections.swapped() ? smaller : larger);
    Cubic smallResult;
    if (reduceOrder(smaller, smallResult,
            kReduceOrder_NoQuadraticsAllowed) <= 2) {
        Cubic largeResult;
        if (reduceOrder(larger, largeResult,
                kReduceOrder_NoQuadraticsAllowed) <= 2) {
            const _Line& smallLine = (const _Line&) smallResult;
            const _Line& largeLine = (const _Line&) largeResult;
            double smallT[2];
            double largeT[2];
            // FIXME: this doesn't detect or deal with coincident lines
            if (!::intersect(smallLine, largeLine, smallT, largeT)) {
                return false;
            }
            if (intersections.swapped()) {
                smallT[0] = interp(minT2, maxT2, smallT[0]);
                largeT[0] = interp(minT1, maxT1, largeT[0]);
            } else {
                smallT[0] = interp(minT1, maxT1, smallT[0]);
                largeT[0] = interp(minT2, maxT2, largeT[0]);
            }
            intersections.add(smallT[0], largeT[0]);
            return true;
        }
    }
    double minT, maxT;
    if (!bezier_clip(smaller, larger, minT, maxT)) {
        if (minT == maxT) {
            if (intersections.swapped()) {
                minT1 = (minT1 + maxT1) / 2;
                minT2 = interp(minT2, maxT2, minT);
            } else {
                minT1 = interp(minT1, maxT1, minT);
                minT2 = (minT2 + maxT2) / 2;
            }
            intersections.add(minT1, minT2);
            return true;
        }
        return false;
    }

    int split;
    if (intersections.swapped()) {
        double newMinT1 = interp(minT1, maxT1, minT);
        double newMaxT1 = interp(minT1, maxT1, maxT);
        split = (newMaxT1 - newMinT1 > (maxT1 - minT1) * tClipLimit) << 1;
#define VERBOSE 0
#if VERBOSE
        printf("%s d=%d s=%d new1=(%g,%g) old1=(%g,%g) split=%d\n",
                __FUNCTION__, depth, splits, newMinT1, newMaxT1, minT1, maxT1,
                split);
#endif
        minT1 = newMinT1;
        maxT1 = newMaxT1;
    } else {
        double newMinT2 = interp(minT2, maxT2, minT);
        double newMaxT2 = interp(minT2, maxT2, maxT);
        split = newMaxT2 - newMinT2 > (maxT2 - minT2) * tClipLimit;
#if VERBOSE
        printf("%s d=%d s=%d new2=(%g,%g) old2=(%g,%g) split=%d\n",
                __FUNCTION__, depth, splits, newMinT2, newMaxT2, minT2, maxT2,
                split);
#endif
        minT2 = newMinT2;
        maxT2 = newMaxT2;
    }
    return chop(minT1, maxT1, minT2, maxT2, split);
}

bool chop(double minT1, double maxT1, double minT2, double maxT2, int split) {
    ++depth;
    intersections.swap();
    if (split) {
        ++splits;
        if (split & 2) {
            double middle1 = (maxT1 + minT1) / 2;
            intersect(minT1, middle1, minT2, maxT2);
            intersect(middle1, maxT1, minT2, maxT2);
        } else {
            double middle2 = (maxT2 + minT2) / 2;
            intersect(minT1, maxT1, minT2, middle2);
            intersect(minT1, maxT1, middle2, maxT2);
        }
        --splits;
        intersections.swap();
        --depth;
        return intersections.intersected();
    }
    bool result = intersect(minT1, maxT1, minT2, maxT2);
    intersections.swap();
    --depth;
    return result;
}

private:

const Cubic& cubic1;
const Cubic& cubic2;
Intersections& intersections;
int depth;
int splits;
};

bool intersect(const Cubic& c1, const Cubic& c2, Intersections& i) {
    CubicIntersections c(c1, c2, i);
    return c.intersect();
}

#if COMPUTE_DELTA
static void cubicTangent(const Cubic& cubic, double t, _Line& tangent, _Point& pt, _Point& dxy) {
    xy_at_t(cubic, t, tangent[0].x, tangent[0].y);
    pt = tangent[1] = tangent[0];
    dxdy_at_t(cubic, t, dxy);
    if (dxy.approximatelyZero()) {
        if (approximately_zero(t)) {
            SkASSERT(cubic[0].approximatelyEqual(cubic[1]));
            dxy = cubic[2];
            dxy -= cubic[0];
        } else {
            SkASSERT(approximately_equal(t, 1));
            SkASSERT(cubic[3].approximatelyEqual(cubic[2]));
            dxy = cubic[3];
            dxy -= cubic[1];
        }
        SkASSERT(!dxy.approximatelyZero());
    }
    tangent[0] -= dxy;
    tangent[1] += dxy;
#if DEBUG_COMPUTE_DELTA
    SkDebugf("%s t=%1.9g tangent=(%1.9g,%1.9g %1.9g,%1.9g)"
            " pt=(%1.9g %1.9g) dxy=(%1.9g %1.9g)\n", __FUNCTION__, t,
            tangent[0].x, tangent[0].y, tangent[1].x, tangent[1].y, pt.x, pt.y,
            dxy.x, dxy.y);
#endif
}
#endif

#if COMPUTE_DELTA
static double cubicDelta(const _Point& dxy, _Line& tangent, double scale)  {
    double tangentLen = dxy.length();
    tangent[0] -= tangent[1];
    double intersectLen = tangent[0].length();
    double result = intersectLen / tangentLen + scale;
#if DEBUG_COMPUTE_DELTA
    SkDebugf("%s tangent=(%1.9g,%1.9g %1.9g,%1.9g) intersectLen=%1.9g tangentLen=%1.9g scale=%1.9g"
            " result=%1.9g\n", __FUNCTION__, tangent[0].x, tangent[0].y, tangent[1].x, tangent[1].y,
            intersectLen, tangentLen, scale, result);
#endif
    return result;
}
#endif

#if COMPUTE_DELTA
// FIXME: after testing, make this static
static void computeDelta(const Cubic& c1, double t1, double scale1, const Cubic& c2, double t2,
        double scale2, double& delta1, double& delta2) {
#if DEBUG_COMPUTE_DELTA
    SkDebugf("%s c1=(%1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g) t1=%1.9g scale1=%1.9g"
            " c2=(%1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g) t2=%1.9g scale2=%1.9g\n",
            __FUNCTION__,
            c1[0].x, c1[0].y, c1[1].x, c1[1].y, c1[2].x, c1[2].y, c1[3].x, c1[3].y, t1, scale1,
            c2[0].x, c2[0].y, c2[1].x, c2[1].y, c2[2].x, c2[2].y, c2[3].x, c2[3].y, t2, scale2);
#endif
    _Line tangent1, tangent2, line1, line2;
    _Point dxy1, dxy2;
    cubicTangent(c1, t1, line1, tangent1[0], dxy1);
    cubicTangent(c2, t2, line2, tangent2[0], dxy2);
    double range1[2], range2[2];
    int found = intersect(line1, line2, range1, range2);
    if (found == 0) {
        range1[0] = 0.5;
    } else {
        SkASSERT(found == 1);
    }
    xy_at_t(line1, range1[0], tangent1[1].x, tangent1[1].y);
#if SK_DEBUG
    if (found == 1) {
        xy_at_t(line2, range2[0], tangent2[1].x, tangent2[1].y);
        SkASSERT(tangent2[1].approximatelyEqual(tangent1[1]));
    }
#endif
    tangent2[1] = tangent1[1];
    delta1 = cubicDelta(dxy1, tangent1, scale1 / precisionUnit);
    delta2 = cubicDelta(dxy2, tangent2, scale2 / precisionUnit);
}

#if SK_DEBUG
int debugDepth;
#endif
#endif

static int quadPart(const Cubic& cubic, double tStart, double tEnd, Quadratic& simple) {
    Cubic part;
    sub_divide(cubic, tStart, tEnd, part);
    Quadratic quad;
    demote_cubic_to_quad(part, quad);
    // FIXME: should reduceOrder be looser in this use case if quartic is going to blow up on an
    // extremely shallow quadratic?
    int order = reduceOrder(quad, simple);
    return order;
}

static void intersectWithOrder(const Quadratic& simple1, int order1, const Quadratic& simple2,
        int order2, Intersections& i) {
    if (order1 == 3 && order2 == 3) {
        intersect2(simple1, simple2, i);
    } else if (order1 <= 2 && order2 <= 2) {
        i.fUsed = intersect((const _Line&) simple1, (const _Line&) simple2, i.fT[0], i.fT[1]);
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

static void doIntersect(const Cubic& cubic1, double t1s, double t1m, double t1e,
        const Cubic& cubic2, double t2s, double t2m, double t2e, Intersections& i) {
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
        #if 0 && SK_DEBUG
            if (0.366025505 >= p1s && 0.366025887 <= p1e
                    && 0.633974342 >= p2s && 0.633975487 <= p2e) {
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
        #if 0 && SK_DEBUG
                SkDebugf("to1=%1.9g p1=(%1.9g,%1.9g) to2=%1.9g p2=(%1.9g,%1.9g) d=%1.9g\n",
                    to1, p1.x, p1.y, to2, p2.x, p2.y, p1.distance(p2));
                    
        #endif
                if (p1.approximatelyEqual(p2)) {
                    i.insert(i.swapped() ? to2 : to1, i.swapped() ? to1 : to2);
                } else {
                    doIntersect(cubic1, p1s, to1, p1e, cubic2, p2s, to2, p2e, i);
                }
            }
            p2s = p2e;
            p2e = t2e;
        }
        p1s = p1e;
        p1e = t1e;
    }
    i.downDepth();
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
                    i.insert(i.swapped() ? to2 : to1, i.swapped() ? to1 : to2);
                } else {
            #if COMPUTE_DELTA
                    double dt1, dt2;
                    computeDelta(cubic1, to1, (t1e - t1s), cubic2, to2, (t2e - t2s), dt1, dt2);
                    double scale = precisionScale;
                    if (dt1 > 0.125 || dt2 > 0.125) {
                        scale /= 2;
                        SkDebugf("%s scale=%1.9g\n", __FUNCTION__, scale);
                    }
#if SK_DEBUG
                    ++debugDepth;
                    SkASSERT(debugDepth < 10);
#endif
                    i.swap();
                    intersect2(cubic2, SkTMax(to2 - dt2, 0.), SkTMin(to2 + dt2, 1.),
                            cubic1, SkTMax(to1 - dt1, 0.), SkTMin(to1 + dt1, 1.), scale, i);
                    i.swap();
#if SK_DEBUG
                    --debugDepth;
#endif
            #else
                    doIntersect(cubic1, t1Start, to1, t1, cubic2, t2Start, to2, t2, i);
            #endif
                }
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
#if SK_DEBUG && COMPUTE_DELTA
    debugDepth = 0;
#endif
    return intersect2(cubic1, start ? 0 : 1, start ? 1.0 / precisionUnit : 1 - 1.0 / precisionUnit,
            cubic2, tMin, tMax, 1, i);
}

// FIXME: add intersection of convex hull on cubics' ends with the opposite cubic. The hull line
// segments can be constructed to be only as long as the calculated precision suggests. If the hull
// line segments intersect the cubic, then use the intersections to construct a subdivision for
// quadratic curve fitting.
bool intersect2(const Cubic& c1, const Cubic& c2, Intersections& i) {
#if SK_DEBUG && COMPUTE_DELTA
    debugDepth = 0;
#endif
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

int intersect(const Cubic& cubic, const Quadratic& quad, Intersections& i) {
    SkTDArray<double> ts;
    double precision = calcPrecision(cubic);
    cubic_to_quadratics(cubic, precision, ts);
    double tStart = 0;
    Cubic part;
    int tsCount = ts.count();
    for (int idx = 0; idx <= tsCount; ++idx) {
        double t = idx < tsCount ? ts[idx] : 1;
        Quadratic q1;
        sub_divide(cubic, tStart, t, part);
        demote_cubic_to_quad(part, q1);
        Intersections locals;
        intersect2(q1, quad, locals);
        for (int tIdx = 0; tIdx < locals.used(); ++tIdx) {
            double globalT = tStart + (t - tStart) * locals.fT[0][tIdx];
            i.insertOne(globalT, 0);
            globalT = locals.fT[1][tIdx];
            i.insertOne(globalT, 1);
        }
        tStart = t;
    }
    return i.used();
}

bool intersect(const Cubic& cubic, Intersections& i) {
    SkTDArray<double> ts;
    double precision = calcPrecision(cubic);
    cubic_to_quadratics(cubic, precision, ts);
    int tsCount = ts.count();
    if (tsCount == 1) {
        return false;
    }
    double t1Start = 0;
    Cubic part;
    for (int idx = 0; idx < tsCount; ++idx) {
        double t1 = ts[idx];
        Quadratic q1;
        sub_divide(cubic, t1Start, t1, part);
        demote_cubic_to_quad(part, q1);
        double t2Start = t1;
        for (int i2 = idx + 1; i2 <= tsCount; ++i2) {
            const double t2 = i2 < tsCount ? ts[i2] : 1;
            Quadratic q2;
            sub_divide(cubic, t2Start, t2, part);
            demote_cubic_to_quad(part, q2);
            Intersections locals;
            intersect2(q1, q2, locals);
            for (int tIdx = 0; tIdx < locals.used(); ++tIdx) {
            // discard intersections at cusp? (maximum curvature)
                double t1sect = locals.fT[0][tIdx];
                double t2sect = locals.fT[1][tIdx];
                if (idx + 1 == i2 && t1sect == 1 && t2sect == 0) {
                    continue;
                }
                double to1 = t1Start + (t1 - t1Start) * t1sect;
                double to2 = t2Start + (t2 - t2Start) * t2sect;
                i.insert(to1, to2);
            }
            t2Start = t2;
        }
        t1Start = t1;
    }
    return i.intersected();
}
