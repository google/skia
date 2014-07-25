/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor_ASTC.h"

#include "SkBlitter.h"
#include "SkEndian.h"

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
static inline uint8_t get_alpha(const uint8_t *src, int rowBytes, int x, int y) {
    SkASSERT(x >= 0 && x < 12);
    SkASSERT(y >= 0 && y < 12);
    SkASSERT(rowBytes >= 12);
    return *(src + y*rowBytes + x);
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
static void compress_a8_astc_block(uint8_t** dst, const uint8_t* src, int rowBytes) {
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
                alphaTot += weight * get_alpha(src, rowBytes, x, y);
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

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

bool CompressA8To12x12ASTC(uint8_t* dst, const uint8_t* src, int width, int height, int rowBytes) {
    if (width < 0 || ((width % 12) != 0) || height < 0 || ((height % 12) != 0)) {
        return false;
    }

    uint8_t** dstPtr = &dst;
    for (int y = 0; y < height; y+=12) {
        for (int x = 0; x < width; x+=12) {
            compress_a8_astc_block(dstPtr, src + y*rowBytes + x, rowBytes);
        }
    }

    return true;
}

SkBlitter* CreateASTCBlitter(int width, int height, void* outputBuffer) {
    // TODO (krajcevski)
    return NULL;
}

}  // SkTextureCompressor
