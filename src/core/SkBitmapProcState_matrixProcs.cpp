/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// The copyright below was added in 2009, but I see no record of moto contributions...?

/* NEON optimized code (C) COPYRIGHT 2009 Motorola
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "include/private/SkTo.h"
#include "src/core/SkBitmapProcState.h"
#include "src/core/SkUtils.h"

/*
 *  The decal_ functions require that
 *  1. dx > 0
 *  2. [fx, fx+dx, fx+2dx, fx+3dx, ... fx+(count-1)dx] are all <= maxX
 *
 *  In addition, we use SkFractionalInt to keep more fractional precision than
 *  just SkFixed, so we will abort the decal_ call if dx is very small, since
 *  the decal_ function just operates on SkFixed. If that were changed, we could
 *  skip the very_small test here.
 */
static inline bool can_truncate_to_fixed_for_decal(SkFixed fx,
                                                   SkFixed dx,
                                                   int count, unsigned max) {
    SkASSERT(count > 0);

    // if decal_ kept SkFractionalInt precision, this would just be dx <= 0
    // I just made up the 1/256. Just don't want to perceive accumulated error
    // if we truncate frDx and lose its low bits.
    if (dx <= SK_Fixed1 / 256) {
        return false;
    }

    // Note: it seems the test should be (fx <= max && lastFx <= max); but
    // historically it's been a strict inequality check, and changing produces
    // unexpected diffs.  Further investigation is needed.

    // We cast to unsigned so we don't have to check for negative values, which
    // will now appear as very large positive values, and thus fail our test!
    if ((unsigned)SkFixedFloorToInt(fx) >= max) {
        return false;
    }

    // Promote to 64bit (48.16) to avoid overflow.
    const uint64_t lastFx = fx + sk_64_mul(dx, count - 1);

    return SkTFitsIn<int32_t>(lastFx) && (unsigned)SkFixedFloorToInt(SkTo<int32_t>(lastFx)) < max;
}

// When not filtering, we store 32-bit y, 16-bit x, 16-bit x, 16-bit x, ...
// When filtering we write out 32-bit encodings, pairing 14.4 x0 with 14-bit x1.

// The clamp routines may try to fall into one of these unclamped decal fast-paths.
// (Only clamp works in the right coordinate space to check for decal.)
static void decal_nofilter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
    // can_truncate_to_fixed_for_decal() checked only that stepping fx+=dx count-1
    // times doesn't overflow fx, so we take unusual care not to step count times.
    for (; count > 2; count -= 2) {
        *dst++ = pack_two_shorts( (fx +  0) >> 16,
                                  (fx + dx) >> 16);
        fx += dx+dx;
    }

    SkASSERT(count <= 2);
    switch (count) {
        case 2: ((uint16_t*)dst)[1] = SkToU16((fx + dx) >> 16);
        case 1: ((uint16_t*)dst)[0] = SkToU16((fx +  0) >> 16);
    }
}

// A generic implementation for unfiltered scale+translate, templated on tiling method.
template <unsigned (*tile)(SkFixed, int), bool tryDecal>
static void nofilter_scale(const SkBitmapProcState& s,
                           uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvMatrix.isScaleTranslate());

    // Write out our 32-bit y, and get our intial fx.
    SkFractionalInt fx;
    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        *xy++ = tile(mapper.fixedY(), s.fPixmap.height() - 1);
        fx = mapper.fractionalIntX();
    }

    const unsigned maxX = s.fPixmap.width() - 1;
    if (0 == maxX) {
        // If width == 1, all the x-values must refer to that pixel, and must be zero.
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    const SkFractionalInt dx = s.fInvSxFractionalInt;

    if (tryDecal) {
        const SkFixed fixedFx = SkFractionalIntToFixed(fx);
        const SkFixed fixedDx = SkFractionalIntToFixed(dx);

        if (can_truncate_to_fixed_for_decal(fixedFx, fixedDx, count, maxX)) {
            decal_nofilter_scale(xy, fixedFx, fixedDx, count);
            return;
        }
    }

    // Remember, each x-coordinate is 16-bit.
    for (; count >= 2; count -= 2) {
        *xy++ = pack_two_shorts(tile(SkFractionalIntToFixed(fx     ), maxX),
                                tile(SkFractionalIntToFixed(fx + dx), maxX));
        fx += dx+dx;
    }

    auto xx = (uint16_t*)xy;
    while (count --> 0) {
        *xx++ = tile(SkFractionalIntToFixed(fx), maxX);
        fx += dx;
    }
}

