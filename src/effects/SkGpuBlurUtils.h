/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuBlurUtils_DEFINED
#define SkGpuBlurUtils_DEFINED

#if SK_SUPPORT_GPU
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
    * @param rect            The destination rectangle.
    * @param cropToRect      If true, do not sample any pixels outside the
    *                        source rect.
    * @param sigmaX          The blur's standard deviation in X.
    * @param sigmaY          The blur's standard deviation in Y.
    * @return the blurred texture, which may be srcTexture reffed, or a
    * new texture.  It is the caller's responsibility to unref this texture.
    */
    GrTexture* GaussianBlur(GrContext* context,
                            GrTexture* srcTexture,
                            bool canClobberSrc,
                            const SkRect& rect,
                            bool cropToRect,
                            float sigmaX,
                            float sigmaY);
#endif

};

#endif
