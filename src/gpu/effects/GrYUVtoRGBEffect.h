/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVtoRGBEffect_DEFINED
#define GrYUVtoRGBEffect_DEFINED

#include "SkImageInfo.h"

class GrFragmentProcessor;
class GrProcessorDataManager;
class GrTexture;

namespace GrYUVtoRGBEffect {
    /**
     * Creates an effect that performs color conversion from YUV to RGB
     */
    GrFragmentProcessor* Create(GrProcessorDataManager*, GrTexture* yTexture, GrTexture* uTexture,
                                GrTexture* vTexture, const SkISize sizes[3],
                                SkYUVColorSpace colorSpace);
};

#endif