template <unsigned (*tile)(SkFixed, int)>
static void nofilter_affine(const SkBitmapProcState& s,
                            uint32_t xy[], int count, int x, int y) {
    SkASSERT(!s.fInvMatrix.hasPerspective());

    const SkBitmapProcStateAutoMapper mapper(s, x, y);

    SkFractionalInt fx = mapper.fractionalIntX(),
                    fy = mapper.fractionalIntY(),
                    dx = s.fInvSxFractionalInt,
                    dy = s.fInvKyFractionalInt;
    int maxX = s.fPixmap.width () - 1,
        maxY = s.fPixmap.height() - 1;

    while (count --> 0) {
        *xy++ = (tile(SkFractionalIntToFixed(fy), maxY) << 16)
              | (tile(SkFractionalIntToFixed(fx), maxX)      );
        fx += dx;
        fy += dy;
    }
}

// Extract the high four fractional bits from fx, the lerp parameter when filtering.
static unsigned extract_low_bits_clamp(SkFixed fx, int /*max*/) {
    // If we're already scaled up to by max like clamp/decal,
    // just grab the high four fractional bits.
    return (fx >> 12) & 0xf;
}
static unsigned extract_low_bits_repeat_mirror(SkFixed fx, int max) {
    // In repeat or mirror fx is in [0,1], so scale up by max first.
    // TODO: remove the +1 here and the -1 at the call sites...
    return extract_low_bits_clamp((fx & 0xffff) * (max+1), max);
}

template <unsigned (*tile)(SkFixed, int), unsigned (*extract_low_bits)(SkFixed, int)>
static uint32_t pack(SkFixed f, unsigned max, SkFixed one) {
    uint32_t packed = tile(f, max);                      // low coordinate in high bits
    packed = (packed <<  4) | extract_low_bits(f, max);  // (lerp weight _is_ coord fractional part)
    packed = (packed << 14) | tile((f + one), max);      // high coordinate in low bits
    return packed;
}

template <unsigned (*tile)(SkFixed, int), unsigned (*extract_low_bits)(SkFixed, int), bool tryDecal>
static void filter_scale(const SkBitmapProcState& s,
                         uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvMatrix.isScaleTranslate());

    const unsigned maxX = s.fPixmap.width() - 1;
    const SkFractionalInt dx = s.fInvSxFractionalInt;
    SkFractionalInt fx;
    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        const unsigned maxY = s.fPixmap.height() - 1;
        // compute our two Y values up front
        *xy++ = pack<tile, extract_low_bits>(mapper.fixedY(), maxY, s.fFilterOneY);
        // now initialize fx
        fx = mapper.fractionalIntX();
    }

    // For historical reasons we check both ends are < maxX rather than <= maxX.
    // TODO: try changing this?  See also can_truncate_to_fixed_for_decal().
    if (tryDecal &&
        (unsigned)SkFractionalIntToInt(fx               ) < maxX &&
        (unsigned)SkFractionalIntToInt(fx + dx*(count-1)) < maxX) {
        while (count --> 0) {
            SkFixed fixedFx = SkFractionalIntToFixed(fx);
            SkASSERT((fixedFx >> (16 + 14)) == 0);
            *xy++ = (fixedFx >> 12 << 14) | ((fixedFx >> 16) + 1);
            fx += dx;
        }
        return;
    }

    while (count --> 0) {
        *xy++ = pack<tile, extract_low_bits>(SkFractionalIntToFixed(fx), maxX, s.fFilterOneX);
        fx += dx;
    }
}

template <unsigned (*tile)(SkFixed, int), unsigned (*extract_low_bits)(SkFixed, int)>
static void filter_affine(const SkBitmapProcState& s,
                          uint32_t xy[], int count, int x, int y) {
    SkASSERT(!s.fInvMatrix.hasPerspective());

    const SkBitmapProcStateAutoMapper mapper(s, x, y);

    SkFixed oneX = s.fFilterOneX,
            oneY = s.fFilterOneY;

    SkFractionalInt fx = mapper.fractionalIntX(),
                    fy = mapper.fractionalIntY(),
                    dx = s.fInvSxFractionalInt,
                    dy = s.fInvKyFractionalInt;
    unsigned maxX = s.fPixmap.width () - 1,
             maxY = s.fPixmap.height() - 1;
    while (count --> 0) {
        *xy++ = pack<tile, extract_low_bits>(SkFractionalIntToFixed(fy), maxY, oneY);
        *xy++ = pack<tile, extract_low_bits>(SkFractionalIntToFixed(fx), maxX, oneX);

        fy += dy;
        fx += dx;
    }
}

