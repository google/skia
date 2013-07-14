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
        s.fInvProc(s.fInvMatrix, SkFloatToScalar(x + 0.5f),
                    SkFloatToScalar(y + 0.5f), &srcPt);
        srcPt.fX -= SK_ScalarHalf;
        srcPt.fY -= SK_ScalarHalf;

        SkScalar weight = 0;
        SkScalar fr = 0, fg = 0, fb = 0, fa = 0;

        int y0 = SkClampMax(SkScalarCeilToInt(srcPt.fY-s.getBitmapFilter()->width()), maxY);
        int y1 = SkClampMax(SkScalarFloorToInt(srcPt.fY+s.getBitmapFilter()->width()), maxY);
        int x0 = SkClampMax(SkScalarCeilToInt(srcPt.fX-s.getBitmapFilter()->width()), maxX);
        int x1 = SkClampMax(SkScalarFloorToInt(srcPt.fX+s.getBitmapFilter()->width()), maxX);

        for (int srcY = y0; srcY <= y1; srcY++) {
            SkScalar yWeight = s.getBitmapFilter()->lookupScalar((srcPt.fY - srcY));

            for (int srcX = x0; srcX <= x1 ; srcX++) {
                SkScalar xWeight = s.getBitmapFilter()->lookupScalar((srcPt.fX - srcX));

                SkScalar combined_weight = SkScalarMul(xWeight, yWeight);

                SkPMColor c = *s.fBitmap->getAddr32(srcX, srcY);
                fr += combined_weight * SkGetPackedR32(c);
                fg += combined_weight * SkGetPackedG32(c);
                fb += combined_weight * SkGetPackedB32(c);
                fa += combined_weight * SkGetPackedA32(c);
                weight += combined_weight;
            }
        }

        fr = SkScalarDiv(fr, weight);
        fg = SkScalarDiv(fg, weight);
        fb = SkScalarDiv(fb, weight);
        fa = SkScalarDiv(fa, weight);

        int a = SkClampMax(SkScalarRoundToInt(fa), 255);
        int r = SkClampMax(SkScalarRoundToInt(fr), a);
        int g = SkClampMax(SkScalarRoundToInt(fg), a);
        int b = SkClampMax(SkScalarRoundToInt(fb), a);

        *colors++ = SkPackARGB32(a, r, g, b);

        x++;
    }
}

