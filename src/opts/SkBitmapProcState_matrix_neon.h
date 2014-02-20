
#include <arm_neon.h>


#define SCALE_NOFILTER_NAME     MAKENAME(_nofilter_scale)
#define SCALE_FILTER_NAME       MAKENAME(_filter_scale)
#define AFFINE_NOFILTER_NAME    MAKENAME(_nofilter_affine)
#define AFFINE_FILTER_NAME      MAKENAME(_filter_affine)
#define PERSP_NOFILTER_NAME     MAKENAME(_nofilter_persp)
#define PERSP_FILTER_NAME       MAKENAME(_filter_persp)

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
    const unsigned maxX = s.fBitmap->width() - 1;
    SkFractionalInt fx;
    {
        SkPoint pt;
        s.fInvProc(s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                                 SkIntToScalar(y) + SK_ScalarHalf, &pt);
        fx = SkScalarToFractionalInt(pt.fY);
        const unsigned maxY = s.fBitmap->height() - 1;
        *xy++ = TILEY_PROCF(SkFractionalIntToFixed(fx), maxY);
        fx = SkScalarToFractionalInt(pt.fX);
    }

    if (0 == maxX) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    const SkFractionalInt dx = s.fInvSxFractionalInt;

#ifdef CHECK_FOR_DECAL
    // test if we don't need to apply the tile proc
    if (can_truncate_to_fixed_for_decal(fx, dx, count, maxX)) {
        decal_nofilter_scale_neon(xy, SkFractionalIntToFixed(fx),
                             SkFractionalIntToFixed(dx), count);
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

static void AFFINE_NOFILTER_NAME(const SkBitmapProcState& s,
                                 uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvType & SkMatrix::kAffine_Mask);
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask |
                             SkMatrix::kAffine_Mask)) == 0);

    PREAMBLE(s);
    SkPoint srcPt;
    s.fInvProc(s.fInvMatrix,
               SkIntToScalar(x) + SK_ScalarHalf,
               SkIntToScalar(y) + SK_ScalarHalf, &srcPt);

    SkFractionalInt fx = SkScalarToFractionalInt(srcPt.fX);
    SkFractionalInt fy = SkScalarToFractionalInt(srcPt.fY);
    SkFractionalInt dx = s.fInvSxFractionalInt;
    SkFractionalInt dy = s.fInvKyFractionalInt;
    int maxX = s.fBitmap->width() - 1;
    int maxY = s.fBitmap->height() - 1;

    if (count >= 8) {
        SkFractionalInt dx4 = dx * 4;
        SkFractionalInt dy4 = dy * 4;
        SkFractionalInt dx8 = dx * 8;
        SkFractionalInt dy8 = dy * 8;

        int32x4_t xbase, ybase;
        int32x4_t x2base, y2base;
        int16_t *dst16 = (int16_t *) xy;

        // now build fx, fx+dx, fx+2dx, fx+3dx
        xbase = vdupq_n_s32(SkFractionalIntToFixed(fx));
        xbase = vsetq_lane_s32(SkFractionalIntToFixed(fx+dx), xbase, 1);
        xbase = vsetq_lane_s32(SkFractionalIntToFixed(fx+dx+dx), xbase, 2);
        xbase = vsetq_lane_s32(SkFractionalIntToFixed(fx+dx+dx+dx), xbase, 3);

        // same for fy
        ybase = vdupq_n_s32(SkFractionalIntToFixed(fy));
        ybase = vsetq_lane_s32(SkFractionalIntToFixed(fy+dy), ybase, 1);
        ybase = vsetq_lane_s32(SkFractionalIntToFixed(fy+dy+dy), ybase, 2);
        ybase = vsetq_lane_s32(SkFractionalIntToFixed(fy+dy+dy+dy), ybase, 3);

        x2base = vaddq_s32(xbase, vdupq_n_s32(SkFractionalIntToFixed(dx4)));
        y2base = vaddq_s32(ybase, vdupq_n_s32(SkFractionalIntToFixed(dy4)));

        // store & bump
        do {
            int16x8x2_t hi16;

            hi16.val[0] = TILEX_PROCF_NEON8(xbase, x2base, maxX);
            hi16.val[1] = TILEY_PROCF_NEON8(ybase, y2base, maxY);

            vst2q_s16(dst16, hi16);

            // moving base and on to the next
            xbase = vaddq_s32(xbase, vdupq_n_s32(SkFractionalIntToFixed(dx8)));
            ybase = vaddq_s32(ybase, vdupq_n_s32(SkFractionalIntToFixed(dy8)));
            x2base = vaddq_s32(x2base, vdupq_n_s32(SkFractionalIntToFixed(dx8)));
            y2base = vaddq_s32(y2base, vdupq_n_s32(SkFractionalIntToFixed(dy8)));

            dst16 += 16; // 8x32 aka 16x16
            count -= 8;
            fx += dx8;
            fy += dy8;
        } while (count >= 8);
        xy = (uint32_t *) dst16;
    }

    for (int i = count; i > 0; --i) {
        *xy++ = (TILEY_PROCF(SkFractionalIntToFixed(fy), maxY) << 16) |
                 TILEX_PROCF(SkFractionalIntToFixed(fx), maxX);
        fx += dx; fy += dy;
    }
}