// Helper to ensure that when we shift down, we do it w/o sign-extension
// so the caller doesn't have to manually mask off the top 16 bits.
static inline unsigned SK_USHIFT16(unsigned x) {
    return x >> 16;
}

static unsigned clamp(SkFixed fx, int max) {
    return SkClampMax(fx >> 16, max);
}
static unsigned repeat(SkFixed fx, int max) {
    SkASSERT(max < 65535);
    return SK_USHIFT16((unsigned)(fx & 0xFFFF) * (max + 1));
}
static unsigned mirror(SkFixed fx, int max) {
    SkASSERT(max < 65535);
    // s is 0xFFFFFFFF if we're on an odd interval, or 0 if an even interval
    SkFixed s = SkLeftShift(fx, 15) >> 31;

    // This should be exactly the same as repeat(fx ^ s, max) from here on.
    return SK_USHIFT16( ((fx ^ s) & 0xFFFF) * (max + 1) );
}

// Mirror/Mirror's always just portable code.
static const SkBitmapProcState::MatrixProc MirrorX_MirrorY_Procs[] = {
    nofilter_scale <mirror, false>, filter_scale <mirror, extract_low_bits_repeat_mirror, false>,
    nofilter_affine<mirror>,        filter_affine<mirror, extract_low_bits_repeat_mirror>,
};

