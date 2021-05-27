/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArcLengthParameterization_DEFINED
#define SkArcLengthParameterization_DEFINED

#include "include/core/SkPath.h"

#include <array>
#include <vector>

/**
 * Computes and stores an arc length reparameterization of an SkPath.
 *
 * Technique from:
 *
 * P. Costantini, R. Farouki, C. Manni, A. Sestini. "Computation of optimal composite
 * re-parameterizations." CAGD 18 (2001) p.875-897.
 * <http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.90.5406&rep=rep1&type=pdf>
 */
class SkArcLengthParameterization {
public:
    // A Curve is a Bezier segment (verb+points) in the original path except in the case of conics,
    // which are converted to quads.
    struct Curve {
        SkPathVerb fVerb;     // Verb
        const SkPoint* fPts;  // Points
        float fLength;        // Arc length of this curve
        float fUMin, fUMax;   // Interval in global u domain occupied by the curve
        size_t fIndex;        // Internal index
    };

    explicit SkArcLengthParameterization(const SkPath& path);

    // Total length of path
    float totalLength() const { return fTotalLength; }

    // Global distance along path -> global u in [0,1]
    float mapDistanceToU(float distance) const { return distance / fTotalLength; }

    // Local t -> global u
    float mapTToU(const Curve& curve, float t) const;

    // Global u -> local t
    float mapUToT(const Curve& curve, float u) const;

    // Get the Bezier segment of the path containing global u, and return the relative t value.
    Curve getCurve(float u, float* t = nullptr) const;

    // From the given curve at global u, advance by du and return the new segment and relative t
    // value. This is more efficient for iteration than repeated calls to getCurve().
    Curve advance(const Curve& curve, float u, float du, float* t = nullptr) const;

    // Gets the next curve after the given curve
    bool advance(const Curve& curve, Curve* nextCurve) const;

private:
    /**
     * Overall knob that controls accuracy of approximation. Approximation is 1+O(1/N^4)
     * (per Bezier path segment) times the true arc length parameterization.
     */
    static constexpr int kN = 5;

    // Fixing the choice of t_i to be even distribution over [0,1] per segment
    static constexpr std::array<float, kN + 1> T = {
            0, 1.f / kN, 2.f / kN, 3.f / kN, 4.f / kN, 1,
    };

    using AlphaVals = std::array<float, kN>;
    // TODO: u[0]=0 and u[kN]=1 always, so we could skip storing them
    // TODO: linear segments don't use the approximation, so these could be vectors
    using UVals = std::array<float, kN + 1>;

    struct CurveInfo {
        SkPathVerb fVerb;        // Verb of curve
        const SkPoint* fPoints;  // Points of curve
        size_t fPointIndex;      // Index into points array
        float fLength;           // Length of curve
        float fUMin, fUMax;      // Interval in global u domain occupied by the curve
        AlphaVals fAlpha;
        UVals fU;
    };

    struct BuildContext {
        size_t fPointIndex{0};
        float fTotalLength{0};
        std::vector<CurveInfo> fCurveInfo;
        std::vector<SkPoint> fConicQuadPts;
    };

    Curve toCurve(const CurveInfo& info, size_t index) const;

    void build(const SkPath& path);

    static void buildLine(const SkPoint* pts, BuildContext* ctx);

    static void buildQuad(const SkPoint* pts, BuildContext* ctx);

    static void buildConic(const SkPoint* pts, float w, BuildContext* ctx);

    static void buildCubic(const SkPoint* pts, BuildContext* ctx);

    // Global u -> t relative to the given curve containing u. This is O(kN).
    static float computeRelativeT(const CurveInfo& info, float u);

    template <int kDeg>
    static void computeParameters(const SkPoint* pts, AlphaVals* alpha, UVals* u);

    const SkPoint* fPathPoints;
    float fTotalLength;
    std::vector<CurveInfo> fCurveInfo;
    std::vector<SkPoint> fConicQuadPts;
};

/** Efficient iterator over arc length domain of a path. */
class SkArcLengthSegmentIter {
public:
    explicit SkArcLengthSegmentIter(const SkArcLengthParameterization& param);

    float currU() const { return fU; }

    void getSegment(float ulen, SkPath* dst);

    bool advance(float du);

private:
    static void appendCurveSection(const SkArcLengthParameterization::Curve& curve,
                                   float tmin,
                                   float tmax,
                                   SkPath* dst,
                                   bool moveTo = true);

    const SkArcLengthParameterization& fParam;
    float fU;
    SkArcLengthParameterization::Curve fCurrCurve;
};

#endif
