/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emmintrin.h>
#include "SkBitmapProcState_opts_SSE2.h"
#include "SkBitmapProcState_utils.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkUtils.h"

void S32_opaque_D32_filter_DX_SSE2(const SkBitmapProcState& s,
                                   const uint32_t* xy,
                                   int count, uint32_t* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
    SkASSERT(kN32_SkColorType == s.fPixmap.colorType());
    SkASSERT(s.fAlphaScale == 256);

    const char* srcAddr = static_cast<const char*>(s.fPixmap.addr());
    size_t rb = s.fPixmap.rowBytes();
    uint32_t XY = *xy++;
    unsigned y0 = XY >> 14;
    const uint32_t* row0 = reinterpret_cast<const uint32_t*>(srcAddr + (y0 >> 4) * rb);
    const uint32_t* row1 = reinterpret_cast<const uint32_t*>(srcAddr + (XY & 0x3FFF) * rb);
    unsigned subY = y0 & 0xF;

    // ( 0,  0,  0,  0,  0,  0,  0, 16)
    __m128i sixteen = _mm_cvtsi32_si128(16);

    // ( 0,  0,  0,  0, 16, 16, 16, 16)
    sixteen = _mm_shufflelo_epi16(sixteen, 0);

    // ( 0,  0,  0,  0,  0,  0,  0,  y)
    __m128i allY = _mm_cvtsi32_si128(subY);

    // ( 0,  0,  0,  0,  y,  y,  y,  y)
    allY = _mm_shufflelo_epi16(allY, 0);

    // ( 0,  0,  0,  0, 16-y, 16-y, 16-y, 16-y)
    __m128i negY = _mm_sub_epi16(sixteen, allY);

    // (16-y, 16-y, 16-y, 16-y, y, y, y, y)
    allY = _mm_unpacklo_epi64(allY, negY);

    // (16, 16, 16, 16, 16, 16, 16, 16 )
    sixteen = _mm_shuffle_epi32(sixteen, 0);

    // ( 0,  0,  0,  0,  0,  0,  0,  0)
    __m128i zero = _mm_setzero_si128();
    do {
        uint32_t XX = *xy++;    // x0:14 | 4 | x1:14
        unsigned x0 = XX >> 18;
        unsigned x1 = XX & 0x3FFF;

        // (0, 0, 0, 0, 0, 0, 0, x)
        __m128i allX = _mm_cvtsi32_si128((XX >> 14) & 0x0F);

        // (0, 0, 0, 0, x, x, x, x)
        allX = _mm_shufflelo_epi16(allX, 0);

        // (x, x, x, x, x, x, x, x)
        allX = _mm_shuffle_epi32(allX, 0);

        // (16-x, 16-x, 16-x, 16-x, 16-x, 16-x, 16-x)
        __m128i negX = _mm_sub_epi16(sixteen, allX);

        // Load 4 samples (pixels).
        __m128i a00 = _mm_cvtsi32_si128(row0[x0]);
        __m128i a01 = _mm_cvtsi32_si128(row0[x1]);
        __m128i a10 = _mm_cvtsi32_si128(row1[x0]);
        __m128i a11 = _mm_cvtsi32_si128(row1[x1]);

        // (0, 0, a00, a10)
        __m128i a00a10 = _mm_unpacklo_epi32(a10, a00);

        // Expand to 16 bits per component.
        a00a10 = _mm_unpacklo_epi8(a00a10, zero);

        // ((a00 * (16-y)), (a10 * y)).
        a00a10 = _mm_mullo_epi16(a00a10, allY);

        // (a00 * (16-y) * (16-x), a10 * y * (16-x)).
        a00a10 = _mm_mullo_epi16(a00a10, negX);

        // (0, 0, a01, a10)
        __m128i a01a11 = _mm_unpacklo_epi32(a11, a01);

        // Expand to 16 bits per component.
        a01a11 = _mm_unpacklo_epi8(a01a11, zero);

        // (a01 * (16-y)), (a11 * y)
        a01a11 = _mm_mullo_epi16(a01a11, allY);

        // (a01 * (16-y) * x), (a11 * y * x)
        a01a11 = _mm_mullo_epi16(a01a11, allX);

        // (a00*w00 + a01*w01, a10*w10 + a11*w11)
        __m128i sum = _mm_add_epi16(a00a10, a01a11);

        // (DC, a00*w00 + a01*w01)
        __m128i shifted = _mm_shuffle_epi32(sum, 0xEE);

        // (DC, a00*w00 + a01*w01 + a10*w10 + a11*w11)
        sum = _mm_add_epi16(sum, shifted);

        // Divide each 16 bit component by 256.
        sum = _mm_srli_epi16(sum, 8);

        // Pack lower 4 16 bit values of sum into lower 4 bytes.
        sum = _mm_packus_epi16(sum, zero);

        // Extract low int and store.
        *colors++ = _mm_cvtsi128_si32(sum);
    } while (--count > 0);
}

