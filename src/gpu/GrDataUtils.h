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
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrSwizzle.h"

class GrImageInfo;

size_t GrCompressedDataSize(SkImage::CompressionType, int w, int h);

// Returns a value that can be used to set rowBytes for a transfer function.
size_t GrCompressedRowBytes(SkImage::CompressionType, int w);

// Compute the size of the buffer required to hold all the mipLevels of the specified type
// of data when all rowBytes are tight.
// Note there may still be padding between the mipLevels to meet alignment requirements.
size_t GrComputeTightCombinedBufferSize(size_t bytesPerPixel, SkISize baseDimensions,
                                        SkTArray<size_t>* individualMipOffsets, int mipLevelCount);

void GrFillInCompressedData(SkImage::CompressionType, int width, int height, char* dest,
                            const SkColor4f& color);

// Swizzle param is applied after loading and before converting from srcInfo to dstInfo.
bool GrConvertPixels(const GrImageInfo& dstInfo,       void* dst, size_t dstRB,
                     const GrImageInfo& srcInfo, const void* src, size_t srcRB,
                     bool flipY = false);

/** Clears the dst image to a constant color. */
bool GrClearImage(const GrImageInfo& dstInfo, void* dst, size_t dstRB, SkColor4f color);

#endif
