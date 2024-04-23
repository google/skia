/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/BlurUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkKnownRuntimeEffects.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace skgpu {

void Compute2DBlurKernel(SkSize sigma,
                         SkISize radius,
                         SkSpan<float> kernel) {
    // Callers likely had to calculate the radius prior to filling out the kernel value, which is
    // why it's provided; but make sure it's consistent with expectations.
    SkASSERT(BlurSigmaRadius(sigma.width()) == radius.width() &&
             BlurSigmaRadius(sigma.height()) == radius.height());

    // Callers are responsible for downscaling large sigmas to values that can be processed by the
    // effects, so ensure the radius won't overflow 'kernel'
    const int width = BlurKernelWidth(radius.width());
    const int height = BlurKernelWidth(radius.height());
    const size_t kernelSize = SkTo<size_t>(sk_64_mul(width, height));
    SkASSERT(kernelSize <= kernel.size());

    // And the definition of an identity blur should be sufficient that 2sigma^2 isn't near zero
    // when there's a non-trivial radius.
    const float twoSigmaSqrdX = 2.0f * sigma.width() * sigma.width();
    const float twoSigmaSqrdY = 2.0f * sigma.height() * sigma.height();
    SkASSERT((radius.width() == 0 || !SkScalarNearlyZero(twoSigmaSqrdX)) &&
             (radius.height() == 0 || !SkScalarNearlyZero(twoSigmaSqrdY)));

    // Setting the denominator to 1 when the radius is 0 automatically converts the remaining math
    // to the 1D Gaussian distribution. When both radii are 0, it correctly computes a weight of 1.0
    const float sigmaXDenom = radius.width() > 0 ? 1.0f / twoSigmaSqrdX : 1.f;
    const float sigmaYDenom = radius.height() > 0 ? 1.0f / twoSigmaSqrdY : 1.f;

    float sum = 0.0f;
    for (int x = 0; x < width; x++) {
        float xTerm = static_cast<float>(x - radius.width());
        xTerm = xTerm * xTerm * sigmaXDenom;
        for (int y = 0; y < height; y++) {
            float yTerm = static_cast<float>(y - radius.height());
            float xyTerm = std::exp(-(xTerm + yTerm * yTerm * sigmaYDenom));
            // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
            // is dropped here, since we renormalize the kernel below.
            kernel[y * width + x] = xyTerm;
            sum += xyTerm;
        }
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (size_t i = 0; i < kernelSize; ++i) {
        kernel[i] *= scale;
    }
    // Zero remainder of the array
    memset(kernel.data() + kernelSize, 0, sizeof(float)*(kernel.size() - kernelSize));
}

void Compute2DBlurKernel(SkSize sigma,
                         SkISize radii,
                         std::array<SkV4, kMaxBlurSamples/4>& kernel) {
    static_assert(sizeof(kernel) == sizeof(std::array<float, kMaxBlurSamples>));
    static_assert(alignof(float) == alignof(SkV4));
    float* data = kernel[0].ptr();
    Compute2DBlurKernel(sigma, radii, SkSpan<float>(data, kMaxBlurSamples));
}

void Compute2DBlurOffsets(SkISize radius, std::array<SkV4, kMaxBlurSamples/2>& offsets) {
    const int kernelArea = BlurKernelWidth(radius.width()) * BlurKernelWidth(radius.height());
    SkASSERT(kernelArea <= kMaxBlurSamples);

    SkSpan<float> offsetView{offsets[0].ptr(), kMaxBlurSamples*2};

    int i = 0;
    for (int y = -radius.height(); y <= radius.height(); ++y) {
        for (int x = -radius.width(); x <= radius.width(); ++x) {
            offsetView[2*i]   = x;
            offsetView[2*i+1] = y;
            ++i;
        }
    }
    SkASSERT(i == kernelArea);
    const int lastValidOffset = 2*(kernelArea - 1);
    for (; i < kMaxBlurSamples; ++i) {
        offsetView[2*i]   = offsetView[lastValidOffset];
        offsetView[2*i+1] = offsetView[lastValidOffset+1];
    }
}

void Compute1DBlurLinearKernel(float sigma,
                               int radius,
                               std::array<SkV4, kMaxBlurSamples/2>& offsetsAndKernel) {
    SkASSERT(sigma <= kMaxLinearBlurSigma);
    SkASSERT(radius == BlurSigmaRadius(sigma));
    SkASSERT(BlurLinearKernelWidth(radius) <= kMaxBlurSamples);

    // Given 2 adjacent gaussian points, they are blended as: Wi * Ci + Wj * Cj.
    // The GPU will mix Ci and Cj as Ci * (1 - x) + Cj * x during sampling.
    // Compute W', x such that W' * (Ci * (1 - x) + Cj * x) = Wi * Ci + Wj * Cj.
    // Solving W' * x = Wj, W' * (1 - x) = Wi:
    // W' = Wi + Wj
    // x = Wj / (Wi + Wj)
    auto get_new_weight = [](float* new_w, float* offset, float wi, float wj) {
        *new_w = wi + wj;
        *offset = wj / (wi + wj);
    };

    // Create a temporary standard kernel. The maximum blur radius that can be passed to this
    // function is (kMaxBlurSamples-1), so make an array large enough to hold the full kernel width.
    static constexpr int kMaxKernelWidth = BlurKernelWidth(kMaxBlurSamples - 1);
    SkASSERT(BlurKernelWidth(radius) <= kMaxKernelWidth);
    std::array<float, kMaxKernelWidth> fullKernel;
    Compute1DBlurKernel(sigma, radius, SkSpan<float>{fullKernel.data(), BlurKernelWidth(radius)});

    std::array<float, kMaxBlurSamples> kernel;
    std::array<float, kMaxBlurSamples> offsets;
    // Note that halfsize isn't just size / 2, but radius + 1. This is the size of the output array.
    int halfSize = skgpu::BlurLinearKernelWidth(radius);
    int halfRadius = halfSize / 2;
    int lowIndex = halfRadius - 1;

    // Compute1DGaussianKernel produces a full 2N + 1 kernel. Since the kernel can be mirrored,
    // compute only the upper half and mirror to the lower half.

    int index = radius;
    if (radius & 1) {
        // If N is odd, then use two samples.
        // The centre texel gets sampled twice, so halve its influence for each sample.
        // We essentially sample like this:
        // Texel edges
        // v    v    v    v
        // |    |    |    |
        // \-----^---/ Lower sample
        //      \---^-----/ Upper sample
        get_new_weight(&kernel[halfRadius],
                       &offsets[halfRadius],
                       fullKernel[index] * 0.5f,
                       fullKernel[index + 1]);
        kernel[lowIndex] = kernel[halfRadius];
        offsets[lowIndex] = -offsets[halfRadius];
        index++;
        lowIndex--;
    } else {
        // If N is even, then there are an even number of texels on either side of the centre texel.
        // Sample the centre texel directly.
        kernel[halfRadius] = fullKernel[index];
        offsets[halfRadius] = 0.0f;
    }
    index++;

    // Every other pair gets one sample.
    for (int i = halfRadius + 1; i < halfSize; index += 2, i++, lowIndex--) {
        get_new_weight(&kernel[i], &offsets[i], fullKernel[index], fullKernel[index + 1]);
        offsets[i] += static_cast<float>(index - radius);

        // Mirror to lower half.
        kernel[lowIndex] = kernel[i];
        offsets[lowIndex] = -offsets[i];
    }

    // Zero out remaining values in the kernel
    memset(kernel.data() + halfSize, 0, sizeof(float)*(kMaxBlurSamples - halfSize));
    // But copy the last valid offset into the remaining offsets, to increase the chance that
    // over-iteration in a fragment shader will have a cache hit.
    for (int i = halfSize; i < kMaxBlurSamples; ++i) {
        offsets[i] = offsets[halfSize - 1];
    }

    // Interleave into the output array to match the 1D SkSL effect
    for (int i = 0; i < skgpu::kMaxBlurSamples / 2; ++i) {
        offsetsAndKernel[i] = SkV4{offsets[2*i], kernel[2*i], offsets[2*i+1], kernel[2*i+1]};
    }
}

static SkKnownRuntimeEffects::StableKey to_stablekey(int kernelWidth, uint32_t baseKey) {
    SkASSERT(kernelWidth >= 2 && kernelWidth <= kMaxBlurSamples);
    switch(kernelWidth) {
        // Batch on multiples of 4 (skipping width=1, since that can't happen)
        case 2:  [[fallthrough]];
        case 3:  [[fallthrough]];
        case 4:  return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey);
        case 5:  [[fallthrough]];
        case 6:  [[fallthrough]];
        case 7:  [[fallthrough]];
        case 8:  return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+1);
        case 9:  [[fallthrough]];
        case 10: [[fallthrough]];
        case 11: [[fallthrough]];
        case 12: return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+2);
        case 13: [[fallthrough]];
        case 14: [[fallthrough]];
        case 15: [[fallthrough]];
        case 16: return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+3);
        case 17: [[fallthrough]];
        case 18: [[fallthrough]];
        case 19: [[fallthrough]];
        // With larger kernels, batch on multiples of eight so up to 7 wasted samples.
        case 20: return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+4);
        case 21: [[fallthrough]];
        case 22: [[fallthrough]];
        case 23: [[fallthrough]];
        case 24: [[fallthrough]];
        case 25: [[fallthrough]];
        case 26: [[fallthrough]];
        case 27: [[fallthrough]];
        case 28: return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+5);
        default:
            SkUNREACHABLE;
    }
}

