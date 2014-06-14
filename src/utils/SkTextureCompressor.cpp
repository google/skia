/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor.h"

#include "SkBitmap.h"
#include "SkData.h"
#include "SkEndian.h"

////////////////////////////////////////////////////////////////////////////////
//
// Utility Functions
//
////////////////////////////////////////////////////////////////////////////////

// Absolute difference between two values. More correct than SkTAbs(a - b)
// because it works on unsigned values.
template <typename T> inline T abs_diff(const T &a, const T &b) {
    return (a > b) ? (a - b) : (b - a);
}

////////////////////////////////////////////////////////////////////////////////
//
// LATC compressor
//
////////////////////////////////////////////////////////////////////////////////

// Return the squared minimum error cost of approximating 'pixel' using the
// provided palette. Return this in the middle 16 bits of the integer. Return
// the best index in the palette for this pixel in the bottom 8 bits.
static uint32_t compute_error(uint8_t pixel, uint8_t palette[8]) {
    int minIndex = 0;
    uint8_t error = abs_diff(palette[0], pixel);
    for (int i = 1; i < 8; ++i) {
        uint8_t diff = abs_diff(palette[i], pixel);
        if (diff < error) {
            minIndex = i;
            error = diff;
        }
    }
    uint16_t errSq = static_cast<uint16_t>(error) * static_cast<uint16_t>(error);
    SkASSERT(minIndex >= 0 && minIndex < 8);
    return (static_cast<uint32_t>(errSq) << 8) | static_cast<uint32_t>(minIndex);
}

// Compress LATC block. Each 4x4 block of pixels is decompressed by LATC from two
// values LUM0 and LUM1, and an index into the generated palette. LATC constructs
// a palette of eight colors from LUM0 and LUM1 using the algorithm:
//
// LUM0,              if lum0 > lum1 and code(x,y) == 0
// LUM1,              if lum0 > lum1 and code(x,y) == 1
// (6*LUM0+  LUM1)/7, if lum0 > lum1 and code(x,y) == 2
// (5*LUM0+2*LUM1)/7, if lum0 > lum1 and code(x,y) == 3
// (4*LUM0+3*LUM1)/7, if lum0 > lum1 and code(x,y) == 4
// (3*LUM0+4*LUM1)/7, if lum0 > lum1 and code(x,y) == 5
// (2*LUM0+5*LUM1)/7, if lum0 > lum1 and code(x,y) == 6
// (  LUM0+6*LUM1)/7, if lum0 > lum1 and code(x,y) == 7
//
// LUM0,              if lum0 <= lum1 and code(x,y) == 0
// LUM1,              if lum0 <= lum1 and code(x,y) == 1
// (4*LUM0+  LUM1)/5, if lum0 <= lum1 and code(x,y) == 2
// (3*LUM0+2*LUM1)/5, if lum0 <= lum1 and code(x,y) == 3
// (2*LUM0+3*LUM1)/5, if lum0 <= lum1 and code(x,y) == 4
// (  LUM0+4*LUM1)/5, if lum0 <= lum1 and code(x,y) == 5
// 0,                 if lum0 <= lum1 and code(x,y) == 6
// 255,               if lum0 <= lum1 and code(x,y) == 7
//
// We compute the LATC palette using the following simple algorithm:
// 1. Choose the minimum and maximum values in the block as LUM0 and LUM1
// 2. Figure out which of the two possible palettes is better.

