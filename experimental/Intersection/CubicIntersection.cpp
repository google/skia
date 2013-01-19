/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "Intersections.h"
#include "IntersectionUtilities.h"
#include "LineIntersection.h"

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

#include "CubicUtilities.h"

// FIXME: ? if needed, compute the error term from the tangent length
static double computeDelta(const Cubic& cubic, double t, double scale) {
    double attempt = scale / precisionUnit;
#if SK_DEBUG
    double precision = calcPrecision(cubic);
    _Point dxy;
    dxdy_at_t(cubic, t, dxy);
    _Point p1, p2;
    xy_at_t(cubic, std::max(t - attempt, 0.), p1.x, p1.y);
    xy_at_t(cubic, std::min(t + attempt, 1.), p2.x, p2.y);
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    double distSq = dx * dx + dy * dy;
    double dist = sqrt(distSq);
    assert(dist > precision);
#endif
    return attempt;
}

// this flavor approximates the cubics with quads to find the intersecting ts
// OPTIMIZE: if this strategy proves successful, the quad approximations, or the ts used
// to create the approximations, could be stored in the cubic segment
// FIXME: this strategy needs to intersect the convex hull on either end with the opposite to
// account for inset quadratics that cause the endpoint intersection to avoid detection
// the segments can be very short -- the length of the maximum quadratic error (precision)
// FIXME: this needs to recurse on itself, taking a range of T values and computing the new
// t range ala is linear inner. The range can be figured by taking the dx/dy and determining
// the fraction that matches the precision. That fraction is the change in t for the smaller cubic.
static bool intersect2(const Cubic& cubic1, double t1s, double t1e, const Cubic& cubic2,
        double t2s, double t2e, Intersections& i) {
    Cubic c1, c2;
    sub_divide(cubic1, t1s, t1e, c1);
    sub_divide(cubic2, t2s, t2e, c2);
    SkTDArray<double> ts1;
    cubic_to_quadratics(c1, calcPrecision(c1), ts1);
    SkTDArray<double> ts2;
    cubic_to_quadratics(c2, calcPrecision(c2), ts2);
    double t1Start = t1s;
    int ts1Count = ts1.count();
    for (int i1 = 0; i1 <= ts1Count; ++i1) {
        const double tEnd1 = i1 < ts1Count ? ts1[i1] : 1;
        const double t1 = t1s + (t1e - t1s) * tEnd1;
        Cubic part1;
        sub_divide(cubic1, t1Start, t1, part1);
        Quadratic q1;
        demote_cubic_to_quad(part1, q1);
  //      start here;
        // should reduceOrder be looser in this use case if quartic is going to blow up on an
        // extremely shallow quadratic?
        Quadratic s1;
        int o1 = reduceOrder(q1, s1);
        double t2Start = t2s;
        int ts2Count = ts2.count();
        for (int i2 = 0; i2 <= ts2Count; ++i2) {
            const double tEnd2 = i2 < ts2Count ? ts2[i2] : 1;
            const double t2 = t2s + (t2e - t2s) * tEnd2;
            Cubic part2;
            sub_divide(cubic2, t2Start, t2, part2);
            Quadratic q2;
            demote_cubic_to_quad(part2, q2);
            Quadratic s2;
            double o2 = reduceOrder(q2, s2);
            Intersections locals;
            if (o1 == 3 && o2 == 3) {
                intersect2(q1, q2, locals);
            } else if (o1 <= 2 && o2 <= 2) {
                locals.fUsed = intersect((const _Line&) s1, (const _Line&) s2, locals.fT[0],
                        locals.fT[1]);
            } else if (o1 == 3 && o2 <= 2) {
                intersect(q1, (const _Line&) s2, locals);
            } else {
                SkASSERT(o1 <= 2 && o2 == 3);
                intersect(q2, (const _Line&) s1, locals);
                for (int s = 0; s < locals.fUsed; ++s) {
                    SkTSwap(locals.fT[0][s], locals.fT[1][s]);
                }
            }
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
                    double dt1 = computeDelta(cubic1, to1, t1e - t1s);
                    double dt2 = computeDelta(cubic2, to2, t2e - t2s);
                    i.swap();
                    intersect2(cubic2, std::max(to2 - dt2, 0.), std::min(to2 + dt2, 1.),
                            cubic1, std::max(to1 - dt1, 0.), std::min(to1 + dt1, 1.), i);
                    i.swap();
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
    line1[0] = line1[1] = cubic1[start ? 0 : 3];
    _Point dxy1 = line1[0] - cubic1[start ? 1 : 2];
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
        tMin = std::min(tMin, local1.fT[0][index]);
        tMax = std::max(tMax, local1.fT[0][index]);
    }
    for (int index = 1; index < local2.fUsed; ++index) {
        tMin = std::min(tMin, local2.fT[0][index]);
        tMax = std::max(tMax, local2.fT[0][index]);
    }
    return intersect2(cubic1, start ? 0 : 1, start ? 1.0 / precisionUnit : 1 - 1.0 / precisionUnit,
            cubic2, tMin, tMax, i);
}

// FIXME: add intersection of convex null on cubics' ends with the opposite cubic. The hull line
// segments can be constructed to be only as long as the calculated precision suggests. If the hull
// line segments intersect the cubic, then use the intersections to construct a subdivision for
// quadratic curve fitting.
bool intersect2(const Cubic& c1, const Cubic& c2, Intersections& i) {
    bool result = intersect2(c1, 0, 1, c2, 0, 1, i);
    // FIXME: pass in cached bounds from caller
    _Rect c1Bounds, c2Bounds;
    c1Bounds.setBounds(c1); // OPTIMIZE use setRawBounds ?
    c2Bounds.setBounds(c2);
    result |= intersectEnd(c1, false, c2, c2Bounds, i);
    result |= intersectEnd(c1, true, c2, c2Bounds, i);
    result |= intersectEnd(c2, false, c1, c1Bounds, i);
    result |= intersectEnd(c2, true, c1, c1Bounds, i);
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
