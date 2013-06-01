/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkUnPreMultiply.h"
#include "SkRTConf.h"
#include "SkShader.h"

#define DS(x) SkDoubleToScalar(x)

#define MUL(a, b)   ((a) * (b))

static inline SkPMColor cubicBlend(const SkFixed cc[4], SkPMColor c0, SkPMColor c1, SkPMColor c2, SkPMColor c3) {
    SkFixed fa = MUL(cc[0], SkGetPackedA32(c0)) + MUL(cc[1], SkGetPackedA32(c1)) + MUL(cc[2], SkGetPackedA32(c2)) + MUL(cc[3], SkGetPackedA32(c3));
    SkFixed fr = MUL(cc[0], SkGetPackedR32(c0)) + MUL(cc[1], SkGetPackedR32(c1)) + MUL(cc[2], SkGetPackedR32(c2)) + MUL(cc[3], SkGetPackedR32(c3));
    SkFixed fg = MUL(cc[0], SkGetPackedG32(c0)) + MUL(cc[1], SkGetPackedG32(c1)) + MUL(cc[2], SkGetPackedG32(c2)) + MUL(cc[3], SkGetPackedG32(c3));
    SkFixed fb = MUL(cc[0], SkGetPackedB32(c0)) + MUL(cc[1], SkGetPackedB32(c1)) + MUL(cc[2], SkGetPackedB32(c2)) + MUL(cc[3], SkGetPackedB32(c3));

    int a = SkClampMax(SkFixedRoundToInt(fa), 255);
    int r = SkClampMax(SkFixedRoundToInt(fr), a);
    int g = SkClampMax(SkFixedRoundToInt(fg), a);
    int b = SkClampMax(SkFixedRoundToInt(fb), a);

    return SkPackARGB32(a, r, g, b);
}

static float poly_eval(const float cc[4], float t) {
    return cc[0] + t * (cc[1] + t * (cc[2] + t * cc[3]));
}

static void build_coeff4(SkFixed dst[4], float t) {
    static const SkScalar coefficients[16] = {
        DS( 1.0 / 18.0), DS(-9.0 / 18.0), DS( 15.0 / 18.0), DS( -7.0 / 18.0),
        DS(16.0 / 18.0), DS( 0.0 / 18.0), DS(-36.0 / 18.0), DS( 21.0 / 18.0),
        DS( 1.0 / 18.0), DS( 9.0 / 18.0), DS( 27.0 / 18.0), DS(-21.0 / 18.0),
        DS( 0.0 / 18.0), DS( 0.0 / 18.0), DS( -6.0 / 18.0), DS(  7.0 / 18.0),
    };

    dst[0] = SkFloatToFixed(poly_eval(&coefficients[ 0], t));
    dst[1] = SkFloatToFixed(poly_eval(&coefficients[ 4], t));
    dst[2] = SkFloatToFixed(poly_eval(&coefficients[ 8], t));
    dst[3] = SkFloatToFixed(poly_eval(&coefficients[12], t));
}

static SkPMColor doBicubicFilter(const SkBitmap *bm, SkFixed coeffX[4], SkFixed coeffY[4],
                            int x0, int x1, int x2, int x3,
                            int y0, int y1, int y2, int y3 )
{
    SkPMColor s00 = *bm->getAddr32(x0, y0);
    SkPMColor s10 = *bm->getAddr32(x1, y0);
    SkPMColor s20 = *bm->getAddr32(x2, y0);
    SkPMColor s30 = *bm->getAddr32(x3, y0);
    SkPMColor s0 = cubicBlend(coeffX, s00, s10, s20, s30);
    SkPMColor s01 = *bm->getAddr32(x0, y1);
    SkPMColor s11 = *bm->getAddr32(x1, y1);
    SkPMColor s21 = *bm->getAddr32(x2, y1);
    SkPMColor s31 = *bm->getAddr32(x3, y1);
    SkPMColor s1 = cubicBlend(coeffX, s01, s11, s21, s31);
    SkPMColor s02 = *bm->getAddr32(x0, y2);
    SkPMColor s12 = *bm->getAddr32(x1, y2);
    SkPMColor s22 = *bm->getAddr32(x2, y2);
    SkPMColor s32 = *bm->getAddr32(x3, y2);
    SkPMColor s2 = cubicBlend(coeffX, s02, s12, s22, s32);
    SkPMColor s03 = *bm->getAddr32(x0, y3);
    SkPMColor s13 = *bm->getAddr32(x1, y3);
    SkPMColor s23 = *bm->getAddr32(x2, y3);
    SkPMColor s33 = *bm->getAddr32(x3, y3);
    SkPMColor s3 = cubicBlend(coeffX, s03, s13, s23, s33);
    return cubicBlend(coeffY, s0, s1, s2, s3);
}

