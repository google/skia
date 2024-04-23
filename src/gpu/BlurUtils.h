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
#include "include/private/base/SkFloatingPoint.h"

#include <array>

class SkBitmap;
class SkRRect;
class SkRuntimeEffect;
struct SkV4;

// TODO(b/): Many of these utilities could be lifted even into src/core as part of the backend
// agnostic blur engine once that API exists.

namespace skgpu {

// The kernel width of a Gaussian blur of the given pixel radius, for when all pixels are sampled.
constexpr int BlurKernelWidth(int radius) { return 2 * radius + 1; }

// The kernel width of a Gaussian blur of the given pixel radius, that relies on HW bilinear
// filtering to combine adjacent pixels.
constexpr int BlurLinearKernelWidth(int radius) { return radius + 1; }

// Any sigmas smaller than this are effectively an identity blur so can skip convolution at a higher
// level. The value was chosen because it corresponds roughly to a radius of 1/10px, and because
// 2*sigma^2 is slightly greater than SK_ScalarNearlyZero.
constexpr bool BlurIsEffectivelyIdentity(float sigma) { return sigma <= 0.03f; }

// Convert from a sigma Gaussian standard deviation to a pixel radius such that pixels outside the
// radius would have an insignificant contribution to the final blurred value.
inline int BlurSigmaRadius(float sigma) {
    // sk_float_ceil2int is not constexpr
    return BlurIsEffectivelyIdentity(sigma) ? 0 : sk_float_ceil2int(3.f * sigma);
}

// The maximum sigma that can be computed without downscaling is based on the number of uniforms and
// texture samples the effects will make in a single pass. For 1D passes, the number of samples
// is equal to `BlurLinearKernelWidth`; for 2D passes, it is equal to
// `BlurKernelWidth(radiusX)*BlurKernelWidth(radiusY)`. This maps back to different maximum sigmas
// depending on the approach used, as well as the ratio between the sigmas for the X and Y axes if
// a 2D blur is performed.
static constexpr int kMaxBlurSamples = 28;

// TODO(b/297393474): Update max linear sigma to 9; it had been 4 when a full 1D kernel was used,
// but never updated after the linear filtering optimization reduced the number of sample() calls
// required. Keep it at 4 for now to better isolate performance changes due to switching to a
// runtime effect and constant loop structure.
static constexpr float kMaxLinearBlurSigma = 4.f; // -> radius = 27 -> linear kernel width = 28
// NOTE: There is no defined kMaxBlurSigma for direct 2D blurs since it is entirely dependent on the
// ratio between the two axes' sigmas, but generally it will be small on the order of a 5x5 kernel.

// Return a runtime effect that applies a 2D Gaussian blur in a single pass. The returned effect can
// perform arbitrarily sized blur kernels so long as the kernel area is less than kMaxBlurSamples.
// An SkRuntimeEffect is returned to give flexibility for callers to convert it to an SkShader or
// a GrFragmentProcessor. Callers are responsible for providing the uniform values (using the
// appropriate API of the target effect type). The effect declares the following uniforms:
//
//    uniform half4  kernel[7];
//    uniform half4  offsets[14];
//    uniform shader child;
//
// 'kernel' should be set to the output of Compute2DBlurKernel(). 'offsets' should be set to the
// output of Compute2DBlurOffsets() with the same 'radii' passed to this function. 'child' should be
// bound to whatever input is intended to be blurred, and can use nearest-neighbor sampling
// (assuming it's an image).
const SkRuntimeEffect* GetBlur2DEffect(const SkISize& radii);

// Return a runtime effect that applies a 1D Gaussian blur, taking advantage of HW linear
// interpolation to accumulate adjacent pixels with fewer samples. The returned effect can be used
// for both X and Y axes by changing the 'dir' uniform value (see below). It can be used for all
// 1D blurs such that BlurLinearKernelWidth(radius) is less than or equal to kMaxBlurSamples.
// Like GetBlur2DEffect(), the caller is free to convert this to an SkShader or a
// GrFragmentProcessor and is responsible for assigning uniforms with the appropriate API. Its
// uniforms are declared as:
//
//     uniform half4  offsetsAndKernel[14];
//     uniform half2  dir;
//     uniform int    radius;
//     uniform shader child;
//
// 'offsetsAndKernel' should be set to the output of Compute1DBlurLinearKernel(). 'radius' should
// match the radius passed to that function. 'dir' should either be the vector {1,0} or {0,1}
// for X and Y axis passes, respectively. 'child' should be bound to whatever input is intended to
// be blurred and must use linear sampling in order for the outer blur effect to function correctly.
const SkRuntimeEffect* GetLinearBlur1DEffect(int radius);

// Calculates a set of weights for a 2D Gaussian blur of the given sigma and radius. It is assumed
// that the radius was from prior calls to BlurSigmaRadius(sigma.width()|height()) and is passed in
// to avoid redundant calculations.
//
// The provided span is fully written. The kernel is stored in row-major order based on the provided
// radius. Any remaining indices in the span are zero initialized. The span must have at least
// BlurKernelWidth(radius.width())*BlurKernelWidth(radius.height()) elements.
//
// NOTE: These take spans because it can be useful to compute full kernels that are larger than what
// is supported in the GPU effects.
void Compute2DBlurKernel(SkSize sigma,
                         SkISize radius,
                         SkSpan<float> kernel);

// A convenience function that packs the kMaxBlurSample scalars into SkV4's to match the required
// type of the uniforms in GetBlur2DEffect().
void Compute2DBlurKernel(SkSize sigma,
                         SkISize radius,
                         std::array<SkV4, kMaxBlurSamples/4>& kernel);

// A convenience for the 2D case where one dimension has a sigma of 0.
inline void Compute1DBlurKernel(float sigma, int radius, SkSpan<float> kernel) {
    Compute2DBlurKernel(SkSize{sigma, 0.f}, SkISize{radius, 0}, kernel);
}

// Utility function to fill in 'offsets' for the effect returned by GetBlur2DEffect(). It
// automatically fills in the elements beyond the kernel size with the last real offset to
// maximize texture cache hits. Each offset is really an SkV2 but are packed into SkV4's to match
// the uniform declaration, and are otherwise ordered row-major.
void Compute2DBlurOffsets(SkISize radius, std::array<SkV4, kMaxBlurSamples/2>& offsets);

// Calculates a set of weights and sampling offsets for a 1D blur that uses GPU hardware to linearly
// combine two logical source pixel values. This assumes that 'radius' was from a prior call to
// BlurSigmaRadius() and is passed in to avoid redundant calculations. To match std140 uniform
// packing, the offset and kernel weight for adjacent samples are packed into a single SkV4 as
//   {offset[2*i], kernel[2*i], offset[2*i+1], kernel[2*i+1]}
//
// The provided array is fully written to. The calculated values are written to indices 0 through
// BlurLinearKernelWidth(radius), with any remaining indices zero initialized. It requires the spans
// to be the same size and have at least BlurLinearKernelWidth(radius) elements.
//
// NOTE: This takes an array of a constrained size because its main use is calculating uniforms for
// an effect with a matching constraint. Knowing the size of the linear kernel means the full kernel
// can be stored on the stack internally.
void Compute1DBlurLinearKernel(float sigma,
                               int radius,
                               std::array<SkV4, kMaxBlurSamples/2>& offsetsAndKernel);

// Calculates the integral table for an analytic rectangle blur. The integral values are stored in
// the red channel of the provided bitmap, which will be 1D with a 1-pixel height.
SkBitmap CreateIntegralTable(float sixSigma);

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
