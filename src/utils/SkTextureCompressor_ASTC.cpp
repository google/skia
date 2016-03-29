/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor_ASTC.h"
#include "SkTextureCompressor_Blitter.h"

#include "SkBlitter.h"
#include "SkEndian.h"
#include "SkMath.h"

// This table contains the weight values for each texel. This is used in determining
// how to convert a 12x12 grid of alpha values into a 6x5 grid of index values. Since
// we have a 6x5 grid, that gives 30 values that we have to compute. For each index,
// we store up to 20 different triplets of values. In order the triplets are:
// weight, texel-x, texel-y
// The weight value corresponds to the amount that this index contributes to the final
// index value of the given texel. Hence, we need to reconstruct the 6x5 index grid
// from their relative contribution to the 12x12 texel grid.
//
// The algorithm is something like this:
// foreach index i:
//    total-weight = 0;
//    total-alpha = 0;
//    for w = 1 to 20:
//       weight = table[i][w*3];
//       texel-x = table[i][w*3 + 1];
//       texel-y = table[i][w*3 + 2];
//       if weight >= 0:
//           total-weight += weight;
//           total-alpha += weight * alphas[texel-x][texel-y];
//
//    total-alpha /= total-weight;
//    index = top three bits of total-alpha
//
// If the associated index does not contribute to 20 different texels (e.g. it's in
// a corner), then the extra texels are stored with -1's in the table.

static const int8_t k6x5To12x12Table[30][60] = {
{ 16, 0, 0, 9, 1, 0, 1, 2, 0, 10, 0, 1, 6, 1, 1, 1, 2, 1, 4, 0, 2, 2,
  1, 2, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 7, 1, 0, 15, 2, 0, 10, 3, 0, 3, 4, 0, 4, 1, 1, 9, 2, 1, 6, 3, 1, 2,
  4, 1, 2, 1, 2, 4, 2, 2, 3, 3, 2, 1, 4, 2, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 6, 3, 0, 13, 4, 0, 12, 5, 0, 4, 6, 0, 4, 3, 1, 8, 4, 1, 8, 5, 1, 3,
  6, 1, 1, 3, 2, 3, 4, 2, 3, 5, 2, 1, 6, 2, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 4, 5, 0, 12, 6, 0, 13, 7, 0, 6, 8, 0, 2, 5, 1, 7, 6, 1, 8, 7, 1, 4,
  8, 1, 1, 5, 2, 3, 6, 2, 3, 7, 2, 2, 8, 2, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 3, 7, 0, 10, 8, 0, 15, 9, 0, 7, 10, 0, 2, 7, 1, 6, 8, 1, 9, 9, 1, 4,
  10, 1, 1, 7, 2, 2, 8, 2, 4, 9, 2, 2, 10, 2, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 1, 9, 0, 9, 10, 0, 16, 11, 0, 1, 9, 1, 6, 10, 1, 10, 11, 1, 2, 10, 2, 4,
  11, 2, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 6, 0, 1, 3, 1, 1, 12, 0, 2, 7, 1, 2, 1, 2, 2, 15, 0, 3, 8, 1, 3, 1,
  2, 3, 9, 0, 4, 5, 1, 4, 1, 2, 4, 3, 0, 5, 2, 1, 5, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 3, 1, 1, 6, 2, 1, 4, 3, 1, 1, 4, 1, 5, 1, 2, 11, 2, 2, 7, 3, 2, 2,
  4, 2, 7, 1, 3, 14, 2, 3, 9, 3, 3, 3, 4, 3, 4, 1, 4, 8, 2, 4, 6, 3,
  4, 2, 4, 4, 1, 1, 5, 3, 2, 5, 2, 3, 5, 1, 4, 5}, // n = 20
{ 2, 3, 1, 5, 4, 1, 4, 5, 1, 1, 6, 1, 5, 3, 2, 10, 4, 2, 9, 5, 2, 3,
  6, 2, 6, 3, 3, 12, 4, 3, 11, 5, 3, 4, 6, 3, 3, 3, 4, 7, 4, 4, 7, 5,
  4, 2, 6, 4, 1, 3, 5, 2, 4, 5, 2, 5, 5, 1, 6, 5}, // n = 20
{ 2, 5, 1, 5, 6, 1, 5, 7, 1, 2, 8, 1, 3, 5, 2, 9, 6, 2, 10, 7, 2, 4,
  8, 2, 4, 5, 3, 11, 6, 3, 12, 7, 3, 6, 8, 3, 2, 5, 4, 7, 6, 4, 7, 7,
  4, 3, 8, 4, 1, 5, 5, 2, 6, 5, 2, 7, 5, 1, 8, 5}, // n = 20
{ 1, 7, 1, 4, 8, 1, 6, 9, 1, 3, 10, 1, 2, 7, 2, 8, 8, 2, 11, 9, 2, 5,
  10, 2, 3, 7, 3, 9, 8, 3, 14, 9, 3, 7, 10, 3, 2, 7, 4, 6, 8, 4, 8, 9,
  4, 4, 10, 4, 1, 7, 5, 2, 8, 5, 3, 9, 5, 1, 10, 5}, // n = 20
{ 3, 10, 1, 6, 11, 1, 1, 9, 2, 7, 10, 2, 12, 11, 2, 1, 9, 3, 8, 10, 3, 15,
  11, 3, 1, 9, 4, 5, 10, 4, 9, 11, 4, 2, 10, 5, 3, 11, 5, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 1, 0, 3, 1, 1, 3, 7, 0, 4, 4, 1, 4, 13, 0, 5, 7, 1, 5, 1, 2, 5, 13,
  0, 6, 7, 1, 6, 1, 2, 6, 7, 0, 7, 4, 1, 7, 1, 0, 8, 1, 1, 8, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 1, 2, 3, 1, 3, 3, 3, 1, 4, 7, 2, 4, 4, 3, 4, 1, 4, 4, 6, 1, 5, 12,
  2, 5, 8, 3, 5, 2, 4, 5, 6, 1, 6, 12, 2, 6, 8, 3, 6, 2, 4, 6, 3, 1,
  7, 7, 2, 7, 4, 3, 7, 1, 4, 7, 1, 2, 8, 1, 3, 8}, // n = 20
{ 1, 4, 3, 1, 5, 3, 3, 3, 4, 6, 4, 4, 5, 5, 4, 2, 6, 4, 5, 3, 5, 11,
  4, 5, 10, 5, 5, 3, 6, 5, 5, 3, 6, 11, 4, 6, 10, 5, 6, 3, 6, 6, 3, 3,
  7, 6, 4, 7, 5, 5, 7, 2, 6, 7, 1, 4, 8, 1, 5, 8}, // n = 20
{ 1, 6, 3, 1, 7, 3, 2, 5, 4, 5, 6, 4, 6, 7, 4, 3, 8, 4, 3, 5, 5, 10,
  6, 5, 11, 7, 5, 5, 8, 5, 3, 5, 6, 10, 6, 6, 11, 7, 6, 5, 8, 6, 2, 5,
  7, 5, 6, 7, 6, 7, 7, 3, 8, 7, 1, 6, 8, 1, 7, 8}, // n = 20
{ 1, 8, 3, 1, 9, 3, 1, 7, 4, 4, 8, 4, 7, 9, 4, 3, 10, 4, 2, 7, 5, 8,
  8, 5, 12, 9, 5, 6, 10, 5, 2, 7, 6, 8, 8, 6, 12, 9, 6, 6, 10, 6, 1, 7,
  7, 4, 8, 7, 7, 9, 7, 3, 10, 7, 1, 8, 8, 1, 9, 8}, // n = 20
{ 1, 10, 3, 1, 11, 3, 4, 10, 4, 7, 11, 4, 1, 9, 5, 7, 10, 5, 13, 11, 5, 1,
  9, 6, 7, 10, 6, 13, 11, 6, 4, 10, 7, 7, 11, 7, 1, 10, 8, 1, 11, 8, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 3, 0, 6, 2, 1, 6, 9, 0, 7, 5, 1, 7, 1, 2, 7, 15, 0, 8, 8, 1, 8, 1,
  2, 8, 12, 0, 9, 7, 1, 9, 1, 2, 9, 6, 0, 10, 3, 1, 10, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 1, 1, 6, 3, 2, 6, 2, 3, 6, 1, 4, 6, 4, 1, 7, 8, 2, 7, 6, 3, 7, 2,
  4, 7, 7, 1, 8, 14, 2, 8, 9, 3, 8, 3, 4, 8, 5, 1, 9, 11, 2, 9, 8, 3,
  9, 2, 4, 9, 3, 1, 10, 6, 2, 10, 4, 3, 10, 1, 4, 10}, // n = 20
{ 1, 3, 6, 2, 4, 6, 2, 5, 6, 1, 6, 6, 3, 3, 7, 7, 4, 7, 7, 5, 7, 2,
  6, 7, 6, 3, 8, 12, 4, 8, 11, 5, 8, 4, 6, 8, 4, 3, 9, 10, 4, 9, 9, 5,
  9, 3, 6, 9, 2, 3, 10, 5, 4, 10, 5, 5, 10, 2, 6, 10}, // n = 20
{ 1, 5, 6, 2, 6, 6, 2, 7, 6, 1, 8, 6, 2, 5, 7, 7, 6, 7, 7, 7, 7, 3,
  8, 7, 4, 5, 8, 11, 6, 8, 12, 7, 8, 6, 8, 8, 3, 5, 9, 9, 6, 9, 10, 7,
  9, 5, 8, 9, 1, 5, 10, 4, 6, 10, 5, 7, 10, 2, 8, 10}, // n = 20
{ 1, 7, 6, 2, 8, 6, 3, 9, 6, 1, 10, 6, 2, 7, 7, 6, 8, 7, 8, 9, 7, 4,
  10, 7, 3, 7, 8, 9, 8, 8, 14, 9, 8, 7, 10, 8, 2, 7, 9, 7, 8, 9, 11, 9,
  9, 5, 10, 9, 1, 7, 10, 4, 8, 10, 6, 9, 10, 3, 10, 10}, // n = 20
{ 2, 10, 6, 3, 11, 6, 1, 9, 7, 5, 10, 7, 9, 11, 7, 1, 9, 8, 8, 10, 8, 15,
  11, 8, 1, 9, 9, 7, 10, 9, 12, 11, 9, 3, 10, 10, 6, 11, 10, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 4, 0, 9, 2, 1, 9, 10, 0, 10, 6, 1, 10, 1, 2, 10, 16, 0, 11, 9, 1, 11, 1,
  2, 11, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 2, 1, 9, 4, 2, 9, 2, 3, 9, 1, 4, 9, 4, 1, 10, 9, 2, 10, 6, 3, 10, 2,
  4, 10, 7, 1, 11, 15, 2, 11, 10, 3, 11, 3, 4, 11, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 2, 3, 9, 3, 4, 9, 3, 5, 9, 1, 6, 9, 4, 3, 10, 8, 4, 10, 7, 5, 10, 2,
  6, 10, 6, 3, 11, 13, 4, 11, 12, 5, 11, 4, 6, 11, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 1, 5, 9, 3, 6, 9, 3, 7, 9, 1, 8, 9, 3, 5, 10, 8, 6, 10, 8, 7, 10, 4,
  8, 10, 4, 5, 11, 12, 6, 11, 13, 7, 11, 6, 8, 11, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 1, 7, 9, 3, 8, 9, 4, 9, 9, 2, 10, 9, 2, 7, 10, 6, 8, 10, 9, 9, 10, 4,
  10, 10, 3, 7, 11, 10, 8, 11, 15, 9, 11, 7, 10, 11, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0}, // n = 20
{ 2, 10, 9, 4, 11, 9, 1, 9, 10, 6, 10, 10, 10, 11, 10, 1, 9, 11, 9, 10, 11, 16,
  11, 11, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
  0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0} // n = 20
};

