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

// LATC compressed texels down into square 4x4 blocks
static const int kPaletteSize = 8;
static const int kLATCBlockSize = 4;
static const int kPixelsPerBlock = kLATCBlockSize * kLATCBlockSize;

// Generates an LATC palette. LATC constructs
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

static void generate_palette(uint8_t palette[], uint8_t lum0, uint8_t lum1) {
    palette[0] = lum0;
    palette[1] = lum1;
    if (lum0 > lum1) {
        for (int i = 1; i < 7; i++) {
            palette[i+1] = ((7-i)*lum0 + i*lum1) / 7;
        }
    } else {
        for (int i = 1; i < 5; i++) {
            palette[i+1] = ((5-i)*lum0 + i*lum1) / 5;
        }
        palette[6] = 0;
        palette[7] = 255;
    }
}

static bool is_extremal(uint8_t pixel) {
    return 0 == pixel || 255 == pixel;
}

// Compress a block by using the bounding box of the pixels. It is assumed that
// there are no extremal pixels in this block otherwise we would have used
// compressBlockBBIgnoreExtremal.
static uint64_t compress_block_bb(const uint8_t pixels[]) {
    uint8_t minVal = 255;
    uint8_t maxVal = 0;
    for (int i = 0; i < kPixelsPerBlock; ++i) {
        minVal = SkTMin(pixels[i], minVal);
        maxVal = SkTMax(pixels[i], maxVal);
    }

    SkASSERT(!is_extremal(minVal));
    SkASSERT(!is_extremal(maxVal));

    uint8_t palette[kPaletteSize];
    generate_palette(palette, maxVal, minVal);

    uint64_t indices = 0;
    for (int i = kPixelsPerBlock - 1; i >= 0; --i) {

        // Find the best palette index
        uint8_t bestError = abs_diff(pixels[i], palette[0]);
        uint8_t idx = 0;
        for (int j = 1; j < kPaletteSize; ++j) {
            uint8_t error = abs_diff(pixels[i], palette[j]);
            if (error < bestError) {
                bestError = error;
                idx = j;
            }
        }

        indices <<= 3;
        indices |= idx;
    }

    return
        SkEndian_SwapLE64(
            static_cast<uint64_t>(maxVal) |
            (static_cast<uint64_t>(minVal) << 8) |
            (indices << 16));
}

// Compress a block by using the bounding box of the pixels without taking into
// account the extremal values. The generated palette will contain extremal values
// and fewer points along the line segment to interpolate.
static uint64_t compress_block_bb_ignore_extremal(const uint8_t pixels[]) {
    uint8_t minVal = 255;
    uint8_t maxVal = 0;
    for (int i = 0; i < kPixelsPerBlock; ++i) {
        if (is_extremal(pixels[i])) {
            continue;
        }

        minVal = SkTMin(pixels[i], minVal);
        maxVal = SkTMax(pixels[i], maxVal);
    }

    SkASSERT(!is_extremal(minVal));
    SkASSERT(!is_extremal(maxVal));

    uint8_t palette[kPaletteSize];
    generate_palette(palette, minVal, maxVal);

    uint64_t indices = 0;
    for (int i = kPixelsPerBlock - 1; i >= 0; --i) {

        // Find the best palette index
        uint8_t idx = 0;
        if (is_extremal(pixels[i])) {
            if (0xFF == pixels[i]) {
                idx = 7;
            } else if (0 == pixels[i]) {
                idx = 6;
            } else {
                SkFAIL("Pixel is extremal but not really?!");
            }
        } else {
            uint8_t bestError = abs_diff(pixels[i], palette[0]);
            for (int j = 1; j < kPaletteSize - 2; ++j) {
                uint8_t error = abs_diff(pixels[i], palette[j]);
                if (error < bestError) {
                    bestError = error;
                    idx = j;
                }
            }
        }

        indices <<= 3;
        indices |= idx;
    }

    return
        SkEndian_SwapLE64(
            static_cast<uint64_t>(minVal) |
            (static_cast<uint64_t>(maxVal) << 8) |
            (indices << 16));        
}


// Compress LATC block. Each 4x4 block of pixels is decompressed by LATC from two
// values LUM0 and LUM1, and an index into the generated palette. Details of how
// the palette is generated can be found in the comments of generatePalette above.
//
// We choose which palette type to use based on whether or not 'pixels' contains
// any extremal values (0 or 255). If there are extremal values, then we use the
// palette that has the extremal values built in. Otherwise, we use the full bounding
// box.

