/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor.h"

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

// This function is used by R11 EAC to compress 4x4 blocks
// of 8-bit alpha into 64-bit values that comprise the compressed data.
// We need to make sure that the dimensions of the src pixels are divisible
// by 4, and copy 4x4 blocks one at a time for compression.
typedef uint64_t (*A84x4To64BitProc)(const uint8_t block[]);

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
#endif  // (COMPRESS_R11_EAC_SLOW) || (COMPRESS_R11_EAC_FAST)

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
static inline uint64_t compress_block_vertical(const uint32_t alphaColumn0,
                                               const uint32_t alphaColumn1,
                                               const uint32_t alphaColumn2,
                                               const uint32_t alphaColumn3) {

    if (alphaColumn0 == alphaColumn1 &&
        alphaColumn2 == alphaColumn3 &&
        alphaColumn0 == alphaColumn2) {

        if (0 == alphaColumn0) {
            // Transparent
            return 0x0020000000002000ULL;
        }
        else if (0xFFFFFFFF == alphaColumn0) {
            // Opaque
            return 0xFFFFFFFFFFFFFFFFULL;
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

    return SkEndian_SwapBE64(0x8490000000000000ULL |
                             (static_cast<uint64_t>(packedIndexColumn0) << 36) |
                             (static_cast<uint64_t>(packedIndexColumn1) << 24) |
                             static_cast<uint64_t>(packedIndexColumn2 << 12) |
                             static_cast<uint64_t>(packedIndexColumn3));
        
}

// Updates the block whose columns are stored in blockColN. curAlphai is expected
// to store, as an integer, the four alpha values that will be placed within each
// of the columns in the range [col, col+colsLeft).
static inline void update_block_columns(uint32_t* block, const int col,
                                        const int colsLeft, const uint32_t curAlphai) {
    SkASSERT(NULL != block);
    SkASSERT(col + colsLeft <= 4);

    for (int i = col; i < (col + colsLeft); ++i) {
        block[i] = curAlphai;
    }
}

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

bool CompressA8ToR11EAC(uint8_t* dst, const uint8_t* src, int width, int height, int rowBytes) {

#if (COMPRESS_R11_EAC_SLOW) || (COMPRESS_R11_EAC_FAST)

    return compress_4x4_a8_to_64bit(dst, src, width, height, rowBytes, compress_r11eac_block);

#elif COMPRESS_R11_EAC_FASTEST

    return compress_a8_to_r11eac_fast(dst, src, width, height, rowBytes);

#else
#error "Must choose R11 EAC algorithm"
#endif
}

// This class implements a blitter that blits directly into a buffer that will
// be used as an R11 EAC compressed texture. We compute this buffer by
// buffering four scan lines and then outputting them all at once. This blitter
// is only expected to be used with alpha masks, i.e. kAlpha8_SkColorType.
class R11_EACBlitter : public SkBlitter {
public:
    R11_EACBlitter(int width, int height, void *compressedBuffer);
    virtual ~R11_EACBlitter() { this->flushRuns(); }

    // Blit a horizontal run of one or more pixels.
    virtual void blitH(int x, int y, int width) SK_OVERRIDE {
        // This function is intended to be called from any standard RGB
        // buffer, so we should never encounter it. However, if some code
        // path does end up here, then this needs to be investigated.
        SkFAIL("Not implemented!");
    }
    
    // Blit a horizontal run of antialiased pixels; runs[] is a *sparse*
    // zero-terminated run-length encoding of spans of constant alpha values.
    virtual void blitAntiH(int x, int y,
                           const SkAlpha antialias[],
                           const int16_t runs[]) SK_OVERRIDE;
    
    // Blit a vertical run of pixels with a constant alpha value.
    virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE {
        // This function is currently not implemented. It is not explicitly
        // required by the contract, but if at some time a code path runs into
        // this function (which is entirely possible), it needs to be implemented.
        //
        // TODO (krajcevski):
        // This function will be most easily implemented in one of two ways:
        // 1. Buffer each vertical column value and then construct a list
        //    of alpha values and output all of the blocks at once. This only
        //    requires a write to the compressed buffer
        // 2. Replace the indices of each block with the proper indices based
        //    on the alpha value. This requires a read and write of the compressed
        //    buffer, but much less overhead.
        SkFAIL("Not implemented!");
    }

    // Blit a solid rectangle one or more pixels wide.
    virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE {
        // Analogous to blitRow, this function is intended for RGB targets
        // and should never be called by this blitter. Any calls to this function
        // are probably a bug and should be investigated.
        SkFAIL("Not implemented!");
    }

    // Blit a rectangle with one alpha-blended column on the left,
    // width (zero or more) opaque pixels, and one alpha-blended column
    // on the right. The result will always be at least two pixels wide.
    virtual void blitAntiRect(int x, int y, int width, int height,
                              SkAlpha leftAlpha, SkAlpha rightAlpha) SK_OVERRIDE {
        // This function is currently not implemented. It is not explicitly
        // required by the contract, but if at some time a code path runs into
        // this function (which is entirely possible), it needs to be implemented.
        //
        // TODO (krajcevski):
        // This function will be most easily implemented as follows:
        // 1. If width/height are smaller than a block, then update the
        //    indices of the affected blocks.
        // 2. If width/height are larger than a block, then construct a 9-patch
        //    of block encodings that represent the rectangle, and write them
        //    to the compressed buffer as necessary. Whether or not the blocks
        //    are overwritten by zeros or just their indices are updated is up
        //    to debate.
        SkFAIL("Not implemented!");
    }

    // Blit a pattern of pixels defined by a rectangle-clipped mask;
    // typically used for text.
    virtual void blitMask(const SkMask&, const SkIRect& clip) SK_OVERRIDE {
        // This function is currently not implemented. It is not explicitly
        // required by the contract, but if at some time a code path runs into
        // this function (which is entirely possible), it needs to be implemented.
        //
        // TODO (krajcevski):
        // This function will be most easily implemented in the same way as
        // blitAntiRect above.
        SkFAIL("Not implemented!");
    }

    // If the blitter just sets a single value for each pixel, return the
    // bitmap it draws into, and assign value. If not, return NULL and ignore
    // the value parameter.
    virtual const SkBitmap* justAnOpaqueColor(uint32_t* value) SK_OVERRIDE {
        return NULL;
    }

    /**
     * Compressed texture blitters only really work correctly if they get
     * four blocks at a time. That being said, this blitter tries it's best
     * to preserve semantics if blitAntiH doesn't get called in too many
     * weird ways...
     */
    virtual int requestRowsPreserved() const { return kR11_EACBlockSz; }

protected:
    virtual void onNotifyFinished() { this->flushRuns(); }

private:
    static const int kR11_EACBlockSz = 4;
    static const int kPixelsPerBlock = kR11_EACBlockSz * kR11_EACBlockSz;

    // The longest possible run of pixels that this blitter will receive.
    // This is initialized in the constructor to 0x7FFE, which is one less
    // than the largest positive 16-bit integer. We make sure that it's one
    // less for debugging purposes. We also don't make this variable static
    // in order to make sure that we can construct a valid pointer to it.
    const int16_t kLongestRun;

    // Usually used in conjunction with kLongestRun. This is initialized to
    // zero.
    const SkAlpha kZeroAlpha;

    // This is the information that we buffer whenever we're asked to blit
    // a row with this blitter.
    struct BufferedRun {
        const SkAlpha* fAlphas;
        const int16_t* fRuns;
        int fX, fY;
    } fBufferedRuns[kR11_EACBlockSz];

    // The next row (0-3) that we need to blit. This value should never exceed
    // the number of rows that we have (kR11_EACBlockSz)
    int fNextRun;

    // The width and height of the image that we're blitting
    const int fWidth;
    const int fHeight;

    // The R11 EAC buffer that we're blitting into. It is assumed that the buffer
    // is large enough to store a compressed image of size fWidth*fHeight.
    uint64_t* const fBuffer;

    // Various utility functions
    int blocksWide() const { return fWidth / kR11_EACBlockSz; }
    int blocksTall() const { return fHeight / kR11_EACBlockSz; }
    int totalBlocks() const { return (fWidth * fHeight) / kPixelsPerBlock; }

    // Returns the block index for the block containing pixel (x, y). Block
    // indices start at zero and proceed in raster order.
    int getBlockOffset(int x, int y) const {
        SkASSERT(x < fWidth);
        SkASSERT(y < fHeight);
        const int blockCol = x / kR11_EACBlockSz;
        const int blockRow = y / kR11_EACBlockSz;
        return blockRow * this->blocksWide() + blockCol;
    }

    // Returns a pointer to the block containing pixel (x, y)
    uint64_t *getBlock(int x, int y) const {
        return fBuffer + this->getBlockOffset(x, y);
    }

    // The following function writes the buffered runs to compressed blocks.
    // If fNextRun < 4, then we fill the runs that we haven't buffered with
    // the constant zero buffer.
    void flushRuns();
};


R11_EACBlitter::R11_EACBlitter(int width, int height, void *latcBuffer)
    // 0x7FFE is one minus the largest positive 16-bit int. We use it for
    // debugging to make sure that we're properly setting the nextX distance
    // in flushRuns(). 
    : kLongestRun(0x7FFE), kZeroAlpha(0)
    , fNextRun(0)
    , fWidth(width)
    , fHeight(height)
    , fBuffer(reinterpret_cast<uint64_t*const>(latcBuffer))
{
    SkASSERT((width % kR11_EACBlockSz) == 0);
    SkASSERT((height % kR11_EACBlockSz) == 0);
}

void R11_EACBlitter::blitAntiH(int x, int y,
                               const SkAlpha* antialias,
                               const int16_t* runs) {
    // Make sure that the new row to blit is either the first
    // row that we're blitting, or it's exactly the next scan row
    // since the last row that we blit. This is to ensure that when
    // we go to flush the runs, that they are all the same four
    // runs.
    if (fNextRun > 0 &&
        ((x != fBufferedRuns[fNextRun-1].fX) ||
         (y-1 != fBufferedRuns[fNextRun-1].fY))) {
        this->flushRuns();
    }

    // Align the rows to a block boundary. If we receive rows that
    // are not on a block boundary, then fill in the preceding runs
    // with zeros. We do this by producing a single RLE that says
    // that we have 0x7FFE pixels of zero (0x7FFE = 32766).
    const int row = y & ~3;
    while ((row + fNextRun) < y) {
        fBufferedRuns[fNextRun].fAlphas = &kZeroAlpha;
        fBufferedRuns[fNextRun].fRuns = &kLongestRun;
        fBufferedRuns[fNextRun].fX = 0;
        fBufferedRuns[fNextRun].fY = row + fNextRun;
        ++fNextRun;
    }

    // Make sure that our assumptions aren't violated...
    SkASSERT(fNextRun == (y & 3));
    SkASSERT(fNextRun == 0 || fBufferedRuns[fNextRun - 1].fY < y);

    // Set the values of the next run
    fBufferedRuns[fNextRun].fAlphas = antialias;
    fBufferedRuns[fNextRun].fRuns = runs;
    fBufferedRuns[fNextRun].fX = x;
    fBufferedRuns[fNextRun].fY = y;

    // If we've output four scanlines in a row that don't violate our
    // assumptions, then it's time to flush them...
    if (4 == ++fNextRun) {
        this->flushRuns();
    }
}

void R11_EACBlitter::flushRuns() {

    // If we don't have any runs, then just return.
    if (0 == fNextRun) {
        return;
    }

#ifndef NDEBUG
    // Make sure that if we have any runs, they all match
    for (int i = 1; i < fNextRun; ++i) {
        SkASSERT(fBufferedRuns[i].fY == fBufferedRuns[i-1].fY + 1);
        SkASSERT(fBufferedRuns[i].fX == fBufferedRuns[i-1].fX);
    }
#endif

    // If we dont have as many runs as we have rows, fill in the remaining
    // runs with constant zeros.
    for (int i = fNextRun; i < kR11_EACBlockSz; ++i) {
        fBufferedRuns[i].fY = fBufferedRuns[0].fY + i;
        fBufferedRuns[i].fX = fBufferedRuns[0].fX;
        fBufferedRuns[i].fAlphas = &kZeroAlpha;
        fBufferedRuns[i].fRuns = &kLongestRun;
    }

    // Make sure that our assumptions aren't violated.
    SkASSERT(fNextRun > 0 && fNextRun <= 4);
    SkASSERT((fBufferedRuns[0].fY & 3) == 0);

    // The following logic walks four rows at a time and outputs compressed
    // blocks to the buffer passed into the constructor.
    // We do the following:
    //
    //      c1 c2 c3 c4
    // -----------------------------------------------------------------------
    // ... |  |  |  |  |  ----> fBufferedRuns[0]
    // -----------------------------------------------------------------------
    // ... |  |  |  |  |  ----> fBufferedRuns[1]
    // -----------------------------------------------------------------------
    // ... |  |  |  |  |  ----> fBufferedRuns[2]
    // -----------------------------------------------------------------------
    // ... |  |  |  |  |  ----> fBufferedRuns[3]
    // -----------------------------------------------------------------------
    // 
    // curX -- the macro X value that we've gotten to.
    // c1, c2, c3, c4 -- the integers that represent the columns of the current block
    //                   that we're operating on
    // curAlphaColumn -- integer containing the column of alpha values from fBufferedRuns.
    // nextX -- for each run, the next point at which we need to update curAlphaColumn
    //          after the value of curX.
    // finalX -- the minimum of all the nextX values.
    //
    // curX advances to finalX outputting any blocks that it passes along
    // the way. Since finalX will not change when we reach the end of a
    // run, the termination criteria will be whenever curX == finalX at the
    // end of a loop.

    // Setup:
    uint32_t c[4] = { 0, 0, 0, 0 };
    uint32_t curAlphaColumn = 0;
    SkAlpha *curAlpha = reinterpret_cast<SkAlpha*>(&curAlphaColumn);

    int nextX[kR11_EACBlockSz];
    for (int i = 0; i < kR11_EACBlockSz; ++i) {
        nextX[i] = 0x7FFFFF;
    }

    uint64_t* outPtr = this->getBlock(fBufferedRuns[0].fX, fBufferedRuns[0].fY);

    // Populate the first set of runs and figure out how far we need to
    // advance on the first step
    int curX = 0;
    int finalX = 0xFFFFF;
    for (int i = 0; i < kR11_EACBlockSz; ++i) {
        nextX[i] = *(fBufferedRuns[i].fRuns);
        curAlpha[i] = *(fBufferedRuns[i].fAlphas);

        finalX = SkMin32(nextX[i], finalX);
    }

    // Make sure that we have a valid right-bound X value
    SkASSERT(finalX < 0xFFFFF);

    // Run the blitter...
    while (curX != finalX) {
        SkASSERT(finalX >= curX);

        // Do we need to populate the rest of the block?
        if ((finalX - (curX & ~3)) >= kR11_EACBlockSz) {
            const int col = curX & 3;
            const int colsLeft = 4 - col;
            SkASSERT(curX + colsLeft <= finalX);

            update_block_columns(c, col, colsLeft, curAlphaColumn);

            // Write this block
            *outPtr = compress_block_vertical(c[0], c[1], c[2], c[3]);
            ++outPtr;
            curX += colsLeft;
        }

        // If we can advance even further, then just keep memsetting the block
        if ((finalX - curX) >= kR11_EACBlockSz) {
            SkASSERT((curX & 3) == 0);

            const int col = 0;
            const int colsLeft = kR11_EACBlockSz;

            update_block_columns(c, col, colsLeft, curAlphaColumn);

            // While we can keep advancing, just keep writing the block.
            uint64_t lastBlock = compress_block_vertical(c[0], c[1], c[2], c[3]);
            while((finalX - curX) >= kR11_EACBlockSz) {
                *outPtr = lastBlock;
                ++outPtr;
                curX += kR11_EACBlockSz;
            }
        }

        // If we haven't advanced within the block then do so.
        if (curX < finalX) {
            const int col = curX & 3;
            const int colsLeft = finalX - curX;

            update_block_columns(c, col, colsLeft, curAlphaColumn);

            curX += colsLeft;
        }

        SkASSERT(curX == finalX);

        // Figure out what the next advancement is...
        for (int i = 0; i < kR11_EACBlockSz; ++i) {
            if (nextX[i] == finalX) {
                const int16_t run = *(fBufferedRuns[i].fRuns);
                fBufferedRuns[i].fRuns += run;
                fBufferedRuns[i].fAlphas += run;
                curAlpha[i] = *(fBufferedRuns[i].fAlphas);
                nextX[i] += *(fBufferedRuns[i].fRuns);
            }
        }

        finalX = 0xFFFFF;
        for (int i = 0; i < kR11_EACBlockSz; ++i) {
            finalX = SkMin32(nextX[i], finalX);
        }
    }

    // If we didn't land on a block boundary, output the block...
    if ((curX & 3) > 1) {
        *outPtr = compress_block_vertical(c[0], c[1], c[2], c[3]);
    }

    fNextRun = 0;
}

SkBlitter* CreateR11EACBlitter(int width, int height, void* outputBuffer) {
    return new R11_EACBlitter(width, height, outputBuffer);
}

}  // namespace SkTextureCompressor