static uint64_t compress_latc_block(uint8_t block[16]) {
    // Just do a simple min/max but choose which of the
    // two palettes is better
    uint8_t maxVal = 0;
    uint8_t minVal = 255;
    for (int i = 0; i < 16; ++i) {
        maxVal = SkMax32(maxVal, block[i]);
        minVal = SkMin32(minVal, block[i]);
    }

    // Generate palettes
    uint8_t palettes[2][8];

    // Straight linear ramp
    palettes[0][0] = maxVal;
    palettes[0][1] = minVal;
    for (int i = 1; i < 7; ++i) {
        palettes[0][i+1] = ((7-i)*maxVal + i*minVal) / 7;
    }

    // Smaller linear ramp with min and max byte values at the end.
    palettes[1][0] = minVal;
    palettes[1][1] = maxVal;
    for (int i = 1; i < 5; ++i) {
        palettes[1][i+1] = ((5-i)*maxVal + i*minVal) / 5;
    }
    palettes[1][6] = 0;
    palettes[1][7] = 255;

    // Figure out which of the two is better:
    //  -  accumError holds the accumulated error for each pixel from
    //     the associated palette
    //  -  indices holds the best indices for each palette in the
    //     bottom 48 (16*3) bits.
    uint32_t accumError[2] = { 0, 0 };
    uint64_t indices[2] = { 0, 0 };
    for (int i = 15; i >= 0; --i) {
        // For each palette:
        // 1. Retreive the result of this pixel
        // 2. Store the error in accumError
        // 3. Store the minimum palette index in indices.
        for (int p = 0; p < 2; ++p) {
            uint32_t result = compute_error(block[i], palettes[p]);
            accumError[p] += (result >> 8);
            indices[p] <<= 3;
            indices[p] |= result & 7;
        }
    }

    SkASSERT(indices[0] < (static_cast<uint64_t>(1) << 48));
    SkASSERT(indices[1] < (static_cast<uint64_t>(1) << 48));

    uint8_t paletteIdx = (accumError[0] > accumError[1]) ? 0 : 1;

    // Assemble the compressed block.
    uint64_t result = 0;

    // Jam the first two palette entries into the bottom 16 bits of
    // a 64 bit integer. Based on the palette that we chose, one will
    // be larger than the other and it will select the proper palette.
    result |= static_cast<uint64_t>(palettes[paletteIdx][0]);
    result |= static_cast<uint64_t>(palettes[paletteIdx][1]) << 8;

    // Jam the indices into the top 48 bits.
    result |= indices[paletteIdx] << 16;

    // We assume everything is little endian, if it's not then make it so.
    return SkEndian_SwapLE64(result);
}

static SkData *compress_a8_to_latc(const SkBitmap &bm) {
    // LATC compressed texels down into square 4x4 blocks
    static const int kLATCBlockSize = 4;

    // Make sure that our data is well-formed enough to be
    // considered for LATC compression
    if (bm.width() == 0 || bm.height() == 0 ||
        (bm.width() % kLATCBlockSize) != 0 ||
        (bm.height() % kLATCBlockSize) != 0 ||
        (bm.colorType() != kAlpha_8_SkColorType)) {
        return NULL;
    }

    // The LATC format is 64 bits per 4x4 block.
    static const int kLATCEncodedBlockSize = 8;

    int blocksX = bm.width() / kLATCBlockSize;
    int blocksY = bm.height() / kLATCBlockSize;

    int compressedDataSize = blocksX * blocksY * kLATCEncodedBlockSize;
    uint64_t* dst = reinterpret_cast<uint64_t*>(sk_malloc_throw(compressedDataSize));

    uint8_t block[16];
    const uint8_t* row = reinterpret_cast<const uint8_t*>(bm.getPixels());
    uint64_t* encPtr = dst;
    for (int y = 0; y < blocksY; ++y) {
        for (int x = 0; x < blocksX; ++x) {
            memcpy(block, row + (kLATCBlockSize * x), 4);
            memcpy(block + 4, row + bm.rowBytes() + (kLATCBlockSize * x), 4);
            memcpy(block + 8, row + 2*bm.rowBytes() + (kLATCBlockSize * x), 4);
            memcpy(block + 12, row + 3*bm.rowBytes() + (kLATCBlockSize * x), 4);

            *encPtr = compress_latc_block(block);
            ++encPtr;
        }
        row += kLATCBlockSize * bm.rowBytes();
    }

    return SkData::NewFromMalloc(dst, compressedDataSize);
}

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

typedef SkData *(*CompressBitmapProc)(const SkBitmap &bitmap);

SkData *CompressBitmapToFormat(const SkBitmap &bitmap, Format format) {
    SkAutoLockPixels alp(bitmap);

    CompressBitmapProc kProcMap[kLastEnum_SkColorType + 1][kFormatCnt];
    memset(kProcMap, 0, sizeof(kProcMap));

    // Map available bitmap configs to compression functions
    kProcMap[kAlpha_8_SkColorType][kLATC_Format] = compress_a8_to_latc;

    CompressBitmapProc proc = kProcMap[bitmap.colorType()][format];
    if (NULL != proc) {
        return proc(bitmap);
    }

    return NULL;
}

}  // namespace SkTextureCompressor
