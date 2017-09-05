/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRGeometry.h"

#include "GrTypes.h"
#include "SkGeometry.h"
#include "SkPoint.h"
#include "../pathops/SkPathOpsCubic.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

// We convert between SkPoint and Sk2f freely throughout this file.
GR_STATIC_ASSERT(SK_SCALAR_IS_FLOAT);
GR_STATIC_ASSERT(2 * sizeof(float) == sizeof(SkPoint));
GR_STATIC_ASSERT(0 == offsetof(SkPoint, fX));

void GrCCPRGeometry::beginPath() {
    SkASSERT(!fBuildingContour);
    fVerbs.push_back(Verb::kBeginPath);
}

void GrCCPRGeometry::beginContour(const SkPoint& devPt) {
    SkASSERT(!fBuildingContour);

    fCurrFanPoint = fCurrAnchorPoint = devPt;

    // Store the current verb count in the fTriangles field for now. When we close the contour we
    // will use this value to calculate the actual number of triangles in its fan.
    fCurrContourTallies = {fVerbs.count(), 0, 0, 0};

    fPoints.push_back(devPt);
    fVerbs.push_back(Verb::kBeginContour);

    SkDEBUGCODE(fBuildingContour = true;)
}

void GrCCPRGeometry::lineTo(const SkPoint& devPt) {
    SkASSERT(fBuildingContour);
    fCurrFanPoint = devPt;
    fPoints.push_back(devPt);
    fVerbs.push_back(Verb::kLineTo);
}

static inline Sk2f normalize(const Sk2f& n) {
    Sk2f nn = n*n;
    return n * (nn + SkNx_shuffle<1,0>(nn)).rsqrt();
}

static inline float dot(const Sk2f& a, const Sk2f& b) {
    float product[2];
    (a * b).store(product);
    return product[0] + product[1];
}

// Returns whether the (convex) curve segment is monotonic with respect to [endPt - startPt].
static inline bool is_convex_curve_monotonic(const Sk2f& startPt, const Sk2f& startTan,
                                             const Sk2f& endPt, const Sk2f& endTan) {
    Sk2f v = endPt - startPt;
    float dot0 = dot(startTan, v);
    float dot1 = dot(endTan, v);

    // A small, negative tolerance handles floating-point error in the case when one tangent
    // approaches 0 length, meaning the (convex) curve segment is effectively a flat line.
    float tolerance = -std::max(std::abs(dot0), std::abs(dot1)) * SK_ScalarNearlyZero;
    return dot0 >= tolerance && dot1 >= tolerance;
}

static inline Sk2f lerp(const Sk2f& a, const Sk2f& b, const Sk2f& t) {
    return SkNx_fma(t, b - a, a);
}

void GrCCPRGeometry::quadraticTo(const SkPoint& devP0, const SkPoint& devP1) {
    SkASSERT(fBuildingContour);

    Sk2f p0 = Sk2f::Load(&fCurrFanPoint);
    Sk2f p1 = Sk2f::Load(&devP0);
    Sk2f p2 = Sk2f::Load(&devP1);
    fCurrFanPoint = devP1;

    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p2 - p1;
    // This should almost always be this case for well-behaved curves in the real world.
    if (is_convex_curve_monotonic(p0, tan0, p2, tan1)) {
        this->appendMonotonicQuadratic(p1, p2);
        return;
    }

    // Chop the curve into two segments with equal curvature. To do this we find the T value whose
    // tangent is perpendicular to the vector that bisects tan0 and -tan1.
    Sk2f n = normalize(tan0) - normalize(tan1);

    // This tangent can be found where (dQ(t) dot n) = 0:
    //
    //   0 = (dQ(t) dot n) = | 2*t  1 | * | p0 - 2*p1 + p2 | * | n |
    //                                    | -2*p0 + 2*p1   |   | . |
    //
    //                     = | 2*t  1 | * | tan1 - tan0 | * | n |
    //                                    | 2*tan0      |   | . |
    //
    //                     = 2*t * ((tan1 - tan0) dot n) + (2*tan0 dot n)
    //
    //   t = (tan0 dot n) / ((tan0 - tan1) dot n)
    Sk2f dQ1n = (tan0 - tan1) * n;
    Sk2f dQ0n = tan0 * n;
    Sk2f t = (dQ0n + SkNx_shuffle<1,0>(dQ0n)) / (dQ1n + SkNx_shuffle<1,0>(dQ1n));
    t = Sk2f::Min(Sk2f::Max(t, 0), 1); // Clamp for FP error.

    Sk2f p01 = SkNx_fma(t, tan0, p0);
    Sk2f p12 = SkNx_fma(t, tan1, p1);
    Sk2f p012 = lerp(p01, p12, t);

    this->appendMonotonicQuadratic(p01, p012);
    this->appendMonotonicQuadratic(p12, p2);
}

