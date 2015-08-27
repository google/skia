/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor.h"
#include "SkTextureCompressor_Blitter.h"
#include "SkTextureCompressor_Utils.h"

#include "SkBlitter.h"
#include "SkEndian.h"

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

#if COMPRESS_R11_EAC_SLOW

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

// This function is used by R11 EAC to compress 4x4 blocks
// of 8-bit alpha into 64-bit values that comprise the compressed data.
// We need to make sure that the dimensions of the src pixels are divisible
// by 4, and copy 4x4 blocks one at a time for compression.
typedef uint64_t (*A84x4To64BitProc)(const uint8_t block[]);

static bool compress_4x4_a8_to_64bit(uint8_t* dst, const uint8_t* src,
                                     int width, int height, size_t rowBytes,
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
#endif  // (COMPRESS_R11_EAC_SLOW) || (COMPRESS_R11_EAC_FAST)

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
    x = SkTextureCompressor::ConvertToThreeBitIndex(x);

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

#if COMPRESS_R11_EAC_FASTEST
template<unsigned shift>
static inline uint64_t swap_shift(uint64_t x, uint64_t mask) {
    const uint64_t t = (x ^ (x >> shift)) & mask;
    return x ^ t ^ (t << shift);
}

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

    x = swap_shift<10>(x, 0x3FC0003FC00000ULL);

    // x: b f 00 00 00 a e c g i m 00 00 00 d h j n 00 k o 00 l p

    x = (x | ((x << 52) & (0x3FULL << 52)) | ((x << 20) & (0x3FULL << 28))) >> 16;

    // x: 00 00 00 00 00 00 00 00 b f l p a e c g i m k o d h j n

    x = swap_shift<6>(x, 0xFC0000ULL);

#if defined (SK_CPU_BENDIAN)
    // x: 00 00 00 00 00 00 00 00 b f l p a e i m c g k o d h j n

    x = swap_shift<36>(x, 0x3FULL);

    // x: 00 00 00 00 00 00 00 00 b f j n a e i m c g k o d h l p

    x = swap_shift<12>(x, 0xFFF000000ULL);
#else
    // If our CPU is little endian, then the above logic will
    // produce the following indices:
    // x: 00 00 00 00 00 00 00 00 c g i m d h l p b f j n a e k o

    x = swap_shift<36>(x, 0xFC0ULL);

    // x: 00 00 00 00 00 00 00 00 a e i m d h l p b f j n c g k o
    
    x = (x & (0xFFFULL << 36)) | ((x & 0xFFFFFFULL) << 12) | ((x >> 24) & 0xFFFULL);
#endif

    // x: 00 00 00 00 00 00 00 00 a e i m b f j n c g k o d h l p
    return x;
}

// This function follows the same basic procedure as compress_heterogeneous_r11eac_block
// above when COMPRESS_R11_EAC_FAST is defined, but it avoids a few loads/stores and
// tries to optimize where it can using SIMD.
static uint64_t compress_r11eac_block_fast(const uint8_t* src, size_t rowBytes) {
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
                                       int width, int height, size_t rowBytes) {
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

////////////////////////////////////////////////////////////////////////////////
//
// Utility functions used by the blitter
//
////////////////////////////////////////////////////////////////////////////////

// The R11 EAC format expects that indices are given in column-major order. Since
// we receive alpha values in raster order, this usually means that we have to use
// pack6 above to properly pack our indices. However, if our indices come from the
// blitter, then each integer will be a column of indices, and hence can be efficiently
// packed. This function takes the bottom three bits of each byte and places them in
// the least significant 12 bits of the resulting integer.
static inline uint32_t pack_indices_vertical(uint32_t x) {
#if defined (SK_CPU_BENDIAN)
    return 
        (x & 7) |
        ((x >> 5) & (7 << 3)) |
        ((x >> 10) & (7 << 6)) |
        ((x >> 15) & (7 << 9));
#else
    return 
        ((x >> 24) & 7) |
        ((x >> 13) & (7 << 3)) |
        ((x >> 2) & (7 << 6)) |
        ((x << 9) & (7 << 9));
#endif
}

// This function returns the compressed format of a block given as four columns of
// alpha values. Each column is assumed to be loaded from top to bottom, and hence
// must first be converted to indices and then packed into the resulting 64-bit
// integer.
inline void compress_block_vertical(uint8_t* dstPtr, const uint8_t *block) {

    const uint32_t* src = reinterpret_cast<const uint32_t*>(block);
    uint64_t* dst = reinterpret_cast<uint64_t*>(dstPtr);

    const uint32_t alphaColumn0 = src[0];
    const uint32_t alphaColumn1 = src[1];
    const uint32_t alphaColumn2 = src[2];
    const uint32_t alphaColumn3 = src[3];

    if (alphaColumn0 == alphaColumn1 &&
        alphaColumn2 == alphaColumn3 &&
        alphaColumn0 == alphaColumn2) {

        if (0 == alphaColumn0) {
            // Transparent
            *dst = 0x0020000000002000ULL;
            return;
        }
        else if (0xFFFFFFFF == alphaColumn0) {
            // Opaque
            *dst = 0xFFFFFFFFFFFFFFFFULL;
            return;
        }
    }

    const uint32_t indexColumn0 = convert_indices(alphaColumn0);
    const uint32_t indexColumn1 = convert_indices(alphaColumn1);
    const uint32_t indexColumn2 = convert_indices(alphaColumn2);
    const uint32_t indexColumn3 = convert_indices(alphaColumn3);

    const uint32_t packedIndexColumn0 = pack_indices_vertical(indexColumn0);
    const uint32_t packedIndexColumn1 = pack_indices_vertical(indexColumn1);
    const uint32_t packedIndexColumn2 = pack_indices_vertical(indexColumn2);
    const uint32_t packedIndexColumn3 = pack_indices_vertical(indexColumn3);

    *dst = SkEndian_SwapBE64(0x8490000000000000ULL |
                             (static_cast<uint64_t>(packedIndexColumn0) << 36) |
                             (static_cast<uint64_t>(packedIndexColumn1) << 24) |
                             static_cast<uint64_t>(packedIndexColumn2 << 12) |
                             static_cast<uint64_t>(packedIndexColumn3));
}

static inline int get_r11_eac_index(uint64_t block, int x, int y) {
    SkASSERT(x >= 0 && x < 4);
    SkASSERT(y >= 0 && y < 4);
    const int idx = x*4 + y;
    return (block >> ((15-idx)*3)) & 0x7;
}

static void decompress_r11_eac_block(uint8_t* dst, int dstRowBytes, const uint8_t* src) {
    const uint64_t block = SkEndian_SwapBE64(*(reinterpret_cast<const uint64_t *>(src)));

    const int base_cw = (block >> 56) & 0xFF;
    const int mod = (block >> 52) & 0xF;
    const int palette_idx = (block >> 48) & 0xF;

    const int* palette = kR11EACModifierPalettes[palette_idx];

    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            const int idx = get_r11_eac_index(block, i, j);
            const int val = base_cw*8 + 4 + palette[idx]*mod*8;
            if (val < 0) {
                dst[i] = 0;
            } else if (val > 2047) {
                dst[i] = 0xFF;
            } else {
                dst[i] = (val >> 3) & 0xFF;
            }
        }
        dst += dstRowBytes;
    }
}

