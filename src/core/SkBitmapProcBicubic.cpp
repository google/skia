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

inline SkPMColor cubicBlend(const SkScalar c[16], SkScalar t, SkPMColor c0, SkPMColor c1, SkPMColor c2, SkPMColor c3) {
    SkScalar t2 = t * t, t3 = t2 * t;
    SkScalar cc[4];
    cc[0] = c[0]  + SkScalarMul(c[1], t) + SkScalarMul(c[2], t2) + SkScalarMul(c[3], t3);
    cc[1] = c[4]  + SkScalarMul(c[5], t) + SkScalarMul(c[6], t2) + SkScalarMul(c[7], t3);
    cc[2] = c[8]  + SkScalarMul(c[9], t) + SkScalarMul(c[10], t2) + SkScalarMul(c[11], t3);
    cc[3] = c[12] + SkScalarMul(c[13], t) + SkScalarMul(c[14], t2) + SkScalarMul(c[15], t3);
    SkScalar a = SkScalarClampMax(SkScalarMul(cc[0], SkGetPackedA32(c0)) + SkScalarMul(cc[1], SkGetPackedA32(c1)) + SkScalarMul(cc[2], SkGetPackedA32(c2)) + SkScalarMul(cc[3], SkGetPackedA32(c3)), 255);
    SkScalar r = SkScalarMul(cc[0], SkGetPackedR32(c0)) + SkScalarMul(cc[1], SkGetPackedR32(c1)) + SkScalarMul(cc[2], SkGetPackedR32(c2)) + SkScalarMul(cc[3], SkGetPackedR32(c3));
    SkScalar g = SkScalarMul(cc[0], SkGetPackedG32(c0)) + SkScalarMul(cc[1], SkGetPackedG32(c1)) + SkScalarMul(cc[2], SkGetPackedG32(c2)) + SkScalarMul(cc[3], SkGetPackedG32(c3));
    SkScalar b = SkScalarMul(cc[0], SkGetPackedB32(c0)) + SkScalarMul(cc[1], SkGetPackedB32(c1)) + SkScalarMul(cc[2], SkGetPackedB32(c2)) + SkScalarMul(cc[3], SkGetPackedB32(c3));
    return SkPackARGB32(SkScalarRoundToInt(a),
                        SkScalarRoundToInt(SkScalarClampMax(r, a)),
                        SkScalarRoundToInt(SkScalarClampMax(g, a)),
                        SkScalarRoundToInt(SkScalarClampMax(b, a)));
}

static void bicubicFilter(const SkBitmapProcState& s, int x, int y,
                          SkPMColor colors[], int count) {

    static const SkScalar coefficients[16] = {
        DS( 1.0 / 18.0), DS(-9.0 / 18.0), DS( 15.0 / 18.0), DS( -7.0 / 18.0),
        DS(16.0 / 18.0), DS( 0.0 / 18.0), DS(-36.0 / 18.0), DS( 21.0 / 18.0),
        DS( 1.0 / 18.0), DS( 9.0 / 18.0), DS( 27.0 / 18.0), DS(-21.0 / 18.0),
        DS( 0.0 / 18.0), DS( 0.0 / 18.0), DS( -6.0 / 18.0), DS(  7.0 / 18.0),
    };

    SkPMColor *dptr = &(colors[0]);
    while (count-- > 0) {
        SkPoint srcPt;
        s.fInvProc(*s.fInvMatrix, SkIntToScalar(x),
                    SkIntToScalar(y), &srcPt);
        srcPt.fX -= SK_ScalarHalf;
        srcPt.fY -= SK_ScalarHalf;
        SkScalar fractx = srcPt.fX - SkScalarFloorToScalar(srcPt.fX);
        SkScalar fracty = srcPt.fY - SkScalarFloorToScalar(srcPt.fY);
        int sx = SkScalarFloorToInt(srcPt.fX);
        int sy = SkScalarFloorToInt(srcPt.fY);
        int x0 = SkClampMax(sx - 1, s.fBitmap->width() - 1);
        int x1 = SkClampMax(sx    , s.fBitmap->width() - 1);
        int x2 = SkClampMax(sx + 1, s.fBitmap->width() - 1);
        int x3 = SkClampMax(sx + 2, s.fBitmap->width() - 1);
        int y0 = SkClampMax(sy - 1, s.fBitmap->height() - 1);
        int y1 = SkClampMax(sy    , s.fBitmap->height() - 1);
        int y2 = SkClampMax(sy + 1, s.fBitmap->height() - 1);
        int y3 = SkClampMax(sy + 2, s.fBitmap->height() - 1);
        SkPMColor s00 = *s.fBitmap->getAddr32(x0, y0);
        SkPMColor s10 = *s.fBitmap->getAddr32(x1, y0);
        SkPMColor s20 = *s.fBitmap->getAddr32(x2, y0);
        SkPMColor s30 = *s.fBitmap->getAddr32(x3, y0);
        SkPMColor s0 = cubicBlend(coefficients, fractx, s00, s10, s20, s30);
        SkPMColor s01 = *s.fBitmap->getAddr32(x0, y1);
        SkPMColor s11 = *s.fBitmap->getAddr32(x1, y1);
        SkPMColor s21 = *s.fBitmap->getAddr32(x2, y1);
        SkPMColor s31 = *s.fBitmap->getAddr32(x3, y1);
        SkPMColor s1 = cubicBlend(coefficients, fractx, s01, s11, s21, s31);
        SkPMColor s02 = *s.fBitmap->getAddr32(x0, y2);
        SkPMColor s12 = *s.fBitmap->getAddr32(x1, y2);
        SkPMColor s22 = *s.fBitmap->getAddr32(x2, y2);
        SkPMColor s32 = *s.fBitmap->getAddr32(x3, y2);
        SkPMColor s2 = cubicBlend(coefficients, fractx, s02, s12, s22, s32);
        SkPMColor s03 = *s.fBitmap->getAddr32(x0, y3);
        SkPMColor s13 = *s.fBitmap->getAddr32(x1, y3);
        SkPMColor s23 = *s.fBitmap->getAddr32(x2, y3);
        SkPMColor s33 = *s.fBitmap->getAddr32(x3, y3);
        SkPMColor s3 = cubicBlend(coefficients, fractx, s03, s13, s23, s33);
        *dptr++ = cubicBlend(coefficients, fracty, s0, s1, s2, s3);
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

    return bicubicFilter;
}