const SkRuntimeEffect* GetLinearBlur1DEffect(int radius) {
    return GetKnownRuntimeEffect(
            to_stablekey(BlurLinearKernelWidth(radius),
                         static_cast<uint32_t>(SkKnownRuntimeEffects::StableKey::k1DBlurBase)));
}

const SkRuntimeEffect* GetBlur2DEffect(const SkISize& radii) {
    int kernelArea = BlurKernelWidth(radii.width()) * BlurKernelWidth(radii.height());
    return GetKnownRuntimeEffect(
            to_stablekey(kernelArea,
                         static_cast<uint32_t>(SkKnownRuntimeEffects::StableKey::k2DBlurBase)));
}

///////////////////////////////////////////////////////////////////////////////
//  Rect Blur
///////////////////////////////////////////////////////////////////////////////

// TODO: it seems like there should be some synergy with SkBlurMask::ComputeBlurProfile
// TODO: maybe cache this on the cpu side?
SkBitmap CreateIntegralTable(float sixSigma) {
    SkBitmap table;

    int width = ComputeIntegralTableWidth(sixSigma);
    if (width == 0) {
        return table;
    }

    if (!table.tryAllocPixels(SkImageInfo::MakeA8(width, 1))) {
        return table;
    }
    *table.getAddr8(0, 0) = 255;
    const float invWidth = 1.f / width;
    for (int i = 1; i < width - 1; ++i) {
        float x = (i + 0.5f) * invWidth;
        x = (-6 * x + 3) * SK_ScalarRoot2Over2;
        float integral = 0.5f * (std::erf(x) + 1.f);
        *table.getAddr8(i, 0) = SkToU8(sk_float_round2int(255.f * integral));
    }

    *table.getAddr8(width - 1, 0) = 0;
    table.setImmutable();
    return table;
}

