/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVtoRGBEffect_DEFINED
#define GrYUVtoRGBEffect_DEFINED

class GrEffect;
class GrTexture;

namespace GrYUVtoRGBEffect {
    /**
     * Creates an effect that performs color conversion from YUV to RGB
     */
    GrEffect* Create(GrTexture* yTexture, GrTexture* uTexture, GrTexture* vTexture);
};

#endif