// Returns the alpha value of a texel at position (x, y) from src.
// (x, y) are assumed to be in the range [0, 12).
inline uint8_t GetAlpha(const uint8_t *src, size_t rowBytes, int x, int y) {
    SkASSERT(x >= 0 && x < 12);
    SkASSERT(y >= 0 && y < 12);
    SkASSERT(rowBytes >= 12);
    return *(src + y*rowBytes + x);
}

inline uint8_t GetAlphaTranspose(const uint8_t *src, size_t rowBytes, int x, int y) {
    return GetAlpha(src, rowBytes, y, x);
}

// Output the 16 bytes stored in top and bottom and advance the pointer. The bytes
// are stored as the integers are represented in memory, so they should be swapped
// if necessary.
static inline void send_packing(uint8_t** dst, const uint64_t top, const uint64_t bottom) {
    uint64_t* dst64 = reinterpret_cast<uint64_t*>(*dst);
    dst64[0] = top;
    dst64[1] = bottom;
    *dst += 16;
}

// Compresses an ASTC block, by looking up the proper contributions from
// k6x5To12x12Table and computing an index from the associated values.
typedef uint8_t (*GetAlphaProc)(const uint8_t* src, size_t rowBytes, int x, int y);

template<GetAlphaProc getAlphaProc>
static void compress_a8_astc_block(uint8_t** dst, const uint8_t* src, size_t rowBytes) {
    // Check for single color
    bool constant = true;
    const uint32_t firstInt = *(reinterpret_cast<const uint32_t*>(src));
    for (int i = 0; i < 12; ++i) {
        const uint32_t *rowInt = reinterpret_cast<const uint32_t *>(src + i*rowBytes);
        constant = constant && (rowInt[0] == firstInt);
        constant = constant && (rowInt[1] == firstInt);
        constant = constant && (rowInt[2] == firstInt);
    }

    if (constant) {
        if (0 == firstInt) {
            // All of the indices are set to zero, and the colors are
            // v0 = 0, v1 = 255, so everything will be transparent.
            send_packing(dst, SkTEndian_SwapLE64(0x0000000001FE000173ULL), 0);
            return;
        } else if (0xFFFFFFFF == firstInt) {
            // All of the indices are set to zero, and the colors are
            // v0 = 255, v1 = 0, so everything will be opaque.
            send_packing(dst, SkTEndian_SwapLE64(0x000000000001FE0173ULL), 0);
            return;
        }
    }

    uint8_t indices[30]; // 6x5 index grid
    for (int idx = 0; idx < 30; ++idx) {
        int weightTot = 0;
        int alphaTot = 0;
        for (int w = 0; w < 20; ++w) {
            const int8_t weight = k6x5To12x12Table[idx][w*3];
            if (weight > 0) {
                const int x = k6x5To12x12Table[idx][w*3 + 1];
                const int y = k6x5To12x12Table[idx][w*3 + 2];
                weightTot += weight;
                alphaTot += weight * getAlphaProc(src, rowBytes, x, y);
            } else {
                // In our table, not every entry has 20 weights, and all
                // of them are nonzero. Once we hit a negative weight, we
                // know that all of the other weights are not valid either.
                break;
            }
        }

        indices[idx] = (alphaTot / weightTot) >> 5;
    }

    // Pack indices... The ASTC block layout is fairly complicated. An extensive
    // description can be found here:
    // https://www.opengl.org/registry/specs/KHR/texture_compression_astc_hdr.txt
    //
    // Here is a summary of the options that we've chosen:
    // 1. Block mode: 0b00101110011
    //     - 6x5 texel grid
    //     - Single plane
    //     - Low-precision index values
    //     - Index range 0-7 (three bits per index)
    // 2. Partitions: 0b00
    //     - One partition
    // 3. Color Endpoint Mode: 0b0000
    //     - Direct luminance -- e0=(v0,v0,v0,0xFF); e1=(v1,v1,v1,0xFF);
    // 4. 8-bit endpoints:
    //     v0 = 0, v1 = 255
    //
    // The rest of the block contains the 30 index values from before, which
    // are currently stored in the indices variable.

    uint64_t top = 0x0000000001FE000173ULL;
    uint64_t bottom = 0;

    for (int idx = 0; idx <= 20; ++idx) {
        const uint8_t index = indices[idx];
        bottom |= static_cast<uint64_t>(index) << (61-(idx*3));
    }

    // index 21 straddles top and bottom
    {
        const uint8_t index = indices[21];
        bottom |= index & 1;
        top |= static_cast<uint64_t>((index >> 2) | (index & 2)) << 62;
    }

    for (int idx = 22; idx < 30; ++idx) {
        const uint8_t index = indices[idx];
        top |= static_cast<uint64_t>(index) << (59-(idx-22)*3);
    }

    // Reverse each 3-bit index since indices are read in reverse order...
    uint64_t t = (bottom ^ (bottom >> 2)) & 0x2492492492492492ULL;
    bottom = bottom ^ t ^ (t << 2);

    t = (top ^ (top >> 2)) & 0x0924924000000000ULL;
    top = top ^ t ^ (t << 2);

    send_packing(dst, SkEndian_SwapLE64(top), SkEndian_SwapLE64(bottom));
}

inline void CompressA8ASTCBlockVertical(uint8_t* dst, const uint8_t* src) {
    compress_a8_astc_block<GetAlphaTranspose>(&dst, src, 12);
}

////////////////////////////////////////////////////////////////////////////////
//
// ASTC Decoder
//
// Full details available in the spec:
// http://www.khronos.org/registry/gles/extensions/OES/OES_texture_compression_astc.txt
//
////////////////////////////////////////////////////////////////////////////////

// Enable this to assert whenever a decoded block has invalid ASTC values. Otherwise,
// each invalid block will result in a disgusting magenta color.
#define ASSERT_ASTC_DECODE_ERROR 0

// Reverse 64-bit integer taken from TAOCP 4a, although it's better
// documented at this site:
// http://matthewarcus.wordpress.com/2012/11/18/reversing-a-64-bit-word/

template <typename T, T m, int k>
static inline T swap_bits(T p) {
    T q = ((p>>k)^p) & m;
    return p^q^(q<<k);
}

static inline uint64_t reverse64(uint64_t n) {
    static const uint64_t m0 = 0x5555555555555555ULL;
    static const uint64_t m1 = 0x0300c0303030c303ULL;
    static const uint64_t m2 = 0x00c0300c03f0003fULL;
    static const uint64_t m3 = 0x00000ffc00003fffULL;
    n = ((n>>1)&m0) | (n&m0)<<1;
    n = swap_bits<uint64_t, m1, 4>(n);
    n = swap_bits<uint64_t, m2, 8>(n);
    n = swap_bits<uint64_t, m3, 20>(n);
    n = (n >> 34) | (n << 30);
    return n;
}

// An ASTC block is 128 bits. We represent it as two 64-bit integers in order
// to efficiently operate on the block using bitwise operations.
struct ASTCBlock {
    uint64_t fLow;
    uint64_t fHigh;

    // Reverses the bits of an ASTC block, making the LSB of the
    // 128 bit block the MSB.
    inline void reverse() {
        const uint64_t newLow = reverse64(this->fHigh);
        this->fHigh = reverse64(this->fLow);
        this->fLow = newLow;
    }
};

// Writes the given color to every pixel in the block. This is used by void-extent
// blocks (a special constant-color encoding of a block) and by the error function.
static inline void write_constant_color(uint8_t* dst, int blockDimX, int blockDimY,
                                        int dstRowBytes, SkColor color) {
    for (int y = 0; y < blockDimY; ++y) {
        SkColor *dstColors = reinterpret_cast<SkColor*>(dst);
        for (int x = 0; x < blockDimX; ++x) {
            dstColors[x] = color;
        }
        dst += dstRowBytes;
    }
}

// Sets the entire block to the ASTC "error" color, a disgusting magenta
// that's not supposed to appear in natural images.
static inline void write_error_color(uint8_t* dst, int blockDimX, int blockDimY,
                                     int dstRowBytes) {
    static const SkColor kASTCErrorColor = SkColorSetRGB(0xFF, 0, 0xFF);

#if ASSERT_ASTC_DECODE_ERROR
    SkDEBUGFAIL("ASTC decoding error!\n");
#endif

    write_constant_color(dst, blockDimX, blockDimY, dstRowBytes, kASTCErrorColor);
}

// Reads up to 64 bits of the ASTC block starting from bit
// 'from' and going up to but not including bit 'to'. 'from' starts
// counting from the LSB, counting up to the MSB. Returns -1 on
// error.
static uint64_t read_astc_bits(const ASTCBlock &block, int from, int to) {
    SkASSERT(0 <= from && from <= 128);
    SkASSERT(0 <= to && to <= 128);

    const int nBits = to - from;
    if (0 == nBits) {
        return 0;
    }

    if (nBits < 0 || 64 <= nBits) {
        SkDEBUGFAIL("ASTC -- shouldn't read more than 64 bits");
        return -1;
    }

    // Remember, the 'to' bit isn't read.
    uint64_t result = 0;
    if (to <= 64) {
        // All desired bits are in the low 64-bits.
        result = (block.fLow >> from) & ((1ULL << nBits) - 1);
    } else if (from >= 64) {
        // All desired bits are in the high 64-bits.
        result = (block.fHigh >> (from - 64)) & ((1ULL << nBits) - 1);
    } else {
        // from < 64 && to > 64
        SkASSERT(nBits > (64 - from));
        const int nLow = 64 - from;
        const int nHigh = nBits - nLow;
        result =
            ((block.fLow >> from) & ((1ULL << nLow) - 1)) |
            ((block.fHigh & ((1ULL << nHigh) - 1)) << nLow);
    }

    return result;
}

