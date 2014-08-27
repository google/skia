/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkErrorInternals.h"
#include "SkConvolver.h"
#include "SkBitmapProcState.h"
#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkConvolver.h"
#include "SkUnPreMultiply.h"
#include "SkShader.h"
#include "SkRTConf.h"
#include "SkMath.h"

// These are the per-scanline callbacks that are used when we must resort to
// resampling an image as it is blitted.  Typically these are used only when
// the image is rotated or has some other complex transformation applied.
// Scaled images will usually be rescaled directly before rasterization.

namespace {

template <typename Color, typename ColorPacker>
void highQualityFilter(ColorPacker pack, const SkBitmapProcState& s, int x, int y, Color* SK_RESTRICT colors, int count) {
    const int maxX = s.fBitmap->width();
    const int maxY = s.fBitmap->height();
    SkAutoTMalloc<SkScalar> xWeights(maxX);

    while (count-- > 0) {
        SkPoint srcPt;
        s.fInvProc(s.fInvMatrix, x + 0.5f,
                    y + 0.5f, &srcPt);
        srcPt.fX -= SK_ScalarHalf;
        srcPt.fY -= SK_ScalarHalf;

        SkScalar weight = 0;
        SkScalar fr = 0, fg = 0, fb = 0, fa = 0;

        int y0 = SkClampMax(SkScalarCeilToInt(srcPt.fY-s.getBitmapFilter()->width()), maxY);
        int y1 = SkClampMax(SkScalarFloorToInt(srcPt.fY+s.getBitmapFilter()->width()+1), maxY);
        int x0 = SkClampMax(SkScalarCeilToInt(srcPt.fX-s.getBitmapFilter()->width()), maxX);
        int x1 = SkClampMax(SkScalarFloorToInt(srcPt.fX+s.getBitmapFilter()->width())+1, maxX);

        for (int srcX = x0; srcX < x1 ; srcX++) {
            // Looking these up once instead of each loop is a ~15% speedup.
            xWeights[srcX - x0] = s.getBitmapFilter()->lookupScalar((srcPt.fX - srcX));
        }

        for (int srcY = y0; srcY < y1; srcY++) {
            SkScalar yWeight = s.getBitmapFilter()->lookupScalar((srcPt.fY - srcY));

            for (int srcX = x0; srcX < x1 ; srcX++) {
                SkScalar xWeight = xWeights[srcX - x0];

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

        *colors++ = pack(a, r, g, b);

        x++;
    }
}

uint16_t PackTo565(int /*a*/, int r, int g, int b) {
    return SkPack888ToRGB16(r, g, b);
}

}  // namespace

void highQualityFilter32(const SkBitmapProcState& s, int x, int y, SkPMColor* SK_RESTRICT colors, int count) {
    highQualityFilter(&SkPackARGB32, s, x, y, colors, count);
}

void highQualityFilter16(const SkBitmapProcState& s, int x, int y, uint16_t* SK_RESTRICT colors, int count) {
    highQualityFilter(&PackTo565, s, x, y, colors, count);
}


SK_CONF_DECLARE(const char *, c_bitmapFilter, "bitmap.filter", "mitchell", "Which scanline bitmap filter to use [mitchell, lanczos, hamming, gaussian, triangle, box]");

SkBitmapFilter *SkBitmapFilter::Allocate() {
    if (!strcmp(c_bitmapFilter, "mitchell")) {
        return SkNEW_ARGS(SkMitchellFilter,(1.f/3.f,1.f/3.f));
    } else if (!strcmp(c_bitmapFilter, "lanczos")) {
        return SkNEW(SkLanczosFilter);
    } else if (!strcmp(c_bitmapFilter, "hamming")) {
        return SkNEW(SkHammingFilter);
    } else if (!strcmp(c_bitmapFilter, "gaussian")) {
        return SkNEW_ARGS(SkGaussianFilter,(2));
    } else if (!strcmp(c_bitmapFilter, "triangle")) {
        return SkNEW(SkTriangleFilter);
    } else if (!strcmp(c_bitmapFilter, "box")) {
        return SkNEW(SkBoxFilter);
    } else {
        SkDEBUGFAIL("Unknown filter type");
    }

    return NULL;
}

bool SkBitmapProcState::setBitmapFilterProcs() {
    if (fFilterLevel != SkPaint::kHigh_FilterLevel) {
        return false;
    }

    if (fAlphaScale != 256) {
        return false;
    }

    // TODO: consider supporting other colortypes (e.g. 565, A8)
    if (fBitmap->colorType() != kN32_SkColorType) {
        return false;
    }

    // TODO: consider supporting repeat and mirror
    if (SkShader::kClamp_TileMode != fTileModeX || SkShader::kClamp_TileMode != fTileModeY) {
        return false;
    }

    // TODO: is this right?  do we want fBitmapFilter allocated even if we can't set shader procs?
    if (fInvType & (SkMatrix::kAffine_Mask | SkMatrix::kScale_Mask)) {
        fBitmapFilter = SkBitmapFilter::Allocate();
    }

    if (fInvType & SkMatrix::kScale_Mask) {
        fShaderProc32 = highQualityFilter32;
        fShaderProc16 = highQualityFilter16;
        return true;
    } else {
        return false;
    }
}
