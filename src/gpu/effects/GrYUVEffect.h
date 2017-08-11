/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVEffect_DEFINED
#define GrYUVEffect_DEFINED

#include "SkImageInfo.h"

class GrFragmentProcessor;
class GrTextureProxy;

namespace GrYUVEffect {
    /**
     * Creates an effect that performs color conversion from YUV to RGB. The input textures are
     * assumed to be kA8_GrPixelConfig.
     */
std::unique_ptr<GrFragmentProcessor> MakeYUVToRGB(sk_sp<GrTextureProxy> yProxy,
                                                  sk_sp<GrTextureProxy> uProxy,
                                                  sk_sp<GrTextureProxy> vProxy,
                                                  const SkISize sizes[3],
                                                  SkYUVColorSpace colorSpace, bool nv12);

/**
 * Creates a processor that performs color conversion from the passed in processor's RGB
 * channels to Y, U ,and V channels. The output color is (y, u, v, a) where a is the passed in
 * processor's alpha output.
 */
std::unique_ptr<GrFragmentProcessor> MakeRGBToYUV(std::unique_ptr<GrFragmentProcessor>,
                                                  SkYUVColorSpace);

/**
 * Creates a processor that performs color conversion from the passed in processor's RGB
 * channels to U and V channels. The output color is (u, v, 0, a) where a is the passed in
 * processor's alpha output.
 */
std::unique_ptr<GrFragmentProcessor> MakeRGBToUV(std::unique_ptr<GrFragmentProcessor>,
                                                 SkYUVColorSpace);
/**
 * Creates a processor that performs color conversion from the passed in fragment processors's
 * RGB channels to Y, U, or V (replicated across all four output color channels). The alpha
 * output of the passed in fragment processor is ignored.
 */
std::unique_ptr<GrFragmentProcessor> MakeRGBToY(std::unique_ptr<GrFragmentProcessor>,
                                                SkYUVColorSpace);
std::unique_ptr<GrFragmentProcessor> MakeRGBToU(std::unique_ptr<GrFragmentProcessor>,
                                                SkYUVColorSpace);
std::unique_ptr<GrFragmentProcessor> MakeRGBToV(std::unique_ptr<GrFragmentProcessor>,
                                                SkYUVColorSpace);
};

#endif
