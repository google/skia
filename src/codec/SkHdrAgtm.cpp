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

     // Shader equivalent of AgtmImpl::ComponentMixingFunction::evaluate.
    "half3 EvalComponentMixing(half3 color, half4 rgbx, half4 Mmcx) {"
      "half common = dot(rgbx.rgb, color) +"
                    "Mmcx[0] * max(max(color.r, color.g), color.b) +"
                    "Mmcx[1] * min(min(color.r, color.g), color.b);"
      "return Mmcx[2] * color + half3(common);"
    "}"

     // Shader equivalent of AgtmImpl::PiecewiseCubicFunction::evaluate.
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

     // Shader equivalent of AgtmImpl::GainFunction::evaluate.
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

     // Shader equivalent of AgtmImpl::applyGain.
    "half4 main(half4 color) {"
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

SkColor4f AgtmImpl::ComponentMixingFunction::evaluate(const SkColor4f& c) const {
    // Assert that the parameters satisfy the constraints in clause 5.2.2.
    SkASSERT(0.f <= fRed        && fRed       <= 1.f);
    SkASSERT(0.f <= fGreen      && fGreen     <= 1.f);
    SkASSERT(0.f <= fBlue       && fBlue      <= 1.f);
    SkASSERT(0.f <= fMax        && fMax       <= 1.f);
    SkASSERT(0.f <= fMin        && fMin       <= 1.f);
    SkASSERT(0.f <= fComponent  && fComponent <= 1.f);
    SkASSERT(0.99999f <= fRed + fGreen + fBlue + fMax + fMin + fComponent);
    SkASSERT(1.00001f >= fRed + fGreen + fBlue + fMax + fMin + fComponent);

    // This implements that math in Formula 3 of SMPTE ST 2094-50.
    float common = fRed * c.fR + fGreen * c.fG + fBlue * c.fB  +
                   fMax * std::max(std::max(c.fR, c.fG), c.fB) +
                   fMin * std::min(std::max(c.fR, c.fG), c.fB);

    // Optimization for when all components are the same.
    if (fComponent == 0.f) {
        return {common, common, common, c.fA};
    }

    // Formula 4 of SMPTE ST 2094-50.
    return {fComponent * c.fR + common, fComponent * c.fG + common, fComponent * c.fB + common, c.fA};
}