// Returns the number of bits needed to represent a number
// in the given power-of-two range (excluding the power of two itself).
static inline int bits_for_range(int x) {
    SkASSERT(SkIsPow2(x));
    SkASSERT(0 != x);
    // Since we know it's a power of two, there should only be one bit set,
    // meaning the number of trailing zeros is 31 minus the number of leading
    // zeros.
    return 31 - SkCLZ(x);
}

// Clamps an integer to the range [0, 255]
static inline int clamp_byte(int x) {
    return SkClampMax(x, 255);
}

// Helper function defined in the ASTC spec, section C.2.14
// It transfers a few bits of precision from one value to another.
static inline void bit_transfer_signed(int *a, int *b) {
    *b >>= 1;
    *b |= *a & 0x80;
    *a >>= 1;
    *a &= 0x3F;
    if ( (*a & 0x20) != 0 ) {
        *a -= 0x40;
    }
}

// Helper function defined in the ASTC spec, section C.2.14
// It uses the value in the blue channel to tint the red and green
static inline SkColor blue_contract(int a, int r, int g, int b) {
    return SkColorSetARGB(a, (r + b) >> 1, (g + b) >> 1, b);
}

// Helper function that decodes two colors from eight values. If isRGB is true,
// then the pointer 'v' contains six values and the last two are considered to be
// 0xFF. If isRGB is false, then all eight values come from the pointer 'v'. This
// corresponds to the decode procedure for the following endpoint modes:
//   kLDR_RGB_Direct_ColorEndpointMode
//   kLDR_RGBA_Direct_ColorEndpointMode
static inline void decode_rgba_direct(const int *v, SkColor *endpoints, bool isRGB) {

    int v6 = 0xFF;
    int v7 = 0xFF;
    if (!isRGB) {
        v6 = v[6];
        v7 = v[7];
    }

    const int s0 = v[0] + v[2] + v[4];
    const int s1 = v[1] + v[3] + v[5];

    if (s1 >= s0) {
        endpoints[0] = SkColorSetARGB(v6, v[0], v[2], v[4]);
        endpoints[1] = SkColorSetARGB(v7, v[1], v[3], v[5]);
    } else {
        endpoints[0] = blue_contract(v7, v[1], v[3], v[5]);
        endpoints[1] = blue_contract(v6, v[0], v[2], v[4]);
    }
}

// Helper function that decodes two colors from six values. If isRGB is true,
// then the pointer 'v' contains four values and the last two are considered to be
// 0xFF. If isRGB is false, then all six values come from the pointer 'v'. This
// corresponds to the decode procedure for the following endpoint modes:
//   kLDR_RGB_BaseScale_ColorEndpointMode
//   kLDR_RGB_BaseScaleWithAlpha_ColorEndpointMode
static inline void decode_rgba_basescale(const int *v, SkColor *endpoints, bool isRGB) {

    int v4 = 0xFF;
    int v5 = 0xFF;
    if (!isRGB) {
        v4 = v[4];
        v5 = v[5];
    }

    endpoints[0] = SkColorSetARGB(v4,
                                  (v[0]*v[3]) >> 8,
                                  (v[1]*v[3]) >> 8,
                                  (v[2]*v[3]) >> 8);
    endpoints[1] = SkColorSetARGB(v5, v[0], v[1], v[2]);
}

// Helper function that decodes two colors from eight values. If isRGB is true,
// then the pointer 'v' contains six values and the last two are considered to be
// 0xFF. If isRGB is false, then all eight values come from the pointer 'v'. This
// corresponds to the decode procedure for the following endpoint modes:
//   kLDR_RGB_BaseOffset_ColorEndpointMode
//   kLDR_RGBA_BaseOffset_ColorEndpointMode
//
// If isRGB is true, then treat this as if v6 and v7 are meant to encode full alpha values.
static inline void decode_rgba_baseoffset(const int *v, SkColor *endpoints, bool isRGB) {
    int v0 = v[0];
    int v1 = v[1];
    int v2 = v[2];
    int v3 = v[3];
    int v4 = v[4];
    int v5 = v[5];
    int v6 = isRGB ? 0xFF : v[6];
    // The 0 is here because this is an offset, not a direct value
    int v7 = isRGB ? 0 : v[7];

    bit_transfer_signed(&v1, &v0);
    bit_transfer_signed(&v3, &v2);
    bit_transfer_signed(&v5, &v4);
    if (!isRGB) {
        bit_transfer_signed(&v7, &v6);
    }

    int c[2][4];
    if ((v1 + v3 + v5) >= 0) {
        c[0][0] = v6;
        c[0][1] = v0;
        c[0][2] = v2;
        c[0][3] = v4;

        c[1][0] = v6 + v7;
        c[1][1] = v0 + v1;
        c[1][2] = v2 + v3;
        c[1][3] = v4 + v5;
    } else {
        c[0][0] = v6 + v7;
        c[0][1] = (v0 + v1 + v4 + v5) >> 1;
        c[0][2] = (v2 + v3 + v4 + v5) >> 1;
        c[0][3] = v4 + v5;

        c[1][0] = v6;
        c[1][1] = (v0 + v4) >> 1;
        c[1][2] = (v2 + v4) >> 1;
        c[1][3] = v4;
    }

    endpoints[0] = SkColorSetARGB(clamp_byte(c[0][0]),
                                  clamp_byte(c[0][1]),
                                  clamp_byte(c[0][2]),
                                  clamp_byte(c[0][3]));

    endpoints[1] = SkColorSetARGB(clamp_byte(c[1][0]),
                                  clamp_byte(c[1][1]),
                                  clamp_byte(c[1][2]),
                                  clamp_byte(c[1][3]));
}


// A helper class used to decode bit values from standard integer values.
// We can't use this class with ASTCBlock because then it would need to
// handle multi-value ranges, and it's non-trivial to lookup a range of bits
// that splits across two different ints.
template <typename T>
class SkTBits {
public:
    SkTBits(const T val) : fVal(val) { }

    // Returns the bit at the given position
    T operator [](const int idx) const {
        return (fVal >> idx) & 1;
    }

    // Returns the bits in the given range, inclusive
    T operator ()(const int end, const int start) const {
        SkASSERT(end >= start);
        return (fVal >> start) & ((1ULL << ((end - start) + 1)) - 1);
    }

private:
    const T fVal;
};

// This algorithm matches the trit block decoding in the spec (Table C.2.14)
static void decode_trit_block(int* dst, int nBits, const uint64_t &block) {

    SkTBits<uint64_t> blockBits(block);

    // According to the spec, a trit block, which contains five values,
    // has the following layout:
    //
    // 27  26  25  24  23  22  21  20  19  18  17  16
    //  -----------------------------------------------
    // |T7 |     m4        |T6  T5 |     m3        |T4 |
    //  -----------------------------------------------
    //
    // 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
    //  --------------------------------------------------------------
    // |    m2        |T3  T2 |      m1       |T1  T0 |      m0       |
    //  --------------------------------------------------------------
    //
    // Where the m's are variable width depending on the number of bits used
    // to encode the values (anywhere from 0 to 6). Since 3^5 = 243, the extra
    // byte labeled T (whose bits are interleaved where 0 is the LSB and 7 is
    // the MSB), contains five trit values. To decode the trit values, the spec
    // says that we need to follow the following algorithm:
    //
    // if T[4:2] = 111
    //     C = { T[7:5], T[1:0] }; t4 = t3 = 2
    // else
    //     C = T[4:0]
    //
    // if T[6:5] = 11
    //     t4 = 2; t3 = T[7]
    // else
    //     t4 = T[7]; t3 = T[6:5]
    //
    // if C[1:0] = 11
    //     t2 = 2; t1 = C[4]; t0 = { C[3], C[2]&~C[3] }
    // else if C[3:2] = 11
    //     t2 = 2; t1 = 2; t0 = C[1:0]
    // else
    //     t2 = C[4]; t1 = C[3:2]; t0 = { C[1], C[0]&~C[1] }
    //
    // The following C++ code is meant to mirror this layout and algorithm as
    // closely as possible.

    int m[5];
    if (0 == nBits) {
        memset(m, 0, sizeof(m));
    } else {
        SkASSERT(nBits < 8);
        m[0] = static_cast<int>(blockBits(nBits - 1, 0));
        m[1] = static_cast<int>(blockBits(2*nBits - 1 + 2, nBits + 2));
        m[2] = static_cast<int>(blockBits(3*nBits - 1 + 4, 2*nBits + 4));
        m[3] = static_cast<int>(blockBits(4*nBits - 1 + 5, 3*nBits + 5));
        m[4] = static_cast<int>(blockBits(5*nBits - 1 + 7, 4*nBits + 7));
    }

    int T =
        static_cast<int>(blockBits(nBits + 1, nBits)) |
        (static_cast<int>(blockBits(2*nBits + 2 + 1, 2*nBits + 2)) << 2) |
        (static_cast<int>(blockBits[3*nBits + 4] << 4)) |
        (static_cast<int>(blockBits(4*nBits + 5 + 1, 4*nBits + 5)) << 5) |
        (static_cast<int>(blockBits[5*nBits + 7] << 7));

    int t[5];

    int C;
    SkTBits<int> Tbits(T);
    if (0x7 == Tbits(4, 2)) {
        C = (Tbits(7, 5) << 2) | Tbits(1, 0);
        t[3] = t[4] = 2;
    } else {
        C = Tbits(4, 0);
        if (Tbits(6, 5) == 0x3) {
            t[4] = 2; t[3] = Tbits[7];
        } else {
            t[4] = Tbits[7]; t[3] = Tbits(6, 5);
        }
    }

    SkTBits<int> Cbits(C);
    if (Cbits(1, 0) == 0x3) {
        t[2] = 2;
        t[1] = Cbits[4];
        t[0] = (Cbits[3] << 1) | (Cbits[2] & (0x1 & ~(Cbits[3])));
    } else if (Cbits(3, 2) == 0x3) {
        t[2] = 2;
        t[1] = 2;
        t[0] = Cbits(1, 0);
    } else {
        t[2] = Cbits[4];
        t[1] = Cbits(3, 2);
        t[0] = (Cbits[1] << 1) | (Cbits[0] & (0x1 & ~(Cbits[1])));
    }

#ifdef SK_DEBUG
    // Make sure all of the decoded values have a trit less than three
    // and a bit value within the range of the allocated bits.
    for (int i = 0; i < 5; ++i) {
        SkASSERT(t[i] < 3);
        SkASSERT(m[i] < (1 << nBits));
    }
#endif

    for (int i = 0; i < 5; ++i) {
        *dst = (t[i] << nBits) + m[i];
        ++dst;
    }
}

