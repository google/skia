/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuBlurUtils_DEFINED
#define SkGpuBlurUtils_DEFINED

#if SK_SUPPORT_GPU
#include "GrTextureProvider.h"

class GrTexture;
class GrContext;
#endif

struct SkRect;

namespace SkGpuBlurUtils {

#if SK_SUPPORT_GPU
  /**
    * Applies a 2D Gaussian blur to a given texture.
    * @param context         The GPU context
    * @param srcTexture      The source texture to be blurred.
    * @param canClobberSrc   If true, srcTexture may be overwritten, and
    *                        may be returned as the result.
    * @param allowSRGBInputs Should sRGB inputs be allowed to perform sRGB to linear conversion.
    * @param dstBounds       The destination bounds, relative to the source texture.
    * @param srcBounds       The source bounds, relative to the source texture. If non-null,
    *                        no pixels will be sampled outside of this rectangle.
    * @param sigmaX          The blur's standard deviation in X.
    * @param sigmaY          The blur's standard deviation in Y.
    * @return the blurred texture, which may be srcTexture reffed, or a
    * new texture.  It is the caller's responsibility to unref this texture.
    */
    GrTexture* GaussianBlur(GrContext* context,
                            GrTexture* srcTexture,
                            bool canClobberSrc,
                            bool allowSRGBInputs,
                            const SkRect& dstBounds,
                            const SkRect* srcBounds,
                            float sigmaX,
                            float sigmaY);
#endif

};

#endif