static void PERSP_NOFILTER_NAME(const SkBitmapProcState& s,
                                uint32_t* SK_RESTRICT xy,
                                int count, int x, int y) {
    SkASSERT(s.fInvType & SkMatrix::kPerspective_Mask);

    PREAMBLE(s);
    // max{X,Y} are int here, but later shown/assumed to fit in 16 bits
    int maxX = s.fBitmap->width() - 1;
    int maxY = s.fBitmap->height() - 1;

    SkPerspIter iter(s.fInvMatrix,
                     SkIntToScalar(x) + SK_ScalarHalf,
                     SkIntToScalar(y) + SK_ScalarHalf, count);

    while ((count = iter.next()) != 0) {
        const SkFixed* SK_RESTRICT srcXY = iter.getXY();

        if (count >= 8) {
            int32_t *mysrc = (int32_t *) srcXY;
            int16_t *mydst = (int16_t *) xy;
            do {
                int16x8x2_t hi16;
                int32x4x2_t xy1, xy2;

                xy1 = vld2q_s32(mysrc);
                xy2 = vld2q_s32(mysrc+8);

                hi16.val[0] = TILEX_PROCF_NEON8(xy1.val[0], xy2.val[0], maxX);
                hi16.val[1] = TILEY_PROCF_NEON8(xy1.val[1], xy2.val[1], maxY);

                vst2q_s16(mydst, hi16);

                count -= 8;  // 8 iterations
                mysrc += 16; // 16 longs
                mydst += 16; // 16 shorts, aka 8 longs
            } while (count >= 8);
            // get xy and srcXY fixed up
            srcXY = (const SkFixed *) mysrc;
            xy = (uint32_t *) mydst;
        }

        while (--count >= 0) {
            *xy++ = (TILEY_PROCF(srcXY[1], maxY) << 16) |
                     TILEX_PROCF(srcXY[0], maxX);
            srcXY += 2;
        }
    }
}

static inline uint32_t PACK_FILTER_Y_NAME(SkFixed f, unsigned max,
                                          SkFixed one PREAMBLE_PARAM_Y) {
    unsigned i = TILEY_PROCF(f, max);
    i = (i << 4) | TILEY_LOW_BITS(f, max);
    return (i << 14) | (TILEY_PROCF((f + one), max));
}