// This algorithm matches the quint block decoding in the spec (Table C.2.15)
static void decode_quint_block(int* dst, int nBits, const uint64_t &block) {
    SkTBits<uint64_t> blockBits(block);

    // According to the spec, a quint block, which contains three values,
    // has the following layout:
    //
    //
    // 18  17  16  15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
    //  --------------------------------------------------------------------------
    // |Q6  Q5 |     m2       |Q4  Q3 |     m1        |Q2  Q1  Q0 |      m0       |
    //  --------------------------------------------------------------------------
    //
    // Where the m's are variable width depending on the number of bits used
    // to encode the values (anywhere from 0 to 4). Since 5^3 = 125, the extra
    // 7-bit value labeled Q (whose bits are interleaved where 0 is the LSB and 6 is
    // the MSB), contains three quint values. To decode the quint values, the spec
    // says that we need to follow the following algorithm:
    //
    // if Q[2:1] = 11 and Q[6:5] = 00
    //     q2 = { Q[0], Q[4]&~Q[0], Q[3]&~Q[0] }; q1 = q0 = 4
    // else
    //     if Q[2:1] = 11
    //         q2 = 4; C = { Q[4:3], ~Q[6:5], Q[0] }
    //     else
    //         q2 = T[6:5]; C = Q[4:0]
    //
    //     if C[2:0] = 101
    //         q1 = 4; q0 = C[4:3]
    //     else
    //         q1 = C[4:3]; q0 = C[2:0]
    //
    // The following C++ code is meant to mirror this layout and algorithm as
    // closely as possible.

    int m[3];
    if (0 == nBits) {
        memset(m, 0, sizeof(m));
    } else {
        SkASSERT(nBits < 8);
        m[0] = static_cast<int>(blockBits(nBits - 1, 0));
        m[1] = static_cast<int>(blockBits(2*nBits - 1 + 3, nBits + 3));
        m[2] = static_cast<int>(blockBits(3*nBits - 1 + 5, 2*nBits + 5));
    }

    int Q =
        static_cast<int>(blockBits(nBits + 2, nBits)) |
        (static_cast<int>(blockBits(2*nBits + 3 + 1, 2*nBits + 3)) << 3) |
        (static_cast<int>(blockBits(3*nBits + 5 + 1, 3*nBits + 5)) << 5);

    int q[3];
    SkTBits<int> Qbits(Q); // quantum?

    if (Qbits(2, 1) == 0x3 && Qbits(6, 5) == 0) {
        const int notBitZero = (0x1 & ~(Qbits[0]));
        q[2] = (Qbits[0] << 2) | ((Qbits[4] & notBitZero) << 1) | (Qbits[3] & notBitZero);
        q[1] = 4;
        q[0] = 4;
    } else {
        int C;
        if (Qbits(2, 1) == 0x3) {
            q[2] = 4;
            C = (Qbits(4, 3) << 3) | ((0x3 & ~(Qbits(6, 5))) << 1) | Qbits[0];
        } else {
            q[2] = Qbits(6, 5);
            C = Qbits(4, 0);
        }

        SkTBits<int> Cbits(C);
        if (Cbits(2, 0) == 0x5) {
            q[1] = 4;
            q[0] = Cbits(4, 3);
        } else {
            q[1] = Cbits(4, 3);
            q[0] = Cbits(2, 0);
        }
    }

#ifdef SK_DEBUG
    for (int i = 0; i < 3; ++i) {
        SkASSERT(q[i] < 5);
        SkASSERT(m[i] < (1 << nBits));
    }
#endif

    for (int i = 0; i < 3; ++i) {
        *dst = (q[i] << nBits) + m[i];
        ++dst;
    }
}

// Function that decodes a sequence of integers stored as an ISE (Integer
// Sequence Encoding) bit stream. The full details of this function are outlined
// in section C.2.12 of the ASTC spec. A brief overview is as follows:
//
// - Each integer in the sequence is bounded by a specific range r.
// - The range of each value determines the way the bit stream is interpreted,
// - If the range is a power of two, then the sequence is a sequence of bits
// - If the range is of the form 3*2^n, then the sequence is stored as a
//   sequence of blocks, each block contains 5 trits and 5 bit sequences, which
//   decodes into 5 values.
// - Similarly, if the range is of the form 5*2^n, then the sequence is stored as a
//   sequence of blocks, each block contains 3 quints and 3 bit sequences, which
//   decodes into 3 values.
static bool decode_integer_sequence(
    int* dst,                 // The array holding the destination bits
    int dstSize,              // The maximum size of the array
    int nVals,                // The number of values that we'd like to decode
    const ASTCBlock &block,   // The block that we're decoding from
    int startBit,             // The bit from which we're going to do the reading
    int endBit,               // The bit at which we stop reading (not inclusive)
    bool bReadForward,        // If true, then read LSB -> MSB, else read MSB -> LSB
    int nBits,                // The number of bits representing this encoding
    int nTrits,               // The number of trits representing this encoding
    int nQuints               // The number of quints representing this encoding
) {
    // If we want more values than we have, then fail.
    if (nVals > dstSize) {
        return false;
    }

    ASTCBlock src = block;

    if (!bReadForward) {
        src.reverse();
        startBit = 128 - startBit;
        endBit = 128 - endBit;
    }

    while (nVals > 0) {

        if (nTrits > 0) {
            SkASSERT(0 == nQuints);

            int endBlockBit = startBit + 8 + 5*nBits;
            if (endBlockBit > endBit) {
                endBlockBit = endBit;
            }

            // Trit blocks are three values large.
            int trits[5];
            decode_trit_block(trits, nBits, read_astc_bits(src, startBit, endBlockBit));
            memcpy(dst, trits, SkMin32(nVals, 5)*sizeof(int));

            dst += 5;
            nVals -= 5;
            startBit = endBlockBit;

        } else if (nQuints > 0) {
            SkASSERT(0 == nTrits);

            int endBlockBit = startBit + 7 + 3*nBits;
            if (endBlockBit > endBit) {
                endBlockBit = endBit;
            }

            // Quint blocks are three values large
            int quints[3];
            decode_quint_block(quints, nBits, read_astc_bits(src, startBit, endBlockBit));
            memcpy(dst, quints, SkMin32(nVals, 3)*sizeof(int));

            dst += 3;
            nVals -= 3;
            startBit = endBlockBit;

        } else {
            // Just read the bits, but don't read more than we have...
            int endValBit = startBit + nBits;
            if (endValBit > endBit) {
                endValBit = endBit;
            }

            SkASSERT(endValBit - startBit < 31);
            *dst = static_cast<int>(read_astc_bits(src, startBit, endValBit));
            ++dst;
            --nVals;
            startBit = endValBit;
        }
    }

    return true;
}

// Helper function that unquantizes some (seemingly random) generated
// numbers... meant to match the ASTC hardware. This function is used
// to unquantize both colors (Table C.2.16) and weights (Table C.2.26)
static inline int unquantize_value(unsigned mask, int A, int B, int C, int D) {
    int T = D * C + B;
    T = T ^ A;
    T = (A & mask) | (T >> 2);
    SkASSERT(T < 256);
    return T;
}

// Helper function to replicate the bits in x that represents an oldPrec
// precision integer into a prec precision integer. For example:
//   255 == replicate_bits(7, 3, 8);
static inline int replicate_bits(int x, int oldPrec, int prec) {
    while (oldPrec < prec) {
        const int toShift = SkMin32(prec-oldPrec, oldPrec);
        x = (x << toShift) | (x >> (oldPrec - toShift));
        oldPrec += toShift;
    }

    // Make sure that no bits are set outside the desired precision.
    SkASSERT((-(1 << prec) & x) == 0);
    return x;
}

// Returns the unquantized value of a color that's represented only as
// a set of bits.
static inline int unquantize_bits_color(int val, int nBits) {
    return replicate_bits(val, nBits, 8);
}

// Returns the unquantized value of a color that's represented as a
// trit followed by nBits bits. This algorithm follows the sequence
// defined in section C.2.13 of the ASTC spec.
static inline int unquantize_trit_color(int val, int nBits) {
    SkASSERT(nBits > 0);
    SkASSERT(nBits < 7);

    const int D = (val >> nBits) & 0x3;
    SkASSERT(D < 3);

    const int A = -(val & 0x1) & 0x1FF;

    static const int Cvals[6] = { 204, 93, 44, 22, 11, 5 };
    const int C = Cvals[nBits - 1];

    int B = 0;
    const SkTBits<int> valBits(val);
    switch (nBits) {
        case 1:
            B = 0;
            break;

        case 2: {
            const int b = valBits[1];
            B = (b << 1) | (b << 2) | (b << 4) | (b << 8);
        }
        break;

        case 3: {
            const int cb = valBits(2, 1);
            B = cb | (cb << 2) | (cb << 7);
        }
        break;

        case 4: {
            const int dcb = valBits(3, 1);
            B = dcb | (dcb << 6);
        }
        break;

        case 5: {
            const int edcb = valBits(4, 1);
            B = (edcb << 5) | (edcb >> 2);
        }
        break;

        case 6: {
            const int fedcb = valBits(5, 1);
            B = (fedcb << 4) | (fedcb >> 4);
        }
        break;
    }

    return unquantize_value(0x80, A, B, C, D);
}

// Returns the unquantized value of a color that's represented as a
// quint followed by nBits bits. This algorithm follows the sequence
// defined in section C.2.13 of the ASTC spec.
static inline int unquantize_quint_color(int val, int nBits) {
    const int D = (val >> nBits) & 0x7;
    SkASSERT(D < 5);

    const int A = -(val & 0x1) & 0x1FF;

    static const int Cvals[5] = { 113, 54, 26, 13, 6 };
    SkASSERT(nBits > 0);
    SkASSERT(nBits < 6);

    const int C = Cvals[nBits - 1];

    int B = 0;
    const SkTBits<int> valBits(val);
    switch (nBits) {
        case 1:
            B = 0;
            break;

        case 2: {
            const int b = valBits[1];
            B = (b << 2) | (b << 3) | (b << 8);
        }
        break;

        case 3: {
            const int cb = valBits(2, 1);
            B = (cb >> 1) | (cb << 1) | (cb << 7);
        }
        break;

        case 4: {
            const int dcb = valBits(3, 1);
            B = (dcb >> 1) | (dcb << 6);
        }
        break;

        case 5: {
            const int edcb = valBits(4, 1);
            B = (edcb << 5) | (edcb >> 3);
        }
        break;
    }

    return unquantize_value(0x80, A, B, C, D);
}