// Clamp/Clamp and Repeat/Repeat have NEON or portable implementations.
#if defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>

    // TODO: this is a fine drop-in for decal_nofilter_scale() generally.
    static void decal_nofilter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
        if (count >= 8) {
            // SkFixed is 16.16 fixed point
            SkFixed dx8 = dx * 8;
            int32x4_t vdx8 = vdupq_n_s32(dx8);

            // setup lbase and hbase
            int32x4_t lbase, hbase;
            lbase = vdupq_n_s32(fx);
            lbase = vsetq_lane_s32(fx + dx, lbase, 1);
            lbase = vsetq_lane_s32(fx + dx + dx, lbase, 2);
            lbase = vsetq_lane_s32(fx + dx + dx + dx, lbase, 3);
            hbase = lbase + vdupq_n_s32(4 * dx);

            do {
                // store the upper 16 bits
                vst1q_u32(dst, vreinterpretq_u32_s16(
                    vuzpq_s16(vreinterpretq_s16_s32(lbase), vreinterpretq_s16_s32(hbase)).val[1]
                ));

                // on to the next group of 8
                lbase += vdx8;
                hbase += vdx8;
                dst += 4; // we did 8 elements but the result is twice smaller
                count -= 8;
                fx += dx8;
            } while (count >= 8);
        }

        uint16_t* xx = (uint16_t*)dst;
        for (int i = count; i > 0; --i) {
            *xx++ = SkToU16(fx >> 16); fx += dx;
        }
    }

    static void decal_filter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
        if (count >= 8) {
            SkFixed dx8 = dx * 8;
            int32x4_t vdx8 = vdupq_n_s32(dx8);

            int32x4_t wide_fx, wide_fx2;
            wide_fx = vdupq_n_s32(fx);
            wide_fx = vsetq_lane_s32(fx + dx, wide_fx, 1);
            wide_fx = vsetq_lane_s32(fx + dx + dx, wide_fx, 2);
            wide_fx = vsetq_lane_s32(fx + dx + dx + dx, wide_fx, 3);

            wide_fx2 = vaddq_s32(wide_fx, vdupq_n_s32(4 * dx));

            while (count >= 8) {
                int32x4_t wide_out;
                int32x4_t wide_out2;

                wide_out = vshlq_n_s32(vshrq_n_s32(wide_fx, 12), 14);
                wide_out = wide_out | (vshrq_n_s32(wide_fx,16) + vdupq_n_s32(1));

                wide_out2 = vshlq_n_s32(vshrq_n_s32(wide_fx2, 12), 14);
                wide_out2 = wide_out2 | (vshrq_n_s32(wide_fx2,16) + vdupq_n_s32(1));

                vst1q_u32(dst, vreinterpretq_u32_s32(wide_out));
                vst1q_u32(dst+4, vreinterpretq_u32_s32(wide_out2));

                dst += 8;
                fx += dx8;
                wide_fx += vdx8;
                wide_fx2 += vdx8;
                count -= 8;
            }
        }

        if (count & 1)
        {
            SkASSERT((fx >> (16 + 14)) == 0);
            *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
            fx += dx;
        }
        while ((count -= 2) >= 0)
        {
            SkASSERT((fx >> (16 + 14)) == 0);
            *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
            fx += dx;

            *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
            fx += dx;
        }
    }

    static inline int16x8_t clamp8(int32x4_t low, int32x4_t high, unsigned max) {
        int16x8_t res;

        // get the hi 16s of all those 32s
        res = vuzpq_s16(vreinterpretq_s16_s32(low), vreinterpretq_s16_s32(high)).val[1];

        // clamp
        res = vmaxq_s16(res, vdupq_n_s16(0));
        res = vminq_s16(res, vdupq_n_s16(max));

        return res;
    }

    static inline int32x4_t clamp4(int32x4_t f, unsigned max) {
        int32x4_t res;

        // get the hi 16s of all those 32s
        res = vshrq_n_s32(f, 16);

        // clamp
        res = vmaxq_s32(res, vdupq_n_s32(0));
        res = vminq_s32(res, vdupq_n_s32(max));

        return res;
    }

    static inline int32x4_t extract_low_bits_clamp4(int32x4_t fx, unsigned) {
        int32x4_t ret;

        ret = vshrq_n_s32(fx, 12);

        /* We don't need the mask below because the caller will
         * overwrite the non-masked bits
         */
        //ret = vandq_s32(ret, vdupq_n_s32(0xF));

        return ret;
    }

    static inline int16x8_t repeat8(int32x4_t low, int32x4_t high, unsigned max) {
        uint16x8_t res;
        uint32x4_t tmpl, tmph;

        // get the lower 16 bits
        res = vuzpq_u16(vreinterpretq_u16_s32(low), vreinterpretq_u16_s32(high)).val[0];

        // bare multiplication, not SkFixedMul
        tmpl = vmull_u16(vget_low_u16(res), vdup_n_u16(max+1));
        tmph = vmull_u16(vget_high_u16(res), vdup_n_u16(max+1));

        // extraction of the 16 upper bits
        res = vuzpq_u16(vreinterpretq_u16_u32(tmpl), vreinterpretq_u16_u32(tmph)).val[1];

        return vreinterpretq_s16_u16(res);
    }

    static inline int32x4_t repeat4(int32x4_t f, unsigned max) {
        uint16x4_t res;
        uint32x4_t tmp;

        // get the lower 16 bits
        res = vmovn_u32(vreinterpretq_u32_s32(f));

        // bare multiplication, not SkFixedMul
        tmp = vmull_u16(res, vdup_n_u16(max+1));

        // extraction of the 16 upper bits
        tmp = vshrq_n_u32(tmp, 16);

        return vreinterpretq_s32_u32(tmp);
    }

    static inline int32x4_t extract_low_bits_repeat_mirror4(int32x4_t fx, unsigned max) {
        uint16x4_t res;
        uint32x4_t tmp;
        int32x4_t ret;

        // get the lower 16 bits
        res = vmovn_u32(vreinterpretq_u32_s32(fx));

        // bare multiplication, not SkFixedMul
        tmp = vmull_u16(res, vdup_n_u16(max + 1));

        // shift and mask
        ret = vshrq_n_s32(vreinterpretq_s32_u32(tmp), 12);

        /* We don't need the mask below because the caller will
         * overwrite the non-masked bits
         */
        //ret = vandq_s32(ret, vdupq_n_s32(0xF));

        return ret;
    }

    template <unsigned   (*tile)(SkFixed, int),
              int16x8_t (*tile8)(int32x4_t, int32x4_t, unsigned),
             bool tryDecal>
    static void nofilter_scale_neon(const SkBitmapProcState& s,
                                    uint32_t xy[], int count, int x, int y) {
        SkASSERT(s.fInvMatrix.isScaleTranslate());

        // we store y, x, x, x, x, x
        const unsigned maxX = s.fPixmap.width() - 1;
        SkFractionalInt fx;
        {
            const SkBitmapProcStateAutoMapper mapper(s, x, y);
            const unsigned maxY = s.fPixmap.height() - 1;
            *xy++ = tile(mapper.fixedY(), maxY);
            fx = mapper.fractionalIntX();
        }

        if (0 == maxX) {
            // all of the following X values must be 0
            memset(xy, 0, count * sizeof(uint16_t));
            return;
        }

        const SkFractionalInt dx = s.fInvSxFractionalInt;

        // test if we don't need to apply the tile proc
        const SkFixed fixedFx = SkFractionalIntToFixed(fx);
        const SkFixed fixedDx = SkFractionalIntToFixed(dx);
        if (tryDecal && can_truncate_to_fixed_for_decal(fixedFx, fixedDx, count, maxX)) {
            decal_nofilter_scale_neon(xy, fixedFx, fixedDx, count);
            return;
        }

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

                fx8 = tile8(lbase, hbase, maxX);

                vst1q_s16(dst16, fx8);

                // but preserving base & on to the next
                lbase = vaddq_s32 (lbase, vdupq_n_s32(SkFractionalIntToFixed(dx8)));
                hbase = vaddq_s32 (hbase, vdupq_n_s32(SkFractionalIntToFixed(dx8)));
                dst16 += 8;
                count -= 8;
                fx += dx8;
            }
            xy = (uint32_t *) dst16;
        }

        uint16_t* xx = (uint16_t*)xy;
        for (int i = count; i > 0; --i) {
            *xx++ = tile(SkFractionalIntToFixed(fx), maxX);
            fx += dx;
        }
    }

    // TODO: nofilter_affine_neon

    template <unsigned              (*tile )(SkFixed, int),
              int32x4_t             (*tile4)(int32x4_t, unsigned),
              unsigned  (*extract_low_bits )(SkFixed, int),
              int32x4_t (*extract_low_bits4)(int32x4_t, unsigned),
              bool tryDecal>
    static void filter_scale_neon(const SkBitmapProcState& s,
                                  uint32_t xy[], int count, int x, int y) {
        SkASSERT(s.fInvMatrix.isScaleTranslate());

        auto pack = [&](SkFixed f, unsigned max, SkFixed one) {
            unsigned i = tile(f, max);
            i = (i << 4) | extract_low_bits(f, max);
            return (i << 14) | (tile((f + one), max));
        };

        auto pack4 = [&](int32x4_t f, unsigned max, SkFixed one) {
            int32x4_t ret, res;

            res = tile4(f, max);

            ret = extract_low_bits4(f, max);
            ret = vsliq_n_s32(ret, res, 4);

            res = tile4(f + vdupq_n_s32(one), max);
            ret = vorrq_s32(vshlq_n_s32(ret, 14), res);

            return ret;
        };

        const unsigned maxX = s.fPixmap.width() - 1;
        const SkFixed one = s.fFilterOneX;
        const SkFractionalInt dx = s.fInvSxFractionalInt;
        SkFractionalInt fx;

        {
            const SkBitmapProcStateAutoMapper mapper(s, x, y);
            const SkFixed fy = mapper.fixedY();
            const unsigned maxY = s.fPixmap.height() - 1;
            // compute our two Y values up front
            *xy++ = pack(fy, maxY, s.fFilterOneY);
            // now initialize fx
            fx = mapper.fractionalIntX();
        }

        // test if we don't need to apply the tile proc
        const SkFixed fixedFx = SkFractionalIntToFixed(fx);
        const SkFixed fixedDx = SkFractionalIntToFixed(dx);
        if (tryDecal && can_truncate_to_fixed_for_decal(fixedFx, fixedDx, count, maxX)) {
            decal_filter_scale_neon(xy, fixedFx, fixedDx, count);
            return;
        }

        if (count >= 4) {
            int32x4_t wide_fx;

            wide_fx = vdupq_n_s32(SkFractionalIntToFixed(fx));
            wide_fx = vsetq_lane_s32(SkFractionalIntToFixed(fx+dx), wide_fx, 1);
            wide_fx = vsetq_lane_s32(SkFractionalIntToFixed(fx+dx+dx), wide_fx, 2);
            wide_fx = vsetq_lane_s32(SkFractionalIntToFixed(fx+dx+dx+dx), wide_fx, 3);

            while (count >= 4) {
                int32x4_t res;

                res = pack4(wide_fx, maxX, one);

                vst1q_u32(xy, vreinterpretq_u32_s32(res));

                wide_fx += vdupq_n_s32(SkFractionalIntToFixed(dx+dx+dx+dx));
                fx += dx+dx+dx+dx;
                xy += 4;
                count -= 4;
            }
        }

        while (--count >= 0) {
            *xy++ = pack(SkFractionalIntToFixed(fx), maxX, one);
            fx += dx;
        }
    }

    // TODO: filter_affine_neon

    static const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs[] = {
        nofilter_scale_neon<clamp, clamp8, true>,
        filter_scale_neon<clamp,
                          clamp4,
                          extract_low_bits_clamp,
                          extract_low_bits_clamp4,
                          true>,

        nofilter_affine<clamp>, filter_affine<clamp, extract_low_bits_clamp>,
    };

    static const SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs[] = {
        nofilter_scale_neon<repeat, repeat8, false>,
        filter_scale_neon<repeat,
                          repeat4,
                          extract_low_bits_repeat_mirror,
                          extract_low_bits_repeat_mirror4,
                          false>,

        nofilter_affine<repeat>, filter_affine<repeat, extract_low_bits_repeat_mirror>,
    };

