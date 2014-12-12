/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompressor_R11EAC_DEFINED
#define SkTextureCompressor_R11EAC_DEFINED

#include "SkBitmapProcShader.h"

class SkBlitter;

namespace SkTextureCompressor {

    bool CompressA8ToR11EAC(uint8_t* dst, const uint8_t* src,
                            int width, int height, size_t rowBytes);

    SkBlitter* CreateR11EACBlitter(int width, int height, void* outputBuffer,
                                   SkTBlitterAllocator* allocator);

    void DecompressR11EAC(uint8_t* dst, int dstRB, const uint8_t* src, int width, int height);
}

#endif  // SkTextureCompressor_R11EAC_DEFINED