static uint64_t compress_block(const uint8_t pixels[]) {
    // Collect unique pixels
    int nUniquePixels = 0;
    uint8_t uniquePixels[kPixelsPerBlock];
    for (int i = 0; i < kPixelsPerBlock; ++i) {
        bool foundPixel = false;
        for (int j = 0; j < nUniquePixels; ++j) {
            foundPixel = foundPixel || uniquePixels[j] == pixels[i];
        }

        if (!foundPixel) {
            uniquePixels[nUniquePixels] = pixels[i];
            ++nUniquePixels;
        }
    }

    // If there's only one unique pixel, then our compression is easy.
    if (1 == nUniquePixels) {
        return SkEndian_SwapLE64(pixels[0] | (pixels[0] << 8));

    // Similarly, if there are only two unique pixels, then our compression is
    // easy again: place the pixels in the block header, and assign the indices
    // with one or zero depending on which pixel they belong to.
    } else if (2 == nUniquePixels) {
        uint64_t outBlock = 0;
        for (int i = kPixelsPerBlock - 1; i >= 0; --i) {
            int idx = 0;
            if (pixels[i] == uniquePixels[1]) {
                idx = 1;
            }

            outBlock <<= 3;
            outBlock |= idx;
        }
        outBlock <<= 16;
        outBlock |= (uniquePixels[0] | (uniquePixels[1] << 8));
        return SkEndian_SwapLE64(outBlock);
    }

    // Count non-maximal pixel values
    int nonExtremalPixels = 0;
    for (int i = 0; i < nUniquePixels; ++i) {
        if (!is_extremal(uniquePixels[i])) {
            ++nonExtremalPixels;
        }
    }

    // If all the pixels are nonmaximal then compute the palette using
    // the bounding box of all the pixels.
    if (nonExtremalPixels == nUniquePixels) {
        // This is really just for correctness, in all of my tests we
        // never take this step. We don't lose too much perf here because
        // most of the processing in this function is worth it for the 
        // 1 == nUniquePixels optimization.
        return compress_block_bb(pixels);
    } else {
        return compress_block_bb_ignore_extremal(pixels);
    }
}

static bool compress_a8_to_latc(uint8_t* dst, const uint8_t* src,
                                int width, int height, int rowBytes) {
    // Make sure that our data is well-formed enough to be
    // considered for LATC compression
    if (0 == width || 0 == height ||
        (width % kLATCBlockSize) != 0 || (height % kLATCBlockSize) != 0) {
        return false;
    }

    int blocksX = width / kLATCBlockSize;
    int blocksY = height / kLATCBlockSize;

    uint8_t block[16];
    uint64_t* encPtr = reinterpret_cast<uint64_t*>(dst);
    for (int y = 0; y < blocksY; ++y) {
        for (int x = 0; x < blocksX; ++x) {
            // Load block
            static const int kBS = kLATCBlockSize;
            for (int k = 0; k < kBS; ++k) {
                memcpy(block + k*kBS, src + k*rowBytes + (kBS * x), kBS);
            }

            // Compress it
            *encPtr = compress_block(block);
            ++encPtr;
        }
        src += kLATCBlockSize * rowBytes;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

static size_t get_compressed_data_size(Format fmt, int width, int height) {
    switch (fmt) {
        case kLATC_Format:
        {
            // The LATC format is 64 bits per 4x4 block.
            static const int kLATCEncodedBlockSize = 8;

            int blocksX = width / kLATCBlockSize;
            int blocksY = height / kLATCBlockSize;

            return blocksX * blocksY * kLATCEncodedBlockSize;
        }

        default:
            SkFAIL("Unknown compressed format!");
            return 0;
    }
}

typedef bool (*CompressBitmapProc)(uint8_t* dst, const uint8_t* src,
                                   int width, int height, int rowBytes);

bool CompressBufferToFormat(uint8_t* dst, const uint8_t* src, SkColorType srcColorType,
                            int width, int height, int rowBytes, Format format) {

    CompressBitmapProc kProcMap[kFormatCnt][kLastEnum_SkColorType + 1];
    memset(kProcMap, 0, sizeof(kProcMap));

    kProcMap[kLATC_Format][kAlpha_8_SkColorType] = compress_a8_to_latc;
    
    CompressBitmapProc proc = kProcMap[format][srcColorType];
    if (NULL != proc) {
        return proc(dst, src, width, height, rowBytes);
    }

    return false;
}

SkData *CompressBitmapToFormat(const SkBitmap &bitmap, Format format) {
    SkAutoLockPixels alp(bitmap);

    int compressedDataSize = get_compressed_data_size(format, bitmap.width(), bitmap.height());
    const uint8_t* src = reinterpret_cast<const uint8_t*>(bitmap.getPixels());
    uint8_t* dst = reinterpret_cast<uint8_t*>(sk_malloc_throw(compressedDataSize));
    if (CompressBufferToFormat(dst, src, bitmap.colorType(), bitmap.width(), bitmap.height(),
                               bitmap.rowBytes(), format)) {
        return SkData::NewFromMalloc(dst, compressedDataSize);
    }

    sk_free(dst);
    return NULL;
}

}  // namespace SkTextureCompressor
