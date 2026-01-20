/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkHdrMetadata.h"
#include "src/codec/SkHdrAgtmPriv.h"

namespace {

// AGTM tone mapping shader.
static constexpr char gAgtmSKSL[] =
    "uniform half scale_factor;"       // The scale to apply in linear space
    "uniform shader curve_xym;"        // The texture containing control points.
    "uniform half weight_i;"           // The weight of gain curve "i"
    "uniform half4 mix_rgbx_i;"        // The red,green,blue mixing coefficients.
    "uniform half4 mix_Mmcx_i;"        // The max,min,component mixing coefficients.
    "uniform half curve_texcoord_y_i;" // The y texture coordinate at which to sample curve_xym.
    "uniform half curve_N_cp_i;"       // The number of control points.
    "uniform half weight_j;"           // All the same parameters, for gain curve "j"
    "uniform half4 mix_rgbx_j;"
    "uniform half4 mix_Mmcx_j;"
    "uniform half curve_texcoord_y_j;"
    "uniform half curve_N_cp_j;"

     // Shader equivalent of AgtmHelpers::EvaluateComponentMixingFunction.
    "half3 EvalComponentMixing(half3 color, half4 rgbx, half4 Mmcx) {"
      "half common = dot(rgbx.rgb, color) +"
                    "Mmcx[0] * max(max(color.r, color.g), color.b) +"
                    "Mmcx[1] * min(min(color.r, color.g), color.b);"
      "return Mmcx[2] * color + half3(common);"
    "}"

     // Shader equivalent of AgtmHelpers::EvaluateGainCurve.
    "half EvalGainCurve(half x, half curve_texcoord_y, half curve_N_cp) {"
       // Handle points to the left of the first control point.
      "half c_min = 0.0;"
      "half4 xym_min = curve_xym.eval(half2(c_min + 0.5, curve_texcoord_y));"
      "if (x <= xym_min.x) {"
        "return xym_min.y;"
      "}"
       // Handle points after the last control point.
      "half c_max = curve_N_cp - 1.0;"
      "half4 xym_max = curve_xym.eval(half2(c_max + 0.5, curve_texcoord_y));"
      "if (x >= xym_max.x) {"
        "return xym_max.y + log2(xym_max.x / x);"
      "}"
       // Binary search for the interval containing x. This will require sampling at most
       // log2(32)=5 more control points.
      "for (int step = 0; step < 5; ++step) {"
         // Early-out if we've already found the interval.
        "if (c_max - c_min <= 1.0) {"
          "break;"
        "}"
         // Test the midpoint and replace one of the endpoints with it.
        "half c_mid = ceil(0.5 * (c_min + c_max));"
        "half4 xym_mid = curve_xym.eval(half2(c_mid + 0.5, curve_texcoord_y));"
        "if (x == xym_mid.x) {"
           // If we hit a control point exactly, then just return.
          "return xym_mid.y;"
        "} else if (x < xym_mid.x) {"
        "c_max = c_mid;"
          "xym_max = xym_mid;"
        "} else {"
        "c_min = c_mid;"
          "xym_min = xym_mid;"
        "}"
      "}"
       // Evaluate the cubic.
      "half h = xym_max.x - xym_min.x;"
      "half mHat_min = xym_min.z * h;"
      "half mHat_max = xym_max.z * h;"
      "half c3 =  2.0 * xym_min.y + mHat_min - 2.0 * xym_max.y + mHat_max;"
      "half c2 = -3.0 * xym_min.y + 3.0 * xym_max.y - 2.0 * mHat_min - mHat_max;"
      "half c1 = mHat_min;"
      "half c0 = xym_min.y;"
      "half t = (x - xym_min.x) / h;"
      "return ((c3*t + c2)*t + c1)*t + c0;"
    "}"

