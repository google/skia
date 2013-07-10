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
#include "SkShader.h"
#include "SkRTConf.h"
#include "SkMath.h"

void highQualityFilter(const SkBitmapProcState& s, int x, int y,
                   SkPMColor* SK_RESTRICT colors, int count) {

    const int maxX = s.fBitmap->width() - 1;
    const int maxY = s.fBitmap->height() - 1;

    while (count-- > 0) {
        SkPoint srcPt;
        s.fInvProc(*s.fInvMatrix, SkFloatToScalar(x + 0.5),
                    SkFloatToScalar(y + 0.5), &srcPt);
        srcPt.fX -= SK_ScalarHalf;
        srcPt.fY -= SK_ScalarHalf;

        int sx = SkScalarFloorToInt(srcPt.fX);
        int sy = SkScalarFloorToInt(srcPt.fY);

        SkFixed weight = 0;
        SkFixed fr = 0, fg = 0, fb = 0, fa = 0;

        int y0 = SkClampMax(sk_float_ceil2int(SkScalarToFloat(srcPt.fY)-s.getBitmapFilter()->width()), maxY);
        int y1 = SkClampMax(sk_float_floor2int(SkScalarToFloat(srcPt.fY)+s.getBitmapFilter()->width()), maxY);
        int x0 = SkClampMax(sk_float_ceil2int(SkScalarToFloat(srcPt.fX)-s.getBitmapFilter()->width()), maxX);
        int x1 = SkClampMax(sk_float_floor2int(SkScalarToFloat(srcPt.fX)+s.getBitmapFilter()->width()), maxX);

        for (int src_y = y0; src_y <= y1; src_y++) {
            SkFixed yweight = s.getBitmapFilter()->lookup((srcPt.fY - src_y));

            for (int src_x = x0; src_x <= x1 ; src_x++) {
                SkFixed xweight = s.getBitmapFilter()->lookup((srcPt.fX - src_x));

                SkFixed combined_weight = SkFixedMul(xweight, yweight);

                SkPMColor c = *s.fBitmap->getAddr32(src_x, src_y);
                fr += combined_weight * SkGetPackedR32(c);
                fg += combined_weight * SkGetPackedG32(c);
                fb += combined_weight * SkGetPackedB32(c);
                fa += combined_weight * SkGetPackedA32(c);
                weight += combined_weight;
            }
        }

        fr = SkFixedDiv(fr, weight);
        fg = SkFixedDiv(fg, weight);
        fb = SkFixedDiv(fb, weight);
        fa = SkFixedDiv(fa, weight);

        int a = SkClampMax(SkFixedRoundToInt(fa), 255);
        int r = SkClampMax(SkFixedRoundToInt(fr), a);
        int g = SkClampMax(SkFixedRoundToInt(fg), a);
        int b = SkClampMax(SkFixedRoundToInt(fb), a);

        *colors++ = SkPackARGB32(a, r, g, b);

        x++;
    }
}