// This algorithm takes a list of integers, stored in vals, and unquantizes them
// in place. This follows the algorithm laid out in section C.2.13 of the ASTC spec.
static void unquantize_colors(int *vals, int nVals, int nBits, int nTrits, int nQuints) {
    for (int i = 0; i < nVals; ++i) {
        if (nTrits > 0) {
            SkASSERT(nQuints == 0);
            vals[i] = unquantize_trit_color(vals[i], nBits);
        } else if (nQuints > 0) {
            SkASSERT(nTrits == 0);
            vals[i] = unquantize_quint_color(vals[i], nBits);
        } else {
            SkASSERT(nQuints == 0 && nTrits == 0);
            vals[i] = unquantize_bits_color(vals[i], nBits);
        }
    }
}

// Returns an interpolated value between c0 and c1 based on the weight. This
// follows the algorithm laid out in section C.2.19 of the ASTC spec.
static int interpolate_channel(int c0, int c1, int weight) {
    SkASSERT(0 <= c0 && c0 < 256);
    SkASSERT(0 <= c1 && c1 < 256);

    c0 = (c0 << 8) | c0;
    c1 = (c1 << 8) | c1;

    const int result = ((c0*(64 - weight) + c1*weight + 32) / 64) >> 8;

    if (result > 255) {
        return 255;
    }

    SkASSERT(result >= 0);
    return result;
}

// Returns an interpolated color between the two endpoints based on the weight.
static SkColor interpolate_endpoints(const SkColor endpoints[2], int weight) {
    return SkColorSetARGB(
        interpolate_channel(SkColorGetA(endpoints[0]), SkColorGetA(endpoints[1]), weight),
        interpolate_channel(SkColorGetR(endpoints[0]), SkColorGetR(endpoints[1]), weight),
        interpolate_channel(SkColorGetG(endpoints[0]), SkColorGetG(endpoints[1]), weight),
        interpolate_channel(SkColorGetB(endpoints[0]), SkColorGetB(endpoints[1]), weight));
}

// Returns an interpolated color between the two endpoints based on the weight.
// It uses separate weights for the channel depending on the value of the 'plane'
// variable. By default, all channels will use weight 0, and the value of plane
// means that weight1 will be used for:
// 0: red
// 1: green
// 2: blue
// 3: alpha
static SkColor interpolate_dual_endpoints(
    const SkColor endpoints[2], int weight0, int weight1, int plane) {
    int a = interpolate_channel(SkColorGetA(endpoints[0]), SkColorGetA(endpoints[1]), weight0);
    int r = interpolate_channel(SkColorGetR(endpoints[0]), SkColorGetR(endpoints[1]), weight0);
    int g = interpolate_channel(SkColorGetG(endpoints[0]), SkColorGetG(endpoints[1]), weight0);
    int b = interpolate_channel(SkColorGetB(endpoints[0]), SkColorGetB(endpoints[1]), weight0);

    switch (plane) {

        case 0:
            r = interpolate_channel(
                SkColorGetR(endpoints[0]), SkColorGetR(endpoints[1]), weight1);
            break;

        case 1:
            g = interpolate_channel(
                SkColorGetG(endpoints[0]), SkColorGetG(endpoints[1]), weight1);
            break;

        case 2:
            b = interpolate_channel(
                SkColorGetB(endpoints[0]), SkColorGetB(endpoints[1]), weight1);
            break;

        case 3:
            a = interpolate_channel(
                SkColorGetA(endpoints[0]), SkColorGetA(endpoints[1]), weight1);
            break;

        default:
            SkDEBUGFAIL("Plane should be 0-3");
            break;
    }

    return SkColorSetARGB(a, r, g, b);
}

// A struct of decoded values that we use to carry around information
// about the block. dimX and dimY are the dimension in texels of the block,
// for which there is only a limited subset of valid values:
//
// 4x4, 5x4, 5x5, 6x5, 6x6, 8x5, 8x6, 8x8, 10x5, 10x6, 10x8, 10x10, 12x10, 12x12

struct ASTCDecompressionData {
    ASTCDecompressionData(int dimX, int dimY) : fDimX(dimX), fDimY(dimY) { }
    const int   fDimX;      // the X dimension of the decompressed block
    const int   fDimY;      // the Y dimension of the decompressed block
    ASTCBlock   fBlock;     // the block data
    int         fBlockMode; // the block header that contains the block mode.

    bool fDualPlaneEnabled; // is this block compressing dual weight planes?
    int  fDualPlane;        // the independent plane in dual plane mode.

    bool fVoidExtent;       // is this block a single color?
    bool fError;            // does this block have an error encoding?

    int  fWeightDimX;       // the x dimension of the weight grid
    int  fWeightDimY;       // the y dimension of the weight grid

    int  fWeightBits;       // the number of bits used for each weight value
    int  fWeightTrits;      // the number of trits used for each weight value
    int  fWeightQuints;     // the number of quints used for each weight value

    int  fPartCount;        // the number of partitions in this block
    int  fPartIndex;        // the partition index: only relevant if fPartCount > 0

    // CEM values can be anything in the range 0-15, and each corresponds to a different
    // mode that represents the color data. We only support LDR modes.
    enum ColorEndpointMode {
        kLDR_Luminance_Direct_ColorEndpointMode          = 0,
        kLDR_Luminance_BaseOffset_ColorEndpointMode      = 1,
        kHDR_Luminance_LargeRange_ColorEndpointMode      = 2,
        kHDR_Luminance_SmallRange_ColorEndpointMode      = 3,
        kLDR_LuminanceAlpha_Direct_ColorEndpointMode     = 4,
        kLDR_LuminanceAlpha_BaseOffset_ColorEndpointMode = 5,
        kLDR_RGB_BaseScale_ColorEndpointMode             = 6,
        kHDR_RGB_BaseScale_ColorEndpointMode             = 7,
        kLDR_RGB_Direct_ColorEndpointMode                = 8,
        kLDR_RGB_BaseOffset_ColorEndpointMode            = 9,
        kLDR_RGB_BaseScaleWithAlpha_ColorEndpointMode    = 10,
        kHDR_RGB_ColorEndpointMode                       = 11,
        kLDR_RGBA_Direct_ColorEndpointMode               = 12,
        kLDR_RGBA_BaseOffset_ColorEndpointMode           = 13,
        kHDR_RGB_LDRAlpha_ColorEndpointMode              = 14,
        kHDR_RGB_HDRAlpha_ColorEndpointMode              = 15
    };
    static const int kMaxColorEndpointModes = 16;

    // the color endpoint modes for this block.
    static const int kMaxPartitions = 4;
    ColorEndpointMode fCEM[kMaxPartitions];

    int  fColorStartBit;    // The bit position of the first bit of the color data
    int  fColorEndBit;      // The bit position of the last *possible* bit of the color data

    // Returns the number of partitions for this block.
    int numPartitions() const {
        return fPartCount;
    }

    // Returns the total number of weight values that are stored in this block
    int numWeights() const {
        return fWeightDimX * fWeightDimY * (fDualPlaneEnabled ? 2 : 1);
    }

#ifdef SK_DEBUG
    // Returns the maximum value that any weight can take. We really only use
    // this function for debugging.
    int maxWeightValue() const {
        int maxVal = (1 << fWeightBits);
        if (fWeightTrits > 0) {
            SkASSERT(0 == fWeightQuints);
            maxVal *= 3;
        } else if (fWeightQuints > 0) {
            SkASSERT(0 == fWeightTrits);
            maxVal *= 5;
        }
        return maxVal - 1;
    }
#endif

    // The number of bits needed to represent the texel weight data. This
    // comes from the 'data size determination' section of the ASTC spec (C.2.22)
    int numWeightBits() const {
        const int nWeights = this->numWeights();
        return
            ((nWeights*8*fWeightTrits + 4) / 5) +
            ((nWeights*7*fWeightQuints + 2) / 3) +
            (nWeights*fWeightBits);
    }

    // Returns the number of color values stored in this block. The number of
    // values stored is directly a function of the color endpoint modes.
    int numColorValues() const {
        int numValues = 0;
        for (int i = 0; i < this->numPartitions(); ++i) {
            int cemInt = static_cast<int>(fCEM[i]);
            numValues += ((cemInt >> 2) + 1) * 2;
        }

        return numValues;
    }

    // Figures out the number of bits available for color values, and fills
    // in the maximum encoding that will fit the number of color values that
    // we need. Returns false on error. (See section C.2.22 of the spec)
    bool getColorValueEncoding(int *nBits, int *nTrits, int *nQuints) const {
        if (nullptr == nBits || nullptr == nTrits || nullptr == nQuints) {
            return false;
        }

        const int nColorVals = this->numColorValues();
        if (nColorVals <= 0) {
            return false;
        }

        const int colorBits = fColorEndBit - fColorStartBit;
        SkASSERT(colorBits > 0);

        // This is the minimum amount of accuracy required by the spec.
        if (colorBits < ((13 * nColorVals + 4) / 5)) {
            return false;
        }

        // Values can be represented as at most 8-bit values.
        // !SPEED! place this in a lookup table based on colorBits and nColorVals
        for (int i = 255; i > 0; --i) {
            int range = i + 1;
            int bits = 0, trits = 0, quints = 0;
            bool valid = false;
            if (SkIsPow2(range)) {
                bits = bits_for_range(range);
                valid = true;
            } else if ((range % 3) == 0 && SkIsPow2(range/3)) {
                trits = 1;
                bits = bits_for_range(range/3);
                valid = true;
            } else if ((range % 5) == 0 && SkIsPow2(range/5)) {
                quints = 1;
                bits = bits_for_range(range/5);
                valid = true;
            }

            if (valid) {
                const int actualColorBits =
                    ((nColorVals*8*trits + 4) / 5) +
                    ((nColorVals*7*quints + 2) / 3) +
                    (nColorVals*bits);
                if (actualColorBits <= colorBits) {
                    *nTrits = trits;
                    *nQuints = quints;
                    *nBits = bits;
                    return true;
                }
            }
        }

        return false;
    }

