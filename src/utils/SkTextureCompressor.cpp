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

#include "SkTextureCompression_opts.h"

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

static bool is_extremal(uint8_t pixel) {
    return 0 == pixel || 255 == pixel;
}

typedef uint64_t (*A84x4To64BitProc)(const uint8_t block[]);

// This function is used by both R11 EAC and LATC to compress 4x4 blocks
// of 8-bit alpha into 64-bit values that comprise the compressed data.
// For both formats, we need to make sure that the dimensions of the 
// src pixels are divisible by 4, and copy 4x4 blocks one at a time
// for compression.
static bool compress_4x4_a8_to_64bit(uint8_t* dst, const uint8_t* src,
                                     int width, int height, int rowBytes,
                                     A84x4To64BitProc proc) {
    // Make sure that our data is well-formed enough to be considered for compression
    if (0 == width || 0 == height || (width % 4) != 0 || (height % 4) != 0) {
        return false;
    }

    int blocksX = width >> 2;
    int blocksY = height >> 2;

    uint8_t block[16];
    uint64_t* encPtr = reinterpret_cast<uint64_t*>(dst);
    for (int y = 0; y < blocksY; ++y) {
        for (int x = 0; x < blocksX; ++x) {
            // Load block
            for (int k = 0; k < 4; ++k) {
                memcpy(block + k*4, src + k*rowBytes + 4*x, 4);
            }

            // Compress it
            *encPtr = proc(block);
            ++encPtr;
        }
        src += 4 * rowBytes;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// LATC compressor
//
////////////////////////////////////////////////////////////////////////////////

// LATC compressed texels down into square 4x4 blocks
static const int kLATCPaletteSize = 8;
static const int kLATCBlockSize = 4;
static const int kLATCPixelsPerBlock = kLATCBlockSize * kLATCBlockSize;

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

static void generate_latc_palette(uint8_t palette[], uint8_t lum0, uint8_t lum1) {
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

// Compress a block by using the bounding box of the pixels. It is assumed that
// there are no extremal pixels in this block otherwise we would have used
// compressBlockBBIgnoreExtremal.
static uint64_t compress_latc_block_bb(const uint8_t pixels[]) {
    uint8_t minVal = 255;
    uint8_t maxVal = 0;
    for (int i = 0; i < kLATCPixelsPerBlock; ++i) {
        minVal = SkTMin(pixels[i], minVal);
        maxVal = SkTMax(pixels[i], maxVal);
    }

    SkASSERT(!is_extremal(minVal));
    SkASSERT(!is_extremal(maxVal));

    uint8_t palette[kLATCPaletteSize];
    generate_latc_palette(palette, maxVal, minVal);

    uint64_t indices = 0;
    for (int i = kLATCPixelsPerBlock - 1; i >= 0; --i) {

        // Find the best palette index
        uint8_t bestError = abs_diff(pixels[i], palette[0]);
        uint8_t idx = 0;
        for (int j = 1; j < kLATCPaletteSize; ++j) {
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
static uint64_t compress_latc_block_bb_ignore_extremal(const uint8_t pixels[]) {
    uint8_t minVal = 255;
    uint8_t maxVal = 0;
    for (int i = 0; i < kLATCPixelsPerBlock; ++i) {
        if (is_extremal(pixels[i])) {
            continue;
        }

        minVal = SkTMin(pixels[i], minVal);
        maxVal = SkTMax(pixels[i], maxVal);
    }

    SkASSERT(!is_extremal(minVal));
    SkASSERT(!is_extremal(maxVal));

    uint8_t palette[kLATCPaletteSize];
    generate_latc_palette(palette, minVal, maxVal);

    uint64_t indices = 0;
    for (int i = kLATCPixelsPerBlock - 1; i >= 0; --i) {

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
            for (int j = 1; j < kLATCPaletteSize - 2; ++j) {
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

static uint64_t compress_latc_block(const uint8_t pixels[]) {
    // Collect unique pixels
    int nUniquePixels = 0;
    uint8_t uniquePixels[kLATCPixelsPerBlock];
    for (int i = 0; i < kLATCPixelsPerBlock; ++i) {
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
        for (int i = kLATCPixelsPerBlock - 1; i >= 0; --i) {
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
        return compress_latc_block_bb(pixels);
    } else {
        return compress_latc_block_bb_ignore_extremal(pixels);
    }
}

static inline bool compress_a8_to_latc(uint8_t* dst, const uint8_t* src,
                                       int width, int height, int rowBytes) {
    return compress_4x4_a8_to_64bit(dst, src, width, height, rowBytes, compress_latc_block);
}

////////////////////////////////////////////////////////////////////////////////
//
// R11 EAC Compressor
//
////////////////////////////////////////////////////////////////////////////////

// #define COMPRESS_R11_EAC_SLOW 1
// #define COMPRESS_R11_EAC_FAST 1
#define COMPRESS_R11_EAC_FASTEST 1

// Blocks compressed into R11 EAC are represented as follows:
// 0000000000000000000000000000000000000000000000000000000000000000
// |base_cw|mod|mul|  ----------------- indices -------------------
//
// To reconstruct the value of a given pixel, we use the formula:
// clamp[0, 2047](base_cw * 8 + 4 + mod_val*mul*8)
//
// mod_val is chosen from a palette of values based on the index of the
// given pixel. The palette is chosen by the value stored in mod.
// This formula returns a value between 0 and 2047, which is converted
// to a float from 0 to 1 in OpenGL.
//
// If mul is zero, then we set mul = 1/8, so that the formula becomes
// clamp[0, 2047](base_cw * 8 + 4 + mod_val)

#if COMPRESS_R11_EAC_SLOW

static const int kNumR11EACPalettes = 16;
static const int kR11EACPaletteSize = 8;
static const int kR11EACModifierPalettes[kNumR11EACPalettes][kR11EACPaletteSize] = {
    {-3, -6, -9, -15, 2, 5, 8, 14},
    {-3, -7, -10, -13, 2, 6, 9, 12},
    {-2, -5, -8, -13, 1, 4, 7, 12},
    {-2, -4, -6, -13, 1, 3, 5, 12},
    {-3, -6, -8, -12, 2, 5, 7, 11},
    {-3, -7, -9, -11, 2, 6, 8, 10},
    {-4, -7, -8, -11, 3, 6, 7, 10},
    {-3, -5, -8, -11, 2, 4, 7, 10},
    {-2, -6, -8, -10, 1, 5, 7, 9},
    {-2, -5, -8, -10, 1, 4, 7, 9},
    {-2, -4, -8, -10, 1, 3, 7, 9},
    {-2, -5, -7, -10, 1, 4, 6, 9},
    {-3, -4, -7, -10, 2, 3, 6, 9},
    {-1, -2, -3, -10, 0, 1, 2, 9},
    {-4, -6, -8, -9, 3, 5, 7, 8},
    {-3, -5, -7, -9, 2, 4, 6, 8}
};

// Pack the base codeword, palette, and multiplier into the 64 bits necessary
// to decode it.
static uint64_t pack_r11eac_block(uint16_t base_cw, uint16_t palette, uint16_t multiplier,
                                  uint64_t indices) {
    SkASSERT(palette < 16);
    SkASSERT(multiplier < 16);
    SkASSERT(indices < (static_cast<uint64_t>(1) << 48));

    const uint64_t b = static_cast<uint64_t>(base_cw) << 56;
    const uint64_t m = static_cast<uint64_t>(multiplier) << 52;
    const uint64_t p = static_cast<uint64_t>(palette) << 48;
    return SkEndian_SwapBE64(b | m | p | indices);
}

// Given a base codeword, a modifier, and a multiplier, compute the proper
// pixel value in the range [0, 2047].
static uint16_t compute_r11eac_pixel(int base_cw, int modifier, int multiplier) {
    int ret = (base_cw * 8 + 4) + (modifier * multiplier * 8);
    return (ret > 2047)? 2047 : ((ret < 0)? 0 : ret);
}

// Compress a block into R11 EAC format.
// The compression works as follows:
// 1. Find the center of the span of the block's values. Use this as the base codeword.
// 2. Choose a multiplier based roughly on the size of the span of block values
// 3. Iterate through each palette and choose the one with the most accurate
// modifiers.
static inline uint64_t compress_heterogeneous_r11eac_block(const uint8_t block[16]) {
    // Find the center of the data...
    uint16_t bmin = block[0];
    uint16_t bmax = block[0];
    for (int i = 1; i < 16; ++i) {
        bmin = SkTMin<uint16_t>(bmin, block[i]);
        bmax = SkTMax<uint16_t>(bmax, block[i]);
    }

    uint16_t center = (bmax + bmin) >> 1;
    SkASSERT(center <= 255);

    // Based on the min and max, we can guesstimate a proper multiplier
    // This is kind of a magic choice to start with.
    uint16_t multiplier = (bmax - center) / 10;

    // Now convert the block to 11 bits and transpose it to match
    // the proper layout
    uint16_t cblock[16];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int srcIdx = i*4+j;
            int dstIdx = j*4+i;
            cblock[dstIdx] = (block[srcIdx] << 3) | (block[srcIdx] >> 5);
        }
    }

    // Finally, choose the proper palette and indices
    uint32_t bestError = 0xFFFFFFFF;
    uint64_t bestIndices = 0;
    uint16_t bestPalette = 0;
    for (uint16_t paletteIdx = 0; paletteIdx < kNumR11EACPalettes; ++paletteIdx) {
        const int *palette = kR11EACModifierPalettes[paletteIdx];

        // Iterate through each pixel to find the best palette index
        // and update the indices with the choice. Also store the error
        // for this palette to be compared against the best error...
        uint32_t error = 0;
        uint64_t indices = 0;
        for (int pixelIdx = 0; pixelIdx < 16; ++pixelIdx) {
            const uint16_t pixel = cblock[pixelIdx];

            // Iterate through each palette value to find the best index
            // for this particular pixel for this particular palette.
            uint16_t bestPixelError =
                abs_diff(pixel, compute_r11eac_pixel(center, palette[0], multiplier));
            int bestIndex = 0;
            for (int i = 1; i < kR11EACPaletteSize; ++i) {
                const uint16_t p = compute_r11eac_pixel(center, palette[i], multiplier);
                const uint16_t perror = abs_diff(pixel, p);

                // Is this index better?
                if (perror < bestPixelError) {
                    bestIndex = i;
                    bestPixelError = perror;
                }
            }

            SkASSERT(bestIndex < 8);

            error += bestPixelError;
            indices <<= 3;
            indices |= bestIndex;
        }

        SkASSERT(indices < (static_cast<uint64_t>(1) << 48));

        // Is this palette better?
        if (error < bestError) {
            bestPalette = paletteIdx;
            bestIndices = indices;
            bestError = error;
        }
    }

    // Finally, pack everything together...
    return pack_r11eac_block(center, bestPalette, multiplier, bestIndices);
}
#endif // COMPRESS_R11_EAC_SLOW

#if COMPRESS_R11_EAC_FAST
// This function takes into account that most blocks that we compress have a gradation from
// fully opaque to fully transparent. The compression scheme works by selecting the
// palette and multiplier that has the tightest fit to the 0-255 range. This is encoded
// as the block header (0x8490). The indices are then selected by considering the top
// three bits of each alpha value. For alpha masks, this reduces the dynamic range from
// 17 to 8, but the quality is still acceptable.
//
// There are a few caveats that need to be taken care of...
//
// 1. The block is read in as scanlines, so the indices are stored as:
//     0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
//    However, the decomrpession routine reads them in column-major order, so they
//    need to be packed as:
//     0 4 8 12 1 5 9 13 2 6 10 14 3 7 11 15
//    So when reading, they must be transposed.
//
// 2. We cannot use the top three bits as an index directly, since the R11 EAC palettes
//    above store the modulation values first decreasing and then increasing:
//      e.g. {-3, -6, -9, -15, 2, 5, 8, 14}
//    Hence, we need to convert the indices with the following mapping:
//      From: 0 1 2 3 4 5 6 7
//      To:   3 2 1 0 4 5 6 7
static inline uint64_t compress_heterogeneous_r11eac_block(const uint8_t block[16]) {
    uint64_t retVal = static_cast<uint64_t>(0x8490) << 48;
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            const int shift = 45-3*(j*4+i);
            SkASSERT(shift <= 45);
            const uint64_t idx = block[i*4+j] >> 5;
            SkASSERT(idx < 8);

            // !SPEED! This is slightly faster than having an if-statement.
            switch(idx) {
                case 0:
                case 1:
                case 2:
                case 3:
                    retVal |= (3-idx) << shift;
                    break;
                default:
                    retVal |= idx << shift;
                    break;
            }
        }
    }

    return SkEndian_SwapBE64(retVal);
}
#endif // COMPRESS_R11_EAC_FAST

#if (COMPRESS_R11_EAC_SLOW) || (COMPRESS_R11_EAC_FAST)
static uint64_t compress_r11eac_block(const uint8_t block[16]) {
    // Are all blocks a solid color?
    bool solid = true;
    for (int i = 1; i < 16; ++i) {
        if (block[i] != block[0]) {
            solid = false;
            break;
        }
    }

    if (solid) {
        switch(block[0]) {
            // Fully transparent? We know the encoding...
            case 0:
                // (0x0020 << 48) produces the following:
                // basw_cw: 0
                // mod: 0, palette: {-3, -6, -9, -15, 2, 5, 8, 14}
                // multiplier: 2
                // mod_val: -3
                //
                // this gives the following formula:
                // clamp[0, 2047](0*8+4+(-3)*2*8) = 0
                // 
                // Furthermore, it is impervious to endianness:
                // 0x0020000000002000ULL
                // Will produce one pixel with index 2, which gives:
                // clamp[0, 2047](0*8+4+(-9)*2*8) = 0
                return 0x0020000000002000ULL;

            // Fully opaque? We know this encoding too...
            case 255:
            
                // -1 produces the following:
                // basw_cw: 255
                // mod: 15, palette: {-3, -5, -7, -9, 2, 4, 6, 8}
                // mod_val: 8
                //
                // this gives the following formula:
                // clamp[0, 2047](255*8+4+8*8*8) = clamp[0, 2047](2556) = 2047
                return 0xFFFFFFFFFFFFFFFFULL;

            default:
                // !TODO! krajcevski:
                // This will probably never happen, since we're using this format
                // primarily for compressing alpha maps. Usually the only
                // non-fullly opaque or fully transparent blocks are not a solid
                // intermediate color. If we notice that they are, then we can
                // add another optimization...
                break;
        }
    }

    return compress_heterogeneous_r11eac_block(block);
}
#endif  // (COMPRESS_R11_EAC_SLOW) || (COMPRESS_R11_EAC_FAST)

#if COMPRESS_R11_EAC_FASTEST
static inline uint64_t interleave6(uint64_t topRows, uint64_t bottomRows) {
    // If our 3-bit block indices are laid out as:
    // a b c d
    // e f g h
    // i j k l
    // m n o p
    //
    // This function expects topRows and bottomRows to contain the first two rows
    // of indices interleaved in the least significant bits of a and b. In other words...
    //
    // If the architecture is big endian, then topRows and bottomRows will contain the following:
    // Bits 31-0:
    // a: 00 a e 00 b f 00 c g 00 d h
    // b: 00 i m 00 j n 00 k o 00 l p
    //
    // If the architecture is little endian, then topRows and bottomRows will contain
    // the following:
    // Bits 31-0:
    // a: 00 d h 00 c g 00 b f 00 a e
    // b: 00 l p 00 k o 00 j n 00 i m
    //
    // This function returns a 48-bit packing of the form:
    // a e i m b f j n c g k o d h l p
    //
    // !SPEED! this function might be even faster if certain SIMD intrinsics are
    // used..

    // For both architectures, we can figure out a packing of the bits by
    // using a shuffle and a few shift-rotates...
    uint64_t x = (static_cast<uint64_t>(topRows) << 32) | static_cast<uint64_t>(bottomRows);

    // x: 00 a e 00 b f 00 c g 00 d h 00 i m 00 j n 00 k o 00 l p

    uint64_t t = (x ^ (x >> 10)) & 0x3FC0003FC00000ULL;
    x = x ^ t ^ (t << 10);

    // x: b f 00 00 00 a e c g i m 00 00 00 d h j n 00 k o 00 l p

    x = (x | ((x << 52) & (0x3FULL << 52)) | ((x << 20) & (0x3FULL << 28))) >> 16;

    // x: 00 00 00 00 00 00 00 00 b f l p a e c g i m k o d h j n

    t = (x ^ (x >> 6)) & 0xFC0000ULL;
    x = x ^ t ^ (t << 6);

#if defined (SK_CPU_BENDIAN)
    // x: 00 00 00 00 00 00 00 00 b f l p a e i m c g k o d h j n

    t = (x ^ (x >> 36)) & 0x3FULL;
    x = x ^ t ^ (t << 36);

    // x: 00 00 00 00 00 00 00 00 b f j n a e i m c g k o d h l p

    t = (x ^ (x >> 12)) & 0xFFF000000ULL;
    x = x ^ t ^ (t << 12);

    // x: 00 00 00 00 00 00 00 00 a e i m b f j n c g k o d h l p
    return x;
#else
    // If our CPU is little endian, then the above logic will
    // produce the following indices:
    // x: 00 00 00 00 00 00 00 00 c g i m d h l p b f j n a e k o

    t = (x ^ (x >> 36)) & 0xFC0ULL;
    x = x ^ t ^ (t << 36);

    // x: 00 00 00 00 00 00 00 00 a e i m d h l p b f j n c g k o
    
    x = (x & (0xFFFULL << 36)) | ((x & 0xFFFFFFULL) << 12) | ((x >> 24) & 0xFFFULL);

    // x: 00 00 00 00 00 00 00 00 a e i m b f j n c g k o d h l p
    
    return x;
#endif
}

// This function converts an integer containing four bytes of alpha
// values into an integer containing four bytes of indices into R11 EAC.
// Note, there needs to be a mapping of indices:
// 0 1 2 3 4 5 6 7
// 3 2 1 0 4 5 6 7
//
// To compute this, we first negate each byte, and then add three, which
// gives the mapping
// 3 2 1 0 -1 -2 -3 -4
//
// Then we mask out the negative values, take their absolute value, and
// add three.
//
// Most of the voodoo in this function comes from Hacker's Delight, section 2-18
static inline uint32_t convert_indices(uint32_t x) {
    // Take the top three bits...
    x = (x & 0xE0E0E0E0) >> 5;

    // Negate...
    x = ~((0x80808080 - x) ^ 0x7F7F7F7F);

    // Add three
    const uint32_t s = (x & 0x7F7F7F7F) + 0x03030303;
    x = ((x ^ 0x03030303) & 0x80808080) ^ s;

    // Absolute value
    const uint32_t a = x & 0x80808080;
    const uint32_t b = a >> 7;

    // Aside: mask negatives (m is three if the byte was negative)
    const uint32_t m = (a >> 6) | b;

    // .. continue absolute value
    x = (x ^ ((a - b) | a)) + b;

    // Add three
    return x + m;
}

// This function follows the same basic procedure as compress_heterogeneous_r11eac_block
// above when COMPRESS_R11_EAC_FAST is defined, but it avoids a few loads/stores and
// tries to optimize where it can using SIMD.
static uint64_t compress_r11eac_block_fast(const uint8_t* src, int rowBytes) {
    // Store each row of alpha values in an integer
    const uint32_t alphaRow1 = *(reinterpret_cast<const uint32_t*>(src));
    const uint32_t alphaRow2 = *(reinterpret_cast<const uint32_t*>(src + rowBytes));
    const uint32_t alphaRow3 = *(reinterpret_cast<const uint32_t*>(src + 2*rowBytes));
    const uint32_t alphaRow4 = *(reinterpret_cast<const uint32_t*>(src + 3*rowBytes));

    // Check for solid blocks. The explanations for these values
    // can be found in the comments of compress_r11eac_block above
    if (alphaRow1 == alphaRow2 && alphaRow1 == alphaRow3 && alphaRow1 == alphaRow4) {
        if (0 == alphaRow1) {
            // Fully transparent block
            return 0x0020000000002000ULL;
        } else if (0xFFFFFFFF == alphaRow1) {
            // Fully opaque block
            return 0xFFFFFFFFFFFFFFFFULL;
        }
    }

    // Convert each integer of alpha values into an integer of indices
    const uint32_t indexRow1 = convert_indices(alphaRow1);
    const uint32_t indexRow2 = convert_indices(alphaRow2);
    const uint32_t indexRow3 = convert_indices(alphaRow3);
    const uint32_t indexRow4 = convert_indices(alphaRow4);

    // Interleave the indices from the top two rows and bottom two rows
    // prior to passing them to interleave6. Since each index is at most
    // three bits, then each byte can hold two indices... The way that the
    // compression scheme expects the packing allows us to efficiently pack
    // the top two rows and bottom two rows. Interleaving each 6-bit sequence
    // and tightly packing it into a uint64_t is a little trickier, which is
    // taken care of in interleave6.
    const uint32_t r1r2 = (indexRow1 << 3) | indexRow2;
    const uint32_t r3r4 = (indexRow3 << 3) | indexRow4;
    const uint64_t indices = interleave6(r1r2, r3r4);

    // Return the packed incdices in the least significant bits with the magic header
    return SkEndian_SwapBE64(0x8490000000000000ULL | indices);
}

static bool compress_a8_to_r11eac_fast(uint8_t* dst, const uint8_t* src,
                                       int width, int height, int rowBytes) {
    // Make sure that our data is well-formed enough to be considered for compression
    if (0 == width || 0 == height || (width % 4) != 0 || (height % 4) != 0) {
        return false;
    }

    const int blocksX = width >> 2;
    const int blocksY = height >> 2;

    uint64_t* encPtr = reinterpret_cast<uint64_t*>(dst);
    for (int y = 0; y < blocksY; ++y) {
        for (int x = 0; x < blocksX; ++x) {
            // Compress it
            *encPtr = compress_r11eac_block_fast(src + 4*x, rowBytes);
            ++encPtr;
        }
        src += 4 * rowBytes;
    }
    return true;
}
#endif // COMPRESS_R11_EAC_FASTEST

static inline bool compress_a8_to_r11eac(uint8_t* dst, const uint8_t* src,
                                         int width, int height, int rowBytes) {
#if (COMPRESS_R11_EAC_SLOW) || (COMPRESS_R11_EAC_FAST)
    return compress_4x4_a8_to_64bit(dst, src, width, height, rowBytes, compress_r11eac_block);
#elif COMPRESS_R11_EAC_FASTEST
    return compress_a8_to_r11eac_fast(dst, src, width, height, rowBytes);
#else
#error "Must choose R11 EAC algorithm"
#endif
}

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

static inline size_t get_compressed_data_size(Format fmt, int width, int height) {
    switch (fmt) {
        // These formats are 64 bits per 4x4 block.
        case kR11_EAC_Format:
        case kLATC_Format:
        {
            static const int kLATCEncodedBlockSize = 8;

            const int blocksX = width / kLATCBlockSize;
            const int blocksY = height / kLATCBlockSize;

            return blocksX * blocksY * kLATCEncodedBlockSize;
        }

        default:
            SkFAIL("Unknown compressed format!");
            return 0;
    }
}

bool CompressBufferToFormat(uint8_t* dst, const uint8_t* src, SkColorType srcColorType,
                            int width, int height, int rowBytes, Format format, bool opt) {
    CompressionProc proc = NULL;
    if (opt) {
        proc = SkTextureCompressorGetPlatformProc(srcColorType, format);
    }

    if (NULL == proc) {
        switch (srcColorType) {
            case kAlpha_8_SkColorType:
            {
                switch (format) {
                    case kLATC_Format:
                        proc = compress_a8_to_latc;
                        break;
                    case kR11_EAC_Format:
                        proc = compress_a8_to_r11eac;
                        break;
                    default:
                        // Do nothing...
                        break;
                }
            }
            break;

            default:
                // Do nothing...
                break;
        }
    }

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
