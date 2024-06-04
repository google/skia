/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_BlurUtils_DEFINED
#define skgpu_BlurUtils_DEFINED

#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "src/core/SkBlurEngine.h"

#include <array>

class SkBitmap;
class SkRRect;
class SkRuntimeEffect;
struct SkV4;

namespace skgpu {

// TODO: Remove functions that just wrap SkBlurEngine utilities once calling code is updated to
// use SkBlurEngine directly or is deleted entirely in favor of SkShaderBlurAlgorithm.
constexpr int BlurKernelWidth(int radius) { return SkShaderBlurAlgorithm::KernelWidth(radius); }
constexpr int BlurLinearKernelWidth(int radius) {
    return SkShaderBlurAlgorithm::LinearKernelWidth(radius);
}
constexpr bool BlurIsEffectivelyIdentity(float sigma) {
    return SkBlurEngine::IsEffectivelyIdentity(sigma);
}
inline int BlurSigmaRadius(float sigma) { return SkBlurEngine::SigmaToRadius(sigma); }

static constexpr int kMaxBlurSamples = SkShaderBlurAlgorithm::kMaxSamples;
static constexpr float kMaxLinearBlurSigma = SkShaderBlurAlgorithm::kMaxLinearSigma;

inline const SkRuntimeEffect* GetBlur2DEffect(const SkISize& radii) {
    return SkShaderBlurAlgorithm::GetBlur2DEffect(radii);
}

inline const SkRuntimeEffect* GetLinearBlur1DEffect(int radius) {
    return SkShaderBlurAlgorithm::GetLinearBlur1DEffect(radius);
}

inline void Compute2DBlurKernel(SkSize sigma, SkISize radius, SkSpan<float> kernel) {
    SkShaderBlurAlgorithm::Compute2DBlurKernel(sigma, radius, kernel);
}

inline void Compute2DBlurKernel(SkSize sigma,
                                SkISize radius,
                                std::array<SkV4, kMaxBlurSamples/4>& kernel) {
    SkShaderBlurAlgorithm::Compute2DBlurKernel(sigma, radius, kernel);
}

inline void Compute1DBlurKernel(float sigma, int radius, SkSpan<float> kernel) {
    SkShaderBlurAlgorithm::Compute1DBlurKernel(sigma, radius, kernel);
}

inline void Compute2DBlurOffsets(SkISize radius, std::array<SkV4, kMaxBlurSamples/2>& offsets) {
    SkShaderBlurAlgorithm::Compute2DBlurOffsets(radius, offsets);
}

inline void Compute1DBlurLinearKernel(float sigma,
                                      int radius,
                                      std::array<SkV4, kMaxBlurSamples/2>& offsetsAndKernel) {
    SkShaderBlurAlgorithm::Compute1DBlurLinearKernel(sigma, radius, offsetsAndKernel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Calculates the integral table for an analytic rectangle blur. The integral values are stored in
// the red channel of the provided bitmap, which will be 1D with a 1-pixel height.
SkBitmap CreateIntegralTable(int width);

// Returns the width of an integral table we will create for the given 6*sigma.
int ComputeIntegralTableWidth(float sixSigma);

// Creates a profile of a blurred circle.
SkBitmap CreateCircleProfile(float sigma, float radius, int profileWidth);

// Creates a half plane approximation profile of a blurred circle.
SkBitmap CreateHalfPlaneProfile(int profileWidth);

// Creates a blurred rounded rectangle mask. 'rrectToDraw' is the original rrect centered within
// bounds defined by 'dimensions', which encompass the entire blurred rrect.
SkBitmap CreateRRectBlurMask(const SkRRect& rrectToDraw, const SkISize& dimensions, float sigma);

} // namespace skgpu

#endif // skgpu_BlurUtils_DEFINED
