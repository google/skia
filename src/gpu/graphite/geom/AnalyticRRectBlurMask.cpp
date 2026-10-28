/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/AnalyticRRectBlurMask.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkAssert.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace skgpu::graphite {

namespace {

static constexpr int   kCdfLutWidth  = 256;
static constexpr int   kCdfLutHeight = 32;
static constexpr float kCdfLutRange  = 4.f;

float texel_u(int i) {
    return -kCdfLutRange + 2.0 * kCdfLutRange * (i + 0.5) / kCdfLutWidth;
}

// Returns e^-z * I1(z), the exponentially scaled modified Bessel function of the first kind of
// order one, for z >= 0. This comes from "Handbook of Mathematical Functions" by Abramowitz and
// Stegun in chapters 9.8.3 and 9.8.4, |error| < 2.2e-7.
double scaled_bessel_i1(double z) {
    SkASSERT(z >= 0.0);
    if (z < 3.75) {
        const double y = (z / 3.75) * (z / 3.75);
        return std::exp(-z) * z *
               (0.5 + y * (0.87890594 + y * (0.51498869 + y * (0.15084934 +
                y * (0.02658733 + y * (0.00301532 + y * 0.00032411))))));
    }
    const double y = 3.75 / z;
    const double p = 0.02282967 + y * (-0.02895312 + y * (0.01787654 - y * 0.00420059));
    return (0.39894228 + y * (-0.03988024 + y * (-0.00362018 + y * (0.00163801 +
                         y * (-0.01031555 + y * p))))) / std::sqrt(z);
}



// Fills `row` (kCdfLutWidth texels) with the normalized radial profile of a circle of radius `r`,
// in units of sigma, convolved with a Gaussian. Texel i maps to the signed distance to the edge
// u = (r - t) / sqrt(2), so t = r - sqrt(2) * u is the distance to the circle's center. sqrt(2)
// comes from when we sample in the fragment shader, dividing our u sqrt(2) * sigma. Texels
// with u > r / sqrt(2) would be "past" the center and are only reached by linear filtering.
// The profile is unnecesary past the center since we rely on distance to the center, so they are
// clamped to the center's coverage.
//
// The coverage at distance t from the center is the CDF of the Rice distribution,
// P(t) = 1 - Q1(t, r) where Q1 is the Marcum Q-function. It has no closed form, but
//     P(0)  = 1 - e^(-r^2 / 2)
//     dP/dt = -r * e^(-(t^2 + r^2) / 2) * I1(r * t)
//           = -r * e^(-(t - r)^2 / 2) * e^(-rt) * I1(r * t)
// so the row is computed by integrating dP/dt outward from the center. Each value is normalized by
// P(0) so that every row uses the full 8 bits, even when the blurred circle's peak coverage is
// tiny (sigma >> r). The shader scales the normalized value back by P(0).
void fill_circle_profile_row(double r, SkSpan<uint8_t> row) {
    const double sqrt2 = std::sqrt(2.0);
    auto negDPdt = [r](double s) {
        return r * std::exp(-0.5 * (s - r) * (s - r)) * scaled_bessel_i1(r * s);
    };
    const double centerCoverage = -std::expm1(-0.5 * r * r); // P(0)

    // Walking from the last texel to the first (center of the circle outwards), so a
    // single pass accumulates the coverage lost so far until we get the peak coverage, P(0).
    // dP/dt is negligible more than 12 sigma inside the circle's edge, so start there.
    // We integrate outwards from the center to ensure any small accumulated error results in the
    // tail of the blur instead of the center, where coverage is already very low.
    double s = std::max(0.0, r - 12.0);
    double negDPdtAtS = negDPdt(s);
    double integral = 0.0;
    for (int i = kCdfLutWidth - 1; i >= 0; i--) {
        const double t = std::max(0.0, r - sqrt2 * texel_u(i));
        if (t > s) {
            // Composite Simpson's rule with an even number of steps no larger than 1/32 sigma,
            // approximating the integral from s to t.
            const int n = 2 * std::max(1, static_cast<int>(std::ceil((t - s) * 16.0)));
            const double h = (t - s) / n;
            const double negDPdtAtT = negDPdt(t);
            double sum = negDPdtAtS + negDPdtAtT;
            for (int k = 1; k < n; k++) {
                sum += ((k & 1) ? 4.0 : 2.0) * negDPdt(s + k * h);
            }
            integral += sum * h / 3.0;
            s = t;
            negDPdtAtS = negDPdtAtT;
        }
        const float normalizedCoverage = (centerCoverage - integral) / centerCoverage;
        row[i] = static_cast<uint8_t>(
                std::round(255.0 * std::clamp(normalizedCoverage, 0.0f, 1.0f)));
    }
}

SkSpan<uint8_t> lut_row(const SkBitmap& lut, int y) {
    SkASSERT(lut.width() == kCdfLutWidth);
    return SkSpan<uint8_t>(lut.getAddr8(0, y), lut.width());
}

SkBitmap create_cdf_lut() {
    SkBitmap bmp;
    if (!bmp.tryAllocPixels(SkImageInfo::Make(kCdfLutWidth, kCdfLutHeight,
                                              kR8_unorm_SkColorType,
                                              kOpaque_SkAlphaType))) {
        return bmp;
    }

    // For row 0, we have the gaussian CDF of a straight edge (half-plane) for x in
    // [-kRange, kRange], sampled with a half pixel offset. This is used by rects and rrects, and
    // for circles can be considered the case where radius is very large as taking the limit of
    // radius -> inf then sigma/radius = 0.
    SkSpan<uint8_t> halfPlaneRow = lut_row(bmp, 0);
    for (int i = 0; i < kCdfLutWidth; i++) {
        float cdf = 0.5f + 0.5f * std::erf(texel_u(i));
        halfPlaneRow[i] = static_cast<uint8_t>(std::round(cdf * 255.0f));
    }

    // Rows [1, kHeight - 2]: blurred circles where row j corresponds to
    // w = sigma / (sigma + radius) = j / (kHeight - 1), i.e. radius / sigma = (1 - w) / w. This
    // spaces the rows linearly in sigma / radius near the half-plane limit and linearly in
    // radius / sigma near the point limit, where the profiles vary the most.
    for (int j = 1; j < kCdfLutHeight - 1; j++) {
        const double w = static_cast<double>(j) / (kCdfLutHeight - 1);
        fill_circle_profile_row((1.0 - w) / w, lut_row(bmp, j));
    }

    // For the last row, we have the case where the sigma is much larger than the radius, then we
    // can simplify the blur profile by treating it as a blur from a point. So we can treat each
    // u value as the gaussian weight based off of the distance to the center of the circle.
    SkSpan<uint8_t> pointRow = lut_row(bmp, kCdfLutHeight - 1);
    for (int i = 0; i < kCdfLutWidth; i++) {
        const float u = texel_u(i);
        pointRow[i] = static_cast<uint8_t>(std::round(255.0 * std::exp(-u * u)));
    }

    bmp.setImmutable();
    return bmp;
}

}  // anonymous namespace

std::optional<AnalyticRRectBlurMask> AnalyticRRectBlurMask::Make(
                                            Recorder* recorder,
                                            const Transform& localToDeviceTransform,
                                            SkV2 localSigma,
                                            const SkRRect& srcRRect) {
    UniqueKey key;
    {
        static const UniqueKey::Domain kCdfLutDomain = UniqueKey::GenerateDomain();
        UniqueKey::Builder builder(&key, kCdfLutDomain, 0, "AnalyticRRectBlurCdfLut");
    }
    sk_sp<TextureProxy> cdfLut = recorder->priv().proxyCache()->findOrCreateCachedProxy(
            recorder, key, nullptr,
            [](const void*) -> SkBitmap {
                return create_cdf_lut();
            });

    if (!cdfLut) {
        return std::nullopt;
    }

    return AnalyticRRectBlurMask(srcRRect,
                                 localSigma,
                                 std::move(cdfLut));
}

bool AnalyticRRectBlurMask::isCircleBlur() const {
    if (!SkRRectPriv::IsNearlySimpleCircular(fRRect) ||
        !SkScalarNearlyEqual(fLocalSigma.x, fLocalSigma.y)) {
        return false;
    }

    const float diameter = fRRect.radii(SkRRect::kUpperLeft_Corner).fX * 2.0;
    return SkScalarNearlyEqual(diameter, fRRect.width()) &&
           SkScalarNearlyEqual(diameter, fRRect.height());
}

SkV2 AnalyticRRectBlurMask::circleParams() const {
    SkASSERT(this->isCircleBlur());
    const float sigma = fLocalSigma.x;
    const float radius = fRRect.radii(SkRRect::kUpperLeft_Corner).fX;

    // Map w = sigma / (sigma + radius) in [0, 1] onto the centers of rows [0, kCdfLutHeight - 1].
    // See create_cdf_lut().
    const float w = sigma / (sigma + radius);
    const float lutRow = (w * (kCdfLutHeight - 1) + 0.5f) / kCdfLutHeight;

    // The coverage at the center of the blurred circle, which the normalized rows are scaled by.
    const float r = radius / sigma;
    const float centerCoverage = -std::expm1(-0.5f * r * r);

    return {lutRow, centerCoverage};
}

}  // namespace skgpu::graphite
