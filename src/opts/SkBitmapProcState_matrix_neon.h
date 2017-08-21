/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <arm_neon.h>

#define SCALE_NOFILTER_NAME     MAKENAME(_nofilter_scale)
#define SCALE_FILTER_NAME       MAKENAME(_filter_scale)

#define PACK_FILTER_X_NAME  MAKENAME(_pack_filter_x)
#define PACK_FILTER_Y_NAME  MAKENAME(_pack_filter_y)
#define PACK_FILTER_X4_NAME MAKENAME(_pack_filter_x4)
#define PACK_FILTER_Y4_NAME MAKENAME(_pack_filter_y4)

#ifndef PREAMBLE
    #define PREAMBLE(state)
    #define PREAMBLE_PARAM_X
    #define PREAMBLE_PARAM_Y
    #define PREAMBLE_ARG_X
    #define PREAMBLE_ARG_Y
#endif

static void SCALE_NOFILTER_NAME(const SkBitmapProcState& s,
                                uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);

    PREAMBLE(s);

    // we store y, x, x, x, x, x
    const unsigned maxX = s.fPixmap.width() - 1;
    SkFractionalInt fx;
    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        const unsigned maxY = s.fPixmap.height() - 1;
        *xy++ = TILEY_PROCF(mapper.fixedY(), maxY);
        fx = mapper.fractionalIntX();
    }

    if (0 == maxX) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    const SkFractionalInt dx = s.fInvSxFractionalInt;

#ifdef CHECK_FOR_DECAL
    // test if we don't need to apply the tile proc
    const SkFixed fixedFx = SkFractionalIntToFixed(fx);
    const SkFixed fixedDx = SkFractionalIntToFixed(dx);
    if (can_truncate_to_fixed_for_decal(fixedFx, fixedDx, count, maxX)) {
        decal_nofilter_scale_neon(xy, fixedFx, fixedDx, count);
        return;
    }
#endif

    if (count >= 8) {
        SkFractionalInt dx2 = dx+dx;
        SkFractionalInt dx4 = dx2+dx2;
        SkFractionalInt dx8 = dx4+dx4;

        // now build fx/fx+dx/fx+2dx/fx+3dx
        SkFractionalInt fx1, fx2, fx3;
        int32x4_t lbase, hbase;
        int16_t *dst16 = (int16_t *)xy;

        fx1 = fx+dx;
        fx2 = fx1+dx;
        fx3 = fx2+dx;

        lbase = vdupq_n_s32(SkFractionalIntToFixed(fx));
        lbase = vsetq_lane_s32(SkFractionalIntToFixed(fx1), lbase, 1);
        lbase = vsetq_lane_s32(SkFractionalIntToFixed(fx2), lbase, 2);
        lbase = vsetq_lane_s32(SkFractionalIntToFixed(fx3), lbase, 3);
        hbase = vaddq_s32(lbase, vdupq_n_s32(SkFractionalIntToFixed(dx4)));

        // store & bump
        while (count >= 8) {

            int16x8_t fx8;

            fx8 = TILEX_PROCF_NEON8(lbase, hbase, maxX);

            vst1q_s16(dst16, fx8);

            // but preserving base & on to the next
            lbase = vaddq_s32 (lbase, vdupq_n_s32(SkFractionalIntToFixed(dx8)));
            hbase = vaddq_s32 (hbase, vdupq_n_s32(SkFractionalIntToFixed(dx8)));
            dst16 += 8;
            count -= 8;
            fx += dx8;
        };
        xy = (uint32_t *) dst16;
    }

    uint16_t* xx = (uint16_t*)xy;
    for (int i = count; i > 0; --i) {
        *xx++ = TILEX_PROCF(SkFractionalIntToFixed(fx), maxX);
        fx += dx;
    }
}

static inline uint32_t PACK_FILTER_Y_NAME(SkFixed f, unsigned max,
                                          SkFixed one PREAMBLE_PARAM_Y) {
    unsigned i = TILEY_PROCF(f, max);
    i = (i << 4) | EXTRACT_LOW_BITS(f, max);
    return (i << 14) | (TILEY_PROCF((f + one), max));
}

static inline uint32_t PACK_FILTER_X_NAME(SkFixed f, unsigned max,
                                          SkFixed one PREAMBLE_PARAM_X) {
    unsigned i = TILEX_PROCF(f, max);
    i = (i << 4) | EXTRACT_LOW_BITS(f, max);
    return (i << 14) | (TILEX_PROCF((f + one), max));
}

