/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuBlurUtils_DEFINED
#define SkGpuBlurUtils_DEFINED

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU
#include "include/core/SkRefCnt.h"
#include "include/private/GrTypesPriv.h"

class GrRecordingContext;
namespace skgpu { namespace v1 { class SurfaceDrawContext; }}
class GrSurfaceProxyView;
class GrTexture;

struct SkRect;

namespace SkGpuBlurUtils {

/** Maximum sigma before the implementation downscales the input image. */
static constexpr float kMaxSigma = 4.f;

#if SK_GPU_V1
/**
 * Applies a 2D Gaussian blur to a given texture. The blurred result is returned
 * as a surfaceDrawContext in case the caller wishes to draw into the result.
 * The GrSurfaceOrigin of the result will watch the GrSurfaceOrigin of srcView. The output
 * color type, color space, and alpha type will be the same as the src.
 *
 * Note: one of sigmaX and sigmaY should be non-zero!
 * @param context         The GPU context
 * @param srcView         The source to be blurred.
 * @param srcColorType    The colorType of srcProxy
 * @param srcAlphaType    The alphaType of srcProxy
 * @param srcColorSpace   Color space of the source.
 * @param dstBounds       The destination bounds, relative to the source texture.
 * @param srcBounds       The source bounds, relative to the source texture's offset. No pixels
 *                        will be sampled outside of this rectangle.
 * @param sigmaX          The blur's standard deviation in X.
 * @param sigmaY          The blur's standard deviation in Y.
 * @param tileMode        The mode to handle samples outside bounds.
 * @param fit             backing fit for the returned render target context
 * @return                The surfaceDrawContext containing the blurred result.
 */
std::unique_ptr<skgpu::v1::SurfaceDrawContext> GaussianBlur(
        GrRecordingContext*,
        GrSurfaceProxyView srcView,
        GrColorType srcColorType,
        SkAlphaType srcAlphaType,
        sk_sp<SkColorSpace> srcColorSpace,
        SkIRect dstBounds,
        SkIRect srcBounds,
        float sigmaX,
        float sigmaY,
        SkTileMode mode,
        SkBackingFit fit = SkBackingFit::kApprox);
#endif // SK_GPU_V1

static const int kBlurRRectMaxDivisions = 6;

// This method computes all the parameters for drawing a partially occluded nine-patched
// blurred rrect mask:
//   rrectToDraw - the integerized rrect to draw in the mask
//   widthHeight - how large to make the mask (rrectToDraw will be centered in this coord sys)
//   rectXs, rectYs - the x & y coordinates of the covering geometry lattice
//   texXs, texYs - the texture coordinate at each point in rectXs & rectYs
// It returns true if 'devRRect' is nine-patchable
bool ComputeBlurredRRectParams(const SkRRect& srcRRect,
                               const SkRRect& devRRect,
                               SkScalar sigma,
                               SkScalar xformedSigma,
                               SkRRect* rrectToDraw,
                               SkISize* widthHeight,
                               SkScalar rectXs[kBlurRRectMaxDivisions],
                               SkScalar rectYs[kBlurRRectMaxDivisions],
                               SkScalar texXs[kBlurRRectMaxDivisions],
                               SkScalar texYs[kBlurRRectMaxDivisions]);

int CreateIntegralTable(float sixSigma, SkBitmap* table);

void Compute1DGaussianKernel(float* kernel, float sigma, int radius);

void Compute1DLinearGaussianKernel(float* kernel, float* offset, float sigma, int radius);

// Any sigmas smaller than this are effectively an identity blur so can skip convolution at a higher
// level. The value was chosen because it corresponds roughly to a radius of 1/10px, and is slightly
// greater than sqrt(1/2*sigma^2) for SK_ScalarNearlyZero.
inline bool IsEffectivelyZeroSigma(float sigma) { return sigma <= 0.03f; }

inline int SigmaRadius(float sigma) {
    return IsEffectivelyZeroSigma(sigma) ? 0 : static_cast<int>(ceilf(sigma * 3.0f));
}

inline int KernelWidth(int radius) { return 2 * radius + 1; }

inline int LinearKernelWidth(int radius) { return radius + 1; }

}  // namespace SkGpuBlurUtils

#endif // SK_SUPPORT_GPU

#endif