void highQualityFilter_ScaleOnly(const SkBitmapProcState &s, int x, int y,
                             SkPMColor *SK_RESTRICT colors, int count) {
     const int maxX = s.fBitmap->width() - 1;
     const int maxY = s.fBitmap->height() - 1;

     SkPoint srcPt;

     s.fInvProc(s.fInvMatrix, SkFloatToScalar(x + 0.5f),
                 SkFloatToScalar(y + 0.5f), &srcPt);
     srcPt.fY -= SK_ScalarHalf;
     int y0 = SkClampMax(SkScalarCeilToInt(srcPt.fY-s.getBitmapFilter()->width()), maxY);
     int y1 = SkClampMax(SkScalarFloorToInt(srcPt.fY+s.getBitmapFilter()->width()), maxY);

     while (count-- > 0) {
         s.fInvProc(s.fInvMatrix, SkFloatToScalar(x + 0.5f),
                     SkFloatToScalar(y + 0.5f), &srcPt);
         srcPt.fX -= SK_ScalarHalf;
         srcPt.fY -= SK_ScalarHalf;

         SkScalar weight = 0;
         SkScalar fr = 0, fg = 0, fb = 0, fa = 0;

         int x0 = SkClampMax(SkScalarCeilToInt(srcPt.fX-s.getBitmapFilter()->width()), maxX);
         int x1 = SkClampMax(SkScalarFloorToInt(srcPt.fX+s.getBitmapFilter()->width()), maxX);

         for (int srcY = y0; srcY <= y1; srcY++) {
             SkScalar yWeight = s.getBitmapFilter()->lookupScalar((srcPt.fY - srcY));

             for (int srcX = x0; srcX <= x1 ; srcX++) {
                 SkScalar xWeight = s.getBitmapFilter()->lookupScalar((srcPt.fX - srcX));

                 SkScalar combined_weight = SkScalarMul(xWeight, yWeight);

                 SkPMColor c = *s.fBitmap->getAddr32(srcX, srcY);
                 fr += combined_weight * SkGetPackedR32(c);
                 fg += combined_weight * SkGetPackedG32(c);
                 fb += combined_weight * SkGetPackedB32(c);
                 fa += combined_weight * SkGetPackedA32(c);
                 weight += combined_weight;
             }
         }

         fr = SkScalarDiv(fr, weight);
         fg = SkScalarDiv(fg, weight);
         fb = SkScalarDiv(fb, weight);
         fa = SkScalarDiv(fa, weight);

         int a = SkClampMax(SkScalarRoundToInt(fa), 255);
         int r = SkClampMax(SkScalarRoundToInt(fr), a);
         int g = SkClampMax(SkScalarRoundToInt(fg), a);
         int b = SkClampMax(SkScalarRoundToInt(fb), a);

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
SkBitmapProcState::chooseBitmapFilterProc() {

    if (fFilterQuality != kHQ_BitmapFilter) {
        return NULL;
    }

    if (fAlphaScale != 256) {
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

static void divideByWeights(SkScalar *sums, SkScalar *weights, SkBitmap *dst) {
    for (int y = 0 ; y < dst->height() ; y++) {
        for (int x = 0 ; x < dst->width() ; x++) {
            SkScalar fr = SkScalarDiv(sums[4*(y*dst->width() + x) + 0], weights[y*dst->width() + x]);
            SkScalar fg = SkScalarDiv(sums[4*(y*dst->width() + x) + 1], weights[y*dst->width() + x]);
            SkScalar fb = SkScalarDiv(sums[4*(y*dst->width() + x) + 2], weights[y*dst->width() + x]);
            SkScalar fa = SkScalarDiv(sums[4*(y*dst->width() + x) + 3], weights[y*dst->width() + x]);
            int a = SkClampMax(SkScalarRoundToInt(fa), 255);
            int r = SkClampMax(SkScalarRoundToInt(fr), a);
            int g = SkClampMax(SkScalarRoundToInt(fg), a);
            int b = SkClampMax(SkScalarRoundToInt(fb), a);

            *dst->getAddr32(x,y) = SkPackARGB32(a, r, g, b);
        }
    }
}

static void upScaleHorizTranspose(const SkBitmap *src, SkBitmap *dst, float scale, SkBitmapFilter *filter) {
    for (int y = 0 ; y < dst->height() ; y++) {
        for (int x = 0 ; x < dst->width() ; x++) {
            float sx = (y + 0.5f) / scale - 0.5f;
            int x0 = SkClampMax(sk_float_ceil2int(sx-filter->width()), src->width()-1);
            int x1 = SkClampMax(sk_float_floor2int(sx+filter->width()), src->width()-1);

            SkScalar totalWeight = 0;
            SkScalar fr = 0, fg = 0, fb = 0, fa = 0;

            for (int srcX = x0 ; srcX <= x1 ; srcX++) {
                SkScalar weight = filter->lookupScalar(sx - srcX);
                SkPMColor c = *src->getAddr32(srcX, x);
                fr += SkScalarMul(weight,SkGetPackedR32(c));
                fg += SkScalarMul(weight,SkGetPackedG32(c));
                fb += SkScalarMul(weight,SkGetPackedB32(c));
                fa += SkScalarMul(weight,SkGetPackedA32(c));
                totalWeight += weight;
            }
            fr = SkScalarDiv(fr,totalWeight);
            fg = SkScalarDiv(fg,totalWeight);
            fb = SkScalarDiv(fb,totalWeight);
            fa = SkScalarDiv(fa,totalWeight);

            int a = SkClampMax(SkScalarRoundToInt(fa), 255);
            int r = SkClampMax(SkScalarRoundToInt(fr), a);
            int g = SkClampMax(SkScalarRoundToInt(fg), a);
            int b = SkClampMax(SkScalarRoundToInt(fb), a);

            *dst->getAddr32(x,y) = SkPackARGB32(a, r, g, b);
        }
    }
}

static void downScaleHoriz(const SkBitmap *src, SkBitmap *dst, float scale, SkBitmapFilter *filter) {
    SkScalar *sums = SkNEW_ARRAY(SkScalar, dst->width() * dst->height() * 4);
    SkScalar *weights = SkNEW_ARRAY(SkScalar, dst->width() * dst->height());

    SkAutoTDeleteArray<SkScalar> ada1(sums);
    SkAutoTDeleteArray<SkScalar> ada2(weights);

    memset(sums, 0, dst->width() * dst->height() * sizeof(SkScalar) * 4);
    memset(weights, 0, dst->width() * dst->height() * sizeof(SkScalar));

    for (int y = 0 ; y < src->height() ; y++) {
        for (int x = 0 ; x < src->width() ; x++) {
            // splat each source pixel into the destination image
            float dx = (x + 0.5f) * scale - 0.5f;
            int x0 = SkClampMax(sk_float_ceil2int(dx-filter->width()), dst->width()-1);
            int x1 = SkClampMax(sk_float_floor2int(dx+filter->width()), dst->width()-1);

            SkPMColor c = *src->getAddr32(x,y);

            for (int dst_x = x0 ; dst_x <= x1 ; dst_x++) {
                SkScalar weight = filter->lookup(dx - dst_x);
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

static void downScaleVert(const SkBitmap *src, SkBitmap *dst, float scale, SkBitmapFilter *filter) {
    SkScalar *sums = SkNEW_ARRAY(SkScalar, dst->width() * dst->height() * 4);
    SkScalar *weights = SkNEW_ARRAY(SkScalar, dst->width() * dst->height());

    SkAutoTDeleteArray<SkScalar> ada1(sums);
    SkAutoTDeleteArray<SkScalar> ada2(weights);

    memset(sums, 0, dst->width() * dst->height() * sizeof(SkScalar) * 4);
    memset(weights, 0, dst->width() * dst->height() * sizeof(SkScalar));

    for (int y = 0 ; y < src->height() ; y++) {
        for (int x = 0 ; x < src->width() ; x++) {
            // splat each source pixel into the destination image
            float dy = (y + 0.5f) * scale - 0.5f;
            int y0 = SkClampMax(sk_float_ceil2int(dy-filter->width()), dst->height()-1);
            int y1 = SkClampMax(sk_float_ceil2int(dy+filter->width()), dst->height()-1);

            SkPMColor c = *src->getAddr32(x,y);

            for (int dst_y = y0 ; dst_y <= y1 ; dst_y++) {
                SkScalar weight = filter->lookupScalar(dy - dst_y);
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

    SkBitmap horizTemp;

    horizTemp.setConfig(SkBitmap::kARGB_8888_Config, height(), dst->width());
    horizTemp.allocPixels();

    SkBitmapFilter *filter = allocateBitmapFilter();

    float horizScale = float(dst->width()) / width();

    if (horizScale >= 1) {
        upScaleHorizTranspose(this, &horizTemp, horizScale, filter);
    } else if (horizScale < 1) {
        downScaleHoriz(this, &horizTemp, horizScale, filter);
    }

    float vertScale = float(dst->height()) / height();

    if (vertScale >= 1) {
        upScaleHorizTranspose(&horizTemp, dst, vertScale, filter);
    } else if (vertScale < 1) {
        downScaleVert(&horizTemp, dst, vertScale, filter);
    }

    SkDELETE(filter);
}
