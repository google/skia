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
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/SkHdrMetadata.h"

#include <optional>
#include <vector>

class SkData;
class SkString;

namespace skhdr {

/**
 * Adaptive global tone mapping
 * This structure contains the metadata items from the ColorVolumeTransform metadata group
 * in Clause 7.1: Metadata set of SMPTE ST 2094-50: Dynamic metadata for color volume transform
 * Application #5
 * https://github.com/SMPTE/st2094-50
 */
struct AdaptiveGlobalToneMap {
    // A GainCurve metadata group.
    struct GainCurve {
        // Structure holding one entry of the GainCurveControlPointX, GainCurveControlPointY, and
        // GainCurveControlPointM metadata items.
        struct ControlPoint {
            float fX = 0.f;
            float fY = 0.f;
            float fM = 0.f;
        };

        // The size of this vector is the value of the GainCurveNumControlPoints metadata item.
        static constexpr size_t kMinNumControlPoints = 1u;
        static constexpr size_t kMaxNumControlPoints = 32u;
        std::vector<ControlPoint> fControlPoints;
    };

    // A ComponentMix metadata group.
    struct ComponentMixingFunction {
        // The ComponentMixRed/Green/Blue/Max/Min/Component metadata items.
        float fRed = 0.f;
        float fGreen = 0.f;
        float fBlue = 0.f;
        float fMax = 0.f;
        float fMin = 0.f;
        float fComponent = 0.f;
    };

    // A ColorGainFunction metadata group.
    struct ColorGainFunction {
        // The ComponentMix metadata group.
        ComponentMixingFunction fComponentMixing;

        // The GainCurve metadata group.
        GainCurve fGainCurve;
    };

    // Structure holding the metadata items and groups for an alternate image.
    struct AlternateImage {
        // The AlternateHdrHeadroom metadata item.
        float fHdrHeadroom = 0.f;

        // The ColorGainFunction metadata group.
        ColorGainFunction fColorGainFunction;
    };

    // HeadroomAdaptiveToneMap metadata group.
    struct HeadroomAdaptiveToneMap {
        HeadroomAdaptiveToneMap();

        // The BaselineHdrHeadroom metadata item.
        float fBaselineHdrHeadroom = 0.f;

        // The GainApplicationSpaceColorPrimaries metadata item.
        SkColorSpacePrimaries fGainApplicationSpacePrimaries =
            {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

        // The size of this vector is the NumAlternateImages metadata item.
        static constexpr size_t kMaxNumAlternateImages = 4u;
        std::vector<AlternateImage> fAlternateImages;
    };

    // The HdrReferenceWhite metadata item.
    float fHdrReferenceWhite = kDefaultHdrReferenceWhite;

    // The HeadroomAdaptiveToneMap metadata group.
    std::optional<HeadroomAdaptiveToneMap> fHeadroomAdaptiveToneMap;

    // The default value for the HdrReferenceWhite metadata item.
    static constexpr float kDefaultHdrReferenceWhite = 203.f;

    /**
     * Decode from the binary encoding in Annex C.
     */
    bool parse(const SkData* data);

    /**
     * Serialize to the encoding used by parse().
     */
    sk_sp<SkData> serialize() const;

    /**
     * Return a human-readable description.
     */
    SkString toString() const;

    bool operator==(const AdaptiveGlobalToneMap& other) const;
    bool operator!=(const AdaptiveGlobalToneMap& other) const {
        return !(*this == other);
    }
};

// Collection of functions and structures that could potentially be moved into
// the AdaptiveGlobalToneMap structure or its sub-structures, but are not exposed yet.
namespace AgtmHelpers {

/**
 * The function evaluation described in Clause 6.3.2.
 */
SkColor4f EvaluateColorGainFunction(const AdaptiveGlobalToneMap::ColorGainFunction& gain,
                                    const SkColor4f& c);

/**
 * The function evaluation described in Clause 5.2.3.
 */
SkColor4f EvaluateComponentMixingFunction(const AdaptiveGlobalToneMap::ComponentMixingFunction& mix,
                                          const SkColor4f& c);

/**
 * The function evaluation described in Clause 6.1.3.
 */
float EvaluateGainCurve(const AdaptiveGlobalToneMap::GainCurve& gainCurve, float x);

/**
 * Populate the fM values using the Piecewise Cubic Hermite Interpolation Package (PCHIP)
 * algorithm, described in Clause C.3.9: Piecewise cubic hermite interpolation package slope
 * computation.
 */
void PopulateSlopeFromPCHIP(AdaptiveGlobalToneMap::GainCurve& gainCurve);

/**
 * Compute the weighting for the specified targeted HDR headroom according to the computations
 * in Clause 6.4.5, Computation of the adaptive tone map.
 */
struct Weighting {
    // The index into fAlternateImages for fWeight. If fWeight[i] is 0 then
    // fAlternateImageIndex[i] is not used and should be set to kInvalidIndex.
    static constexpr uint8_t kInvalidIndex = 255;
    uint8_t fAlternateImageIndex[2] = {kInvalidIndex, kInvalidIndex};

    // The value of fWeight[i] is weight for the fAlternateImageIndex[i]-th alternate image.
    float fWeight[2] = {0.f, 0.f};
};
Weighting ComputeWeighting(const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm,
                           float targetedHdrHeadroom);

/**
 * This will populate the metadata with the Reference White Tone Mapping Operator (RWTMO)
 * parameters, based on the value of fBaselineHdrHeadroom.
 */
void PopulateUsingRwtmo(AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm);

}  // namespace AgtmHelpers

/**
 * Interface for adaptive global tone mapping.
 * TODO(https://crbug.com/468928417): This structure was originally designed to be the interface
 * for parsing SMPTE ST 2094-50 metadata. It is no longer being used in this way, and should be
 * removed or recycled.
 */
class AgtmImpl final : public Agtm {
  public:
    AdaptiveGlobalToneMap fMetadata;

    // SkImage containing the control point values for use by the color filter, populated by
    // populateGainCurvesXYM.
    sk_sp<SkImage> fGainCurvesXYM;

    /**
     * Populate the fGainCurvesXYM which will cache the gain curves' values in an SkImage.
     */
    void populateGainCurvesXYM();

    /**
     * The encoding is defined in SMPTE ST 2094-50 candidate draft 2. This will deserialize the
     * smpte_st_2094_50_application_info_v0() bitstream. Return false if parsing fails.
     */
    bool parse(const SkData* data);

    /**
     * Apply the tone mapping to `colors` in the gain application color space, targeting the
     * specified `targetedHdrHeadroom`.
     */
    void applyGain(SkSpan<SkColor4f> colors, float targetedHdrHeadroom) const;

    /**
     * Return the gain application color space.
     */
    sk_sp<SkColorSpace> getGainApplicationSpace() const;

    /**
     * Implementation of the Agtm interface.
     */
    ~AgtmImpl() override = default;
    sk_sp<SkData> serialize() const override;
    float getHdrReferenceWhite() const override;
    bool hasBaselineHdrHeadroom() const override;
    float getBaselineHdrHeadroom() const override;
    bool isClamp() const override;
    sk_sp<SkColorFilter> makeColorFilter(float targetedHdrHeadroom) const override;
    SkString toString() const override;
};

}  // namespace skhdr

#endif