float AgtmImpl::PiecewiseCubicFunction::evaluate(float x) const {
    // This implements that math in Formula 1 of SMPTE ST 2094-50.
    SkASSERT(fNumControlPoints > 0 && fNumControlPoints <= 32);

    // Handle points off of the left endpoint.
    size_t i = 0;
    if (x <= fX[i]) {
        return fY[i];
    }

    // Handle points off of the right endpoint.
    size_t j = fNumControlPoints - 1;
    if (x >= fX[j]) {
        return fY[j] + std::log2(fX[j] / x);
    }

    // Binary search for i, j bracket in which we find x.
    while (j - i > 1) {
        size_t m = (i + j) / 2;
        if (x < fX[m]) {
            j = m;
        } else {
            i = m;
        }
    }

    // Cache short names for the parameters for computing the cubic coefficients.
    const float x_i = fX[i];
    const float y_i = fY[i];
    const float x_j = fX[j];
    const float y_j = fY[j];
    const float h_i = x_j - x_i;
    const float mHat_i = fM[i] * h_i;
    const float mHat_j = fM[j] * h_i;

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

SkColor4f AgtmImpl::GainFunction::evaluate(const SkColor4f& c) const {
    SkColor4f m = fComponentMixing.evaluate(c);
    SkColor4f result = {0.f, 0.f, 0.f, c.fA};
    result.fR = fPiecewiseCubic.evaluate(m.fR);
    if (m.fR == m.fG && m.fG == m.fB) {
      result.fG = result.fR;
      result.fB = result.fR;
    } else {
      result.fG = fPiecewiseCubic.evaluate(m.fG);
      result.fB = fPiecewiseCubic.evaluate(m.fB);
    }
    return result;
}

void AgtmImpl::PiecewiseCubicFunction::populateSlopeFromPCHIP() {
    size_t N = fNumControlPoints;

    // Compute the interval width (h) and piecewise linear slope (s).
    float s[AgtmImpl::PiecewiseCubicFunction::kMaxNumControlPoints];
    float h[AgtmImpl::PiecewiseCubicFunction::kMaxNumControlPoints];
    for (size_t i = 0; i < N - 1; ++i) {
        h[i] = fX[i+1] - fX[i];
    }
    for (size_t i = 0; i < N - 1; ++i) {
        s[i] = (fY[i+1] - fY[i]) / h[i];
    }

    // Handle the left and right control points.
    if (N >= 3) {
        // From Formulas 3 and 4 of ST 2094-50 candidate draft 2.
        fM[0]   = ((2 * h[0]   + h[1]  ) * s[0]   - h[0]   * s[1]  ) / (h[0]   + h[1]  );
        fM[N-1] = ((2 * h[N-2] + h[N-3]) * s[N-2] - h[N-2] * s[N-3]) / (h[N-2] + h[N-3]);
    } else if (N == 2) {
        fM[0]   = s[0];
        fM[N-1] = s[0];
    } else {
        fM[0]   = 0.f;
        fM[N-1] = 0.f;
    }

    // Populate internal control points.
    for (size_t i = 1; i <= N - 2; ++i) {
        // From Formula 5 of ST 2094-50 candidate draft 2.
        if (s[i-1] * s[i] < 0.f) {
            fM[i] = 0.f;
        } else {
            float num = 3 * (h[i-1] + h[i]) * s[i-1] * s[i];
            float den = (2 * h[i-1] + h[i]) * s[i-1] + (h[i-1] + 2 * h[i]) * s[i];
            fM[i] = num / den;
        }
    }
}

void AgtmImpl::populateGainCurvesXYM() {
    SkBitmap curve_xym_bm;
    curve_xym_bm.allocPixels(SkImageInfo::Make(
            PiecewiseCubicFunction::kMaxNumControlPoints, kMaxNumAlternateImages,
            kRGBA_F32_SkColorType, kUnpremul_SkAlphaType));
    for (uint8_t a = 0; a < fNumAlternateImages; ++a) {
        auto& cubic = fGainFunction[a].fPiecewiseCubic;
        for (uint8_t c = 0; c < cubic.fNumControlPoints; ++c) {
            float* xymX = reinterpret_cast<float*>(curve_xym_bm.getAddr(c, a));
            xymX[0] = cubic.fX[c];
            xymX[1] = cubic.fY[c];
            xymX[2] = cubic.fM[c];
            xymX[3] = 1.f;
        }
    }
    curve_xym_bm.setImmutable();
    fGainCurvesXYM = SkImages::RasterFromBitmap(curve_xym_bm);
}

void AgtmImpl::populateUsingRwtmo() {
   fType = Type::kReferenceWhite;
   fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec2020;

    if (fBaselineHdrHeadroom == 0.f) {
        fNumAlternateImages = 0;
        return;
    }

    // Set the two alternate image headrooms using Formula D.1 from ST 2094-50 candidate draft 2.
    fNumAlternateImages = 2;
    fAlternateHdrHeadroom[0] = 0.f;
    fAlternateHdrHeadroom[1] =
        std::log2(8.f / 3.f) * std::min(fBaselineHdrHeadroom / std::log2(1000/203.f), 1.f);

    for (size_t a = 0; a <fNumAlternateImages; ++a) {
        fGainFunction[a] = GainFunction();

        // Use maxRGB for applying the curve.
        fGainFunction[a].fComponentMixing.fMax = 1.f;

        // Compute the image of white under the tone mapping from Formula D.2 from ST 2094-50
        // candidate draft 2.
        const float yWhite =
            (a == 1) ? 1.f
                     : 1.f - 0.5f * std::min(fBaselineHdrHeadroom / std::log2(1000/203.f), 1.f);

        // Compute the Bezier control points using Formula D.5 from ST 2094-50 candidate draft 2.
        const float kappa = 0.65f;
        const float xKnee = 1.f;
        const float yKnee = yWhite;
        const float xMax = std::exp2(fBaselineHdrHeadroom);
        const float yMax = std::exp2(fAlternateHdrHeadroom[a]);
        const float xMid = (1.f - kappa) * xKnee + kappa * (xKnee * yMax / yKnee);
        const float yMid = (1.f - kappa) * yKnee + kappa * yMax;

        // Compute the cubic coefficients using Formula D.5 from ST 2094-50 candidate draft 2.
        const float xA = xKnee - 2.f * xMid + xMax;
        const float yA = yKnee - 2.f * yMid + yMax;
        const float xB = 2.f * xMid - 2.f * xKnee;
        const float yB = 2.f * yMid - 2.f * yKnee;
        const float xC = xKnee;
        const float yC = yKnee;

        auto& cubic = fGainFunction[a].fPiecewiseCubic;
        cubic.fNumControlPoints = 8;
        for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
            // Compute the linear domain curve values using Formula D.4 from ST 2094-50 candidate
            // draft 2.
            const float t = c / (cubic.fNumControlPoints - 1.f);
            const float x = xC + t * (xB + t * xA);
            const float y = yC + t * (yB + t * yA);
            const float m = (2.f * yA * t + yB) / (2.f * xA * t + xB);

            // Compute the log domain curve values using Formula D.3 from ST 2094-50 candidate
            // draft 2.
            cubic.fX[c] = x;
            cubic.fY[c] = std::log2(y / x);
            cubic.fM[c] = (x * m - y) / (std::log(2.f) * x * y);
        }
    }
}