#else
    static const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs[] = {
        nofilter_scale <clamp, true>, filter_scale <clamp, extract_low_bits_clamp, true>,
        nofilter_affine<clamp>,       filter_affine<clamp, extract_low_bits_clamp>,
    };

    static const SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs[] = {
        nofilter_scale <repeat, false>, filter_scale <repeat, extract_low_bits_repeat_mirror,false>,
        nofilter_affine<repeat>,        filter_affine<repeat, extract_low_bits_repeat_mirror>,
    };
#endif


///////////////////////////////////////////////////////////////////////////////
// This next chunk has some specializations for unfiltered translate-only matrices.

static inline U16CPU int_clamp(int x, int n) {
    if (x <  0) { x = 0; }
    if (x >= n) { x = n - 1; }
    return x;
}

/*  returns 0...(n-1) given any x (positive or negative).

    As an example, if n (which is always positive) is 5...

          x: -8 -7 -6 -5 -4 -3 -2 -1  0  1  2  3  4  5  6  7  8
    returns:  2  3  4  0  1  2  3  4  0  1  2  3  4  0  1  2  3
 */
static inline int sk_int_mod(int x, int n) {
    SkASSERT(n > 0);
    if ((unsigned)x >= (unsigned)n) {
        if (x < 0) {
            x = n + ~(~x % n);
        } else {
            x = x % n;
        }
    }
    return x;
}