void S32_alpha_D32_filter_DX_SSE2(const SkBitmapProcState& s,
                                  const uint32_t* xy,
                                  int count, uint32_t* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
    SkASSERT(kN32_SkColorType == s.fPixmap.colorType());
    SkASSERT(s.fAlphaScale < 256);

    const char* srcAddr = static_cast<const char*>(s.fPixmap.addr());
    size_t rb = s.fPixmap.rowBytes();
    uint32_t XY = *xy++;
    unsigned y0 = XY >> 14;
    const uint32_t* row0 = reinterpret_cast<const uint32_t*>(srcAddr + (y0 >> 4) * rb);
    const uint32_t* row1 = reinterpret_cast<const uint32_t*>(srcAddr + (XY & 0x3FFF) * rb);
    unsigned subY = y0 & 0xF;

    // ( 0,  0,  0,  0,  0,  0,  0, 16)
    __m128i sixteen = _mm_cvtsi32_si128(16);

    // ( 0,  0,  0,  0, 16, 16, 16, 16)
    sixteen = _mm_shufflelo_epi16(sixteen, 0);

    // ( 0,  0,  0,  0,  0,  0,  0,  y)
    __m128i allY = _mm_cvtsi32_si128(subY);

    // ( 0,  0,  0,  0,  y,  y,  y,  y)
    allY = _mm_shufflelo_epi16(allY, 0);

    // ( 0,  0,  0,  0, 16-y, 16-y, 16-y, 16-y)
    __m128i negY = _mm_sub_epi16(sixteen, allY);

    // (16-y, 16-y, 16-y, 16-y, y, y, y, y)
    allY = _mm_unpacklo_epi64(allY, negY);

    // (16, 16, 16, 16, 16, 16, 16, 16 )
    sixteen = _mm_shuffle_epi32(sixteen, 0);

    // ( 0,  0,  0,  0,  0,  0,  0,  0)
    __m128i zero = _mm_setzero_si128();

    // ( alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha )
    __m128i alpha = _mm_set1_epi16(s.fAlphaScale);

    do {
        uint32_t XX = *xy++;    // x0:14 | 4 | x1:14
        unsigned x0 = XX >> 18;
        unsigned x1 = XX & 0x3FFF;

        // (0, 0, 0, 0, 0, 0, 0, x)
        __m128i allX = _mm_cvtsi32_si128((XX >> 14) & 0x0F);

        // (0, 0, 0, 0, x, x, x, x)
        allX = _mm_shufflelo_epi16(allX, 0);

        // (x, x, x, x, x, x, x, x)
        allX = _mm_shuffle_epi32(allX, 0);

        // (16-x, 16-x, 16-x, 16-x, 16-x, 16-x, 16-x)
        __m128i negX = _mm_sub_epi16(sixteen, allX);

        // Load 4 samples (pixels).
        __m128i a00 = _mm_cvtsi32_si128(row0[x0]);
        __m128i a01 = _mm_cvtsi32_si128(row0[x1]);
        __m128i a10 = _mm_cvtsi32_si128(row1[x0]);
        __m128i a11 = _mm_cvtsi32_si128(row1[x1]);

        // (0, 0, a00, a10)
        __m128i a00a10 = _mm_unpacklo_epi32(a10, a00);

        // Expand to 16 bits per component.
        a00a10 = _mm_unpacklo_epi8(a00a10, zero);

        // ((a00 * (16-y)), (a10 * y)).
        a00a10 = _mm_mullo_epi16(a00a10, allY);

        // (a00 * (16-y) * (16-x), a10 * y * (16-x)).
        a00a10 = _mm_mullo_epi16(a00a10, negX);

        // (0, 0, a01, a10)
        __m128i a01a11 = _mm_unpacklo_epi32(a11, a01);

        // Expand to 16 bits per component.
        a01a11 = _mm_unpacklo_epi8(a01a11, zero);

        // (a01 * (16-y)), (a11 * y)
        a01a11 = _mm_mullo_epi16(a01a11, allY);

        // (a01 * (16-y) * x), (a11 * y * x)
        a01a11 = _mm_mullo_epi16(a01a11, allX);

        // (a00*w00 + a01*w01, a10*w10 + a11*w11)
        __m128i sum = _mm_add_epi16(a00a10, a01a11);

        // (DC, a00*w00 + a01*w01)
        __m128i shifted = _mm_shuffle_epi32(sum, 0xEE);

        // (DC, a00*w00 + a01*w01 + a10*w10 + a11*w11)
        sum = _mm_add_epi16(sum, shifted);

        // Divide each 16 bit component by 256.
        sum = _mm_srli_epi16(sum, 8);

        // Multiply by alpha.
        sum = _mm_mullo_epi16(sum, alpha);

        // Divide each 16 bit component by 256.
        sum = _mm_srli_epi16(sum, 8);

        // Pack lower 4 16 bit values of sum into lower 4 bytes.
        sum = _mm_packus_epi16(sum, zero);

        // Extract low int and store.
        *colors++ = _mm_cvtsi128_si32(sum);
    } while (--count > 0);
}