int ComputeIntegralTableWidth(float sixSigma) {
    // Check for NaN/infinity
    if (!SkIsFinite(sixSigma)) {
        return 0;
    }
    // Avoid overflow, covers both multiplying by 2 and finding next power of 2:
    // 2*((2^31-1)/4 + 1) = 2*(2^29-1) + 2 = 2^30 and SkNextPow2(2^30) = 2^30
    if (sixSigma > SK_MaxS32 / 4 + 1) {
        return 0;
    }
    // The texture we're producing represents the integral of a normal distribution over a
    // six-sigma range centered at zero. We want enough resolution so that the linear
    // interpolation done in texture lookup doesn't introduce noticeable artifacts. We
    // conservatively choose to have 2 texels for each dst pixel.
    int minWidth = 2 * ((int)std::ceil(sixSigma));
    // Bin by powers of 2 with a minimum so we get good profile reuse.
    return std::max(SkNextPow2(minWidth), 32);
}

///////////////////////////////////////////////////////////////////////////////
//  Circle Blur
///////////////////////////////////////////////////////////////////////////////

// Computes an unnormalized half kernel (right side). Returns the summation of all the half
// kernel values.
static float make_unnormalized_half_kernel(float* halfKernel, int halfKernelSize, float sigma) {
    const float invSigma = 1.0f / sigma;
    const float b = -0.5f * invSigma * invSigma;
    float tot = 0.0f;
    // Compute half kernel values at half pixel steps out from the center.
    float t = 0.5f;
    for (int i = 0; i < halfKernelSize; ++i) {
        float value = expf(t * t * b);
        tot += value;
        halfKernel[i] = value;
        t += 1.0f;
    }
    return tot;
}