static inline uint32_t PACK_FILTER_X_NAME(SkFixed f, unsigned max,
                                          SkFixed one PREAMBLE_PARAM_X) {
    unsigned i = TILEX_PROCF(f, max);
    i = (i << 4) | TILEX_LOW_BITS(f, max);
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
    ret = TILEX_LOW_BITS_NEON4(f, max);
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
    ret = TILEY_LOW_BITS_NEON4(f, max);
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

    const unsigned maxX = s.fBitmap->width() - 1;
    const SkFixed one = s.fFilterOneX;
    const SkFractionalInt dx = s.fInvSxFractionalInt;
    SkFractionalInt fx;

    {
        SkPoint pt;
        s.fInvProc(s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                                 SkIntToScalar(y) + SK_ScalarHalf, &pt);
        const SkFixed fy = SkScalarToFixed(pt.fY) - (s.fFilterOneY >> 1);
        const unsigned maxY = s.fBitmap->height() - 1;
        // compute our two Y values up front
        *xy++ = PACK_FILTER_Y_NAME(fy, maxY, s.fFilterOneY PREAMBLE_ARG_Y);
        // now initialize fx
        fx = SkScalarToFractionalInt(pt.fX) - (SkFixedToFractionalInt(one) >> 1);
    }

#ifdef CHECK_FOR_DECAL
    // test if we don't need to apply the tile proc
    if (can_truncate_to_fixed_for_decal(fx, dx, count, maxX)) {
        decal_filter_scale_neon(xy, SkFractionalIntToFixed(fx),
                             SkFractionalIntToFixed(dx), count);
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

static void AFFINE_FILTER_NAME(const SkBitmapProcState& s,
                               uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvType & SkMatrix::kAffine_Mask);
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask |
                             SkMatrix::kAffine_Mask)) == 0);

    PREAMBLE(s);
    SkPoint srcPt;
    s.fInvProc(s.fInvMatrix,
               SkIntToScalar(x) + SK_ScalarHalf,
               SkIntToScalar(y) + SK_ScalarHalf, &srcPt);

    SkFixed oneX = s.fFilterOneX;
    SkFixed oneY = s.fFilterOneY;
    SkFixed fx = SkScalarToFixed(srcPt.fX) - (oneX >> 1);
    SkFixed fy = SkScalarToFixed(srcPt.fY) - (oneY >> 1);
    SkFixed dx = s.fInvSx;
    SkFixed dy = s.fInvKy;
    unsigned maxX = s.fBitmap->width() - 1;
    unsigned maxY = s.fBitmap->height() - 1;

    if (count >= 4) {
        int32x4_t wide_fy, wide_fx;

        wide_fx = vdupq_n_s32(fx);
        wide_fx = vsetq_lane_s32(fx+dx, wide_fx, 1);
        wide_fx = vsetq_lane_s32(fx+dx+dx, wide_fx, 2);
        wide_fx = vsetq_lane_s32(fx+dx+dx+dx, wide_fx, 3);

        wide_fy = vdupq_n_s32(fy);
        wide_fy = vsetq_lane_s32(fy+dy, wide_fy, 1);
        wide_fy = vsetq_lane_s32(fy+dy+dy, wide_fy, 2);
        wide_fy = vsetq_lane_s32(fy+dy+dy+dy, wide_fy, 3);

        while (count >= 4) {
            int32x4x2_t vxy;

            // do the X side, then the Y side, then interleave them
            vxy.val[0] = PACK_FILTER_Y4_NAME(wide_fy, maxY, oneY PREAMBLE_ARG_Y);
            vxy.val[1] = PACK_FILTER_X4_NAME(wide_fx, maxX, oneX PREAMBLE_ARG_X);

            // interleave as YXYXYXYX as part of the storing
            vst2q_s32((int32_t*)xy, vxy);

            // prepare next iteration
            wide_fx += vdupq_n_s32(dx+dx+dx+dx);
            fx += dx + dx + dx + dx;
            wide_fy += vdupq_n_s32(dy+dy+dy+dy);
            fy += dy+dy+dy+dy;
            xy += 8; // 4 x's, 4 y's
            count -= 4;
        }
    }

    while (--count >= 0) {
        // NB: writing Y/X
        *xy++ = PACK_FILTER_Y_NAME(fy, maxY, oneY PREAMBLE_ARG_Y);
        fy += dy;
        *xy++ = PACK_FILTER_X_NAME(fx, maxX, oneX PREAMBLE_ARG_X);
        fx += dx;
    }
}