     // Shader equivalent of AgtmHelpers::EvaluateColorGainFunction.
    "half3 EvalColorGainFunction(half3 color,"
                                "half4 mix_rgbx, half4 mix_Mmcx,"
                                "float curve_texcoord_y, float curve_N_cp) {"
      "half3 M = EvalComponentMixing(color, mix_rgbx, mix_Mmcx);"
      "if (mix_Mmcx.b == 0.0) {"
         // If the kComponent coefficient is zero, only evalute the curve once.
        "return half3(EvalGainCurve(M.r, curve_texcoord_y, curve_N_cp));"
      "}"
      "return half3(EvalGainCurve(M.r, curve_texcoord_y, curve_N_cp),"
                   "EvalGainCurve(M.g, curve_texcoord_y, curve_N_cp),"
                   "EvalGainCurve(M.b, curve_texcoord_y, curve_N_cp));"
    "}"

     // Shader equivalent of AgtmHelpers::ApplyGain.
    "half4 main(half4 color) {"
      "color.rgb *= scale_factor;"
      "if (weight_i > 0.0) {"
         // Unpremultiply alpha is needed.
        "float a_inv = (color.a == 0.0) ? 1.0 : 1.0 / color.a;"
        "half3 G = half3(0.0);"
        "G += weight_i * EvalColorGainFunction(color.rgb * a_inv,"
                                              "mix_rgbx_i, mix_Mmcx_i,"
                                              "curve_texcoord_y_i, curve_N_cp_i);"
        "if (weight_j > 0.0) {"
          "G += weight_j * EvalColorGainFunction(color.rgb * a_inv,"
                                                "mix_rgbx_j, mix_Mmcx_j,"
                                                "curve_texcoord_y_j, curve_N_cp_j);"
        "}"
        "color.rgb *= exp2(G);"
      "}"
      "return color;"
    "}";

static sk_sp<SkRuntimeEffect> agtm_runtime_effect() {
    auto init_lambda = []() {
        auto result = SkRuntimeEffect::MakeForColorFilter(SkString(gAgtmSKSL), {});
        SkASSERTF(result.effect, "Agtm shader log:\n%s\n", result.errorText.c_str());
        return result.effect.release();
    };
    static SkRuntimeEffect* effect = init_lambda();
    return sk_ref_sp(effect);
}

}  // namespace

