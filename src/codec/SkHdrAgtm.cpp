/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkHdrMetadata.h"
#include "src/codec/SkHdrAgtmPriv.h"
#include "src/core/SkStreamPriv.h"

namespace skhdr {

SkColor4f Agtm::ComponentMixingFunction::evaluate(const SkColor4f& c) const {
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

float Agtm::PiecewiseCubicFunction::evaluate(float x) const {
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

SkColor4f Agtm::GainFunction::evaluate(const SkColor4f& c) const {
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

void Agtm::PiecewiseCubicFunction::populateSlopeFromPCHIP() {
    size_t N = fNumControlPoints;

    // Compute the interval width (h) and piecewise linear slope (s).
    float s[Agtm::PiecewiseCubicFunction::kMaxNumControlPoints];
    float h[Agtm::PiecewiseCubicFunction::kMaxNumControlPoints];
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

void Agtm::populateUsingRwtmo() {
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

Agtm::Weighting Agtm::ComputeWeighting(float targetedHdrHeadroom) const {
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

}  // namespace skhdr