// Create a Gaussian half-kernel (right side) and a summed area table given a sigma and number
// of discrete steps. The half kernel is normalized to sum to 0.5.
static void make_half_kernel_and_summed_table(float* halfKernel,
                                              float* summedHalfKernel,
                                              int halfKernelSize,
                                              float sigma) {
    // The half kernel should sum to 0.5 not 1.0.
    const float tot = 2.0f * make_unnormalized_half_kernel(halfKernel, halfKernelSize, sigma);
    float sum = 0.0f;
    for (int i = 0; i < halfKernelSize; ++i) {
        halfKernel[i] /= tot;
        sum += halfKernel[i];
        summedHalfKernel[i] = sum;
    }
}

// Applies the 1D half kernel vertically at points along the x axis to a circle centered at the
// origin with radius circleR.
static void apply_kernel_in_y(float* results,
                              int numSteps,
                              float firstX,
                              float circleR,
                              int halfKernelSize,
                              const float* summedHalfKernelTable) {
    float x = firstX;
    for (int i = 0; i < numSteps; ++i, x += 1.0f) {
        if (x < -circleR || x > circleR) {
            results[i] = 0;
            continue;
        }
        float y = sqrtf(circleR * circleR - x * x);
        // In the column at x we exit the circle at +y and -y
        // The summed table entry j is actually reflects an offset of j + 0.5.
        y -= 0.5f;
        int yInt = SkScalarFloorToInt(y);
        SkASSERT(yInt >= -1);
        if (y < 0) {
            results[i] = (y + 0.5f) * summedHalfKernelTable[0];
        } else if (yInt >= halfKernelSize - 1) {
            results[i] = 0.5f;
        } else {
            float yFrac = y - yInt;
            results[i] = (1.0f - yFrac) * summedHalfKernelTable[yInt] +
                         yFrac * summedHalfKernelTable[yInt + 1];
        }
    }
}

// Apply a Gaussian at point (evalX, 0) to a circle centered at the origin with radius circleR.
// This relies on having a half kernel computed for the Gaussian and a table of applications of
// the half kernel in y to columns at (evalX - halfKernel, evalX - halfKernel + 1, ..., evalX +
// halfKernel) passed in as yKernelEvaluations.
static uint8_t eval_at(float evalX,
                       float circleR,
                       const float* halfKernel,
                       int halfKernelSize,
                       const float* yKernelEvaluations) {
    float acc = 0;

    float x = evalX - halfKernelSize;
    for (int i = 0; i < halfKernelSize; ++i, x += 1.0f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i];
        acc += verticalEval * halfKernel[halfKernelSize - i - 1];
    }
    for (int i = 0; i < halfKernelSize; ++i, x += 1.0f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i + halfKernelSize];
        acc += verticalEval * halfKernel[i];
    }
    // Since we applied a half kernel in y we multiply acc by 2 (the circle is symmetric about
    // the x axis).
    return SkUnitScalarClampToByte(2.0f * acc);
}

