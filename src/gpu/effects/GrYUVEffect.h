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
};

#endif