void highQualityFilter_ScaleOnly(const SkBitmapProcState &s, int x, int y,
                             SkPMColor *SK_RESTRICT colors, int count) {
     const int maxX = s.fBitmap->width() - 1;
     const int maxY = s.fBitmap->height() - 1;

     SkPoint srcPt;

     s.fInvProc(*s.fInvMatrix, SkFloatToScalar(x + 0.5),
                 SkFloatToScalar(y + 0.5), &srcPt);
     srcPt.fY -= SK_ScalarHalf;
     int sy = SkScalarFloorToInt(srcPt.fY);
     int y0 = SkClampMax(sk_float_ceil2int(SkScalarToFloat(srcPt.fY)-s.getBitmapFilter()->width()), maxY);
     int y1 = SkClampMax(sk_float_floor2int(SkScalarToFloat(srcPt.fY)+s.getBitmapFilter()->width()), maxY);

     while (count-- > 0) {
         s.fInvProc(*s.fInvMatrix, SkFloatToScalar(x + 0.5),
                     SkFloatToScalar(y + 0.5), &srcPt);
         srcPt.fX -= SK_ScalarHalf;
         srcPt.fY -= SK_ScalarHalf;

         int sx = SkScalarFloorToInt(srcPt.fX);

         SkFixed weight = 0;
         SkFixed fr = 0, fg = 0, fb = 0, fa = 0;

         int x0 = SkClampMax(sk_float_ceil2int(SkScalarToFloat(srcPt.fX)-s.getBitmapFilter()->width()), maxX);
         int x1 = SkClampMax(sk_float_floor2int(SkScalarToFloat(srcPt.fX)+s.getBitmapFilter()->width()), maxX);

         for (int src_y = y0; src_y <= y1; src_y++) {
             SkFixed yweight = s.getBitmapFilter()->lookup((srcPt.fY - src_y));

             for (int src_x = x0; src_x <= x1 ; src_x++) {
                 SkFixed xweight = s.getBitmapFilter()->lookup((srcPt.fX - src_x));

                 SkFixed combined_weight = SkFixedMul(xweight, yweight);

                 SkPMColor c = *s.fBitmap->getAddr32(src_x, src_y);
                 fr += combined_weight * SkGetPackedR32(c);
                 fg += combined_weight * SkGetPackedG32(c);
                 fb += combined_weight * SkGetPackedB32(c);
                 fa += combined_weight * SkGetPackedA32(c);
                 weight += combined_weight;
             }
         }

         fr = SkFixedDiv(fr, weight);
         fg = SkFixedDiv(fg, weight);
         fb = SkFixedDiv(fb, weight);
         fa = SkFixedDiv(fa, weight);

         int a = SkClampMax(SkFixedRoundToInt(fa), 255);
         int r = SkClampMax(SkFixedRoundToInt(fr), a);
         int g = SkClampMax(SkFixedRoundToInt(fg), a);
         int b = SkClampMax(SkFixedRoundToInt(fb), a);

         *colors++ = SkPackARGB32(a, r, g, b);

         x++;
     }
}

SK_CONF_DECLARE(const char *, c_bitmapFilter, "bitmap.filter", "mitchell", "Which bitmap filter to use [mitchell, sinc, gaussian, triangle, box]");

static SkBitmapFilter *allocateBitmapFilter() {
    if (!strcmp(c_bitmapFilter, "mitchell")) {
        return SkNEW_ARGS(SkMitchellFilter,(1.f/3.f,1.f/3.f));
    } else if (!strcmp(c_bitmapFilter, "sinc")) {
        return SkNEW_ARGS(SkSincFilter,(3));
    } else if (!strcmp(c_bitmapFilter, "gaussian")) {
        return SkNEW_ARGS(SkGaussianFilter,(2));
    } else if (!strcmp(c_bitmapFilter, "triangle")) {
        return SkNEW(SkTriangleFilter);
    } else if (!strcmp(c_bitmapFilter, "box")) {
        return SkNEW(SkBoxFilter);
    } else {
        SkASSERT(!!!"Unknown filter type");
    }

    return NULL;
}

SkBitmapProcState::ShaderProc32
SkBitmapProcState::chooseBitmapFilterProc(const SkPaint& paint) {
    // we need to be requested
    uint32_t mask = SkPaint::kFilterBitmap_Flag
                  | SkPaint::kHighQualityFilterBitmap_Flag
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

    if (fInvType & (SkMatrix::kAffine_Mask | SkMatrix::kScale_Mask)) {
        fBitmapFilter = allocateBitmapFilter();
    }

    if (fInvType & SkMatrix::kAffine_Mask) {
        return highQualityFilter;
    } else if (fInvType & SkMatrix::kScale_Mask) {
        return highQualityFilter_ScaleOnly;
    } else {
        return NULL;
    }
}

static void divideByWeights(SkFixed *sums, SkFixed *weights, SkBitmap *dst) {
    for (int y = 0 ; y < dst->height() ; y++) {
        for (int x = 0 ; x < dst->width() ; x++) {
            SkFixed fr = SkFixedDiv(sums[4*(y*dst->width() + x) + 0], weights[y*dst->width() + x]);
            SkFixed fg = SkFixedDiv(sums[4*(y*dst->width() + x) + 1], weights[y*dst->width() + x]);
            SkFixed fb = SkFixedDiv(sums[4*(y*dst->width() + x) + 2], weights[y*dst->width() + x]);
            SkFixed fa = SkFixedDiv(sums[4*(y*dst->width() + x) + 3], weights[y*dst->width() + x]);
            int a = SkClampMax(SkFixedRoundToInt(fa), 255);
            int r = SkClampMax(SkFixedRoundToInt(fr), a);
            int g = SkClampMax(SkFixedRoundToInt(fg), a);
            int b = SkClampMax(SkFixedRoundToInt(fb), a);

            *dst->getAddr32(x,y) = SkPackARGB32(a, r, g, b);
        }
    }
}

