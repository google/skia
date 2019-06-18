/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDataUtils_DEFINED
#define GrDataUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/private/GrTypesPriv.h"

// TODO: consolidate all the backend-specific flavors of this method to this
size_t GrETC1CompressedDataSize(int w, int h);

// TODO: should this be grown into a replacement for GrPixelConfig?
enum class GrCompression {
    kNone,
    kETC1,
};

// Compute the size of the buffer required to hold all the mipLevels of the specified type
// of data when all rowBytes are tight.
// Note there may still be padding between the mipLevels to meet alignment requirements.
size_t GrComputeTightCombinedBufferSize(GrCompression, size_t bytesPerPixel,
                                        int baseWidth, int baseHeight,
                                        SkTArray<size_t>* individualMipOffsets,
                                        int mipLevelCount);

void GrFillInData(GrCompression, GrPixelConfig,
                  int baseWidth, int baseHeight,
                  const SkTArray<size_t>& individualMipOffsets,
                  char* dest, const SkColor4f& color);

#endif