    // Converts the sequence of color values into endpoints. The algorithm here
    // corresponds to the values determined by section C.2.14 of the ASTC spec
    void colorEndpoints(SkColor endpoints[4][2], const int* colorValues) const {
        for (int i = 0; i < this->numPartitions(); ++i) {
            switch (fCEM[i]) {
                case kLDR_Luminance_Direct_ColorEndpointMode: {
                    const int* v = colorValues;
                    endpoints[i][0] = SkColorSetARGB(0xFF, v[0], v[0], v[0]);
                    endpoints[i][1] = SkColorSetARGB(0xFF, v[1], v[1], v[1]);

                    colorValues += 2;
                }
                break;

                case kLDR_Luminance_BaseOffset_ColorEndpointMode: {
                    const int* v = colorValues;
                    const int L0 = (v[0] >> 2) | (v[1] & 0xC0);
                    const int L1 = clamp_byte(L0 + (v[1] & 0x3F));

                    endpoints[i][0] = SkColorSetARGB(0xFF, L0, L0, L0);
                    endpoints[i][1] = SkColorSetARGB(0xFF, L1, L1, L1);

                    colorValues += 2;
                }
                break;

                case kLDR_LuminanceAlpha_Direct_ColorEndpointMode: {
                    const int* v = colorValues;

                    endpoints[i][0] = SkColorSetARGB(v[2], v[0], v[0], v[0]);
                    endpoints[i][1] = SkColorSetARGB(v[3], v[1], v[1], v[1]);

                    colorValues += 4;
                }
                break;

                case kLDR_LuminanceAlpha_BaseOffset_ColorEndpointMode: {
                    int v0 = colorValues[0];
                    int v1 = colorValues[1];
                    int v2 = colorValues[2];
                    int v3 = colorValues[3];

                    bit_transfer_signed(&v1, &v0);
                    bit_transfer_signed(&v3, &v2);

                    endpoints[i][0] = SkColorSetARGB(v2, v0, v0, v0);
                    endpoints[i][1] = SkColorSetARGB(
                        clamp_byte(v3+v2),
                        clamp_byte(v1+v0),
                        clamp_byte(v1+v0),
                        clamp_byte(v1+v0));

                    colorValues += 4;
                }
                break;

                case kLDR_RGB_BaseScale_ColorEndpointMode: {
                    decode_rgba_basescale(colorValues, endpoints[i], true);
                    colorValues += 4;
                }
                break;

                case kLDR_RGB_Direct_ColorEndpointMode: {
                    decode_rgba_direct(colorValues, endpoints[i], true);
                    colorValues += 6;
                }
                break;

                case kLDR_RGB_BaseOffset_ColorEndpointMode: {
                    decode_rgba_baseoffset(colorValues, endpoints[i], true);
                    colorValues += 6;
                }
                break;

                case kLDR_RGB_BaseScaleWithAlpha_ColorEndpointMode: {
                    decode_rgba_basescale(colorValues, endpoints[i], false);
                    colorValues += 6;
                }
                break;

                case kLDR_RGBA_Direct_ColorEndpointMode: {
                    decode_rgba_direct(colorValues, endpoints[i], false);
                    colorValues += 8;
                }
                break;

                case kLDR_RGBA_BaseOffset_ColorEndpointMode: {
                    decode_rgba_baseoffset(colorValues, endpoints[i], false);
                    colorValues += 8;
                }
                break;

                default:
                    SkDEBUGFAIL("HDR mode unsupported! This should be caught sooner.");
                    break;
            }
        }
    }

    // Follows the procedure from section C.2.17 of the ASTC specification
    int unquantizeWeight(int x) const {
        SkASSERT(x <= this->maxWeightValue());

        const int D = (x >> fWeightBits) & 0x7;
        const int A = -(x & 0x1) & 0x7F;

        SkTBits<int> xbits(x);

        int T = 0;
        if (fWeightTrits > 0) {
            SkASSERT(0 == fWeightQuints);
            switch (fWeightBits) {
                case 0: {
                    // x is a single trit
                    SkASSERT(x < 3);

                    static const int kUnquantizationTable[3] = { 0, 32, 63 };
                    T = kUnquantizationTable[x];
                }
                break;

                case 1: {
                    const int B = 0;
                    const int C = 50;
                    T = unquantize_value(0x20, A, B, C, D);
                }
                break;

                case 2: {
                    const int b = xbits[1];
                    const int B = b | (b << 2) | (b << 6);
                    const int C = 23;
                    T = unquantize_value(0x20, A, B, C, D);
                }
                break;

                case 3: {
                    const int cb = xbits(2, 1);
                    const int B = cb | (cb << 5);
                    const int C = 11;
                    T = unquantize_value(0x20, A, B, C, D);
                }
                break;

                default:
                    SkDEBUGFAIL("Too many bits for trit encoding");
                    break;
            }

        } else if (fWeightQuints > 0) {
            SkASSERT(0 == fWeightTrits);
            switch (fWeightBits) {
                case 0: {
                    // x is a single quint
                    SkASSERT(x < 5);

                    static const int kUnquantizationTable[5] = { 0, 16, 32, 47, 63 };
                    T = kUnquantizationTable[x];
                }
                break;

                case 1: {
                    const int B = 0;
                    const int C = 28;
                    T = unquantize_value(0x20, A, B, C, D);
                }
                break;

                case 2: {
                    const int b = xbits[1];
                    const int B = (b << 1) | (b << 6);
                    const int C = 13;
                    T = unquantize_value(0x20, A, B, C, D);
                }
                break;

                default:
                    SkDEBUGFAIL("Too many bits for quint encoding");
                    break;
            }
        } else {
            SkASSERT(0 == fWeightTrits);
            SkASSERT(0 == fWeightQuints);

            T = replicate_bits(x, fWeightBits, 6);
        }

        // This should bring the value within [0, 63]..
        SkASSERT(T <= 63);

        if (T > 32) {
            T += 1;
        }

        SkASSERT(T <= 64);

        return T;
    }

    // Returns the weight at the associated index. If the index is out of bounds, it
    // returns zero. It also chooses the weight appropriately based on the given dual
    // plane.
    int getWeight(const int* unquantizedWeights, int idx, bool dualPlane) const {
        const int maxIdx = (fDualPlaneEnabled ? 2 : 1) * fWeightDimX * fWeightDimY - 1;
        if (fDualPlaneEnabled) {
            const int effectiveIdx = 2*idx + (dualPlane ? 1 : 0);
            if (effectiveIdx > maxIdx) {
                return 0;
            }
            return unquantizedWeights[effectiveIdx];
        }

        SkASSERT(!dualPlane);

        if (idx > maxIdx) {
            return 0;
        } else {
            return unquantizedWeights[idx];
        }
    }

    // This computes the effective weight at location (s, t) of the block. This
    // weight is computed by sampling the texel weight grid (it's usually not 1-1), and
    // then applying a bilerp. The algorithm outlined here follows the algorithm
    // defined in section C.2.18 of the ASTC spec.
    int infillWeight(const int* unquantizedValues, int s, int t, bool dualPlane) const {
        const int Ds = (1024 + fDimX/2) / (fDimX - 1);
        const int Dt = (1024 + fDimY/2) / (fDimY - 1);

        const int cs = Ds * s;
        const int ct = Dt * t;

        const int gs = (cs*(fWeightDimX - 1) + 32) >> 6;
        const int gt = (ct*(fWeightDimY - 1) + 32) >> 6;

        const int js = gs >> 4;
        const int jt = gt >> 4;

        const int fs = gs & 0xF;
        const int ft = gt & 0xF;

        const int idx = js + jt*fWeightDimX;
        const int p00 = this->getWeight(unquantizedValues, idx, dualPlane);
        const int p01 = this->getWeight(unquantizedValues, idx + 1, dualPlane);
        const int p10 = this->getWeight(unquantizedValues, idx + fWeightDimX, dualPlane);
        const int p11 = this->getWeight(unquantizedValues, idx + fWeightDimX + 1, dualPlane);

        const int w11 = (fs*ft + 8) >> 4;
        const int w10 = ft - w11;
        const int w01 = fs - w11;
        const int w00 = 16 - fs - ft + w11;

        const int weight = (p00*w00 + p01*w01 + p10*w10 + p11*w11 + 8) >> 4;
        SkASSERT(weight <= 64);
        return weight;
    }

    // Unquantizes the decoded texel weights as described in section C.2.17 of
    // the ASTC specification. Additionally, it populates texelWeights with
    // the expanded weight grid, which is computed according to section C.2.18
    void texelWeights(int texelWeights[2][12][12], const int* texelValues) const {
        // Unquantized texel weights...
        int unquantizedValues[144*2]; // 12x12 blocks with dual plane decoding...
        SkASSERT(this->numWeights() <= 144*2);

        // Unquantize the weights and cache them
        for (int j = 0; j < this->numWeights(); ++j) {
            unquantizedValues[j] = this->unquantizeWeight(texelValues[j]);
        }

        // Do weight infill...
        for (int y = 0; y < fDimY; ++y) {
            for (int x = 0; x < fDimX; ++x) {
                texelWeights[0][x][y] = this->infillWeight(unquantizedValues, x, y, false);
                if (fDualPlaneEnabled) {
                    texelWeights[1][x][y] = this->infillWeight(unquantizedValues, x, y, true);
                }
            }
        }
    }