static inline uint32_t ClampX_ClampY_pack_filter(SkFixed f, unsigned max,
                                                 SkFixed one) {
    unsigned i = SkClampMax(f >> 16, max);
    i = (i << 4) | ((f >> 12) & 0xF);
    return (i << 14) | SkClampMax((f + one) >> 16, max);
}

/*  SSE version of ClampX_ClampY_filter_scale()
 *  portable version is in core/SkBitmapProcState_matrix.h
 */
void ClampX_ClampY_filter_scale_SSE2(const SkBitmapProcState& s, uint32_t xy[],
                                     int count, int x, int y) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);

    const unsigned maxX = s.fPixmap.width() - 1;
    const SkFixed one = s.fFilterOneX;
    const SkFixed dx = s.fInvSx;

    const SkBitmapProcStateAutoMapper mapper(s, x, y);
    const SkFixed fy = mapper.fixedY();
    const unsigned maxY = s.fPixmap.height() - 1;
    // compute our two Y values up front
    *xy++ = ClampX_ClampY_pack_filter(fy, maxY, s.fFilterOneY);
    // now initialize fx
    SkFixed fx = mapper.fixedX();

    // test if we don't need to apply the tile proc
    if (can_truncate_to_fixed_for_decal(fx, dx, count, maxX)) {
        if (count >= 4) {
            // SSE version of decal_filter_scale
            while ((size_t(xy) & 0x0F) != 0) {
                SkASSERT((fx >> (16 + 14)) == 0);
                *xy++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
                fx += dx;
                count--;
            }

            __m128i wide_1    = _mm_set1_epi32(1);
            __m128i wide_dx4  = _mm_set1_epi32(dx * 4);
            __m128i wide_fx   = _mm_set_epi32(fx + dx * 3, fx + dx * 2,
                                              fx + dx, fx);

            while (count >= 4) {
                __m128i wide_out;

                wide_out = _mm_slli_epi32(_mm_srai_epi32(wide_fx, 12), 14);
                wide_out = _mm_or_si128(wide_out, _mm_add_epi32(
                                        _mm_srai_epi32(wide_fx, 16), wide_1));

                _mm_store_si128(reinterpret_cast<__m128i*>(xy), wide_out);

                xy += 4;
                fx += dx * 4;
                wide_fx  = _mm_add_epi32(wide_fx, wide_dx4);
                count -= 4;
            } // while count >= 4
        } // if count >= 4

        while (count-- > 0) {
            SkASSERT((fx >> (16 + 14)) == 0);
            *xy++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
            fx += dx;
        }
    } else {
        // SSE2 only support 16bit interger max & min, so only process the case
        // maxX less than the max 16bit interger. Actually maxX is the bitmap's
        // height, there should be rare bitmap whose height will be greater
        // than max 16bit interger in the real world.
        if ((count >= 4) && (maxX <= 0xFFFF)) {
            while (((size_t)xy & 0x0F) != 0) {
                *xy++ = ClampX_ClampY_pack_filter(fx, maxX, one);
                fx += dx;
                count--;
            }

            __m128i wide_fx   = _mm_set_epi32(fx + dx * 3, fx + dx * 2,
                                              fx + dx, fx);
            __m128i wide_dx4  = _mm_set1_epi32(dx * 4);
            __m128i wide_one  = _mm_set1_epi32(one);
            __m128i wide_maxX = _mm_set1_epi32(maxX);
            __m128i wide_mask = _mm_set1_epi32(0xF);

             while (count >= 4) {
                __m128i wide_i;
                __m128i wide_lo;
                __m128i wide_fx1;

                // i = SkClampMax(f>>16,maxX)
                wide_i = _mm_max_epi16(_mm_srli_epi32(wide_fx, 16),
                                       _mm_setzero_si128());
                wide_i = _mm_min_epi16(wide_i, wide_maxX);

                // i<<4 | EXTRACT_LOW_BITS(fx)
                wide_lo = _mm_srli_epi32(wide_fx, 12);
                wide_lo = _mm_and_si128(wide_lo, wide_mask);
                wide_i  = _mm_slli_epi32(wide_i, 4);
                wide_i  = _mm_or_si128(wide_i, wide_lo);

                // i<<14
                wide_i = _mm_slli_epi32(wide_i, 14);

                // SkClampMax(((f+one))>>16,max)
                wide_fx1 = _mm_add_epi32(wide_fx, wide_one);
                wide_fx1 = _mm_max_epi16(_mm_srli_epi32(wide_fx1, 16),
                                                        _mm_setzero_si128());
                wide_fx1 = _mm_min_epi16(wide_fx1, wide_maxX);

                // final combination
                wide_i = _mm_or_si128(wide_i, wide_fx1);
                _mm_store_si128(reinterpret_cast<__m128i*>(xy), wide_i);

                wide_fx = _mm_add_epi32(wide_fx, wide_dx4);
                fx += dx * 4;
                xy += 4;
                count -= 4;
            } // while count >= 4
        } // if count >= 4

        while (count-- > 0) {
            *xy++ = ClampX_ClampY_pack_filter(fx, maxX, one);
            fx += dx;
        }
    }
}

