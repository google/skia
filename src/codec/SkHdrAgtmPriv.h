/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHdrAgtmPriv_DEFINED
#define SkHdrAgtmPriv_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
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
        static constexpr uint8_t kMinNumControlPoints = 1u;
        static constexpr uint8_t kMaxNumControlPoints = 32u;
        uint8_t fNumControlPoints = 0;

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

    // Characterization of the type of tone mapping specified.
    enum class Type {
        // Did not specify an AdaptiveToneMap.
        kNone,
        // Specified to use RWTMO as the tone mapping.
        kReferenceWhite,
        // Specified its own custom parameters.
        kCustom,
    };
    Type fType = Type::kNone;

    // The HdrReferenceWhite metadata item.
    static constexpr float kDefaultHdrReferenceWhite = 203.f;
    float fHdrReferenceWhite = kDefaultHdrReferenceWhite;

    // The BaselineHdrHeadroom metadata item.
    float fBaselineHdrHeadroom = 0.f;

    // The GainApplicationSpaceColorPrimaries metadata item.
    SkColorSpacePrimaries fGainApplicationSpacePrimaries = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

    // The NumAlternateImages metadata item.
    static constexpr uint8_t kMaxNumAlternateImages = 4u;
    uint8_t fNumAlternateImages = 0u;

    // The AlternateHdrHeadroom metadata item list.
    float fAlternateHdrHeadroom[kMaxNumAlternateImages];

    // The GainFunction metadata item list.
    GainFunction fGainFunction[kMaxNumAlternateImages];

    // SkImage containing the control point values for use by the color filter. This is
    // lazily allocated by makeColorFilter().
    mutable sk_sp<SkImage> fGainCurvesXYM;

    /**
     * This will populate the metadata with the Reference White Tone Mapping Operator (RWTMO)
     * parameters, based on the value of fBaselineHdrHeadroom.
     */
    void populateUsingRwtmo();

    /**
     * The encoding is defined in SMPTE ST 2094-50 candidate draft 2. This will deserialize the
     * smpte_st_2094_50_application_info_v0() bitstream. Return false if parsing fails.
     */
    bool parse(const SkData* data);

    /**
     * Serialize to a smpte_st_2094_50_application_info_v0() bitstream.
     */
    sk_sp<SkData> serialize() const;

    /**
     * Compute the weighting for the specified targeted HDR headroom according to the computations
     * in Clause 5.4.5, Computation of the adaptive tone map.
     */
    struct Weighting {
        // The index into fAlternateImages for fWeight. If fWeight[i] is 0 then
        // fAlternateImageIndex[i] is not used and should be set to kInvalidIndex.
        static constexpr uint8_t kInvalidIndex = 255;
        uint8_t fAlternateImageIndex[2] = {kInvalidIndex, kInvalidIndex};

        // The value of fWeight[i] is weight for the fAlternateImageIndex[i]-th alternate image.
        float fWeight[2] = {0.f, 0.f};
    };
    Weighting computeWeighting(float targetedHdrHeadroom) const;

    /**
     * Apply the tone mapping to `colors` in the gain application color space, targeting the
     * specified `targetedHdrHeadroom`.
     */
    void applyGain(SkSpan<SkColor4f> colors, float targetedHdrHeadroom) const;

    /**
     * Return a color filter to apply tone mapping targeting the specified `targetedHdrHeadroom`.
     */
    sk_sp<SkColorFilter> makeColorFilter(float targetedHdrHeadroom) const;

    /**
     * Return the gain application color space.
     */
    sk_sp<SkColorSpace> getGainApplicationSpace() const;
};

}  // namespace skhdr

#endif