// This function creates a profile of a blurred circle. It does this by computing a kernel for
// half the Gaussian and a matching summed area table. The summed area table is used to compute
// an array of vertical applications of the half kernel to the circle along the x axis. The
// table of y evaluations has 2 * k + n entries where k is the size of the half kernel and n is
// the size of the profile being computed. Then for each of the n profile entries we walk out k
// steps in each horizontal direction multiplying the corresponding y evaluation by the half
// kernel entry and sum these values to compute the profile entry.
SkBitmap CreateCircleProfile(float sigma, float radius, int profileWidth) {
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(SkImageInfo::MakeA8(profileWidth, 1))) {
        return bitmap;
    }

    uint8_t* profile = bitmap.getAddr8(0, 0);

    const int numSteps = profileWidth;

    // The full kernel is 6 sigmas wide.
    int halfKernelSize = SkScalarCeilToInt(6.0f * sigma);
    // Round up to next multiple of 2 and then divide by 2.
    halfKernelSize = ((halfKernelSize + 1) & ~1) >> 1;

    // Number of x steps at which to apply kernel in y to cover all the profile samples in x.
    const int numYSteps = numSteps + 2 * halfKernelSize;

    skia_private::AutoTArray<float> bulkAlloc(halfKernelSize + halfKernelSize + numYSteps);
    float* halfKernel = bulkAlloc.get();
    float* summedKernel = bulkAlloc.get() + halfKernelSize;
    float* yEvals = bulkAlloc.get() + 2 * halfKernelSize;
    make_half_kernel_and_summed_table(halfKernel, summedKernel, halfKernelSize, sigma);

    float firstX = -halfKernelSize + 0.5f;
    apply_kernel_in_y(yEvals, numYSteps, firstX, radius, halfKernelSize, summedKernel);

    for (int i = 0; i < numSteps - 1; ++i) {
        float evalX = i + 0.5f;
        profile[i] = eval_at(evalX, radius, halfKernel, halfKernelSize, yEvals + i);
    }
    // Ensure the tail of the Gaussian goes to zero.
    profile[numSteps - 1] = 0;

    bitmap.setImmutable();
    return bitmap;
}

SkBitmap CreateHalfPlaneProfile(int profileWidth) {
    SkASSERT(!(profileWidth & 0x1));

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(SkImageInfo::MakeA8(profileWidth, 1))) {
        return bitmap;
    }

    uint8_t* profile = bitmap.getAddr8(0, 0);

    // The full kernel is 6 sigmas wide.
    const float sigma = profileWidth / 6.0f;
    const int halfKernelSize = profileWidth / 2;

    skia_private::AutoTArray<float> halfKernel(halfKernelSize);

    // The half kernel should sum to 0.5.
    const float tot = 2.0f * make_unnormalized_half_kernel(halfKernel.get(), halfKernelSize, sigma);
    float sum = 0.0f;
    // Populate the profile from the right edge to the middle.
    for (int i = 0; i < halfKernelSize; ++i) {
        halfKernel[halfKernelSize - i - 1] /= tot;
        sum += halfKernel[halfKernelSize - i - 1];
        profile[profileWidth - i - 1] = SkUnitScalarClampToByte(sum);
    }
    // Populate the profile from the middle to the left edge (by flipping the half kernel and
    // continuing the summation).
    for (int i = 0; i < halfKernelSize; ++i) {
        sum += halfKernel[i];
        profile[halfKernelSize - i - 1] = SkUnitScalarClampToByte(sum);
    }
    // Ensure the tail of the Gaussian goes to zero.
    profile[profileWidth - 1] = 0;

    bitmap.setImmutable();
    return bitmap;
}

///////////////////////////////////////////////////////////////////////////////
//  RRect Blur
///////////////////////////////////////////////////////////////////////////////