// This is the type passed as the CompressorType argument of the compressed
// blitter for the R11 EAC format. The static functions required to be in this
// struct are documented in SkTextureCompressor_Blitter.h
struct CompressorR11EAC {
    static inline void CompressA8Vertical(uint8_t* dst, const uint8_t* src) {
        compress_block_vertical(dst, src);
    }

    static inline void CompressA8Horizontal(uint8_t* dst, const uint8_t* src,
                                            int srcRowBytes) {
        *(reinterpret_cast<uint64_t*>(dst)) = compress_r11eac_block_fast(src, srcRowBytes);
    }

#if PEDANTIC_BLIT_RECT
    static inline void UpdateBlock(uint8_t* dst, const uint8_t* src, int srcRowBytes,
                                   const uint8_t* mask) {
        // TODO: krajcevski
        // The implementation of this function should be similar to that of LATC, since
        // the R11EAC indices directly correspond to pixel values.
        SkFAIL("Implement me!");
    }
#endif
};

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

bool CompressA8ToR11EAC(uint8_t* dst, const uint8_t* src, int width, int height, size_t rowBytes) {

#if (COMPRESS_R11_EAC_SLOW) || (COMPRESS_R11_EAC_FAST)

    return compress_4x4_a8_to_64bit(dst, src, width, height, rowBytes, compress_r11eac_block);

#elif COMPRESS_R11_EAC_FASTEST

    return compress_a8_to_r11eac_fast(dst, src, width, height, rowBytes);

#else
#error "Must choose R11 EAC algorithm"
#endif
}

SkBlitter* CreateR11EACBlitter(int width, int height, void* outputBuffer,
                               SkTBlitterAllocator* allocator) {

    if ((width % 4) != 0 || (height % 4) != 0) {
        return nullptr;
    }

    // Memset the output buffer to an encoding that decodes to zero. We must do this
    // in order to avoid having uninitialized values in the buffer if the blitter
    // decides not to write certain scanlines (and skip entire rows of blocks).
    // In the case of R11, we use the encoding from recognizing all zero pixels from above.
    const int nBlocks = (width * height / 16);  // 4x4 pixel blocks.
    uint64_t *dst = reinterpret_cast<uint64_t *>(outputBuffer);
    for (int i = 0; i < nBlocks; ++i) {
        *dst = 0x0020000000002000ULL;
        ++dst;
    }

    return allocator->createT<
        SkTCompressedAlphaBlitter<4, 8, CompressorR11EAC>, int, int, void*>
        (width, height, outputBuffer);
}

void DecompressR11EAC(uint8_t* dst, int dstRowBytes, const uint8_t* src, int width, int height) {
    for (int j = 0; j < height; j += 4) {
        for (int i = 0; i < width; i += 4) {
            decompress_r11_eac_block(dst + i, dstRowBytes, src);
            src += 8;
        }
        dst += 4 * dstRowBytes;
    }    
}

}  // namespace SkTextureCompressor
