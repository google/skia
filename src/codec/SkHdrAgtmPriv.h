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

class Metadata;

// Collection of functions and structures that could potentially be moved into
// the AdaptiveGlobalToneMap structure or its sub-structures, but are not exposed yet.
namespace AgtmHelpers {

/**
 * The function evaluation described in Clause 6.3.2.
 */
SkColor4f EvaluateColorGainFunction(const AdaptiveGlobalToneMap::ColorGainFunction& gain,
                                    const SkColor4f& c);

/**
 * The function evaluation described in Clause 6.4.3.
 */
SkColor4f EvaluateComponentMixingFunction(const AdaptiveGlobalToneMap::ComponentMixingFunction& mix,
                                          const SkColor4f& c);

/**
 * The function evaluation described in Clause 6.5.3.
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
 * in Clause 6.2.5, Computation of the headroom-adaptive tone map.
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

/**
 * If tone mapping is to be performed for `inputColorSpace` with `metadata, then populate `outAgtm`
 * and `outScaleFactor` with the parameters to use for tone mapping with MakeColorFilter. If tone
 * mapping should not be performed, return false.
 */
bool PopulateToneMapAgtmParams(const Metadata& metadata,
                               const SkColorSpace* inputColorSpace,
                               AdaptiveGlobalToneMap* outAgtm,
                               float* outScaleFactor);

/**
 * Apply the tone mapping to `colors` in the gain application color space, targeting the
 * specified `targetedHdrHeadroom`.
 */
void ApplyGain(
    const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm,
    SkSpan<SkColor4f> colors,
    float targetedHdrHeadroom);

/**
 * Return an SkColorFilter that will first scale by `scaleFactor`, and then tone map to
 * the specified `targetedHdrHeadroom`.
 */
sk_sp<SkColorFilter> MakeColorFilter(
    const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm,
    float targetedHdrHeadroom,
    float scaleFactor);

/**
 * Return an SkImage containing the control point values for use by the color filter.
 */
sk_sp<SkImage> MakeGainCurveXYMImage(
    const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm);

/**
 * Return the gain application color space.
 */
sk_sp<SkColorSpace> GetGainApplicationSpace(
    const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm);

/**
 * Return true if `hatm` or `agtm` satisfies all normative constraints.
 */
bool Validate(const AdaptiveGlobalToneMap& agtm);
bool Validate(const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm);

}  // namespace AgtmHelpers

}  // namespace skhdr

#endif