// Evaluate the vertical blur at the specified 'y' value given the location of the top of the
// rrect.
static uint8_t eval_V(float top, int y, const uint8_t* integral, int integralSize, float sixSigma) {
    if (top < 0) {
        return 0;  // an empty column
    }

    float fT = (top - y - 0.5f) * (integralSize / sixSigma);
    if (fT < 0) {
        return 255;
    } else if (fT >= integralSize - 1) {
        return 0;
    }

    int lower = (int)fT;
    float frac = fT - lower;

    SkASSERT(lower + 1 < integralSize);

    return integral[lower] * (1.0f - frac) + integral[lower + 1] * frac;
}

// Apply a gaussian 'kernel' horizontally at the specified 'x', 'y' location.
static uint8_t eval_H(int x,
                      int y,
                      const std::vector<float>& topVec,
                      const float* kernel,
                      int kernelSize,
                      const uint8_t* integral,
                      int integralSize,
                      float sixSigma) {
    SkASSERT(0 <= x && x < (int)topVec.size());
    SkASSERT(kernelSize % 2);

    float accum = 0.0f;

    int xSampleLoc = x - (kernelSize / 2);
    for (int i = 0; i < kernelSize; ++i, ++xSampleLoc) {
        if (xSampleLoc < 0 || xSampleLoc >= (int)topVec.size()) {
            continue;
        }

        accum += kernel[i] * eval_V(topVec[xSampleLoc], y, integral, integralSize, sixSigma);
    }

    return accum + 0.5f;
}

SkBitmap CreateRRectBlurMask(const SkRRect& rrectToDraw, const SkISize& dimensions, float sigma) {
    SkASSERT(!skgpu::BlurIsEffectivelyIdentity(sigma));
    int radius = skgpu::BlurSigmaRadius(sigma);
    int kernelSize = skgpu::BlurKernelWidth(radius);

    SkASSERT(kernelSize % 2);
    SkASSERT(dimensions.width() % 2);
    SkASSERT(dimensions.height() % 2);

    SkVector radii = rrectToDraw.getSimpleRadii();
    SkASSERT(SkScalarNearlyEqual(radii.fX, radii.fY));

    const int halfWidthPlus1 = (dimensions.width() / 2) + 1;
    const int halfHeightPlus1 = (dimensions.height() / 2) + 1;

    std::unique_ptr<float[]> kernel(new float[kernelSize]);
    skgpu::Compute1DBlurKernel(sigma, radius, SkSpan<float>(kernel.get(), kernelSize));

    SkBitmap integral = CreateIntegralTable(6.0f * sigma);
    if (integral.empty()) {
        return {};
    }

    SkBitmap result;
    if (!result.tryAllocPixels(SkImageInfo::MakeA8(dimensions.width(), dimensions.height()))) {
        return {};
    }

    std::vector<float> topVec;
    topVec.reserve(dimensions.width());
    for (int x = 0; x < dimensions.width(); ++x) {
        if (x < rrectToDraw.rect().fLeft || x > rrectToDraw.rect().fRight) {
            topVec.push_back(-1);
        } else {
            if (x + 0.5f < rrectToDraw.rect().fLeft + radii.fX) {  // in the circular section
                float xDist = rrectToDraw.rect().fLeft + radii.fX - x - 0.5f;
                float h = sqrtf(radii.fX * radii.fX - xDist * xDist);
                SkASSERT(0 <= h && h < radii.fY);
                topVec.push_back(rrectToDraw.rect().fTop + radii.fX - h + 3 * sigma);
            } else {
                topVec.push_back(rrectToDraw.rect().fTop + 3 * sigma);
            }
        }
    }

    for (int y = 0; y < halfHeightPlus1; ++y) {
        uint8_t* scanline = result.getAddr8(0, y);

        for (int x = 0; x < halfWidthPlus1; ++x) {
            scanline[x] = eval_H(x,
                                 y,
                                 topVec,
                                 kernel.get(),
                                 kernelSize,
                                 integral.getAddr8(0, 0),
                                 integral.width(),
                                 6.0f * sigma);
            scanline[dimensions.width() - x - 1] = scanline[x];
        }

        memcpy(result.getAddr8(0, dimensions.height() - y - 1), scanline, result.rowBytes());
    }

    result.setImmutable();
    return result;
}

} // namespace skgpu
