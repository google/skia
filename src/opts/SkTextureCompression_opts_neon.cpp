/*
 * Copyright 2014
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor.h"
#include "SkTextureCompression_opts.h"

#include <arm_neon.h>

// Converts indices in each of the four bits of the register from
// 0, 1, 2, 3, 4, 5, 6, 7
// to
// 3, 2, 1, 0, 4, 5, 6, 7
//
// A more detailed explanation can be found in SkTextureCompressor::convert_indices
static inline uint8x16_t convert_indices(const uint8x16_t &x) {
    static const int8x16_t kThree = {
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    };

    static const int8x16_t kZero = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    
    // Take top three bits
    int8x16_t sx = vreinterpretq_s8_u8(x);

    // Negate ...
    sx = vnegq_s8(sx);

    // Add three...
    sx = vaddq_s8(sx, kThree);

    // Generate negatives mask
    const int8x16_t mask = vreinterpretq_s8_u8(vcltq_s8(sx, kZero));

    // Absolute value
    sx = vabsq_s8(sx);

    // Add three to the values that were negative...
    return vreinterpretq_u8_s8(vaddq_s8(sx, vandq_s8(mask, kThree)));
}

template<unsigned shift>
static inline uint64x2_t shift_swap(const uint64x2_t &x, const uint64x2_t &mask) {
    uint64x2_t t = vandq_u64(mask, veorq_u64(x, vshrq_n_u64(x, shift)));
    return veorq_u64(x, veorq_u64(t, vshlq_n_u64(t, shift)));
}

static inline uint64x2_t pack_indices(const uint64x2_t &x) {
    // x: 00 a e 00 b f 00 c g 00 d h 00 i m 00 j n 00 k o 00 l p

    static const uint64x2_t kMask1 = { 0x3FC0003FC00000ULL, 0x3FC0003FC00000ULL };
    uint64x2_t ret = shift_swap<10>(x, kMask1);

    // x: b f 00 00 00 a e c g i m 00 00 00 d h j n 00 k o 00 l p
    static const uint64x2_t kMask2 = { (0x3FULL << 52), (0x3FULL << 52) };
    static const uint64x2_t kMask3 = { (0x3FULL << 28), (0x3FULL << 28) };
    const uint64x2_t x1 = vandq_u64(vshlq_n_u64(ret, 52), kMask2);
    const uint64x2_t x2 = vandq_u64(vshlq_n_u64(ret, 20), kMask3);
    ret = vshrq_n_u64(vorrq_u64(ret, vorrq_u64(x1, x2)), 16);

    // x: 00 00 00 00 00 00 00 00 b f l p a e c g i m k o d h j n

    static const uint64x2_t kMask4 = { 0xFC0000ULL, 0xFC0000ULL };
    ret = shift_swap<6>(ret, kMask4);

#if defined (SK_CPU_BENDIAN)
    // x: 00 00 00 00 00 00 00 00 b f l p a e i m c g k o d h j n

    static const uint64x2_t kMask5 = { 0x3FULL, 0x3FULL };
    ret = shift_swap<36>(ret, kMask5);

    // x: 00 00 00 00 00 00 00 00 b f j n a e i m c g k o d h l p

    static const uint64x2_t kMask6 = { 0xFFF000000ULL, 0xFFF000000ULL };
    ret = shift_swap<12>(ret, kMask6);
#else
    // x: 00 00 00 00 00 00 00 00 c g i m d h l p b f j n a e k o

    static const uint64x2_t kMask5 = { 0xFC0ULL, 0xFC0ULL };
    ret = shift_swap<36>(ret, kMask5);

    // x: 00 00 00 00 00 00 00 00 a e i m d h l p b f j n c g k o

    static const uint64x2_t kMask6 = { (0xFFFULL << 36), (0xFFFULL << 36) };
    static const uint64x2_t kMask7 = { 0xFFFFFFULL, 0xFFFFFFULL };
    static const uint64x2_t kMask8 = { 0xFFFULL, 0xFFFULL };
    const uint64x2_t y1 = vandq_u64(ret, kMask6);
    const uint64x2_t y2 = vshlq_n_u64(vandq_u64(ret, kMask7), 12);
    const uint64x2_t y3 = vandq_u64(vshrq_n_u64(ret, 24), kMask8);
    ret = vorrq_u64(y1, vorrq_u64(y2, y3));
#endif

    // x: 00 00 00 00 00 00 00 00 a e i m b f j n c g k o d h l p

    // Set the header
    static const uint64x2_t kHeader = { 0x8490000000000000ULL, 0x8490000000000000ULL };
    return vorrq_u64(kHeader, ret);
}

// Takes a row of alpha values and places the most significant three bits of each byte into
// the least significant bits of the same byte
static inline uint8x16_t make_index_row(const uint8x16_t &x) {
    static const uint8x16_t kTopThreeMask = {
        0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
        0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
    };
    return vshrq_n_u8(vandq_u8(x, kTopThreeMask), 5);
}

// Returns true if all of the bits in x are 0.
static inline bool is_zero(uint8x16_t x) {
// First experiments say that this is way slower than just examining the lanes
// but it might need a little more investigation.
#if 0
    // This code path tests the system register for overflow. We trigger
    // overflow by adding x to a register with all of its bits set. The
    // first instruction sets the bits.
    int reg;
    asm ("VTST.8   %%q0, %q1, %q1\n"
         "VQADD.u8 %q1, %%q0\n"
         "VMRS     %0, FPSCR\n"
         : "=r"(reg) : "w"(vreinterpretq_f32_u8(x)) : "q0", "q1");

    // Bit 21 corresponds to the overflow flag.
    return reg & (0x1 << 21);
#else
    const uint64x2_t cvt = vreinterpretq_u64_u8(x);
    const uint64_t l1 = vgetq_lane_u64(cvt, 0);
    return (l1 == 0) && (l1 == vgetq_lane_u64(cvt, 1));
#endif
}

#if defined (SK_CPU_BENDIAN)
static inline uint64x2_t fix_endianness(uint64x2_t x) {
    return x;
}
#else
static inline uint64x2_t fix_endianness(uint64x2_t x) {
    return vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(x)));
}
#endif

static void compress_r11eac_blocks(uint64_t* dst, const uint8_t* src, int rowBytes) {

    // Try to avoid switching between vector and non-vector ops...
    const uint8_t *const src1 = src;
    const uint8_t *const src2 = src + rowBytes;
    const uint8_t *const src3 = src + 2*rowBytes;
    const uint8_t *const src4 = src + 3*rowBytes;
    uint64_t *const dst1 = dst;
    uint64_t *const dst2 = dst + 2;

    const uint8x16_t alphaRow1 = vld1q_u8(src1);
    const uint8x16_t alphaRow2 = vld1q_u8(src2);
    const uint8x16_t alphaRow3 = vld1q_u8(src3);
    const uint8x16_t alphaRow4 = vld1q_u8(src4);

    const uint8x16_t cmp12 = vceqq_u8(alphaRow1, alphaRow2);
    const uint8x16_t cmp34 = vceqq_u8(alphaRow3, alphaRow4);
    const uint8x16_t cmp13 = vceqq_u8(alphaRow1, alphaRow3);

    const uint8x16_t cmp = vandq_u8(vandq_u8(cmp12, cmp34), cmp13);
    const uint8x16_t ncmp = vmvnq_u8(cmp);
    const uint8x16_t nAlphaRow1 = vmvnq_u8(alphaRow1);
    if (is_zero(ncmp)) {
        if (is_zero(alphaRow1)) {
            static const uint64x2_t kTransparent = { 0x0020000000002000ULL,
                                                     0x0020000000002000ULL };
            vst1q_u64(dst1, kTransparent);
            vst1q_u64(dst2, kTransparent);
            return;
        } else if (is_zero(nAlphaRow1)) {
            vst1q_u64(dst1, vreinterpretq_u64_u8(cmp));
            vst1q_u64(dst2, vreinterpretq_u64_u8(cmp));
            return;
        }
    }

    const uint8x16_t indexRow1 = convert_indices(make_index_row(alphaRow1));
    const uint8x16_t indexRow2 = convert_indices(make_index_row(alphaRow2));
    const uint8x16_t indexRow3 = convert_indices(make_index_row(alphaRow3));
    const uint8x16_t indexRow4 = convert_indices(make_index_row(alphaRow4));

    const uint64x2_t indexRow12 = vreinterpretq_u64_u8(
        vorrq_u8(vshlq_n_u8(indexRow1, 3), indexRow2));
    const uint64x2_t indexRow34 = vreinterpretq_u64_u8(
        vorrq_u8(vshlq_n_u8(indexRow3, 3), indexRow4));

    const uint32x4x2_t blockIndices = vtrnq_u32(vreinterpretq_u32_u64(indexRow12),
                                                vreinterpretq_u32_u64(indexRow34));
    const uint64x2_t blockIndicesLeft = vreinterpretq_u64_u32(vrev64q_u32(blockIndices.val[0]));
    const uint64x2_t blockIndicesRight = vreinterpretq_u64_u32(vrev64q_u32(blockIndices.val[1]));

    const uint64x2_t indicesLeft = fix_endianness(pack_indices(blockIndicesLeft));
    const uint64x2_t indicesRight = fix_endianness(pack_indices(blockIndicesRight));

    const uint64x2_t d1 = vcombine_u64(vget_low_u64(indicesLeft), vget_low_u64(indicesRight));
    const uint64x2_t d2 = vcombine_u64(vget_high_u64(indicesLeft), vget_high_u64(indicesRight));
    vst1q_u64(dst1, d1);
    vst1q_u64(dst2, d2);
}

bool CompressA8toR11EAC_NEON(uint8_t* dst, const uint8_t* src,
                             int width, int height, int rowBytes) {

    // Since we're going to operate on 4 blocks at a time, the src width
    // must be a multiple of 16. However, the height only needs to be a
    // multiple of 4
    if (0 == width || 0 == height || (width % 16) != 0 || (height % 4) != 0) {
        return SkTextureCompressor::CompressBufferToFormat(
            dst, src,
            kAlpha_8_SkColorType,
            width, height, rowBytes,
            SkTextureCompressor::kR11_EAC_Format, false);
    }

    const int blocksX = width >> 2;
    const int blocksY = height >> 2;

    SkASSERT((blocksX % 4) == 0);

    uint64_t* encPtr = reinterpret_cast<uint64_t*>(dst);
    for (int y = 0; y < blocksY; ++y) {
        for (int x = 0; x < blocksX; x+=4) {
            // Compress it
            compress_r11eac_blocks(encPtr, src + 4*x, rowBytes);
            encPtr += 4;
        }
        src += 4 * rowBytes;
    }
    return true;
}