namespace skhdr {

SkColor4f AgtmHelpers::EvaluateComponentMixingFunction(
        const AdaptiveGlobalToneMap::ComponentMixingFunction& mix, const SkColor4f& c) {
    // Assert that the parameters satisfy the constraints in clause 5.2.2.
    SkASSERT(0.f <= mix.fRed        && mix.fRed       <= 1.f);
    SkASSERT(0.f <= mix.fGreen      && mix.fGreen     <= 1.f);
    SkASSERT(0.f <= mix.fBlue       && mix.fBlue      <= 1.f);
    SkASSERT(0.f <= mix.fMax        && mix.fMax       <= 1.f);
    SkASSERT(0.f <= mix.fMin        && mix.fMin       <= 1.f);
    SkASSERT(0.f <= mix.fComponent  && mix.fComponent <= 1.f);
    SkASSERT(0.99999f <= mix.fRed + mix.fGreen + mix.fBlue + mix.fMax + mix.fMin + mix.fComponent);
    SkASSERT(1.00001f >= mix.fRed + mix.fGreen + mix.fBlue + mix.fMax + mix.fMin + mix.fComponent);

    // This implements that math in Formula 3 of SMPTE ST 2094-50.
    float common = mix.fRed * c.fR + mix.fGreen * c.fG + mix.fBlue * c.fB  +
                   mix.fMax * std::max(std::max(c.fR, c.fG), c.fB) +
                   mix.fMin * std::min(std::max(c.fR, c.fG), c.fB);

    // Optimization for when all components are the same.
    if (mix.fComponent == 0.f) {
        return {common, common, common, c.fA};
    }

    // Formula 4 of SMPTE ST 2094-50.
    return {mix.fComponent * c.fR + common,
            mix.fComponent * c.fG + common,
            mix.fComponent * c.fB + common,
            c.fA};
}

namespace AgtmHelpers {

float EvaluateGainCurve(const AdaptiveGlobalToneMap::GainCurve& gainCurve, float x) {
    auto& cp = gainCurve.fControlPoints;
    size_t N = cp.size();

    // This implements that math in Formula 1 of SMPTE ST 2094-50.
    SkASSERT(N > 0 && N <= 32);

    // Handle points off of the left endpoint.
    size_t i = 0;
    if (x <= cp[i].fX) {
        return cp[i].fY;
    }

    // Handle points off of the right endpoint.
    size_t j = N - 1;
    if (x >= cp[j].fX) {
        return cp[j].fY + std::log2(cp[j].fX / x);
    }

    // Binary search for i, j bracket in which we find x.
    while (j - i > 1) {
        size_t m = (i + j) / 2;
        if (x < cp[m].fX) {
            j = m;
        } else {
            i = m;
        }
    }

    // Cache short names for the parameters for computing the cubic coefficients.
    const float x_i = cp[i].fX;
    const float y_i = cp[i].fY;
    const float x_j = cp[j].fX;
    const float y_j = cp[j].fY;
    const float h_i = x_j - x_i;
    const float mHat_i = cp[i].fM * h_i;
    const float mHat_j = cp[j].fM * h_i;

    // Handle intervals that are a point.
    if (h_i == 0.f) {
        return y_i;
    }

    // Compute the coefficients and evaluate the polynomial.
    const float c3 =  2.f * y_i + mHat_i - 2.f * y_j + mHat_j;
    const float c2 = -3.f * y_i + 3.f * y_j - 2.f * mHat_i - mHat_j;
    const float c1 = mHat_i;
    const float c0 = y_i;
    const float t = (x - x_i) / h_i;

    return ((c3*t + c2)*t + c1)*t + c0;
}

SkColor4f EvaluateColorGainFunction(
        const AdaptiveGlobalToneMap::ColorGainFunction& gain, const SkColor4f& c) {
    SkColor4f m = EvaluateComponentMixingFunction(gain.fComponentMixing, c);
    SkColor4f result = {0.f, 0.f, 0.f, c.fA};
    result.fR = EvaluateGainCurve(gain.fGainCurve, m.fR);
    if (m.fR == m.fG && m.fG == m.fB) {
      result.fG = result.fR;
      result.fB = result.fR;
    } else {
      result.fG = EvaluateGainCurve(gain.fGainCurve, m.fG);
      result.fB = EvaluateGainCurve(gain.fGainCurve, m.fB);
    }
    return result;
}

void PopulateSlopeFromPCHIP(AdaptiveGlobalToneMap::GainCurve& gainCurve) {
    auto& cp = gainCurve.fControlPoints;
    size_t N = cp.size();

    // Compute the interval width (h) and piecewise linear slope (s).
    float s[AdaptiveGlobalToneMap::GainCurve::kMaxNumControlPoints];
    float h[AdaptiveGlobalToneMap::GainCurve::kMaxNumControlPoints];
    for (size_t i = 0; i < N - 1; ++i) {
        h[i] = cp[i+1].fX - cp[i].fX;
    }
    for (size_t i = 0; i < N - 1; ++i) {
        s[i] = (cp[i+1].fY - cp[i].fY) / h[i];
    }

    // Handle the left and right control points.
    if (N >= 3) {
        // From Formulas 3 and 4 of ST 2094-50 candidate draft 2.
        cp[0].fM   = ((2 * h[0]   + h[1]  ) * s[0]   - h[0]   * s[1]  ) / (h[0]   + h[1]  );
        cp[N-1].fM = ((2 * h[N-2] + h[N-3]) * s[N-2] - h[N-2] * s[N-3]) / (h[N-2] + h[N-3]);
    } else if (N == 2) {
        cp[0].fM   = s[0];
        cp[N-1].fM = s[0];
    } else {
        cp[0].fM   = 0.f;
        cp[N-1].fM = 0.f;
    }

    // Populate internal control points.
    for (size_t i = 1; i <= N - 2; ++i) {
        // From Formula 5 of ST 2094-50 candidate draft 2.
        if (s[i-1] * s[i] < 0.f) {
            cp[i].fM = 0.f;
        } else {
            float num = 3 * (h[i-1] + h[i]) * s[i-1] * s[i];
            float den = (2 * h[i-1] + h[i]) * s[i-1] + (h[i-1] + 2 * h[i]) * s[i];
            cp[i].fM = num / den;
        }
    }
}

sk_sp<SkImage>
MakeGainCurveXYMImage(const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm) {
    SkBitmap curve_xym_bm;
    curve_xym_bm.allocPixels(SkImageInfo::Make(
            AdaptiveGlobalToneMap::GainCurve::kMaxNumControlPoints,
            AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap::kMaxNumAlternateImages,
            kRGBA_F32_SkColorType, kUnpremul_SkAlphaType));
    for (size_t a = 0; a < hatm.fAlternateImages.size(); ++a) {
        auto& cubic = hatm.fAlternateImages[a].fColorGainFunction.fGainCurve;
        for (size_t c = 0; c < cubic.fControlPoints.size(); ++c) {
            float* xymX = reinterpret_cast<float*>(curve_xym_bm.getAddr(c, a));
            xymX[0] = cubic.fControlPoints[c].fX;
            xymX[1] = cubic.fControlPoints[c].fY;
            xymX[2] = cubic.fControlPoints[c].fM;
            xymX[3] = 1.f;
        }
    }
    curve_xym_bm.setImmutable();
    return SkImages::RasterFromBitmap(curve_xym_bm);
}

void PopulateUsingRwtmo(AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm) {
    hatm.fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec2020;

    if (hatm.fBaselineHdrHeadroom == 0.f) {
        hatm.fAlternateImages.clear();
        return;
    }

    // Set the two alternate image headrooms using Formula D.1 from ST 2094-50 candidate draft 2.
    hatm.fAlternateImages.resize(2);
    hatm.fAlternateImages[0].fHdrHeadroom = 0.f;
    hatm.fAlternateImages[1].fHdrHeadroom =
        std::log2(8.f / 3.f) * std::min(hatm.fBaselineHdrHeadroom / std::log2(1000/203.f), 1.f);

    for (size_t a = 0; a < hatm.fAlternateImages.size(); ++a) {
        auto& gain = hatm.fAlternateImages[a].fColorGainFunction;
        gain = AdaptiveGlobalToneMap::ColorGainFunction();

        // Use maxRGB for applying the curve.
        gain.fComponentMixing.fMax = 1.f;

        // Compute the image of white under the tone mapping from Formula D.2 from ST 2094-50
        // candidate draft 2.
        const float yWhite =
            (a == 1) ? 1.f
                     : 1.f - 0.5f * std::min(hatm.fBaselineHdrHeadroom / std::log2(1000/203.f), 1.f);

        // Compute the Bezier control points using Formula D.5 from ST 2094-50 candidate draft 2.
        const float kappa = 0.65f;
        const float xKnee = 1.f;
        const float yKnee = yWhite;
        const float xMax = std::exp2(hatm.fBaselineHdrHeadroom);
        const float yMax = std::exp2(hatm.fAlternateImages[a].fHdrHeadroom);
        const float xMid = (1.f - kappa) * xKnee + kappa * (xKnee * yMax / yKnee);
        const float yMid = (1.f - kappa) * yKnee + kappa * yMax;

        // Compute the cubic coefficients using Formula D.5 from ST 2094-50 candidate draft 2.
        const float xA = xKnee - 2.f * xMid + xMax;
        const float yA = yKnee - 2.f * yMid + yMax;
        const float xB = 2.f * xMid - 2.f * xKnee;
        const float yB = 2.f * yMid - 2.f * yKnee;
        const float xC = xKnee;
        const float yC = yKnee;

        auto& cubic = gain.fGainCurve;
        cubic.fControlPoints.resize(8);
        for (size_t c = 0; c < cubic.fControlPoints.size(); ++c) {
            // Compute the linear domain curve values using Formula D.4 from ST 2094-50 candidate
            // draft 2.
            const float t = c / (cubic.fControlPoints.size() - 1.f);
            const float x = xC + t * (xB + t * xA);
            const float y = yC + t * (yB + t * yA);
            const float m = (2.f * yA * t + yB) / (2.f * xA * t + xB);

            // Compute the log domain curve values using Formula D.3 from ST 2094-50 candidate
            // draft 2.
            cubic.fControlPoints[c].fX = x;
            cubic.fControlPoints[c].fY = std::log2(y / x);
            cubic.fControlPoints[c].fM = (x * m - y) / (std::log(2.f) * x * y);
        }
    }
}

Weighting ComputeWeighting(const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm,
                           float targetedHdrHeadroom) {
    Weighting result;

    // Create the list of HDR headrooms including the baseline image and all alternate images, as
    // described in SMPTE ST 2094-50, clause 5.4.5, Computation of the adaptive tone map.

    // Let N be the length of the combined list.
    size_t N = 0;

    // Let H be the sorted list of HDR headrooms.
    float H[AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap::kMaxNumAlternateImages + 1];

    // Let indices list the index of each entry of H in fAlternateHdrHeadroom. The index for
    // fBaselineHdrHeadroom is Weighting::kInvalidIndex.
    size_t indices[AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap::kMaxNumAlternateImages + 1];
    for (size_t i = 0; i < hatm.fAlternateImages.size(); ++i) {
        if (N == i && hatm.fBaselineHdrHeadroom < hatm.fAlternateImages[i].fHdrHeadroom) {
            // Insert the baseline HDR headroom before the indices as they are visited.
            indices[N] = Weighting::kInvalidIndex;
            H[N++] = hatm.fBaselineHdrHeadroom;
        }
        indices[N] = i;
        H[N++] = hatm.fAlternateImages[i].fHdrHeadroom;
    }
    if (N == hatm.fAlternateImages.size()) {
        // Insert the baseline HDR headroom at the end if it has not yet been inserted.
        indices[N] = Weighting::kInvalidIndex;
        H[N++] = hatm.fBaselineHdrHeadroom;
    }

    // Find the indices for the contributing images.
    if (targetedHdrHeadroom <= H[0]) {
        // The case of Formula 10 in SMPTE ST 2094-50.
        result.fWeight[0] = 1.f;
        result.fAlternateImageIndex[0] = indices[0];
    } else if (targetedHdrHeadroom >= H[N-1]) {
        // The case of Formula 11 in SMPTE ST 2094-50.
        result.fWeight[0] = 1.f;
        result.fAlternateImageIndex[0] = indices[N-1];
    } else {
        // The case of Formula 12 in SMPTE ST 2094-50.
        size_t i = 0;
        for (i = 0; i < N - 1; ++i) {
            if (H[i] <= targetedHdrHeadroom && targetedHdrHeadroom <= H[i+1]) {
                break;
            }
        }
        result.fWeight[0] = (targetedHdrHeadroom - H[i+1]) / (H[i] - H[i+1]);
        result.fWeight[1] = 1.f - result.fWeight[0];
        result.fAlternateImageIndex[0] = indices[i];
        result.fAlternateImageIndex[1] = indices[i+1];
    }

    // The baseline image always has a gain of 0, so set the weight for the baseline image to 0.
    for (size_t i = 0; i < 2; ++i) {
        if (result.fAlternateImageIndex[i] == Weighting::kInvalidIndex) {
            result.fWeight[i] = 0;
        } else if (result.fWeight[i] == 0) {
            result.fAlternateImageIndex[i] = Weighting::kInvalidIndex;
        }
    }

    // Sort the weights so that the first weight is always greater.
    if (result.fWeight[1] > result.fWeight[0]) {
        std::swap(result.fWeight[0], result.fWeight[1]);
        std::swap(result.fAlternateImageIndex[0], result.fAlternateImageIndex[1]);
    }

    return result;
}

void ApplyGain(const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm,
               SkSpan<SkColor4f> colors,
               float targetedHdrHeadroom) {
    const auto weighting = AgtmHelpers::ComputeWeighting(hatm, targetedHdrHeadroom);
    if (weighting.fWeight[0] == 0.f) {
        // If no weight is non-zero, then no gain will be applied. Leave the points unchanged.
        return;
    } else if (weighting.fWeight[1] == 0.f) {
        // Special case the case of there being only one weighted gain function.
        const auto& gain =
            hatm.fAlternateImages[weighting.fAlternateImageIndex[0]].fColorGainFunction;
        const float w = weighting.fWeight[0];
        for (auto& C : colors) {
            SkColor4f G = AgtmHelpers::EvaluateColorGainFunction(gain, C);
            C = {
                C.fR * std::exp2(w * G.fR),
                C.fG * std::exp2(w * G.fG),
                C.fB * std::exp2(w * G.fB),
                C.fA,
            };
        }
    } else {
        // The general case of two weighted gain functions.
        const auto& gain0 =
            hatm.fAlternateImages[weighting.fAlternateImageIndex[0]].fColorGainFunction;
        const float w0 = weighting.fWeight[0];
        const auto& gain1 =
            hatm.fAlternateImages[weighting.fAlternateImageIndex[1]].fColorGainFunction;
        const float w1 = weighting.fWeight[1];
        for (auto& C : colors) {
            SkColor4f G0 = AgtmHelpers::EvaluateColorGainFunction(gain0, C);
            SkColor4f G1 = AgtmHelpers::EvaluateColorGainFunction(gain1, C);
            C = {
                C.fR * std::exp2(w0 * G0.fR + w1 * G1.fR),
                C.fG * std::exp2(w0 * G0.fG + w1 * G1.fG),
                C.fB * std::exp2(w0 * G0.fB + w1 * G1.fB),
                C.fA,
            };
        }
    }
}

sk_sp<SkColorSpace> GetGainApplicationSpace(
        const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm) {
    skcms_Matrix3x3 toXYZD50;
    if (!hatm.fGainApplicationSpacePrimaries.toXYZD50(&toXYZD50)) {
        return nullptr;
    }
    return SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, toXYZD50);
}

}  // namespace AgtmHelpers