AgtmImpl::Weighting AgtmImpl::computeWeighting(float targetedHdrHeadroom) const {
    Weighting result;

    // Create the list of HDR headrooms including the baseline image and all alternate images, as
    // described in SMPTE ST 2094-50, clause 5.4.5, Computation of the adaptive tone map.

    // Let N be the length of the combined list.
    size_t N = 0;

    // Let H be the sorted list of HDR headrooms.
    float H[kMaxNumAlternateImages + 1];

    // Let indices list the index of each entry of H in fAlternateHdrHeadroom. The index for
    // fBaselineHdrHeadroom is Weighting::kInvalidIndex.
    size_t indices[kMaxNumAlternateImages + 1];
    for (size_t i = 0; i < fNumAlternateImages; ++i) {
        if (N == i && fBaselineHdrHeadroom < fAlternateHdrHeadroom[i]) {
            // Insert the baseline HDR headroom before the indices as they are visited.
            indices[N] = Weighting::kInvalidIndex;
            H[N++] = fBaselineHdrHeadroom;
        }
        indices[N] = i;
        H[N++] = fAlternateHdrHeadroom[i];
    }
    if (N == fNumAlternateImages) {
        // Insert the baseline HDR headroom at the end if it has not yet been inserted.
        indices[N] = Weighting::kInvalidIndex;
        H[N++] = fBaselineHdrHeadroom;
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

void AgtmImpl::applyGain(SkSpan<SkColor4f> colors, float targetedHdrHeadroom) const {
    const auto weighting = computeWeighting(targetedHdrHeadroom);
    if (weighting.fWeight[0] == 0.f) {
        // If no weight is non-zero, then no gain will be applied. Leave the points unchanged.
        return;
    } else if (weighting.fWeight[1] == 0.f) {
        // Special case the case of there being only one weighted gain function.
        const auto& gain = fGainFunction[weighting.fAlternateImageIndex[0]];
        const float w = weighting.fWeight[0];
        for (auto& C : colors) {
            SkColor4f G = gain.evaluate(C);
            C = {
                C.fR * std::exp2(w * G.fR),
                C.fG * std::exp2(w * G.fG),
                C.fB * std::exp2(w * G.fB),
                C.fA,
            };
        }
    } else {
        // The general case of two weighted gain functions.
        const auto& gain0 = fGainFunction[weighting.fAlternateImageIndex[0]];
        const float w0 = weighting.fWeight[0];
        const auto& gain1 = fGainFunction[weighting.fAlternateImageIndex[1]];
        const float w1 = weighting.fWeight[1];
        for (auto& C : colors) {
            SkColor4f G0 = gain0.evaluate(C);
            SkColor4f G1 = gain1.evaluate(C);
            C = {
                C.fR * std::exp2(w0 * G0.fR + w1 * G1.fR),
                C.fG * std::exp2(w0 * G0.fG + w1 * G1.fG),
                C.fB * std::exp2(w0 * G0.fB + w1 * G1.fB),
                C.fA,
            };
        }
    }
}

sk_sp<SkColorSpace> AgtmImpl::getGainApplicationSpace() const {
    skcms_Matrix3x3 toXYZD50;
    if (!fGainApplicationSpacePrimaries.toXYZD50(&toXYZD50)) {
        return nullptr;
    }
    return SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, toXYZD50);
}

float AgtmImpl::getHdrReferenceWhite() const {
    return fHdrReferenceWhite;
}

bool AgtmImpl::hasBaselineHdrHeadroom() const {
    return fType != Type::kNone;
}

float AgtmImpl::getBaselineHdrHeadroom() const {
    SkASSERT(fType != Type::kNone);
    return fBaselineHdrHeadroom;
}

bool AgtmImpl::isClamp() const {
    if (fType == Type::kNone) {
        return false;
    }
    return fNumAlternateImages == 0;
}

sk_sp<SkColorFilter> AgtmImpl::makeColorFilter(float targetedHdrHeadroom) const {
    auto effect = agtm_runtime_effect();
    if (!effect) {
        return nullptr;
    }
    const auto weighting = computeWeighting(targetedHdrHeadroom);

    SkRuntimeShaderBuilder builder(effect);
    for (uint8_t a = 0; a < 2; ++a) {
        const char* weight_str[2] = {"weight_i", "weight_j"};
        builder.uniform(weight_str[a]) = weighting.fWeight[a];

        if (weighting.fWeight[a] == 0.f) {
            continue;
        }
        const auto& gain = fGainFunction[weighting.fAlternateImageIndex[a]];

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
            gain.fPiecewiseCubic.fNumControlPoints);
    }
    builder.child("curve_xym") = fGainCurvesXYM->makeRawShader(
        SkSamplingOptions(SkFilterMode::kNearest));

    auto filter = builder.makeColorFilter();
    SkASSERT(filter);
    return filter->makeWithWorkingColorSpace(getGainApplicationSpace());
}

