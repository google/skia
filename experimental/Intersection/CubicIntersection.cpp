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

// this flavor approximates the cubics with quads to find the intersecting ts
// OPTIMIZE: if this strategy proves successful, the quad approximations, or the ts used
// to create the approximations, could be stored in the cubic segment
// fixme: this strategy needs to add short line segments on either end, or similarly extend the
// initial and final quadratics
bool intersect2(const Cubic& c1, const Cubic& c2, Intersections& i) {
    SkTDArray<double> ts1;
    double precision1 = calcPrecision(c1);
    cubic_to_quadratics(c1, precision1, ts1);
    SkTDArray<double> ts2;
    double precision2 = calcPrecision(c2);
    cubic_to_quadratics(c2, precision2, ts2);
    double t1Start = 0;
    int ts1Count = ts1.count();
    for (int i1 = 0; i1 <= ts1Count; ++i1) {
        const double t1 = i1 < ts1Count ? ts1[i1] : 1;
        Cubic part1;
        sub_divide(c1, t1Start, t1, part1);
        Quadratic q1;
        demote_cubic_to_quad(part1, q1);
  //      start here;
        // should reduceOrder be looser in this use case if quartic is going to blow up on an
        // extremely shallow quadratic?
        // maybe quadratics to lines need the same sort of recursive solution that I hope to find
        // for cubics to quadratics ...
        Quadratic s1;
        int o1 = reduceOrder(q1, s1);
        double t2Start = 0;
        int ts2Count = ts2.count();
        for (int i2 = 0; i2 <= ts2Count; ++i2) {
            const double t2 = i2 < ts2Count ? ts2[i2] : 1;
            Cubic part2;
            sub_divide(c2, t2Start, t2, part2);
            Quadratic q2;
            demote_cubic_to_quad(part2, q2);
            Quadratic s2;
            double o2 = reduceOrder(q2, s2);
            Intersections locals;
            if (o1 == 3 && o2 == 3) {
                intersect2(q1, q2, locals);
            } else if (o1 <= 2 && o2 <= 2) {
                i.fUsed = intersect((const _Line&) s1, (const _Line&) s2, i.fT[0], i.fT[1]);
            } else if (o1 == 3 && o2 <= 2) {
                intersect(q1, (const _Line&) s2, i);
            } else {
                SkASSERT(o1 <= 2 && o2 == 3);
                intersect(q2, (const _Line&) s1, i);
                for (int s = 0; s < i.fUsed; ++s) {
                    SkTSwap(i.fT[0][s], i.fT[1][s]);
                }
            }
            for (int tIdx = 0; tIdx < locals.used(); ++tIdx) {
                double to1 = t1Start + (t1 - t1Start) * locals.fT[0][tIdx];
                double to2 = t2Start + (t2 - t2Start) * locals.fT[1][tIdx];
                i.insert(to1, to2);
            }
            t2Start = t2;
        }
        t1Start = t1;
    }
    return i.intersected();
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