static void PERSP_FILTER_NAME(const SkBitmapProcState& s,
                              uint32_t* SK_RESTRICT xy, int count,
                              int x, int y) {
    SkASSERT(s.fInvType & SkMatrix::kPerspective_Mask);

    PREAMBLE(s);
    unsigned maxX = s.fBitmap->width() - 1;
    unsigned maxY = s.fBitmap->height() - 1;
    SkFixed oneX = s.fFilterOneX;
    SkFixed oneY = s.fFilterOneY;

    SkPerspIter iter(s.fInvMatrix,
                     SkIntToScalar(x) + SK_ScalarHalf,
                     SkIntToScalar(y) + SK_ScalarHalf, count);

    while ((count = iter.next()) != 0) {
        const SkFixed* SK_RESTRICT srcXY = iter.getXY();

        while (count >= 4) {
            int32x4_t wide_x, wide_y;
            int32x4x2_t vxy, vresyx;

            // load src:  x-y-x-y-x-y-x-y
            vxy = vld2q_s32(srcXY);

            // do the X side, then the Y side, then interleave them
            wide_x = vsubq_s32(vxy.val[0], vdupq_n_s32(oneX>>1));
            wide_y = vsubq_s32(vxy.val[1], vdupq_n_s32(oneY>>1));

            vresyx.val[0] = PACK_FILTER_Y4_NAME(wide_y, maxY, oneY PREAMBLE_ARG_Y);
            vresyx.val[1] = PACK_FILTER_X4_NAME(wide_x, maxX, oneX PREAMBLE_ARG_X);

            // store interleaved as y-x-y-x-y-x-y-x (NB != read order)
            vst2q_s32((int32_t*)xy, vresyx);

            // on to the next iteration
            srcXY += 2*4;
            count -= 4;
            xy += 2*4;
        }

        while (--count >= 0) {
            // NB: we read x/y, we write y/x
            *xy++ = PACK_FILTER_Y_NAME(srcXY[1] - (oneY >> 1), maxY,
                                       oneY PREAMBLE_ARG_Y);
            *xy++ = PACK_FILTER_X_NAME(srcXY[0] - (oneX >> 1), maxX,
                                       oneX PREAMBLE_ARG_X);
            srcXY += 2;
        }
    }
}

const SkBitmapProcState::MatrixProc MAKENAME(_Procs)[] = {
    SCALE_NOFILTER_NAME,
    SCALE_FILTER_NAME,
    AFFINE_NOFILTER_NAME,
    AFFINE_FILTER_NAME,
    PERSP_NOFILTER_NAME,
    PERSP_FILTER_NAME
};

#undef TILEX_PROCF_NEON8
#undef TILEY_PROCF_NEON8
#undef TILEX_PROCF_NEON4
#undef TILEY_PROCF_NEON4
#undef TILEX_LOW_BITS_NEON4
#undef TILEY_LOW_BITS_NEON4

#undef MAKENAME
#undef TILEX_PROCF
#undef TILEY_PROCF
#ifdef CHECK_FOR_DECAL
    #undef CHECK_FOR_DECAL
#endif

#undef SCALE_NOFILTER_NAME
#undef SCALE_FILTER_NAME
#undef AFFINE_NOFILTER_NAME
#undef AFFINE_FILTER_NAME
#undef PERSP_NOFILTER_NAME
#undef PERSP_FILTER_NAME

#undef PREAMBLE
#undef PREAMBLE_PARAM_X
#undef PREAMBLE_PARAM_Y
#undef PREAMBLE_ARG_X
#undef PREAMBLE_ARG_Y

#undef TILEX_LOW_BITS
#undef TILEY_LOW_BITS