inline void GrCCPRGeometry::appendMonotonicQuadratic(const Sk2f& p1, const Sk2f& p2) {
    p1.store(&fPoints.push_back());
    p2.store(&fPoints.push_back());
    fVerbs.push_back(Verb::kMonotonicQuadraticTo);
    ++fCurrContourTallies.fQuadratics;
}

void GrCCPRGeometry::cubicTo(const SkPoint& devP1, const SkPoint& devP2, const SkPoint& devP3) {
    SkASSERT(fBuildingContour);

    SkPoint P[4] = {fCurrFanPoint, devP1, devP2, devP3};
    double t[2], s[2];
    SkCubicType type = SkClassifyCubic(P, t, s);

    if (SkCubicType::kLineOrPoint == type) {
        this->lineTo(P[3]);
        return;
    }

    if (SkCubicType::kQuadratic == type) {
        SkPoint quadP1 = (devP1 + devP2) * .75f - (fCurrFanPoint + devP3) * .25f;
        this->quadraticTo(quadP1, devP3);
        return;
    }

    fCurrFanPoint = devP3;

    SkDCubic C;
    C.set(P);

    for (int x = 0; x <= 1; ++x) {
        if (t[x] * s[x] <= 0) { // This is equivalent to tx/sx <= 0.
            // This technically also gets taken if tx/sx = infinity, but the code still does
            // the right thing in that edge case.
            continue; // Don't increment x0.
        }
        if (fabs(t[x]) >= fabs(s[x])) { // tx/sx >= 1.
            break;
        }

        const double chopT = double(t[x]) / double(s[x]);
        SkASSERT(chopT >= 0 && chopT <= 1);
        if (chopT <= 0 || chopT >= 1) { // floating-point error.
            continue;
        }

        SkDCubicPair chopped = C.chopAt(chopT);

        // Ensure the double points are identical if this is a loop (more workarounds for FP error).
        if (SkCubicType::kLoop == type && 0 == t[0]) {
            chopped.pts[3] = chopped.pts[0];
        }

        // (This might put ts0/ts1 out of order, but it doesn't matter anymore at this point.)
        this->appendConvexCubic(type, chopped.first());
        t[x] = 0;
        s[x] = 1;

        const double r = s[1 - x] * chopT;
        t[1 - x] -= r;
        s[1 - x] -= r;

        C = chopped.second();
    }

    this->appendConvexCubic(type, C);
}

static SkPoint to_skpoint(const SkDPoint& dpoint) {
    return {static_cast<SkScalar>(dpoint.fX), static_cast<SkScalar>(dpoint.fY)};
}

inline void GrCCPRGeometry::appendConvexCubic(SkCubicType type, const SkDCubic& C) {
    fPoints.push_back(to_skpoint(C[1]));
    fPoints.push_back(to_skpoint(C[2]));
    fPoints.push_back(to_skpoint(C[3]));
    if (SkCubicType::kLoop != type) {
        fVerbs.push_back(Verb::kConvexSerpentineTo);
        ++fCurrContourTallies.fSerpentines;
    } else {
        fVerbs.push_back(Verb::kConvexLoopTo);
        ++fCurrContourTallies.fLoops;
    }
}

GrCCPRGeometry::PrimitiveTallies GrCCPRGeometry::endContour() {
    SkASSERT(fBuildingContour);
    SkASSERT(fVerbs.count() >= fCurrContourTallies.fTriangles);

    // The fTriangles field currently contains this contour's starting verb index. We can now
    // use it to calculate the size of the contour's fan.
    int fanSize = fVerbs.count() - fCurrContourTallies.fTriangles;
    if (fCurrFanPoint == fCurrAnchorPoint) {
        --fanSize;
        fVerbs.push_back(Verb::kEndClosedContour);
    } else {
        fVerbs.push_back(Verb::kEndOpenContour);
    }

    fCurrContourTallies.fTriangles = SkTMax(fanSize - 2, 0);

    SkDEBUGCODE(fBuildingContour = false;)
    return fCurrContourTallies;
}