float AgtmImpl::getHdrReferenceWhite() const {
    return fMetadata.fHdrReferenceWhite;
}

bool AgtmImpl::hasBaselineHdrHeadroom() const {
    return fMetadata.fHeadroomAdaptiveToneMap.has_value();
}

float AgtmImpl::getBaselineHdrHeadroom() const {
    SkASSERT(fMetadata.fHeadroomAdaptiveToneMap.has_value());
    auto& hatm = fMetadata.fHeadroomAdaptiveToneMap.value();
    return hatm.fBaselineHdrHeadroom;
}

bool AgtmImpl::isClamp() const {
    const auto& hatm = fMetadata.fHeadroomAdaptiveToneMap;
    if (!hatm.has_value()) {
        return false;
    }
    return hatm->fAlternateImages.size() == 0;
}

sk_sp<SkColorFilter> AgtmImpl::makeColorFilter(float targetedHdrHeadroom) const {
    const auto& hatm = fMetadata.fHeadroomAdaptiveToneMap;
    if (!hatm.has_value()) {
        return nullptr;
    }
    return AgtmHelpers::MakeColorFilter(hatm.value(), targetedHdrHeadroom, 1.f);
}

namespace AgtmHelpers {

sk_sp<SkColorFilter> MakeColorFilter(
        const AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap& hatm,
        float targetedHdrHeadroom,
        float scaleFactor) {
    const auto weighting = ComputeWeighting(hatm, targetedHdrHeadroom);

    auto effect = agtm_runtime_effect();
    if (!effect) {
        return nullptr;
    }
    SkRuntimeShaderBuilder builder(effect);
    builder.uniform("scale_factor") = scaleFactor;
    for (size_t a = 0; a < 2; ++a) {
        const char* weight_str[2] = {"weight_i", "weight_j"};
        builder.uniform(weight_str[a]) = weighting.fWeight[a];

        if (weighting.fWeight[a] == 0.f) {
            continue;
        }
        const auto& gain = hatm.fAlternateImages[
            weighting.fAlternateImageIndex[a]].fColorGainFunction;

        const char* mix_rgbx_str[2] = {"mix_rgbx_i", "mix_rgbx_j"};
        builder.uniform(mix_rgbx_str[a]) = SkColor4f({
            gain.fComponentMixing.fRed,
            gain.fComponentMixing.fGreen,
            gain.fComponentMixing.fBlue,
            0.f,
        });

        const char* mix_Mmcx_str[2] = {"mix_Mmcx_i", "mix_Mmcx_j"};
        builder.uniform(mix_Mmcx_str[a]) = SkColor4f({
            gain.fComponentMixing.fMax,
            gain.fComponentMixing.fMin,
            gain.fComponentMixing.fComponent,
            0.f,
        });

        const char* curve_texcoord_y_str[2] = {"curve_texcoord_y_i", "curve_texcoord_y_j"};
        builder.uniform(curve_texcoord_y_str[a]) = (weighting.fAlternateImageIndex[a] + 0.5f);

        const char* curve_N_cp_str[2] = {"curve_N_cp_i", "curve_N_cp_j"};
        builder.uniform(curve_N_cp_str[a]) = static_cast<float>(
            gain.fGainCurve.fControlPoints.size());
    }

    if (auto gainCurvesXYM = MakeGainCurveXYMImage(hatm)) {
        builder.child("curve_xym") = gainCurvesXYM->makeRawShader(
            SkSamplingOptions(SkFilterMode::kNearest));
    }

    auto gainApplicationColorSpace = GetGainApplicationSpace(hatm);
    if (!gainApplicationColorSpace) {
        return nullptr;
    }

    auto filter = builder.makeColorFilter();
    SkASSERT(filter);
    return filter->makeWithWorkingColorSpace(gainApplicationColorSpace);
}

// Return the maximum luminance from CLLI, MDCV, or a default.
static float get_max_luminance(const Metadata& metadata) {
    if (metadata.getContentLightLevelInformation(nullptr)) {
        ContentLightLevelInformation clli;
        if (metadata.getContentLightLevelInformation(&clli) && clli.fMaxCLL > 0.f) {
            return clli.fMaxCLL;
        }
    }
    if (metadata.getMasteringDisplayColorVolume(nullptr)) {
        MasteringDisplayColorVolume mdcv;
        if (metadata.getMasteringDisplayColorVolume(&mdcv) &&
            mdcv.fMaximumDisplayMasteringLuminance > 0.f) {
            return mdcv.fMaximumDisplayMasteringLuminance;
        }
    }
    return 1000.f;
}

bool PopulateToneMapAgtmParams(const Metadata& metadata,
                               const SkColorSpace* inputColorSpace,
                               AdaptiveGlobalToneMap* outAgtm,
                               float* outScaleFactor) {
    // If `inputColorSpace` is HLG or PQ, find the HDR reference white value. When the shader
    // starts, this is the luminance that will have been mapped to 1.0. We will populate
    // `outScaleFactor` with a scale such that the AGTM HDR reference white luminance (if specified
    // will be mapped to 1.0).
    bool inputIsPqOrHlg = false;
    float inputPqOrHlgWhite = AdaptiveGlobalToneMap::kDefaultHdrReferenceWhite;
    if (inputColorSpace) {
        skcms_TransferFunction trfn;
        inputColorSpace->transferFn(&trfn);
        switch (skcms_TransferFunction_getType(&trfn)) {
            case skcms_TFType_PQ:
            case skcms_TFType_HLG:
                inputIsPqOrHlg = true;
                inputPqOrHlgWhite = trfn.a;
                break;
            default:
                break;
        }
    }

    AdaptiveGlobalToneMap agtm;
    auto& hatm = agtm.fHeadroomAdaptiveToneMap;
    bool hadAgtmMetadata = metadata.getAdaptiveGlobalToneMap(&agtm);

    // SDR content that does not specify an inverse tone mapping will not have a default tone
    // mapping added.
    if (!inputIsPqOrHlg) {
        if (!hadAgtmMetadata || !hatm.has_value()) {
            return false;
        }
    }

    // If no AGTM was specified, populate the HDR reference white from the input color space.
    if (!hadAgtmMetadata) {
        agtm.fHdrReferenceWhite = inputPqOrHlgWhite;
    }

    // If no tone mapping was specified, then use RWTMO with the baseline HDR headroom computed
    // from the CLLI and MDCV metadata.
    if (!hatm.has_value()) {
        hatm = {{
            .fBaselineHdrHeadroom = std::log2(
                std::max(get_max_luminance(metadata) / agtm.fHdrReferenceWhite, 1.f))
        }};
        AgtmHelpers::PopulateUsingRwtmo(hatm.value());
    }

    if (outAgtm) {
        *outAgtm = agtm;
    }
    if (outScaleFactor) {
        *outScaleFactor = inputIsPqOrHlg ? inputPqOrHlgWhite / agtm.fHdrReferenceWhite : 1.f;
    }
    return true;
}

}  // namespace AgtmHelpers