/*  SSE version of ClampX_ClampY_nofilter_scale()
 *  portable version is in core/SkBitmapProcState_matrix.h
 */
void ClampX_ClampY_nofilter_scale_SSE2(const SkBitmapProcState& s,
                                    uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);

    // we store y, x, x, x, x, x
    const unsigned maxX = s.fPixmap.width() - 1;
    const SkBitmapProcStateAutoMapper mapper(s, x, y);
    const unsigned maxY = s.fPixmap.height() - 1;
    *xy++ = SkClampMax(mapper.intY(), maxY);
    SkFixed fx = mapper.fixedX();

    if (0 == maxX) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    const SkFixed dx = s.fInvSx;

    // test if we don't need to apply the tile proc
    if ((unsigned)(fx >> 16) <= maxX &&
        (unsigned)((fx + dx * (count - 1)) >> 16) <= maxX) {
        // SSE version of decal_nofilter_scale
        if (count >= 8) {
            while (((size_t)xy & 0x0F) != 0) {
                *xy++ = pack_two_shorts(fx >> 16, (fx + dx) >> 16);
                fx += 2 * dx;
                count -= 2;
            }

            __m128i wide_dx4 = _mm_set1_epi32(dx * 4);
            __m128i wide_dx8 = _mm_add_epi32(wide_dx4, wide_dx4);

            __m128i wide_low = _mm_set_epi32(fx + dx * 3, fx + dx * 2,
                                             fx + dx, fx);
            __m128i wide_high = _mm_add_epi32(wide_low, wide_dx4);

            while (count >= 8) {
                __m128i wide_out_low = _mm_srli_epi32(wide_low, 16);
                __m128i wide_out_high = _mm_srli_epi32(wide_high, 16);

                __m128i wide_result = _mm_packs_epi32(wide_out_low,
                                                      wide_out_high);
                _mm_store_si128(reinterpret_cast<__m128i*>(xy), wide_result);

                wide_low = _mm_add_epi32(wide_low, wide_dx8);
                wide_high = _mm_add_epi32(wide_high, wide_dx8);

                xy += 4;
                fx += dx * 8;
                count -= 8;
            }
        } // if count >= 8

        uint16_t* xx = reinterpret_cast<uint16_t*>(xy);
        while (count-- > 0) {
            *xx++ = SkToU16(fx >> 16);
            fx += dx;
        }
    } else {
        // SSE2 only support 16bit interger max & min, so only process the case
        // maxX less than the max 16bit interger. Actually maxX is the bitmap's
        // height, there should be rare bitmap whose height will be greater
        // than max 16bit interger in the real world.
        if ((count >= 8) && (maxX <= 0xFFFF)) {
            while (((size_t)xy & 0x0F) != 0) {
                *xy++ = pack_two_shorts(SkClampMax((fx + dx) >> 16, maxX),
                                        SkClampMax(fx >> 16, maxX));
                fx += 2 * dx;
                count -= 2;
            }

            __m128i wide_dx4 = _mm_set1_epi32(dx * 4);
            __m128i wide_dx8 = _mm_add_epi32(wide_dx4, wide_dx4);

            __m128i wide_low = _mm_set_epi32(fx + dx * 3, fx + dx * 2,
                                             fx + dx, fx);
            __m128i wide_high = _mm_add_epi32(wide_low, wide_dx4);
            __m128i wide_maxX = _mm_set1_epi32(maxX);

            while (count >= 8) {
                __m128i wide_out_low = _mm_srli_epi32(wide_low, 16);
                __m128i wide_out_high = _mm_srli_epi32(wide_high, 16);

                wide_out_low  = _mm_max_epi16(wide_out_low,
                                              _mm_setzero_si128());
                wide_out_low  = _mm_min_epi16(wide_out_low, wide_maxX);
                wide_out_high = _mm_max_epi16(wide_out_high,
                                              _mm_setzero_si128());
                wide_out_high = _mm_min_epi16(wide_out_high, wide_maxX);

                __m128i wide_result = _mm_packs_epi32(wide_out_low,
                                                      wide_out_high);
                _mm_store_si128(reinterpret_cast<__m128i*>(xy), wide_result);

                wide_low  = _mm_add_epi32(wide_low, wide_dx8);
                wide_high = _mm_add_epi32(wide_high, wide_dx8);

                xy += 4;
                fx += dx * 8;
                count -= 8;
            }
        } // if count >= 8

        uint16_t* xx = reinterpret_cast<uint16_t*>(xy);
        while (count-- > 0) {
            *xx++ = SkClampMax(fx >> 16, maxX);
            fx += dx;
        }
    }
}

