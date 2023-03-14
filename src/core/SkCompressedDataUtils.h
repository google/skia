/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCompressedDataUtils_DEFINED
#define SkCompressedDataUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"

#include <cstddef>

class SkBitmap;
class SkData;
struct SkISize;

static constexpr bool SkTextureCompressionTypeIsOpaque(SkTextureCompressionType compression) {
    switch (compression) {
        case SkTextureCompressionType::kNone:            return true;
        case SkTextureCompressionType::kETC2_RGB8_UNORM: return true;
        case SkTextureCompressionType::kBC1_RGB8_UNORM:  return true;
        case SkTextureCompressionType::kBC1_RGBA8_UNORM: return false;
    }

    SkUNREACHABLE;
}

size_t SkCompressedDataSize(SkTextureCompressionType, SkISize baseDimensions,
                            skia_private::TArray<size_t>* individualMipOffsets, bool mipmapped);
size_t SkCompressedBlockSize(SkTextureCompressionType type);

/**
 * Returns the data size for the given SkTextureCompressionType
 */
size_t SkCompressedFormatDataSize(SkTextureCompressionType compressionType,
                                  SkISize dimensions, bool mipmapped);

 /*
  * This method will decompress the bottommost level in 'data' into 'dst'.
  */
bool SkDecompress(sk_sp<SkData> data,
                  SkISize dimensions,
                  SkTextureCompressionType compressionType,
                  SkBitmap* dst);

#endif