static void bicubicFilter(const SkBitmapProcState& s, int x, int y,
                          SkPMColor* SK_RESTRICT colors, int count) {

    const int maxX = s.fBitmap->width() - 1;
    const int maxY = s.fBitmap->height() - 1;

    while (count-- > 0) {
        SkPoint srcPt;
        s.fInvProc(*s.fInvMatrix, SkIntToScalar(x),
                    SkIntToScalar(y), &srcPt);
        srcPt.fX -= SK_ScalarHalf;
        srcPt.fY -= SK_ScalarHalf;
        SkScalar fractx = srcPt.fX - SkScalarFloorToScalar(srcPt.fX);
        SkScalar fracty = srcPt.fY - SkScalarFloorToScalar(srcPt.fY);

        SkFixed coeffX[4], coeffY[4];
        build_coeff4(coeffX, fractx);
        build_coeff4(coeffY, fracty);

        int sx = SkScalarFloorToInt(srcPt.fX);
        int sy = SkScalarFloorToInt(srcPt.fY);

        // Here is where we can support other tile modes (e.g. repeat or mirror)
        int x0 = SkClampMax(sx - 1, maxX);
        int x1 = SkClampMax(sx    , maxX);
        int x2 = SkClampMax(sx + 1, maxX);
        int x3 = SkClampMax(sx + 2, maxX);
        int y0 = SkClampMax(sy - 1, maxY);
        int y1 = SkClampMax(sy    , maxY);
        int y2 = SkClampMax(sy + 1, maxY);
        int y3 = SkClampMax(sy + 2, maxY);

        *colors++ = doBicubicFilter( s.fBitmap, coeffX, coeffY, x0, x1, x2, x3, y0, y1, y2, y3 );

        x++;
    }
}

static void bicubicFilter_ScaleOnly(const SkBitmapProcState &s, int x, int y,
                                    SkPMColor *SK_RESTRICT colors, int count) {
    const int maxX = s.fBitmap->width() - 1;
    const int maxY = s.fBitmap->height() - 1;

    SkPoint srcPt;
    s.fInvProc(*s.fInvMatrix, SkIntToScalar(x), SkIntToScalar(y), &srcPt);
    srcPt.fY -= SK_ScalarHalf;
    SkScalar fracty = srcPt.fY - SkScalarFloorToScalar(srcPt.fY);
    SkFixed coeffX[4], coeffY[4];
    build_coeff4(coeffY, fracty);
    int sy = SkScalarFloorToInt(srcPt.fY);
    int y0 = SkClampMax(sy - 1, maxY);
    int y1 = SkClampMax(sy    , maxY);
    int y2 = SkClampMax(sy + 1, maxY);
    int y3 = SkClampMax(sy + 2, maxY);

    while (count-- > 0) {
        s.fInvProc(*s.fInvMatrix, SkIntToScalar(x), SkIntToScalar(y), &srcPt);
        srcPt.fX -= SK_ScalarHalf;
        SkScalar fractx = srcPt.fX - SkScalarFloorToScalar(srcPt.fX);

        build_coeff4(coeffX, fractx);

        int sx = SkScalarFloorToInt(srcPt.fX);

        // Here is where we can support other tile modes (e.g. repeat or mirror)
        int x0 = SkClampMax(sx - 1, maxX);
        int x1 = SkClampMax(sx    , maxX);
        int x2 = SkClampMax(sx + 1, maxX);
        int x3 = SkClampMax(sx + 2, maxX);

        *colors++ = doBicubicFilter( s.fBitmap, coeffX, coeffY, x0, x1, x2, x3, y0, y1, y2, y3 );

        x++;
    }
}

SkBitmapProcState::ShaderProc32
SkBitmapProcState::chooseBicubicFilterProc(const SkPaint& paint) {
    // we need to be requested
    uint32_t mask = SkPaint::kFilterBitmap_Flag
                  | SkPaint::kBicubicFilterBitmap_Flag
                  ;
    if ((paint.getFlags() & mask) != mask) {
        return NULL;
    }

    // TODO: consider supporting other configs (e.g. 565, A8)
    if (fBitmap->config() != SkBitmap::kARGB_8888_Config) {
        return NULL;
    }

    // TODO: consider supporting repeat and mirror
    if (SkShader::kClamp_TileMode != fTileModeX || SkShader::kClamp_TileMode != fTileModeY) {
        return NULL;
    }

    // TODO: support blending inside our procs
    if (0xFF != paint.getAlpha()) {
        return NULL;
    }

    if (fInvType & SkMatrix::kAffine_Mask) {
        return bicubicFilter;
    } else if (fInvType & SkMatrix::kScale_Mask) {
        return bicubicFilter_ScaleOnly;
    } else {
        return NULL;
    }
}
