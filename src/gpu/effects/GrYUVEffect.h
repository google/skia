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
class GrTexture;

namespace GrYUVEffect {
    /**
     * Creates an effect that performs color conversion from YUV to RGB. The input textures are
     * assumed to be kA8_GrPixelConfig.
     */
    const GrFragmentProcessor* CreateYUVToRGB(GrTexture* yTexture, GrTexture* uTexture,
                                              GrTexture* vTexture, const SkISize sizes[3],
                                              SkYUVColorSpace colorSpace);

    /**
     * Creates a processor that performs color conversion from the passed in processor's RGB
     * channels to Y, U ,and V channels. The output color is (y, u, v, a) where a is the passed in
     * processor's alpha output.
     */
    const GrFragmentProcessor* CreateRGBToYUV(const GrFragmentProcessor*,
                                              SkYUVColorSpace colorSpace);

    /**
     * Creates a processor that performs color conversion from the passed in processor's RGB
     * channels to U and V channels. The output color is (u, v, 0, a) where a is the passed in
     * processor's alpha output.
     */
    const GrFragmentProcessor* CreateRGBToUV(const GrFragmentProcessor*,
                                             SkYUVColorSpace colorSpace);
    /**
     * Creates a processor that performs color conversion from the passed in fragment processors's
     * RGB channels to Y, U, or V (replicated across all four output color channels). The alpha
     * output of the passed in fragment processor is ignored.
     */
    const GrFragmentProcessor* CreateRGBToY(const GrFragmentProcessor*, SkYUVColorSpace colorSpace);
    const GrFragmentProcessor* CreateRGBToU(const GrFragmentProcessor*, SkYUVColorSpace colorSpace);
    const GrFragmentProcessor* CreateRGBToV(const GrFragmentProcessor*, SkYUVColorSpace colorSpace);
};

#endif