static inline int32x4_t PACK_FILTER_X4_NAME(int32x4_t f, unsigned max,
                                          SkFixed one PREAMBLE_PARAM_X) {
    int32x4_t ret, res, wide_one;

    // Prepare constants
    wide_one = vdupq_n_s32(one);

    // Step 1
    res = TILEX_PROCF_NEON4(f, max);

    // Step 2
    ret = EXTRACT_LOW_BITS_NEON4(f, max);
    ret = vsliq_n_s32(ret, res, 4);

    // Step 3
    res = TILEX_PROCF_NEON4(f + wide_one, max);
    ret = vorrq_s32(vshlq_n_s32(ret, 14), res);

    return ret;
}

static inline int32x4_t PACK_FILTER_Y4_NAME(int32x4_t f, unsigned max,
                                          SkFixed one PREAMBLE_PARAM_X) {
    int32x4_t ret, res, wide_one;

    // Prepare constants
    wide_one = vdupq_n_s32(one);

    // Step 1
    res = TILEY_PROCF_NEON4(f, max);

    // Step 2
    ret = EXTRACT_LOW_BITS_NEON4(f, max);
    ret = vsliq_n_s32(ret, res, 4);

    // Step 3
    res = TILEY_PROCF_NEON4(f + wide_one, max);
    ret = vorrq_s32(vshlq_n_s32(ret, 14), res);

    return ret;
}

static void SCALE_FILTER_NAME(const SkBitmapProcState& s,
                              uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);

    PREAMBLE(s);

    const unsigned maxX = s.fPixmap.width() - 1;
    const SkFixed one = s.fFilterOneX;
    const SkFractionalInt dx = s.fInvSxFractionalInt;
    SkFractionalInt fx;

    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        const SkFixed fy = mapper.fixedY();
        const unsigned maxY = s.fPixmap.height() - 1;
        // compute our two Y values up front
        *xy++ = PACK_FILTER_Y_NAME(fy, maxY, s.fFilterOneY PREAMBLE_ARG_Y);
        // now initialize fx
        fx = mapper.fractionalIntX();
    }

#ifdef CHECK_FOR_DECAL
    // test if we don't need to apply the tile proc
    const SkFixed fixedFx = SkFractionalIntToFixed(fx);
    const SkFixed fixedDx = SkFractionalIntToFixed(dx);
    if (can_truncate_to_fixed_for_decal(fixedFx, fixedDx, count, maxX)) {
        decal_filter_scale_neon(xy, fixedFx, fixedDx, count);
        return;
    }
#endif
    {

    if (count >= 4) {
        int32x4_t wide_fx;

        wide_fx = vdupq_n_s32(SkFractionalIntToFixed(fx));
        wide_fx = vsetq_lane_s32(SkFractionalIntToFixed(fx+dx), wide_fx, 1);
        wide_fx = vsetq_lane_s32(SkFractionalIntToFixed(fx+dx+dx), wide_fx, 2);
        wide_fx = vsetq_lane_s32(SkFractionalIntToFixed(fx+dx+dx+dx), wide_fx, 3);

        while (count >= 4) {
            int32x4_t res;

            res = PACK_FILTER_X4_NAME(wide_fx, maxX, one PREAMBLE_ARG_X);

            vst1q_u32(xy, vreinterpretq_u32_s32(res));

            wide_fx += vdupq_n_s32(SkFractionalIntToFixed(dx+dx+dx+dx));
            fx += dx+dx+dx+dx;
            xy += 4;
            count -= 4;
        }
    }

    while (--count >= 0) {
        *xy++ = PACK_FILTER_X_NAME(SkFractionalIntToFixed(fx), maxX, one PREAMBLE_ARG_X);
        fx += dx;
    }

    }
}

const SkBitmapProcState::MatrixProc MAKENAME(_Procs)[] = {
    SCALE_NOFILTER_NAME,
    SCALE_FILTER_NAME,
};

#undef TILEX_PROCF_NEON8
#undef TILEY_PROCF_NEON8
#undef TILEX_PROCF_NEON4
#undef TILEY_PROCF_NEON4
#undef EXTRACT_LOW_BITS_NEON4

#undef MAKENAME
#undef TILEX_PROCF
#undef TILEY_PROCF
#ifdef CHECK_FOR_DECAL
    #undef CHECK_FOR_DECAL
#endif

#undef SCALE_NOFILTER_NAME
#undef SCALE_FILTER_NAME

#undef PREAMBLE
#undef PREAMBLE_PARAM_X
#undef PREAMBLE_PARAM_Y
#undef PREAMBLE_ARG_X
#undef PREAMBLE_ARG_Y

#undef EXTRACT_LOW_BITS
