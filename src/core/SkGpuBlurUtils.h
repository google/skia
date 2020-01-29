/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuBlurUtils_DEFINED
#define SkGpuBlurUtils_DEFINED

#if SK_SUPPORT_GPU
#include "src/gpu/GrRenderTargetContext.h"

class GrContext;
class GrTexture;

struct SkRect;

namespace SkGpuBlurUtils {
/**
  * Applies a 2D Gaussian blur to a given texture. The blurred result is returned
  * as a renderTargetContext in case the caller wishes to draw into the result.
  *
  * The 'proxyOffset' is kept separate form 'srcBounds' because they exist in different
  * coordinate spaces. 'srcBounds' exists in the content space of the special image, and
  * 'proxyOffset' maps from the content space to the proxy's space.
  *
  * Note: one of sigmaX and sigmaY should be non-zero!
  * @param context         The GPU context
  * @param srcView         The source to be blurred.
  * @param srcColorType    The colorType of srcProxy
  * @param srcAlphaType    The alphaType of srcProxy
  * @param colorSpace      Color space of the source (used for the renderTargetContext result,
  *                        too).
  * @param dstBounds       The destination bounds, relative to the source texture.
  * @param srcBounds       The source bounds, relative to the source texture's offset. No pixels
  *                        will be sampled outside of this rectangle.
  * @param sigmaX          The blur's standard deviation in X.
  * @param sigmaY          The blur's standard deviation in Y.
  * @param tileMode        The mode to handle samples outside bounds.
  * @param fit             backing fit for the returned render target context
  * @return                The renderTargetContext containing the blurred result.
  */
std::unique_ptr<GrRenderTargetContext> GaussianBlur(GrRecordingContext* context,
                                                    GrSurfaceProxyView srcView,
                                                    GrColorType srcColorType,
                                                    SkAlphaType srcAlphaType,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    const SkIRect& dstBounds,
                                                    const SkIRect& srcBounds,
                                                    float sigmaX,
                                                    float sigmaY,
                                                    SkTileMode mode,
                                                    SkBackingFit fit = SkBackingFit::kApprox);
};

#endif
#endif
