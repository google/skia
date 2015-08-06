/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor.h"
#include "SkTextureCompressor_ASTC.h"
#include "SkTextureCompressor_LATC.h"
#include "SkTextureCompressor_R11EAC.h"

#include "SkBitmap.h"
#include "SkBitmapProcShader.h"
#include "SkData.h"
#include "SkEndian.h"
#include "SkOpts.h"

#ifndef SK_IGNORE_ETC1_SUPPORT
#  include "etc1.h"
#endif

// Convert ETC1 functions to our function signatures
static bool compress_etc1_565(uint8_t* dst, const uint8_t* src,
                              int width, int height, size_t rowBytes) {
#ifndef SK_IGNORE_ETC1_SUPPORT
    return 0 == etc1_encode_image(src, width, height, 2, SkToInt(rowBytes), dst);
#else
    return false;
#endif
}

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

void GetBlockDimensions(Format format, int* dimX, int* dimY, bool matchSpec) {
    if (NULL == dimX || NULL == dimY) {
        return;
    }

    if (!matchSpec && SkOpts::fill_block_dimensions(format, dimX, dimY)) {
        return;
    }

    // No specialized arguments, return the dimensions as they are in the spec.
    static const struct FormatDimensions {
        const int fBlockSizeX;
        const int fBlockSizeY;
    } kFormatDimensions[kFormatCnt] = {
        { 4, 4 }, // kLATC_Format
        { 4, 4 }, // kR11_EAC_Format
        { 4, 4 }, // kETC1_Format
        { 4, 4 }, // kASTC_4x4_Format
        { 5, 4 }, // kASTC_5x4_Format
        { 5, 5 }, // kASTC_5x5_Format
        { 6, 5 }, // kASTC_6x5_Format
        { 6, 6 }, // kASTC_6x6_Format
        { 8, 5 }, // kASTC_8x5_Format
        { 8, 6 }, // kASTC_8x6_Format
        { 8, 8 }, // kASTC_8x8_Format
        { 10, 5 }, // kASTC_10x5_Format
        { 10, 6 }, // kASTC_10x6_Format
        { 10, 8 }, // kASTC_10x8_Format
        { 10, 10 }, // kASTC_10x10_Format
        { 12, 10 }, // kASTC_12x10_Format
        { 12, 12 }, // kASTC_12x12_Format
    };

    *dimX = kFormatDimensions[format].fBlockSizeX;
    *dimY = kFormatDimensions[format].fBlockSizeY;
}

int GetCompressedDataSize(Format fmt, int width, int height) {
    int dimX, dimY;
    GetBlockDimensions(fmt, &dimX, &dimY, true);

    int encodedBlockSize = 0;

    switch (fmt) {
        // These formats are 64 bits per 4x4 block.
        case kLATC_Format:
        case kR11_EAC_Format:
        case kETC1_Format:
            encodedBlockSize = 8;
            break;

        // This format is 128 bits.
        case kASTC_4x4_Format:
        case kASTC_5x4_Format:
        case kASTC_5x5_Format:
        case kASTC_6x5_Format:
        case kASTC_6x6_Format:
        case kASTC_8x5_Format:
        case kASTC_8x6_Format:
        case kASTC_8x8_Format:
        case kASTC_10x5_Format:
        case kASTC_10x6_Format:
        case kASTC_10x8_Format:
        case kASTC_10x10_Format:
        case kASTC_12x10_Format:
        case kASTC_12x12_Format:
            encodedBlockSize = 16;
            break;

        default:
            SkFAIL("Unknown compressed format!");
            return -1;
    }

    if(((width % dimX) == 0) && ((height % dimY) == 0)) {
        const int blocksX = width / dimX;
        const int blocksY = height / dimY;

        return blocksX * blocksY * encodedBlockSize;
    }

    return -1;
}

bool CompressBufferToFormat(uint8_t* dst, const uint8_t* src, SkColorType srcColorType,
                            int width, int height, size_t rowBytes, Format format) {
    SkOpts::TextureCompressor proc = SkOpts::texture_compressor(srcColorType, format);
    if (proc && proc(dst, src, width, height, rowBytes)) {
        return true;
    }

    switch (srcColorType) {
        case kAlpha_8_SkColorType:
            if (format == kLATC_Format)       { proc = CompressA8ToLATC;      }
            if (format == kR11_EAC_Format)    { proc = CompressA8ToR11EAC;    }
            if (format == kASTC_12x12_Format) { proc = CompressA8To12x12ASTC; }
            break;
        case kRGB_565_SkColorType:
            if (format == kETC1_Format) { proc = compress_etc1_565; }
            break;
        default:
            break;
    }
    if (proc && proc(dst, src, width, height, rowBytes)) {
        return true;
    }

    return false;
}

SkData* CompressBitmapToFormat(const SkPixmap& pixmap, Format format) {
    int compressedDataSize = GetCompressedDataSize(format, pixmap.width(), pixmap.height());
    if (compressedDataSize < 0) {
        return NULL;
    }

    const uint8_t* src = reinterpret_cast<const uint8_t*>(pixmap.addr());
    SkData* dst = SkData::NewUninitialized(compressedDataSize);

    if (!CompressBufferToFormat((uint8_t*)dst->writable_data(), src, pixmap.colorType(),
                                pixmap.width(), pixmap.height(), pixmap.rowBytes(), format)) {
        dst->unref();
        dst = NULL;
    }
    return dst;
}

SkBlitter* CreateBlitterForFormat(int width, int height, void* compressedBuffer,
                                  SkTBlitterAllocator *allocator, Format format) {
    switch(format) {
        case kLATC_Format:
            return CreateLATCBlitter(width, height, compressedBuffer, allocator);

        case kR11_EAC_Format:
            return CreateR11EACBlitter(width, height, compressedBuffer, allocator);

        case kASTC_12x12_Format:
            return CreateASTCBlitter(width, height, compressedBuffer, allocator);

        default:
            return NULL;
    }

    return NULL;
}

bool DecompressBufferFromFormat(uint8_t* dst, int dstRowBytes, const uint8_t* src,
                                int width, int height, Format format) {
    int dimX, dimY;
    GetBlockDimensions(format, &dimX, &dimY, true);

    if (width < 0 || ((width % dimX) != 0) || height < 0 || ((height % dimY) != 0)) {
        return false;
    }

    switch(format) {
        case kLATC_Format:
            DecompressLATC(dst, dstRowBytes, src, width, height);
            return true;

        case kR11_EAC_Format:
            DecompressR11EAC(dst, dstRowBytes, src, width, height);
            return true;

#ifndef SK_IGNORE_ETC1_SUPPORT
        case kETC1_Format:
            return 0 == etc1_decode_image(src, dst, width, height, 3, dstRowBytes);
#endif

        case kASTC_4x4_Format:
        case kASTC_5x4_Format:
        case kASTC_5x5_Format:
        case kASTC_6x5_Format:
        case kASTC_6x6_Format:
        case kASTC_8x5_Format:
        case kASTC_8x6_Format:
        case kASTC_8x8_Format:
        case kASTC_10x5_Format:
        case kASTC_10x6_Format:
        case kASTC_10x8_Format:
        case kASTC_10x10_Format:
        case kASTC_12x10_Format:
        case kASTC_12x12_Format:
            DecompressASTC(dst, dstRowBytes, src, width, height, dimX, dimY);
            return true;

        default:
            // Do nothing...
            break;
    }

    return false;
}

}  // namespace SkTextureCompressor
