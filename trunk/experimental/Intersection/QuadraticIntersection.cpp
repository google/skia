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
#include "LineUtilities.h"
#include "QuadraticLineSegments.h"
#include "QuadraticUtilities.h"
#include <algorithm> // for swap

static const double tClipLimit = 0.8; // http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf see Multiple intersections

class QuadraticIntersections {
public:

QuadraticIntersections(const Quadratic& q1, const Quadratic& q2, Intersections& i)
    : quad1(q1)
    , quad2(q2)
    , intersections(i)
    , depth(0)
    , splits(0)
    , coinMinT1(-1) {
}

bool intersect() {
    double minT1, minT2, maxT1, maxT2;
    if (!bezier_clip(quad2, quad1, minT1, maxT1)) {
        return false;
    }
    if (!bezier_clip(quad1, quad2, minT2, maxT2)) {
        return false;
    }
    quad1Divisions = 1 / subDivisions(quad1);
    quad2Divisions = 1 / subDivisions(quad2);
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
    bool t1IsLine = maxT1 - minT1 <= quad1Divisions;
    bool t2IsLine = maxT2 - minT2 <= quad2Divisions;
    if (t1IsLine | t2IsLine) {
        return intersectAsLine(minT1, maxT1, minT2, maxT2, t1IsLine, t2IsLine);
    }
    Quadratic smaller, larger;
    // FIXME: carry last subdivide and reduceOrder result with quad
    sub_divide(quad1, minT1, maxT1, intersections.swapped() ? larger : smaller);
    sub_divide(quad2, minT2, maxT2, intersections.swapped() ? smaller : larger);
    double minT, maxT;
    if (!bezier_clip(smaller, larger, minT, maxT)) {
        if (approximately_equal(minT, maxT)) {
            double smallT, largeT;
            _Point q2pt, q1pt;
            if (intersections.swapped()) {
                largeT = interp(minT2, maxT2, minT);
                xy_at_t(quad2, largeT, q2pt.x, q2pt.y);
                xy_at_t(quad1, minT1, q1pt.x, q1pt.y);
                if (approximately_equal(q2pt.x, q1pt.x) && approximately_equal(q2pt.y, q1pt.y)) {
                    smallT = minT1;
                } else {
                    xy_at_t(quad1, maxT1, q1pt.x, q1pt.y); // FIXME: debug code
                    assert(approximately_equal(q2pt.x, q1pt.x) && approximately_equal(q2pt.y, q1pt.y));
                    smallT = maxT1;
                }
            } else {
                smallT = interp(minT1, maxT1, minT);
                xy_at_t(quad1, smallT, q1pt.x, q1pt.y);
                xy_at_t(quad2, minT2, q2pt.x, q2pt.y);
                if (approximately_equal(q2pt.x, q1pt.x) && approximately_equal(q2pt.y, q1pt.y)) {
                    largeT = minT2;
                } else {
                    xy_at_t(quad2, maxT2, q2pt.x, q2pt.y); // FIXME: debug code
                    assert(approximately_equal(q2pt.x, q1pt.x) && approximately_equal(q2pt.y, q1pt.y));
                    largeT = maxT2;
                }
            }
            intersections.add(smallT, largeT);
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
        printf("%s d=%d s=%d new1=(%g,%g) old1=(%g,%g) split=%d\n", __FUNCTION__, depth,
            splits, newMinT1, newMaxT1, minT1, maxT1, split);
#endif
        minT1 = newMinT1;
        maxT1 = newMaxT1;
    } else {
        double newMinT2 = interp(minT2, maxT2, minT);
        double newMaxT2 = interp(minT2, maxT2, maxT);
        split = newMaxT2 - newMinT2 > (maxT2 - minT2) * tClipLimit;
#if VERBOSE
        printf("%s d=%d s=%d new2=(%g,%g) old2=(%g,%g) split=%d\n", __FUNCTION__, depth,
            splits, newMinT2, newMaxT2, minT2, maxT2, split);
#endif
        minT2 = newMinT2;
        maxT2 = newMaxT2;
    }
    return chop(minT1, maxT1, minT2, maxT2, split);
}

bool intersectAsLine(double minT1, double maxT1, double minT2, double maxT2,
       bool treat1AsLine, bool treat2AsLine)
{
    _Line line1, line2;
    if (intersections.swapped()) {
        std::swap(treat1AsLine, treat2AsLine);
        std::swap(minT1, minT2);
        std::swap(maxT1, maxT2);
    }
    if (coinMinT1 >= 0) {
        bool earlyExit;
        if ((earlyExit = coinMaxT1 == minT1)) {
            coinMaxT1 = maxT1;
        }
        if (coinMaxT2 == minT2) {
            coinMaxT2 = maxT2;
            return true;
        }
        if (earlyExit) {
            return true;
        }
        coinMinT1 = -1;
    }
    // do line/quadratic or even line/line intersection instead
    if (treat1AsLine) {
        xy_at_t(quad1, minT1, line1[0].x, line1[0].y);
        xy_at_t(quad1, maxT1, line1[1].x, line1[1].y);
    }
    if (treat2AsLine) {
        xy_at_t(quad2, minT2, line2[0].x, line2[0].y);
        xy_at_t(quad2, maxT2, line2[1].x, line2[1].y);
    }
    int pts;
    double smallT1, largeT1, smallT2, largeT2;
    if (treat1AsLine & treat2AsLine) {
        double t1[2], t2[2];
        pts = ::intersect(line1, line2, t1, t2);
        if (pts == 2) {
            smallT1 = interp(minT1, maxT1, t1[0]);
            largeT1 = interp(minT2, maxT2, t2[0]);
            smallT2 = interp(minT1, maxT1, t1[1]);
            largeT2 = interp(minT2, maxT2, t2[1]);
            intersections.addCoincident(smallT1, smallT2, largeT1, largeT2);
        } else {
            smallT1 = interp(minT1, maxT1, t1[0]);
            largeT1 = interp(minT2, maxT2, t2[0]);
            intersections.add(smallT1, largeT1);
        }
    } else {
        Intersections lq;
        pts = ::intersect(treat1AsLine ? quad2 : quad1,
                treat1AsLine ? line1 : line2, lq);
        if (pts == 2) { // if the line and edge are coincident treat differently
            _Point midQuad, midLine;
            double midQuadT = (lq.fT[0][0] + lq.fT[0][1]) / 2;
            xy_at_t(treat1AsLine ? quad2 : quad1, midQuadT, midQuad.x, midQuad.y);
            double lineT = t_at(treat1AsLine ? line1 : line2, midQuad);
            xy_at_t(treat1AsLine ? line1 : line2, lineT, midLine.x, midLine.y);
            if (approximately_equal(midQuad.x, midLine.x)
                    && approximately_equal(midQuad.y, midLine.y)) {
                smallT1 = lq.fT[0][0];
                largeT1 = lq.fT[1][0];
                smallT2 = lq.fT[0][1];
                largeT2 = lq.fT[1][1];
                if (treat2AsLine) {
                    smallT1 = interp(minT1, maxT1, smallT1);
                    smallT2 = interp(minT1, maxT1, smallT2);
                } else {
                    largeT1 = interp(minT2, maxT2, largeT1);
                    largeT2 = interp(minT2, maxT2, largeT2);
                }
                intersections.addCoincident(smallT1, smallT2, largeT1, largeT2);
                goto setCoinMinMax;
            }
        }
        for (int index = 0; index < pts; ++index) {
            smallT1 = lq.fT[0][index];
            largeT1 = lq.fT[1][index];
            if (treat2AsLine) {
                smallT1 = interp(minT1, maxT1, smallT1);
            } else {
                largeT1 = interp(minT2, maxT2, largeT1);
            }
            intersections.add(smallT1, largeT1);
        }
    }
    if (pts > 0) {
setCoinMinMax:
        coinMinT1 = minT1;
        coinMaxT1 = maxT1;
        coinMinT2 = minT2;
        coinMaxT2 = maxT2;
    }
    return pts > 0;
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

const Quadratic& quad1;
const Quadratic& quad2;
Intersections& intersections;
int depth;
int splits;
double quad1Divisions; // line segments to approximate original within error
double quad2Divisions;
double coinMinT1; // range of Ts where approximate line intersected curve
double coinMaxT1;
double coinMinT2;
double coinMaxT2;
};

#include "LineParameters.h"

static void hackToFixPartialCoincidence(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
    // look to see if non-coincident data basically has unsortable tangents

    // look to see if a point between non-coincident data is on the curve
    int cIndex;
    for (int uIndex = 0; uIndex < i.fUsed; ) {
        double bestDist1 = 1;
        double bestDist2 = 1;
        int closest1 = -1;
        int closest2 = -1;
        for (cIndex = 0; cIndex < i.fCoincidentUsed; ++cIndex) {
            double dist = fabs(i.fT[0][uIndex] - i.fCoincidentT[0][cIndex]);
            if (bestDist1 > dist) {
                bestDist1 = dist;
                closest1 = cIndex;
            }
            dist = fabs(i.fT[1][uIndex] - i.fCoincidentT[1][cIndex]);
            if (bestDist2 > dist) {
                bestDist2 = dist;
                closest2 = cIndex;
            }
        }
        _Line ends;
        _Point mid;
        double t1 = i.fT[0][uIndex];
        xy_at_t(q1, t1, ends[0].x, ends[0].y);
        xy_at_t(q1, i.fCoincidentT[0][closest1], ends[1].x, ends[1].y);
        double midT = (t1 + i.fCoincidentT[0][closest1]) / 2;
        xy_at_t(q1, midT, mid.x, mid.y);
        LineParameters params;
        params.lineEndPoints(ends);
        double midDist = params.pointDistance(mid);
        // Note that we prefer to always measure t error, which does not scale,
        // instead of point error, which is scale dependent. FIXME
        if (!approximately_zero(midDist)) {
            ++uIndex;
            continue;
        }
        double t2 = i.fT[1][uIndex];
        xy_at_t(q2, t2, ends[0].x, ends[0].y);
        xy_at_t(q2, i.fCoincidentT[1][closest2], ends[1].x, ends[1].y);
        midT = (t2 + i.fCoincidentT[1][closest2]) / 2;
        xy_at_t(q2, midT, mid.x, mid.y);
        params.lineEndPoints(ends);
        midDist = params.pointDistance(mid);
        if (!approximately_zero(midDist)) {
            ++uIndex;
            continue;
        }
        // if both midpoints are close to the line, lengthen coincident span
        int cEnd = closest1 ^ 1; // assume coincidence always travels in pairs
        if (!between(i.fCoincidentT[0][cEnd], t1, i.fCoincidentT[0][closest1])) {
            i.fCoincidentT[0][closest1] = t1;
        }
        cEnd = closest2 ^ 1;
        if (!between(i.fCoincidentT[0][cEnd], t2, i.fCoincidentT[0][closest2])) {
            i.fCoincidentT[0][closest2] = t2;
        }
        int remaining = --i.fUsed - uIndex;
        if (remaining > 0) {
            memmove(&i.fT[0][uIndex], &i.fT[0][uIndex + 1], sizeof(i.fT[0][0]) * remaining);
            memmove(&i.fT[1][uIndex], &i.fT[1][uIndex + 1], sizeof(i.fT[1][0]) * remaining);
        }
    }
    // if coincident data is subjectively a tiny span, replace it with a single point
    for (cIndex = 0; cIndex < i.fCoincidentUsed; ) {
        double start1 = i.fCoincidentT[0][cIndex];
        double end1 = i.fCoincidentT[0][cIndex + 1];
        _Line ends1;
        xy_at_t(q1, start1, ends1[0].x, ends1[0].y);
        xy_at_t(q1, end1, ends1[1].x, ends1[1].y);
        if (!approximately_equal(ends1[0].x, ends1[1].x) || approximately_equal(ends1[0].y, ends1[1].y)) {
            cIndex += 2;
            continue;
        }
        double start2 = i.fCoincidentT[1][cIndex];
        double end2 = i.fCoincidentT[1][cIndex + 1];
        _Line ends2;
        xy_at_t(q2, start2, ends2[0].x, ends2[0].y);
        xy_at_t(q2, end2, ends2[1].x, ends2[1].y);
        // again, approximately should be used with T values, not points FIXME
        if (!approximately_equal(ends2[0].x, ends2[1].x) || approximately_equal(ends2[0].y, ends2[1].y)) {
            cIndex += 2;
            continue;
        }
        if (approximately_less_than_zero(start1) || approximately_less_than_zero(end1)) {
            start1 = 0;
        } else if (approximately_greater_than_one(start1) || approximately_greater_than_one(end1)) {
            start1 = 1;
        } else {
            start1 = (start1 + end1) / 2;
        }
        if (approximately_less_than_zero(start2) || approximately_less_than_zero(end2)) {
            start2 = 0;
        } else if (approximately_greater_than_one(start2) || approximately_greater_than_one(end2)) {
            start2 = 1;
        } else {
            start2 = (start2 + end2) / 2;
        }
        i.insert(start1, start2);
        i.fCoincidentUsed -= 2;
        int remaining = i.fCoincidentUsed - cIndex;
        if (remaining > 0) {
            memmove(&i.fCoincidentT[0][cIndex], &i.fCoincidentT[0][cIndex + 2], sizeof(i.fCoincidentT[0][0]) * remaining);
            memmove(&i.fCoincidentT[1][cIndex], &i.fCoincidentT[1][cIndex + 2], sizeof(i.fCoincidentT[1][0]) * remaining);
        }
    }
}

bool intersect(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
    if (implicit_matches(q1, q2)) {
        // FIXME: compute T values
        // compute the intersections of the ends to find the coincident span
        bool useVertical = fabs(q1[0].x - q1[2].x) < fabs(q1[0].y - q1[2].y);
        double t;
        if ((t = axialIntersect(q1, q2[0], useVertical)) >= 0) {
            i.addCoincident(t, 0);
        }
        if ((t = axialIntersect(q1, q2[2], useVertical)) >= 0) {
            i.addCoincident(t, 1);
        }
        useVertical = fabs(q2[0].x - q2[2].x) < fabs(q2[0].y - q2[2].y);
        if ((t = axialIntersect(q2, q1[0], useVertical)) >= 0) {
            i.addCoincident(0, t);
        }
        if ((t = axialIntersect(q2, q1[2], useVertical)) >= 0) {
            i.addCoincident(1, t);
        }
        assert(i.fCoincidentUsed <= 2);
        return i.fCoincidentUsed > 0;
    }
    QuadraticIntersections q(q1, q2, i);
    bool result = q.intersect();
    // FIXME: partial coincidence detection is currently poor. For now, try
    // to fix up the data after the fact. In the future, revisit the error
    // term to try to avoid this kind of result in the first place.
    if (i.fUsed && i.fCoincidentUsed) {
        hackToFixPartialCoincidence(q1, q2, i);
    }
    return result;
}