/*  SSE version of ClampX_ClampY_filter_affine()
 *  portable version is in core/SkBitmapProcState_matrix.h
 */
void ClampX_ClampY_filter_affine_SSE2(const SkBitmapProcState& s,
                                      uint32_t xy[], int count, int x, int y) {
    const SkBitmapProcStateAutoMapper mapper(s, x, y);

    SkFixed oneX = s.fFilterOneX;
    SkFixed oneY = s.fFilterOneY;
    SkFixed fx = mapper.fixedX();
    SkFixed fy = mapper.fixedY();
    SkFixed dx = s.fInvSx;
    SkFixed dy = s.fInvKy;
    unsigned maxX = s.fPixmap.width() - 1;
    unsigned maxY = s.fPixmap.height() - 1;

    if (count >= 2 && (maxX <= 0xFFFF)) {
        SkFixed dx2 = dx + dx;
        SkFixed dy2 = dy + dy;

        __m128i wide_f = _mm_set_epi32(fx + dx, fy + dy, fx, fy);
        __m128i wide_d2  = _mm_set_epi32(dx2, dy2, dx2, dy2);
        __m128i wide_one  = _mm_set_epi32(oneX, oneY, oneX, oneY);
        __m128i wide_max = _mm_set_epi32(maxX, maxY, maxX, maxY);
        __m128i wide_mask = _mm_set1_epi32(0xF);

        while (count >= 2) {
            // i = SkClampMax(f>>16,maxX)
            __m128i wide_i = _mm_max_epi16(_mm_srli_epi32(wide_f, 16),
                                           _mm_setzero_si128());
            wide_i = _mm_min_epi16(wide_i, wide_max);

            // i<<4 | EXTRACT_LOW_BITS(f)
            __m128i wide_lo = _mm_srli_epi32(wide_f, 12);
            wide_lo = _mm_and_si128(wide_lo, wide_mask);
            wide_i  = _mm_slli_epi32(wide_i, 4);
            wide_i  = _mm_or_si128(wide_i, wide_lo);

            // i<<14
            wide_i = _mm_slli_epi32(wide_i, 14);

            // SkClampMax(((f+one))>>16,max)
            __m128i wide_f1 = _mm_add_epi32(wide_f, wide_one);
            wide_f1 = _mm_max_epi16(_mm_srli_epi32(wide_f1, 16),
                                                   _mm_setzero_si128());
            wide_f1 = _mm_min_epi16(wide_f1, wide_max);

            // final combination
            wide_i = _mm_or_si128(wide_i, wide_f1);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(xy), wide_i);

            wide_f = _mm_add_epi32(wide_f, wide_d2);

            fx += dx2;
            fy += dy2;
            xy += 4;
            count -= 2;
        } // while count >= 2
    } // if count >= 2

    while (count-- > 0) {
        *xy++ = ClampX_ClampY_pack_filter(fy, maxY, oneY);
        fy += dy;
        *xy++ = ClampX_ClampY_pack_filter(fx, maxX, oneX);
        fx += dx;
    }
}