    // Returns the partition for the texel located at position (x, y).
    // Adapted from C.2.21 of the ASTC specification
    int getPartition(int x, int y) const {
        const int partitionCount = this->numPartitions();
        int seed = fPartIndex;
        if ((fDimX * fDimY) < 31) {
            x <<= 1;
            y <<= 1;
        }

        seed += (partitionCount - 1) * 1024;

        uint32_t p = seed;
        p ^= p >> 15;  p -= p << 17;  p += p << 7; p += p <<  4;
        p ^= p >>  5;  p += p << 16;  p ^= p >> 7; p ^= p >> 3;
        p ^= p <<  6;  p ^= p >> 17;

        uint32_t rnum = p;
        uint8_t seed1  =  rnum        & 0xF;
        uint8_t seed2  = (rnum >>  4) & 0xF;
        uint8_t seed3  = (rnum >>  8) & 0xF;
        uint8_t seed4  = (rnum >> 12) & 0xF;
        uint8_t seed5  = (rnum >> 16) & 0xF;
        uint8_t seed6  = (rnum >> 20) & 0xF;
        uint8_t seed7  = (rnum >> 24) & 0xF;
        uint8_t seed8  = (rnum >> 28) & 0xF;
        uint8_t seed9  = (rnum >> 18) & 0xF;
        uint8_t seed10 = (rnum >> 22) & 0xF;
        uint8_t seed11 = (rnum >> 26) & 0xF;
        uint8_t seed12 = ((rnum >> 30) | (rnum << 2)) & 0xF;

        seed1 *= seed1;     seed2 *= seed2;
        seed3 *= seed3;     seed4 *= seed4;
        seed5 *= seed5;     seed6 *= seed6;
        seed7 *= seed7;     seed8 *= seed8;
        seed9 *= seed9;     seed10 *= seed10;
        seed11 *= seed11;   seed12 *= seed12;

        int sh1, sh2, sh3;
        if (0 != (seed & 1)) {
            sh1 = (0 != (seed & 2))? 4 : 5;
            sh2 = (partitionCount == 3)? 6 : 5;
        } else {
            sh1 = (partitionCount==3)? 6 : 5;
            sh2 = (0 != (seed & 2))? 4 : 5;
        }
        sh3 = (0 != (seed & 0x10))? sh1 : sh2;

        seed1 >>= sh1; seed2  >>= sh2; seed3  >>= sh1; seed4  >>= sh2;
        seed5 >>= sh1; seed6  >>= sh2; seed7  >>= sh1; seed8  >>= sh2;
        seed9 >>= sh3; seed10 >>= sh3; seed11 >>= sh3; seed12 >>= sh3;

        const int z = 0;
        int a = seed1*x + seed2*y + seed11*z + (rnum >> 14);
        int b = seed3*x + seed4*y + seed12*z + (rnum >> 10);
        int c = seed5*x + seed6*y + seed9 *z + (rnum >>  6);
        int d = seed7*x + seed8*y + seed10*z + (rnum >>  2);

        a &= 0x3F;
        b &= 0x3F;
        c &= 0x3F;
        d &= 0x3F;

        if (partitionCount < 4) {
            d = 0;
        }

        if (partitionCount < 3) {
            c = 0;
        }

        if (a >= b && a >= c && a >= d) {
            return 0;
        } else if (b >= c && b >= d) {
            return 1;
        } else if (c >= d) {
            return 2;
        } else {
            return 3;
        }
    }

    // Performs the proper interpolation of the texel based on the
    // endpoints and weights.
    SkColor getTexel(const SkColor endpoints[4][2],
                     const int weights[2][12][12],
                     int x, int y) const {
        int part = 0;
        if (this->numPartitions() > 1) {
            part = this->getPartition(x, y);
        }

        SkColor result;
        if (fDualPlaneEnabled) {
            result = interpolate_dual_endpoints(
                endpoints[part], weights[0][x][y], weights[1][x][y], fDualPlane);
        } else {
            result = interpolate_endpoints(endpoints[part], weights[0][x][y]);
        }

#if 1
        // !FIXME! if we're writing directly to a bitmap, then we don't need
        // to swap the red and blue channels, but since we're usually being used
        // by the SkImageDecoder_astc module, the results are expected to be in RGBA.
        result = SkColorSetARGB(
            SkColorGetA(result), SkColorGetB(result), SkColorGetG(result), SkColorGetR(result));
#endif

        return result;
    }

    void decode() {
        // First decode the block mode.
        this->decodeBlockMode();

        // Now we can decode the partition information.
        fPartIndex = static_cast<int>(read_astc_bits(fBlock, 11, 23));
        fPartCount = (fPartIndex & 0x3) + 1;
        fPartIndex >>= 2;

        // This is illegal
        if (fDualPlaneEnabled && this->numPartitions() == 4) {
            fError = true;
            return;
        }

        // Based on the partition info, we can decode the color information.
        this->decodeColorData();
    }

    // Decodes the dual plane based on the given bit location. The final
    // location, if the dual plane is enabled, is also the end of our color data.
    // This function is only meant to be used from this->decodeColorData()
    void decodeDualPlane(int bitLoc) {
        if (fDualPlaneEnabled) {
            fDualPlane = static_cast<int>(read_astc_bits(fBlock, bitLoc - 2, bitLoc));
            fColorEndBit = bitLoc - 2;
        } else {
            fColorEndBit = bitLoc;
        }
    }

    // Decodes the color information based on the ASTC spec.
    void decodeColorData() {

        // By default, the last color bit is at the end of the texel weights
        const int lastWeight = 128 - this->numWeightBits();

        // If we have a dual plane then it will be at this location, too.
        int dualPlaneBitLoc = lastWeight;

        // If there's only one partition, then our job is (relatively) easy.
        if (this->numPartitions() == 1) {
            fCEM[0] = static_cast<ColorEndpointMode>(read_astc_bits(fBlock, 13, 17));
            fColorStartBit = 17;

            // Handle dual plane mode...
            this->decodeDualPlane(dualPlaneBitLoc);

            return;
        }

        // If we have more than one partition, then we need to make
        // room for the partition index.
        fColorStartBit = 29;

        // Read the base CEM. If it's zero, then we have no additional
        // CEM data and the endpoints for each partition share the same CEM.
        const int baseCEM = static_cast<int>(read_astc_bits(fBlock, 23, 25));
        if (0 == baseCEM) {

            const ColorEndpointMode sameCEM =
                static_cast<ColorEndpointMode>(read_astc_bits(fBlock, 25, 29));

            for (int i = 0; i < kMaxPartitions; ++i) {
                fCEM[i] = sameCEM;
            }

            // Handle dual plane mode...
            this->decodeDualPlane(dualPlaneBitLoc);

            return;
        }

        // Move the dual plane selector bits down based on how many
        // partitions the block contains.
        switch (this->numPartitions()) {
            case 2:
                dualPlaneBitLoc -= 2;
                break;

            case 3:
                dualPlaneBitLoc -= 5;
                break;

            case 4:
                dualPlaneBitLoc -= 8;
                break;

            default:
                SkDEBUGFAIL("Internal ASTC decoding error.");
                break;
        }

        // The rest of the CEM config will be between the dual plane bit selector
        // and the texel weight grid.
        const int lowCEM = static_cast<int>(read_astc_bits(fBlock, 23, 29));
        SkASSERT(lastWeight >= dualPlaneBitLoc);
        SkASSERT(lastWeight - dualPlaneBitLoc < 31);
        int fullCEM = static_cast<int>(read_astc_bits(fBlock, dualPlaneBitLoc, lastWeight));

        // Attach the config at the end of the weight grid to the CEM values
        // in the beginning of the block.
        fullCEM = (fullCEM << 6) | lowCEM;

        // Ignore the two least significant bits, since those are our baseCEM above.
        fullCEM = fullCEM >> 2;

        int C[kMaxPartitions]; // Next, decode C and M from the spec (Table C.2.12)
        for (int i = 0; i < this->numPartitions(); ++i) {
            C[i] = fullCEM & 1;
            fullCEM = fullCEM >> 1;
        }

        int M[kMaxPartitions];
        for (int i = 0; i < this->numPartitions(); ++i) {
            M[i] = fullCEM & 0x3;
            fullCEM = fullCEM >> 2;
        }

        // Construct our CEMs..
        SkASSERT(baseCEM > 0);
        for (int i = 0; i < this->numPartitions(); ++i) {
            int cem = (baseCEM - 1) * 4;
            cem += (0 == C[i])? 0 : 4;
            cem += M[i];

            SkASSERT(cem < 16);
            fCEM[i] = static_cast<ColorEndpointMode>(cem);
        }

        // Finally, if we have dual plane mode, then read the plane selector.
        this->decodeDualPlane(dualPlaneBitLoc);
    }

    // Decodes the block mode. This function determines whether or not we use
    // dual plane encoding, the size of the texel weight grid, and the number of
    // bits, trits and quints that are used to encode it. For more information,
    // see section C.2.10 of the ASTC spec.
    //
    // For 2D blocks, the Block Mode field is laid out as follows:
    //
    // -------------------------------------------------------------------------
    // 10  9   8   7   6   5   4   3   2   1   0   Width Height Notes
    // -------------------------------------------------------------------------
    // D   H     B       A     R0  0   0   R2  R1  B+4   A+2
    // D   H     B       A     R0  0   1   R2  R1  B+8   A+2
    // D   H     B       A     R0  1   0   R2  R1  A+2   B+8
    // D   H   0   B     A     R0  1   1   R2  R1  A+2   B+6
    // D   H   1   B     A     R0  1   1   R2  R1  B+2   A+2
    // D   H   0   0     A     R0  R2  R1  0   0   12    A+2
    // D   H   0   1     A     R0  R2  R1  0   0   A+2   12
    // D   H   1   1   0   0   R0  R2  R1  0   0   6     10
    // D   H   1   1   0   1   R0  R2  R1  0   0   10    6
    //   B     1   0     A     R0  R2  R1  0   0   A+6   B+6   D=0, H=0
    // x   x   1   1   1   1   1   1   1   0   0   -     -     Void-extent
    // x   x   1   1   1   x   x   x   x   0   0   -     -     Reserved*
    // x   x   x   x   x   x   x   0   0   0   0   -     -     Reserved
    // -------------------------------------------------------------------------
    //
    // D - dual plane enabled
    // H, R - used to determine the number of bits/trits/quints in texel weight encoding
    //        R is a three bit value whose LSB is R0 and MSB is R1
    // Width, Height - dimensions of the texel weight grid (determined by A and B)

