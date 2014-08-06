/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompressor_ASTC_DEFINED
#define SkTextureCompressor_ASTC_DEFINED

#include <stdint.h>

// Forward declare
class SkBlitter;

namespace SkTextureCompressor {

    bool CompressA8To12x12ASTC(uint8_t* dst, const uint8_t* src,
                               int width, int height, int rowBytes);

    SkBlitter* CreateASTCBlitter(int width, int height, void* outputBuffer);
}

#endif  // SkTextureCompressor_ASTC_DEFINED