/*  SSE version of ClampX_ClampY_nofilter_affine()
 *  portable version is in core/SkBitmapProcState_matrix.h
 */
void ClampX_ClampY_nofilter_affine_SSE2(const SkBitmapProcState& s,
                                      uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvType & SkMatrix::kAffine_Mask);
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask |
                             SkMatrix::kAffine_Mask)) == 0);

    const SkBitmapProcStateAutoMapper mapper(s, x, y);

    SkFixed fx = mapper.fixedX();
    SkFixed fy = mapper.fixedY();
    SkFixed dx = s.fInvSx;
    SkFixed dy = s.fInvKy;
    int maxX = s.fPixmap.width() - 1;
    int maxY = s.fPixmap.height() - 1;

    if (count >= 4 && (maxX <= 0xFFFF)) {
        while (((size_t)xy & 0x0F) != 0) {
            *xy++ = (SkClampMax(fy >> 16, maxY) << 16) |
                                  SkClampMax(fx >> 16, maxX);
            fx += dx;
            fy += dy;
            count--;
        }

        SkFixed dx4 = dx * 4;
        SkFixed dy4 = dy * 4;

        __m128i wide_fx   = _mm_set_epi32(fx + dx * 3, fx + dx * 2,
                                          fx + dx, fx);
        __m128i wide_fy   = _mm_set_epi32(fy + dy * 3, fy + dy * 2,
                                          fy + dy, fy);
        __m128i wide_dx4  = _mm_set1_epi32(dx4);
        __m128i wide_dy4  = _mm_set1_epi32(dy4);

        __m128i wide_maxX = _mm_set1_epi32(maxX);
        __m128i wide_maxY = _mm_set1_epi32(maxY);

        while (count >= 4) {
            // SkClampMax(fx>>16,maxX)
            __m128i wide_lo = _mm_max_epi16(_mm_srli_epi32(wide_fx, 16),
                                            _mm_setzero_si128());
            wide_lo = _mm_min_epi16(wide_lo, wide_maxX);

            // SkClampMax(fy>>16,maxY)
            __m128i wide_hi = _mm_max_epi16(_mm_srli_epi32(wide_fy, 16),
                                            _mm_setzero_si128());
            wide_hi = _mm_min_epi16(wide_hi, wide_maxY);

            // final combination
            __m128i wide_i = _mm_or_si128(_mm_slli_epi32(wide_hi, 16),
                                          wide_lo);
            _mm_store_si128(reinterpret_cast<__m128i*>(xy), wide_i);

            wide_fx = _mm_add_epi32(wide_fx, wide_dx4);
            wide_fy = _mm_add_epi32(wide_fy, wide_dy4);

            fx += dx4;
            fy += dy4;
            xy += 4;
            count -= 4;
        } // while count >= 4
    } // if count >= 4

    while (count-- > 0) {
        *xy++ = (SkClampMax(fy >> 16, maxY) << 16) |
                              SkClampMax(fx >> 16, maxX);
        fx += dx;
        fy += dy;
    }
}