SkString AdaptiveGlobalToneMap::toString() const {
    SkString result = SkStringPrintf("{hdrReferenceWhite:%f", fHdrReferenceWhite);
    if (!fHeadroomAdaptiveToneMap.has_value()) {
        result += "}";
        return result;
    }
    auto& hatm = fHeadroomAdaptiveToneMap.value();

    result += SkStringPrintf(", baselineHdrHeadroom:%f", hatm.fBaselineHdrHeadroom);
    result += ", alternateHdrHeadrooms:[";
    for (size_t a = 0; a < hatm.fAlternateImages.size(); ++a) {
        result += SkStringPrintf("%f", hatm.fAlternateImages[a].fHdrHeadroom);
        if (a != hatm.fAlternateImages.size() - 1) {
            result += ", ";
        }
    }
    result += "]}";
    return result;
}

bool AgtmImpl::parse(const SkData* data) {
    return fMetadata.parse(data);
}

sk_sp<SkData> AgtmImpl::serialize() const {
    return fMetadata.serialize();
}

SkString AgtmImpl::toString() const {
    return fMetadata.toString();
}

// static
std::unique_ptr<Agtm> Agtm::Make(const SkData* data) {
    auto result = std::make_unique<AgtmImpl>();
    if (!result->parse(data)) {
        return nullptr;
    }
    return result;
}

// static
std::unique_ptr<Agtm> Agtm::MakeReferenceWhite(float hdrReferenceWhite, float baselineHdrHeadroom) {
    SkASSERT(baselineHdrHeadroom >= 0.f);
    auto result = std::make_unique<AgtmImpl>();
    result->fMetadata = {
        .fHdrReferenceWhite = hdrReferenceWhite,
        .fHeadroomAdaptiveToneMap = {{
            .fBaselineHdrHeadroom = baselineHdrHeadroom,
        }},
    };
    AgtmHelpers::PopulateUsingRwtmo(result->fMetadata.fHeadroomAdaptiveToneMap.value());
    return result;
}

// static
std::unique_ptr<Agtm> Agtm::MakeClamp(float hdrReferenceWhite, float baselineHdrHeadroom) {
    SkASSERT(baselineHdrHeadroom >= 0.f);
    auto result = std::make_unique<AgtmImpl>();
    result->fMetadata = {
        .fHdrReferenceWhite = hdrReferenceWhite,
        .fHeadroomAdaptiveToneMap = {{
            .fBaselineHdrHeadroom = baselineHdrHeadroom,
            .fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec2020,
            .fAlternateImages = {},
        }},
    };
    return result;
}

}  // namespace skhdr

