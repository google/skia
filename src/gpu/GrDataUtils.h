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
#include "src/gpu/GrSwizzle.h"

size_t GrCompressedDataSize(SkImage::CompressionType, int w, int h);

// Compute the size of the buffer required to hold all the mipLevels of the specified type
// of data when all rowBytes are tight.
// Note there may still be padding between the mipLevels to meet alignment requirements.
size_t GrComputeTightCombinedBufferSize(size_t bytesPerPixel, int baseWidth, int baseHeight,
                                        SkTArray<size_t>* individualMipOffsets, int mipLevelCount);

void GrFillInData(GrPixelConfig, int baseWidth, int baseHeight,
                  const SkTArray<size_t>& individualMipOffsets, char* dest, const SkColor4f& color);

void GrFillInCompressedData(SkImage::CompressionType, int width, int height, char* dest,
                            const SkColor4f& color);

struct GrColorInfo {
    GrColorType fColorType = GrColorType::kUnknown;
    SkColorSpace* fColorSpace = nullptr;
    SkAlphaType fAlphaType = kPremul_SkAlphaType;
};

struct GrPixelInfo {
    GrColorInfo fColorInfo = {};
    GrSurfaceOrigin fOrigin = kTopLeft_GrSurfaceOrigin;
    int fWidth = 0;
    int fHeight = 0;
    size_t fRowBytes = 0;
};

// Swizzle param is applied after loading and before converting from srcInfo to dstInfo.
bool GrConvertPixels(const GrPixelInfo& dstInfo, void* dst, const GrPixelInfo& srcInfo,
                     const void* src, GrSwizzle swizzle = GrSwizzle{});

#endif
