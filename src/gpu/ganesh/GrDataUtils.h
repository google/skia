/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDataUtils_DEFINED
#define GrDataUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

#include <cstddef>

class GrImageInfo;
class GrCPixmap;
class GrPixmap;
class SkPixmap;
enum class SkTextureCompressionType;

size_t GrNumBlocks(SkTextureCompressionType, SkISize baseDimensions);

// Returns a value that can be used to set rowBytes for a transfer function.
size_t GrCompressedRowBytes(SkTextureCompressionType, int w);

// Return the pixel dimensions of a compressed texture. The topmost levels
// of a compressed mipmapped texture (i.e., 1x1 or 2x2) still occupy a full
// block and thus objectively take up more pixels (e.g., 4x4 pixels for ETC1).
SkISize GrCompressedDimensions(SkTextureCompressionType, SkISize baseDimensions);

// Compute the size of the buffer required to hold all the mipLevels of the specified type
// of data when all rowBytes are tight.
// Note there may still be padding between the mipLevels to meet alignment requirements.
size_t GrComputeTightCombinedBufferSize(
        size_t bytesPerPixel, SkISize baseDimensions,
        skia_private::TArray<size_t>* individualMipOffsets, int mipLevelCount);

void GrFillInCompressedData(SkTextureCompressionType,
                            SkISize dimensions,
                            skgpu::Mipmapped,
                            char* dest,
                            const SkColor4f& color);

bool GrConvertPixels(const GrPixmap& dst, const GrCPixmap& src, bool flipY = false);

/** Clears the dst image to a constant color. */
bool GrClearImage(const GrImageInfo& dstInfo, void* dst, size_t dstRB, std::array<float, 4> color);

#if defined(GR_TEST_UTILS)
/**
 * BC1 compress an image that contains only either opaque black or transparent black and one
 * other color.
 *   opaque pixmaps      -> kBC1_RGB8_UNORM
 *   transparent pixmaps -> kBC1_RGBA8_UNORM
 */
void GrTwoColorBC1Compress(const SkPixmap& pixmap, SkColor otherColor, char* dstPixels);
#endif

#endif
