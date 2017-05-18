/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuBlurUtils_DEFINED
#define SkGpuBlurUtils_DEFINED

#if SK_SUPPORT_GPU
#include "GrRenderTargetContext.h"

class GrContext;
class GrTexture;

struct SkRect;

namespace SkGpuBlurUtils {
  /**
    * Applies a 2D Gaussian blur to a given texture. The blurred result is returned
    * as a renderTargetContext in case the caller wishes to future draw into the result.
    * Note: one of sigmaX and sigmaY should be non-zero!
    * @param context         The GPU context
    * @param src             The source to be blurred.
    * @param colorSpace      Color space of the source (used for the renderTargetContext result,
    *                        too).
    * @param dstBounds       The destination bounds, relative to the source texture.
    * @param srcBounds       The source bounds, relative to the source texture. If non-null,
    *                        no pixels will be sampled outside of this rectangle.
    * @param sigmaX          The blur's standard deviation in X.
    * @param sigmaY          The blur's standard deviation in Y.
    * @param fit             backing fit for the returned render target context
    * @return                The renderTargetContext containing the blurred result.
    */
    sk_sp<GrRenderTargetContext> GaussianBlur(GrContext* context,
                                              sk_sp<GrTextureProxy> src,
                                              sk_sp<SkColorSpace> colorSpace,
                                              const SkIRect& dstBounds,
                                              const SkIRect* srcBounds,
                                              float sigmaX,
                                              float sigmaY,
                                              SkBackingFit fit = SkBackingFit::kApprox);
};

#endif
#endif