static inline U16CPU int_repeat(int x, int n) {
    return sk_int_mod(x, n);
}

static inline U16CPU int_mirror(int x, int n) {
    x = sk_int_mod(x, 2 * n);
    if (x >= n) {
        x = n + ~(x - n);
    }
    return x;
}

static void fill_sequential(uint16_t xptr[], int pos, int count) {
    while (count --> 0) {
        *xptr++ = pos++;
    }
}

static void fill_backwards(uint16_t xptr[], int pos, int count) {
    while (count --> 0) {
        SkASSERT(pos >= 0);
        *xptr++ = pos--;
    }
}

static void clampx_nofilter_trans(const SkBitmapProcState& s,
                                  uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvMatrix.isTranslate());

    const SkBitmapProcStateAutoMapper mapper(s, x, y);
    *xy++ = int_clamp(mapper.intY(), s.fPixmap.height());
    int xpos = mapper.intX();

    const int width = s.fPixmap.width();
    if (1 == width) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    uint16_t* xptr = reinterpret_cast<uint16_t*>(xy);
    int n;

    // fill before 0 as needed
    if (xpos < 0) {
        n = -xpos;
        if (n > count) {
            n = count;
        }
        memset(xptr, 0, n * sizeof(uint16_t));
        count -= n;
        if (0 == count) {
            return;
        }
        xptr += n;
        xpos = 0;
    }

    // fill in 0..width-1 if needed
    if (xpos < width) {
        n = width - xpos;
        if (n > count) {
            n = count;
        }
        fill_sequential(xptr, xpos, n);
        count -= n;
        if (0 == count) {
            return;
        }
        xptr += n;
    }

    // fill the remaining with the max value
    sk_memset16(xptr, width - 1, count);
}