static void upScaleHoriz(const SkBitmap *src, SkBitmap *dst, float scale, SkBitmapFilter *filter) {
    for (int y = 0 ; y < src->height() ; y++) {
        for (int x = 0 ; x < dst->width() ; x++) {
            float sx = (x + 0.5f) / scale - 0.5f;
            int x0 = SkClampMax(sk_float_ceil2int(sx-filter->width()), src->width()-1);
            int x1 = SkClampMax(sk_float_floor2int(sx+filter->width()), src->width()-1);

            SkFixed total_weight = 0;
            SkFixed fr = 0, fg = 0, fb = 0, fa = 0;

            for (int src_x = x0 ; src_x <= x1 ; src_x++) {
                SkFixed weight = filter->lookup(sx - src_x);
                SkPMColor c = *src->getAddr32(src_x,y);
                fr += weight * SkGetPackedR32(c);
                fg += weight * SkGetPackedG32(c);
                fb += weight * SkGetPackedB32(c);
                fa += weight * SkGetPackedA32(c);
                total_weight += weight;
            }
            fr = SkFixedDiv(fr, total_weight);
            fg = SkFixedDiv(fg, total_weight);
            fb = SkFixedDiv(fb, total_weight);
            fa = SkFixedDiv(fa, total_weight);

            int a = SkClampMax(SkFixedRoundToInt(fa), 255);
            int r = SkClampMax(SkFixedRoundToInt(fr), a);
            int g = SkClampMax(SkFixedRoundToInt(fg), a);
            int b = SkClampMax(SkFixedRoundToInt(fb), a);

            *dst->getAddr32(x,y) = SkPackARGB32(a, r, g, b);
        }
    }
}

static void downScaleHoriz(const SkBitmap *src, SkBitmap *dst, float scale, SkBitmapFilter *filter) {
    SkFixed *sums = SkNEW_ARRAY(SkFixed, dst->width() * dst->height() * 4);
    SkFixed *weights = SkNEW_ARRAY(SkFixed, dst->width() * dst->height());

    SkAutoTDeleteArray<SkFixed> ada1(sums);
    SkAutoTDeleteArray<SkFixed> ada2(weights);

    memset(sums, 0, dst->width() * dst->height() * sizeof(SkFixed) * 4);
    memset(weights, 0, dst->width() * dst->height() * sizeof(SkFixed));

    for (int y = 0 ; y < src->height() ; y++) {
        for (int x = 0 ; x < src->width() ; x++) {
            // splat each source pixel into the destination image
            float dx = (x + 0.5f) * scale - 0.5f;
            int x0 = SkClampMax(sk_float_ceil2int(dx-filter->width()), dst->width()-1);
            int x1 = SkClampMax(sk_float_floor2int(dx+filter->width()), dst->width()-1);

            SkPMColor c = *src->getAddr32(x,y);

            for (int dst_x = x0 ; dst_x <= x1 ; dst_x++) {
                SkFixed weight = filter->lookup(dx - dst_x);
                sums[4*(y*dst->width() + dst_x) + 0] += weight*SkGetPackedR32(c);
                sums[4*(y*dst->width() + dst_x) + 1] += weight*SkGetPackedG32(c);
                sums[4*(y*dst->width() + dst_x) + 2] += weight*SkGetPackedB32(c);
                sums[4*(y*dst->width() + dst_x) + 3] += weight*SkGetPackedA32(c);
                weights[y*dst->width() + dst_x] += weight;
            }
        }
    }

    divideByWeights(sums, weights, dst);
}

