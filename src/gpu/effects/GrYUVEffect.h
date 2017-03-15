/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVEffect_DEFINED
#define GrYUVEffect_DEFINED

#include "SkImageInfo.h"

class GrResourceProvider;
class GrFragmentProcessor;
class GrTextureProxy;

namespace GrYUVEffect {
    /**
     * Creates an effect that performs color conversion from YUV to RGB. The input textures are
     * assumed to be kA8_GrPixelConfig.
     */
    sk_sp<GrFragmentProcessor> MakeYUVToRGB(GrResourceProvider* resourceProvider,
                                            sk_sp<GrTextureProxy> yProxy,
                                            sk_sp<GrTextureProxy> uProxy,
                                            sk_sp<GrTextureProxy> vProxy, const SkISize sizes[3],
                                            SkYUVColorSpace colorSpace, bool nv12);

    /**
     * Creates a processor that performs color conversion from the passed in processor's RGB
     * channels to Y, U ,and V channels. The output color is (y, u, v, a) where a is the passed in
     * processor's alpha output.
     */
    sk_sp<GrFragmentProcessor> MakeRGBToYUV(sk_sp<GrFragmentProcessor>, SkYUVColorSpace);

    /**
     * Creates a processor that performs color conversion from the passed in processor's RGB
     * channels to U and V channels. The output color is (u, v, 0, a) where a is the passed in
     * processor's alpha output.
     */
    sk_sp<GrFragmentProcessor> MakeRGBToUV(sk_sp<GrFragmentProcessor>, SkYUVColorSpace);
    /**
     * Creates a processor that performs color conversion from the passed in fragment processors's
     * RGB channels to Y, U, or V (replicated across all four output color channels). The alpha
     * output of the passed in fragment processor is ignored.
     */
    sk_sp<GrFragmentProcessor> MakeRGBToY(sk_sp<GrFragmentProcessor>, SkYUVColorSpace);
    sk_sp<GrFragmentProcessor> MakeRGBToU(sk_sp<GrFragmentProcessor>, SkYUVColorSpace);
    sk_sp<GrFragmentProcessor> MakeRGBToV(sk_sp<GrFragmentProcessor>, SkYUVColorSpace);
};

#endif