    void decodeBlockMode() {
        const int blockMode = static_cast<int>(read_astc_bits(fBlock, 0, 11));

        // Check for special void extent encoding
        fVoidExtent = (blockMode & 0x1FF) == 0x1FC;

        // Check for reserved block modes
        fError = ((blockMode & 0x1C3) == 0x1C0) || ((blockMode & 0xF) == 0);

        // Neither reserved nor void-extent, decode as usual
        // This code corresponds to table C.2.8 of the ASTC spec
        bool highPrecision = false;
        int R = 0;
        if ((blockMode & 0x3) == 0) {
            R = ((0xC & blockMode) >> 1) | ((0x10 & blockMode) >> 4);
            const int bitsSevenAndEight = (blockMode & 0x180) >> 7;
            SkASSERT(0 <= bitsSevenAndEight && bitsSevenAndEight < 4);

            const int A = (blockMode >> 5) & 0x3;
            const int B = (blockMode >> 9) & 0x3;

            fDualPlaneEnabled = (blockMode >> 10) & 0x1;
            highPrecision = (blockMode >> 9) & 0x1;

            switch (bitsSevenAndEight) {
                default:
                case 0:
                    fWeightDimX = 12;
                    fWeightDimY = A + 2;
                    break;

                case 1:
                    fWeightDimX = A + 2;
                    fWeightDimY = 12;
                    break;

                case 2:
                    fWeightDimX = A + 6;
                    fWeightDimY = B + 6;
                    fDualPlaneEnabled = false;
                    highPrecision = false;
                    break;

                case 3:
                    if (0 == A) {
                        fWeightDimX = 6;
                        fWeightDimY = 10;
                    } else {
                        fWeightDimX = 10;
                        fWeightDimY = 6;
                    }
                    break;
            }
        } else { // (blockMode & 0x3) != 0
            R = ((blockMode & 0x3) << 1) | ((blockMode & 0x10) >> 4);

            const int bitsTwoAndThree = (blockMode >> 2) & 0x3;
            SkASSERT(0 <= bitsTwoAndThree && bitsTwoAndThree < 4);

            const int A = (blockMode >> 5) & 0x3;
            const int B = (blockMode >> 7) & 0x3;

            fDualPlaneEnabled = (blockMode >> 10) & 0x1;
            highPrecision = (blockMode >> 9) & 0x1;

            switch (bitsTwoAndThree) {
                case 0:
                    fWeightDimX = B + 4;
                    fWeightDimY = A + 2;
                    break;
                case 1:
                    fWeightDimX = B + 8;
                    fWeightDimY = A + 2;
                    break;
                case 2:
                    fWeightDimX = A + 2;
                    fWeightDimY = B + 8;
                    break;
                case 3:
                    if ((B & 0x2) == 0) {
                        fWeightDimX = A + 2;
                        fWeightDimY = (B & 1) + 6;
                    } else {
                        fWeightDimX = (B & 1) + 2;
                        fWeightDimY = A + 2;
                    }
                    break;
            }
        }

        // We should have set the values of R and highPrecision
        // from decoding the block mode, these are used to determine
        // the proper dimensions of our weight grid.
        if ((R & 0x6) == 0) {
            fError = true;
        } else {
            static const int kBitAllocationTable[2][6][3] = {
                {
                    {  1, 0, 0 },
                    {  0, 1, 0 },
                    {  2, 0, 0 },
                    {  0, 0, 1 },
                    {  1, 1, 0 },
                    {  3, 0, 0 }
                },
                {
                    {  1, 0, 1 },
                    {  2, 1, 0 },
                    {  4, 0, 0 },
                    {  2, 0, 1 },
                    {  3, 1, 0 },
                    {  5, 0, 0 }
                }
            };

            fWeightBits = kBitAllocationTable[highPrecision][R - 2][0];
            fWeightTrits = kBitAllocationTable[highPrecision][R - 2][1];
            fWeightQuints = kBitAllocationTable[highPrecision][R - 2][2];
        }
    }
};

// Reads an ASTC block from the given pointer.
static inline void read_astc_block(ASTCDecompressionData *dst, const uint8_t* src) {
    const uint64_t* qword = reinterpret_cast<const uint64_t*>(src);
    dst->fBlock.fLow = SkEndian_SwapLE64(qword[0]);
    dst->fBlock.fHigh = SkEndian_SwapLE64(qword[1]);
    dst->decode();
}

// Take a known void-extent block, and write out the values as a constant color.
static void decompress_void_extent(uint8_t* dst, int dstRowBytes,
                                   const ASTCDecompressionData &data) {
    // The top 64 bits contain 4 16-bit RGBA values.
    int a = (static_cast<int>(read_astc_bits(data.fBlock, 112, 128)) + 255) >> 8;
    int b = (static_cast<int>(read_astc_bits(data.fBlock, 96, 112)) + 255) >> 8;
    int g = (static_cast<int>(read_astc_bits(data.fBlock, 80, 96)) + 255) >> 8;
    int r = (static_cast<int>(read_astc_bits(data.fBlock, 64, 80)) + 255) >> 8;

    write_constant_color(dst, data.fDimX, data.fDimY, dstRowBytes, SkColorSetARGB(a, r, g, b));
}

// Decompresses a single ASTC block. It's assumed that data.fDimX and data.fDimY are
// set and that the block has already been decoded (i.e. data.decode() has been called)
static void decompress_astc_block(uint8_t* dst, int dstRowBytes,
                                  const ASTCDecompressionData &data) {
    if (data.fError) {
        write_error_color(dst, data.fDimX, data.fDimY, dstRowBytes);
        return;
    }

    if (data.fVoidExtent) {
        decompress_void_extent(dst, dstRowBytes, data);
        return;
    }

    // According to the spec, any more than 64 values is illegal. (C.2.24)
    static const int kMaxTexelValues = 64;

    // Decode the texel weights.
    int texelValues[kMaxTexelValues];
    bool success = decode_integer_sequence(
        texelValues, kMaxTexelValues, data.numWeights(),
        // texel data goes to the end of the 128 bit block.
        data.fBlock, 128, 128 - data.numWeightBits(), false,
        data.fWeightBits, data.fWeightTrits, data.fWeightQuints);

    if (!success) {
        write_error_color(dst, data.fDimX, data.fDimY, dstRowBytes);
        return;
    }

    // Decode the color endpoints
    int colorBits, colorTrits, colorQuints;
    if (!data.getColorValueEncoding(&colorBits, &colorTrits, &colorQuints)) {
        write_error_color(dst, data.fDimX, data.fDimY, dstRowBytes);
        return;
    }

    // According to the spec, any more than 18 color values is illegal. (C.2.24)
    static const int kMaxColorValues = 18;

    int colorValues[kMaxColorValues];
    success = decode_integer_sequence(
        colorValues, kMaxColorValues, data.numColorValues(),
        data.fBlock, data.fColorStartBit, data.fColorEndBit, true,
        colorBits, colorTrits, colorQuints);

    if (!success) {
        write_error_color(dst, data.fDimX, data.fDimY, dstRowBytes);
        return;
    }

    // Unquantize the color values after they've been decoded.
    unquantize_colors(colorValues, data.numColorValues(), colorBits, colorTrits, colorQuints);

    // Decode the colors into the appropriate endpoints.
    SkColor endpoints[4][2];
    data.colorEndpoints(endpoints, colorValues);

    // Do texel infill and decode the texel values.
    int texelWeights[2][12][12];
    data.texelWeights(texelWeights, texelValues);

    // Write the texels by interpolating them based on the information
    // stored in the block.
    dst += data.fDimY * dstRowBytes;
    for (int y = 0; y < data.fDimY; ++y) {
        dst -= dstRowBytes;
        SkColor* colorPtr = reinterpret_cast<SkColor*>(dst);
        for (int x = 0; x < data.fDimX; ++x) {
            colorPtr[x] = data.getTexel(endpoints, texelWeights, x, y);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// ASTC Comrpession Struct
//
////////////////////////////////////////////////////////////////////////////////

// This is the type passed as the CompressorType argument of the compressed
// blitter for the ASTC format. The static functions required to be in this
// struct are documented in SkTextureCompressor_Blitter.h
struct CompressorASTC {
    static inline void CompressA8Vertical(uint8_t* dst, const uint8_t* src) {
        compress_a8_astc_block<GetAlphaTranspose>(&dst, src, 12);
    }

    static inline void CompressA8Horizontal(uint8_t* dst, const uint8_t* src,
                                            int srcRowBytes) {
        compress_a8_astc_block<GetAlpha>(&dst, src, srcRowBytes);
    }

#if PEDANTIC_BLIT_RECT
    static inline void UpdateBlock(uint8_t* dst, const uint8_t* src, int srcRowBytes,
                                   const uint8_t* mask) {
        // TODO: krajcevski
        // This is kind of difficult for ASTC because the weight values are calculated
        // as an average of the actual weights. The best we can do is decompress the
        // weights and recalculate them based on the new texel values. This should
        // be "not too bad" since we know that anytime we hit this function, we're
        // compressing 12x12 block dimension alpha-only, and we know the layout
        // of the block
        SkFAIL("Implement me!");
    }
#endif
};

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

bool CompressA8To12x12ASTC(uint8_t* dst, const uint8_t* src,
                           int width, int height, size_t rowBytes) {
    if (width < 0 || ((width % 12) != 0) || height < 0 || ((height % 12) != 0)) {
        return false;
    }

    uint8_t** dstPtr = &dst;
    for (int y = 0; y < height; y += 12) {
        for (int x = 0; x < width; x += 12) {
            compress_a8_astc_block<GetAlpha>(dstPtr, src + y*rowBytes + x, rowBytes);
        }
    }

    return true;
}

SkBlitter* CreateASTCBlitter(int width, int height, void* outputBuffer,
                             SkTBlitterAllocator* allocator) {
    if ((width % 12) != 0 || (height % 12) != 0) {
        return nullptr;
    }

    // Memset the output buffer to an encoding that decodes to zero. We must do this
    // in order to avoid having uninitialized values in the buffer if the blitter
    // decides not to write certain scanlines (and skip entire rows of blocks).
    // In the case of ASTC, if everything index is zero, then the interpolated value
    // will decode to zero provided we have the right header. We use the encoding
    // from recognizing all zero blocks from above.
    const int nBlocks = (width * height / 144);
    uint8_t *dst = reinterpret_cast<uint8_t *>(outputBuffer);
    for (int i = 0; i < nBlocks; ++i) {
        send_packing(&dst, SkTEndian_SwapLE64(0x0000000001FE000173ULL), 0);
    }

    return allocator->createT<
        SkTCompressedAlphaBlitter<12, 16, CompressorASTC>, int, int, void* >
        (width, height, outputBuffer);
}

void DecompressASTC(uint8_t* dst, int dstRowBytes, const uint8_t* src,
                    int width, int height, int blockDimX, int blockDimY) {
    // ASTC is encoded in what they call "raster order", so that the first
    // block is the bottom-left block in the image, and the first pixel
    // is the bottom-left pixel of the image
    dst += height * dstRowBytes;

    ASTCDecompressionData data(blockDimX, blockDimY);
    for (int y = 0; y < height; y += blockDimY) {
        dst -= blockDimY * dstRowBytes;
        SkColor *colorPtr = reinterpret_cast<SkColor*>(dst);
        for (int x = 0; x < width; x += blockDimX) {
            read_astc_block(&data, src);
            decompress_astc_block(reinterpret_cast<uint8_t*>(colorPtr + x), dstRowBytes, data);

            // ASTC encoded blocks are 16 bytes (128 bits) large.
            src += 16;
        }
    }
}

}  // SkTextureCompressor