static void upScaleVert(const SkBitmap *src, SkBitmap *dst, float scale, SkBitmapFilter *filter) {
    for (int y = 0 ; y < dst->height() ; y++) {
        for (int x = 0 ; x < dst->width() ; x++) {
            float sy = (y + 0.5f) / scale - 0.5f;
            int y0 = SkClampMax(sk_float_ceil2int(sy-filter->width()), src->height()-1);
            int y1 = SkClampMax(sk_float_floor2int(sy+filter->width()), src->height()-1);

            SkFixed total_weight = 0;
            SkFixed fr = 0, fg = 0, fb = 0, fa = 0;

            for (int src_y = y0 ; src_y <= y1 ; src_y++) {
                SkFixed weight = filter->lookup(sy - src_y);
                SkPMColor c = *src->getAddr32(x,src_y);
                fr += weight * SkGetPackedR32(c);
                fg += weight * SkGetPackedG32(c);
                fb += weight * SkGetPackedB32(c);
                fa += weight * SkGetPackedA32(c);
                total_weight += weight;
            }
            fr = SkFixedDiv(fr, total_weight);
            fg = SkFixedDiv(fg, total_weight);
            fb = SkFixedDiv(fb, total_weight);
            fa = SkFixedDiv(fa, total_weight);

            int a = SkClampMax(SkFixedRoundToInt(fa), 255);
            int r = SkClampMax(SkFixedRoundToInt(fr), a);
            int g = SkClampMax(SkFixedRoundToInt(fg), a);
            int b = SkClampMax(SkFixedRoundToInt(fb), a);

            *dst->getAddr32(x,y) = SkPackARGB32(a, r, g, b);
        }
    }
}

static void downScaleVert(const SkBitmap *src, SkBitmap *dst, float scale, SkBitmapFilter *filter) {
    SkFixed *sums = SkNEW_ARRAY(SkFixed, dst->width() * dst->height() * 4);
    SkFixed *weights = SkNEW_ARRAY(SkFixed, dst->width() * dst->height());

    SkAutoTDeleteArray<SkFixed> ada1(sums);
    SkAutoTDeleteArray<SkFixed> ada2(weights);

    memset(sums, 0, dst->width() * dst->height() * sizeof(SkFixed) * 4);
    memset(weights, 0, dst->width() * dst->height() * sizeof(SkFixed));

    for (int y = 0 ; y < src->height() ; y++) {
        for (int x = 0 ; x < src->width() ; x++) {
            // splat each source pixel into the destination image
            float dy = (y + 0.5f) * scale - 0.5f;
            int y0 = SkClampMax(sk_float_ceil2int(dy-filter->width()), dst->height()-1);
            int y1 = SkClampMax(sk_float_ceil2int(dy+filter->width()), dst->height()-1);

            SkPMColor c = *src->getAddr32(x,y);

            for (int dst_y = y0 ; dst_y <= y1 ; dst_y++) {
                SkFixed weight = filter->lookup(dy - dst_y);
                sums[4*(dst_y*dst->width() + x) + 0] += weight*SkGetPackedR32(c);
                sums[4*(dst_y*dst->width() + x) + 1] += weight*SkGetPackedG32(c);
                sums[4*(dst_y*dst->width() + x) + 2] += weight*SkGetPackedB32(c);
                sums[4*(dst_y*dst->width() + x) + 3] += weight*SkGetPackedA32(c);
                weights[dst_y*dst->width() + x] += weight;
            }
        }
    }

    divideByWeights(sums, weights, dst);
}

void SkBitmap::scale(SkBitmap *dst) const {

    SkBitmap horiz_temp;

    horiz_temp.setConfig(SkBitmap::kARGB_8888_Config, dst->width(), height());
    horiz_temp.allocPixels();

    SkBitmapFilter *filter = allocateBitmapFilter();

    float horiz_scale = float(dst->width()) / width();

    if (horiz_scale == 1) {
        this->copyPixelsTo(horiz_temp.getPixels(), getSize());
    } else if (horiz_scale > 1) {
        upScaleHoriz(this, &horiz_temp, horiz_scale, filter);
    } else if (horiz_scale < 1) {
        downScaleHoriz(this, &horiz_temp, horiz_scale, filter);
    }

    float vert_scale = float(dst->height()) / height();

    if (vert_scale == 1) {
        horiz_temp.copyPixelsTo(dst->getPixels(), dst->getSize());
    } else if (vert_scale > 1) {
        upScaleVert(&horiz_temp, dst, vert_scale, filter);
    } else if (vert_scale < 1) {
        downScaleVert(&horiz_temp, dst, vert_scale, filter);
    }

    SkDELETE(filter);
}
