/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkCurve_DEFINED
#define SkCurve_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkScalar.h"
#include "include/private/SkTArray.h"
#include "modules/particles/include/SkParticleData.h"

class SkFieldVisitor;

/**
 * SkCurve implements a keyframed 1D function, useful for animating values over time. This pattern
 * is common in digital content creation tools. An SkCurve might represent rotation, scale, opacity,
 * or any other scalar quantity.
 *
 * An SkCurve has a logical domain of [0, 1], and is made of one or more SkCurveSegments.
 * Each segment describes the behavior of the curve in some sub-domain. For an SkCurve with N
 * segments, there are (N - 1) intermediate x-values that subdivide the domain. The first and last
 * x-values are implicitly 0 and 1:
 *
 * 0    ...    x[0]    ...    x[1]   ...      ...    1
 *   Segment_0      Segment_1     ...     Segment_N-1
 *
 * Each segment describes a function over [0, 1] - x-values are re-normalized to the segment's
 * domain when being evaluated. The segments are cubic polynomials, defined by four values (fMin).
 * These are the values at x=0 and x=1, as well as control points at x=1/3 and x=2/3.
 *
 * For segments with fConstant == true, only the first value is used (fMin[0]).
 *
 * Each segment has two additional features for creating interesting (and varied) animation:
 *   - A segment can be ranged. Ranged segments have two sets of coefficients, and a random value
 *     taken from the particle's SkRandom is used to lerp betwen them. Typically, the SkRandom is
 *     in the same state at each call, so this value is stable. That causes a ranged SkCurve to
 *     produce a single smooth cubic function somewhere within the range defined by fMin and fMax.
 *   - A segment can be bidirectional. In that case, after a value is computed, it will be negated
 *     50% of the time.
 */

enum SkCurveSegmentType {
    kConstant_SegmentType,
    kLinear_SegmentType,
    kCubic_SegmentType,
};

struct SkCurveSegment {
    SkScalar eval(SkScalar x, SkScalar t, bool negate) const;
    void visitFields(SkFieldVisitor* v);

    void setConstant(SkScalar c) {
        fType   = kConstant_SegmentType;
        fRanged = false;
        fMin[0] = c;
    }

    SkScalar fMin[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    SkScalar fMax[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    int  fType          = kConstant_SegmentType;
    bool fRanged        = false;
    bool fBidirectional = false;
};

struct SkCurve {
    SkCurve(SkScalar c = 0.0f) {
        fSegments.push_back().setConstant(c);
    }

    SkScalar eval(const SkParticleUpdateParams& params, SkParticleState& ps) const;
    void visitFields(SkFieldVisitor* v);

    // Parameters that determine our x-value during evaluation
    SkParticleValue                fInput;

    // It should always be true that (fXValues.count() + 1) == fSegments.count()
    SkTArray<SkScalar, true>       fXValues;
    SkTArray<SkCurveSegment, true> fSegments;
};

/**
 * SkColorCurve is similar to SkCurve, but keyframes 4D values - specifically colors. Because
 * negative colors rarely make sense, SkColorCurves do not support bidirectional segments, but
 * support all other features (including cubic interpolation).
 */

struct SkColorCurveSegment {
    SkColorCurveSegment() {
        for (int i = 0; i < 4; ++i) {
            fMin[i] = { 1.0f, 1.0f, 1.0f, 1.0f };
            fMax[i] = { 1.0f, 1.0f, 1.0f, 1.0f };
        }
    }

    SkColor4f eval(SkScalar x, SkScalar t) const;
    void visitFields(SkFieldVisitor* v);

    void setConstant(SkColor4f c) {
        fType   = kConstant_SegmentType;
        fRanged = false;
        fMin[0] = c;
    }

    SkColor4f fMin[4];
    SkColor4f fMax[4];

    int  fType   = kConstant_SegmentType;
    bool fRanged = false;
};

struct SkColorCurve {
    SkColorCurve(SkColor4f c = { 1.0f, 1.0f, 1.0f, 1.0f }) {
        fSegments.push_back().setConstant(c);
    }

    SkColor4f eval(const SkParticleUpdateParams& params, SkParticleState& ps) const;
    void visitFields(SkFieldVisitor* v);

    SkParticleValue                     fInput;
    SkTArray<SkScalar, true>            fXValues;
    SkTArray<SkColorCurveSegment, true> fSegments;
};

#endif // SkCurve_DEFINED
