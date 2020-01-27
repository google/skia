/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCompressedDataUtils_DEFINED
#define SkCompressedDataUtils_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/SkTArray.h"

class SkBitmap;
class SkData;

static constexpr bool SkCompressionTypeIsOpaque(SkImage::CompressionType compression) {
    switch (compression) {
        case SkImage::CompressionType::kNone:            return true;
        case SkImage::CompressionType::kETC2_RGB8_UNORM: return true;
        case SkImage::CompressionType::kBC1_RGB8_UNORM:  return true;
        case SkImage::CompressionType::kBC1_RGBA8_UNORM: return false;
    }

    SkUNREACHABLE;
}

size_t SkCompressedDataSize(SkImage::CompressionType, SkISize baseDimensions,
                            SkTArray<size_t>* individualMipOffsets, bool mipMapped);


/**
 * Returns the data size for the given SkImage::CompressionType
 */
size_t SkCompressedFormatDataSize(SkImage::CompressionType compressionType,
                                  SkISize dimensions, bool mipMapped);

 /*
  * This method will decompress the bottommost level in 'data' into 'dst'.
  */
bool SkDecompress(sk_sp<SkData> data,
                  SkISize dimensions,
                  SkImage::CompressionType compressionType,
                  SkBitmap* dst);

#endif
