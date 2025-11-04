/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHdrAgtmPriv_DEFINED
#define SkHdrAgtmPriv_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

#include <optional>
#include <vector>

class SkData;
class SkString;

namespace skhdr {

/**
 * Adaptive global tone mapping
 * The structures and functions for this metadata are defined in
 * SMPTE ST 2094-50: Dynamic metadata for color volume transform — Application #5
 * https://github.com/SMPTE/st2094-50
 */
struct SK_API Agtm {
    // A PiecewiseCubic metadata group, described in Clause 5.1, Piecewise cubic function.
    struct PiecewiseCubicFunction {
        // The GainCurveNumControlPoints metadata item.
        static constexpr size_t kMaxNumControlPoints = 32;
        size_t fNumControlPoints = 0;

        // The GainCurveControlPointX, GainCurveControlPointY, and GainCurveControlPointM metadata
        // items.
        float fX[kMaxNumControlPoints];
        float fY[kMaxNumControlPoints];
        float fM[kMaxNumControlPoints];

        /**
         * Populate the fM values using the Piecewise Cubic Hermite Interpolation Package (PCHIP)
         * algorithm, described in Clause 6.1.3 of candidate draft 2.
         */
        void populateSlopeFromPCHIP();

        /**
         * The function evaluation described in Clause 5.1.3.
         */
        float evaluate(float x) const;
    };

    // A ComponentMix metadata group, described in Clause 5.2, Component mixing function.
    struct ComponentMixingFunction {
        // The ComponentMixRed/Green/Blue/Max/Min/Component metadata items.
        float fRed = 0.f;
        float fGreen = 0.f;
        float fBlue = 0.f;
        float fMax = 0.f;
        float fMin = 0.f;
        float fComponent = 0.f;

        // The function evaluation described in Clause 5.2.3.
        SkColor4f evaluate(const SkColor4f& c) const;
    };

    // A GainFunction metadata group, described in Clause 5.3, Gain function.
    struct GainFunction {
        // The ComponentMix metadata group.
        ComponentMixingFunction fComponentMixing;

        // The PiecewiseCubic metadata group.
        PiecewiseCubicFunction fPiecewiseCubic;

        // The function evaluation described in Clause 5.3.2.
        SkColor4f evaluate(const SkColor4f& c) const;
    };

    // The HdrReferenceWhite metadata item.
    float fHdrReferenceWhite = 203.f;

    // The BaselineHdrHeadroom metadata item.
    float fBaselineHdrHeadroom = 0.f;

    // The GainApplicationSpaceColorPrimaries metadata item.
    SkColorSpacePrimaries fGainApplicationSpacePrimaries = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

    // The GainApplicationOffset metadata item.
    float fGainApplicationOffset = 0.f;

    // The NumAlternateImages metadata item.
    static constexpr size_t kMaxNumAlternateImages = 4;
    size_t fNumAlternateImages = 0;

    // The AlternateHdrHeadroom metadata item list.
    float fAlternateHdrHeadroom[kMaxNumAlternateImages];

    // The GainFunction metadata item list.
    GainFunction fGainFunction[kMaxNumAlternateImages];

    /**
     * This will populate the metadata with the Reference White Tone Mapping Operator (RWTMO)
     * parameters, based on the value of fBaselineHdrHeadroom.
     */
    void populateUsingRwtmo();

    /**
     * Compute the weighting for the specified targeted HDR headroom according to the computations
     * in Clause 5.4.5, Computation of the adaptive tone map.
     */
    struct Weighting {
        // The index into fAlternateImages for fWeight. If fWeight[i] is 0 then
        // fAlternateImageIndex[i] is not used and should be set to kInvalidIndex.
        static constexpr size_t kInvalidIndex = SIZE_MAX;
        size_t fAlternateImageIndex[2] = {kInvalidIndex, kInvalidIndex};

        // The value of fWeight[i] is weight for the fAlternateImageIndex[i]-th alternate image.
        float fWeight[2] = {0.f, 0.f};
    };
    Weighting ComputeWeighting(float targetedHdrHeadroom) const;
};

}  // namespace skhdr

#endif