SkString AgtmImpl::toString() const {
    SkString result = SkStringPrintf("{hdrReferenceWhite:%f", fHdrReferenceWhite);
    if (fType == Type::kNone) {
        result += "}";
        return result;
    }
    result += SkStringPrintf(", baselineHdrHeadroom:%f", fBaselineHdrHeadroom);
    if (fType == Type::kReferenceWhite) {
        result += ", RWTMO}";
        return result;
    }
    result += ", alternateHdrHeadrooms:[";
    for (uint8_t a = 0; a < fNumAlternateImages; ++a) {
        result += SkStringPrintf("%f", fAlternateHdrHeadroom[a]);
        if (a != fNumAlternateImages - 1) {
            result += ", ";
        }
    }
    result += "]}";
    return result;
}

// static
std::unique_ptr<Agtm> Agtm::Make(const SkData* data) {
    auto result = std::make_unique<AgtmImpl>();
    if (!result->parse(data)) {
        return nullptr;
    }
    result->populateGainCurvesXYM();
    return result;
}

// static
std::unique_ptr<Agtm> Agtm::MakeReferenceWhite(float hdrReferenceWhite, float baselineHdrHeadroom) {
    SkASSERT(baselineHdrHeadroom >= 0.f);
    auto result = std::make_unique<AgtmImpl>();
    result->fHdrReferenceWhite = hdrReferenceWhite;
    result->fBaselineHdrHeadroom = baselineHdrHeadroom;
    result->populateUsingRwtmo();
    result->populateGainCurvesXYM();
    return result;
}

// static
std::unique_ptr<Agtm> Agtm::MakeClamp(float hdrReferenceWhite, float baselineHdrHeadroom) {
    SkASSERT(baselineHdrHeadroom >= 0.f);
    auto result = std::make_unique<AgtmImpl>();
    result->fHdrReferenceWhite = hdrReferenceWhite;
    result->fBaselineHdrHeadroom = baselineHdrHeadroom;
    result->fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec2020;
    result->populateGainCurvesXYM();
    return result;
}

}  // namespace skhdr