static void repeatx_nofilter_trans(const SkBitmapProcState& s,
                                   uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvMatrix.isTranslate());

    const SkBitmapProcStateAutoMapper mapper(s, x, y);
    *xy++ = int_repeat(mapper.intY(), s.fPixmap.height());
    int xpos = mapper.intX();

    const int width = s.fPixmap.width();
    if (1 == width) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    uint16_t* xptr = reinterpret_cast<uint16_t*>(xy);
    int start = sk_int_mod(xpos, width);
    int n = width - start;
    if (n > count) {
        n = count;
    }
    fill_sequential(xptr, start, n);
    xptr += n;
    count -= n;

    while (count >= width) {
        fill_sequential(xptr, 0, width);
        xptr += width;
        count -= width;
    }

    if (count > 0) {
        fill_sequential(xptr, 0, count);
    }
}

static void mirrorx_nofilter_trans(const SkBitmapProcState& s,
                                   uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvMatrix.isTranslate());

    const SkBitmapProcStateAutoMapper mapper(s, x, y);
    *xy++ = int_mirror(mapper.intY(), s.fPixmap.height());
    int xpos = mapper.intX();

    const int width = s.fPixmap.width();
    if (1 == width) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    uint16_t* xptr = reinterpret_cast<uint16_t*>(xy);
    // need to know our start, and our initial phase (forward or backward)
    bool forward;
    int n;
    int start = sk_int_mod(xpos, 2 * width);
    if (start >= width) {
        start = width + ~(start - width);
        forward = false;
        n = start + 1;  // [start .. 0]
    } else {
        forward = true;
        n = width - start;  // [start .. width)
    }
    if (n > count) {
        n = count;
    }
    if (forward) {
        fill_sequential(xptr, start, n);
    } else {
        fill_backwards(xptr, start, n);
    }
    forward = !forward;
    xptr += n;
    count -= n;

    while (count >= width) {
        if (forward) {
            fill_sequential(xptr, 0, width);
        } else {
            fill_backwards(xptr, width - 1, width);
        }
        forward = !forward;
        xptr += width;
        count -= width;
    }

    if (count > 0) {
        if (forward) {
            fill_sequential(xptr, 0, count);
        } else {
            fill_backwards(xptr, width - 1, count);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// The main entry point to the file, choosing between everything above.

SkBitmapProcState::MatrixProc SkBitmapProcState::chooseMatrixProc(bool translate_only_matrix) {
    SkASSERT(!fInvMatrix.hasPerspective());
    SkASSERT(fTileModeX == fTileModeY);
    SkASSERT(fTileModeX != SkTileMode::kDecal);

    // Check for our special case translate methods when there is no scale/affine/perspective.
    if (translate_only_matrix && kNone_SkFilterQuality == fFilterQuality) {
        switch (fTileModeX) {
            default: SkASSERT(false);
            case SkTileMode::kClamp:  return  clampx_nofilter_trans;
            case SkTileMode::kRepeat: return repeatx_nofilter_trans;
            case SkTileMode::kMirror: return mirrorx_nofilter_trans;
        }
    }

    // The arrays are all [ nofilter, filter ].
    int index = fFilterQuality > kNone_SkFilterQuality ? 1 : 0;
    if (!fInvMatrix.isScaleTranslate()) {
        index |= 2;
    }

    if (fTileModeX == SkTileMode::kClamp) {
        // clamp gets special version of filterOne, working in non-normalized space (allowing decal)
        fFilterOneX = SK_Fixed1;
        fFilterOneY = SK_Fixed1;
        return ClampX_ClampY_Procs[index];
    }

    // all remaining procs use this form for filterOne, putting them into normalized space.
    fFilterOneX = SK_Fixed1 / fPixmap.width();
    fFilterOneY = SK_Fixed1 / fPixmap.height();

    if (fTileModeX == SkTileMode::kRepeat) {
        return RepeatX_RepeatY_Procs[index];
    }

    return MirrorX_MirrorY_Procs[index];
}
