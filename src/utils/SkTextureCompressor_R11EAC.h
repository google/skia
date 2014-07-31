/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompressor_R11EAC_DEFINED
#define SkTextureCompressor_R11EAC_DEFINED

#include "SkBlitter.h"

namespace SkTextureCompressor {

    bool CompressA8ToR11EAC(uint8_t* dst, const uint8_t* src,
                            int width, int height, int rowBytes);

    SkBlitter* CreateR11EACBlitter(int width, int height, void* outputBuffer);
}

#endif  // SkTextureCompressor_R11EAC_DEFINED
