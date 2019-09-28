/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrCCFillGeometry_DEFINED
#define GrGrCCFillGeometry_DEFINED

#include "include/core/SkPoint.h"
#include "include/private/SkNx.h"
#include "include/private/SkTArray.h"
#include "src/core/SkGeometry.h"

/**
 * This class chops device-space contours up into a series of segments that CCPR knows how to
 * fill. (See GrCCFillGeometry::Verb.)
 *
 * NOTE: This must be done in device space, since an affine transformation can change whether a
 * curve is monotonic.
 */
class GrCCFillGeometry {
public:
    // These are the verbs that CCPR knows how to fill. If a path has any segments that don't map to
    // this list, then they are chopped into smaller ones that do. A list of these comprise a
    // compact representation of what can later be expanded into GPU instance data.
    enum class Verb : uint8_t {
        kBeginPath, // Included only for caller convenience.
        kBeginContour,
        kLineTo,
        kMonotonicQuadraticTo, // Monotonic relative to the vector between its endpoints [P2 - P0].
        kMonotonicCubicTo,
        kMonotonicConicTo,
        kEndClosedContour, // endPt == startPt.
        kEndOpenContour // endPt != startPt.
    };

    // These tallies track numbers of CCPR primitives that are required to draw a contour.
    struct PrimitiveTallies {
        int fTriangles; // Number of triangles in the contour's fan.
        int fWeightedTriangles; // Triangles (from the tessellator) whose winding magnitude > 1.
        int fQuadratics;
        int fCubics;
        int fConics;

        void operator+=(const PrimitiveTallies&);
        PrimitiveTallies operator-(const PrimitiveTallies&) const;
        bool operator==(const PrimitiveTallies&);
    };

    GrCCFillGeometry(int numSkPoints = 0, int numSkVerbs = 0, int numConicWeights = 0)
            : fPoints(numSkPoints * 3) // Reserve for a 3x expansion in points and verbs.
            , fVerbs(numSkVerbs * 3)
            , fConicWeights(numConicWeights * 3/2) {}

    const SkTArray<SkPoint, true>& points() const { SkASSERT(!fBuildingContour); return fPoints; }
    const SkTArray<Verb, true>& verbs() const { SkASSERT(!fBuildingContour); return fVerbs; }
    float getConicWeight(int idx) const { SkASSERT(!fBuildingContour); return fConicWeights[idx]; }

    void reset() {
        SkASSERT(!fBuildingContour);
        fPoints.reset();
        fVerbs.reset();
    }

    void beginPath();
    void beginContour(const SkPoint&);
    void lineTo(const SkPoint P[2]);
    void quadraticTo(const SkPoint[3]);

    // We pass through inflection points and loop intersections using a line and quadratic(s)
    // respectively. 'inflectPad' and 'loopIntersectPad' specify how close (in pixels) cubic
    // segments are allowed to get to these points. For normal rendering you will want to use the
    // default values, but these can be overridden for testing purposes.
    //
    // NOTE: loops do appear to require two full pixels of padding around the intersection point.
    //       With just one pixel-width of pad, we start to see bad pixels. Ultimately this has a
    //       minimal effect on the total amount of segments produced. Most sections that pass
    //       through the loop intersection can be approximated with a single quadratic anyway,
    //       regardless of whether we are use one pixel of pad or two (1.622 avg. quads per loop
    //       intersection vs. 1.489 on the tiger).
    void cubicTo(const SkPoint[4], float inflectPad = 0.55f, float loopIntersectPad = 2);

    void conicTo(const SkPoint[3], float w);

    PrimitiveTallies endContour(); // Returns the numbers of primitives needed to draw the contour.

private:
    inline void appendLine(const Sk2f& p0, const Sk2f& p1);

    inline void appendQuadratics(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2);
    inline void appendMonotonicQuadratic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2);

    enum class AppendCubicMode : bool {
        kLiteral,
        kApproximate
    };
    void appendCubics(AppendCubicMode, const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                      const Sk2f& p3, const float chops[], int numChops, float localT0 = 0,
                     float localT1 = 1);
    void appendCubics(AppendCubicMode, const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                      const Sk2f& p3, int maxSubdivisions = 2);
    void chopAndAppendCubicAtMidTangent(AppendCubicMode, const Sk2f& p0, const Sk2f& p1,
                                        const Sk2f& p2, const Sk2f& p3, const Sk2f& tan0,
                                        const Sk2f& tan1, int maxFutureSubdivisions);

    void appendMonotonicConic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2, float w);

    // Transient state used while building a contour.
    SkPoint fCurrAnchorPoint;
    PrimitiveTallies fCurrContourTallies;
    SkCubicType fCurrCubicType;
    SkDEBUGCODE(bool fBuildingContour = false);

    SkSTArray<128, SkPoint, true> fPoints;
    SkSTArray<128, Verb, true> fVerbs;
    SkSTArray<32, float, true> fConicWeights;
};

inline void GrCCFillGeometry::PrimitiveTallies::operator+=(const PrimitiveTallies& b) {
    fTriangles += b.fTriangles;
    fWeightedTriangles += b.fWeightedTriangles;
    fQuadratics += b.fQuadratics;
    fCubics += b.fCubics;
    fConics += b.fConics;
}

GrCCFillGeometry::PrimitiveTallies
inline GrCCFillGeometry::PrimitiveTallies::operator-(const PrimitiveTallies& b) const {
    return {fTriangles - b.fTriangles,
            fWeightedTriangles - b.fWeightedTriangles,
            fQuadratics - b.fQuadratics,
            fCubics - b.fCubics,
            fConics - b.fConics};
}

inline bool GrCCFillGeometry::PrimitiveTallies::operator==(const PrimitiveTallies& b) {
    return fTriangles == b.fTriangles && fWeightedTriangles == b.fWeightedTriangles &&
           fQuadratics == b.fQuadratics && fCubics == b.fCubics && fConics == b.fConics;
}

#endif
