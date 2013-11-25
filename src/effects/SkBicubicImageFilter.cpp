/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBicubicImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "SkUnPreMultiply.h"

#if SK_SUPPORT_GPU
#include "effects/GrBicubicEffect.h"
#include "GrContext.h"
#include "GrTexture.h"
#include "SkImageFilterUtils.h"
#endif

#define DS(x) SkDoubleToScalar(x)

static const SkScalar gMitchellCoefficients[16] = {
    DS( 1.0 / 18.0), DS(-9.0 / 18.0), DS( 15.0 / 18.0), DS( -7.0 / 18.0),
    DS(16.0 / 18.0), DS( 0.0 / 18.0), DS(-36.0 / 18.0), DS( 21.0 / 18.0),
    DS( 1.0 / 18.0), DS( 9.0 / 18.0), DS( 27.0 / 18.0), DS(-21.0 / 18.0),
    DS( 0.0 / 18.0), DS( 0.0 / 18.0), DS( -6.0 / 18.0), DS(  7.0 / 18.0),
};

SkBicubicImageFilter::SkBicubicImageFilter(const SkSize& scale, const SkScalar coefficients[16], SkImageFilter* input)
  : INHERITED(input),
    fScale(scale) {
    memcpy(fCoefficients, coefficients, sizeof(fCoefficients));
}

SkBicubicImageFilter* SkBicubicImageFilter::CreateMitchell(const SkSize& scale,
                                                           SkImageFilter* input) {
    return SkNEW_ARGS(SkBicubicImageFilter, (scale, gMitchellCoefficients, input));
}

SkBicubicImageFilter::SkBicubicImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(1, buffer) {
    SkDEBUGCODE(bool success =) buffer.readScalarArray(fCoefficients, 16);
    SkASSERT(success);
    fScale.fWidth = buffer.readScalar();
    fScale.fHeight = buffer.readScalar();
    buffer.validate(SkScalarIsFinite(fScale.fWidth) &&
                    SkScalarIsFinite(fScale.fHeight) &&
                    (fScale.fWidth >= 0) &&
                    (fScale.fHeight >= 0));
}

void SkBicubicImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalarArray(fCoefficients, 16);
    buffer.writeScalar(fScale.fWidth);
    buffer.writeScalar(fScale.fHeight);
}

SkBicubicImageFilter::~SkBicubicImageFilter() {
}

inline SkPMColor cubicBlend(const SkScalar c[16], SkScalar t, SkPMColor c0, SkPMColor c1, SkPMColor c2, SkPMColor c3) {
    SkScalar t2 = t * t, t3 = t2 * t;
    SkScalar cc[4];
    // FIXME:  For the fractx case, this should be refactored out of this function.
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

bool SkBicubicImageFilter::onFilterImage(Proxy* proxy,
                                         const SkBitmap& source,
                                         const SkMatrix& matrix,
                                         SkBitmap* result,
                                         SkIPoint* loc) {
    SkBitmap src = source;
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, matrix, &src, loc)) {
        return false;
    }

    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    SkRect dstRect = SkRect::MakeWH(SkScalarMul(SkIntToScalar(src.width()), fScale.fWidth),
                                    SkScalarMul(SkIntToScalar(src.height()), fScale.fHeight));
    SkIRect dstIRect;
    dstRect.roundOut(&dstIRect);
    if (dstIRect.isEmpty()) {
        return false;
    }
    result->setConfig(src.config(), dstIRect.width(), dstIRect.height());
    result->allocPixels();
    if (!result->getPixels()) {
        return false;
    }

    SkRect srcRect;
    src.getBounds(&srcRect);
    SkMatrix inverse;
    inverse.setRectToRect(dstRect, srcRect, SkMatrix::kFill_ScaleToFit);
    inverse.postTranslate(-0.5f, -0.5f);

    for (int y = dstIRect.fTop; y < dstIRect.fBottom; ++y) {
        SkPMColor* dptr = result->getAddr32(dstIRect.fLeft, y);
        for (int x = dstIRect.fLeft; x < dstIRect.fRight; ++x) {
            SkPoint srcPt, dstPt = SkPoint::Make(SkIntToScalar(x), SkIntToScalar(y));
            inverse.mapPoints(&srcPt, &dstPt, 1);
            SkScalar fractx = srcPt.fX - SkScalarFloorToScalar(srcPt.fX);
            SkScalar fracty = srcPt.fY - SkScalarFloorToScalar(srcPt.fY);
            int sx = SkScalarFloorToInt(srcPt.fX);
            int sy = SkScalarFloorToInt(srcPt.fY);
            int x0 = SkClampMax(sx - 1, src.width() - 1);
            int x1 = SkClampMax(sx    , src.width() - 1);
            int x2 = SkClampMax(sx + 1, src.width() - 1);
            int x3 = SkClampMax(sx + 2, src.width() - 1);
            int y0 = SkClampMax(sy - 1, src.height() - 1);
            int y1 = SkClampMax(sy    , src.height() - 1);
            int y2 = SkClampMax(sy + 1, src.height() - 1);
            int y3 = SkClampMax(sy + 2, src.height() - 1);
            SkPMColor s00 = *src.getAddr32(x0, y0);
            SkPMColor s10 = *src.getAddr32(x1, y0);
            SkPMColor s20 = *src.getAddr32(x2, y0);
            SkPMColor s30 = *src.getAddr32(x3, y0);
            SkPMColor s0 = cubicBlend(fCoefficients, fractx, s00, s10, s20, s30);
            SkPMColor s01 = *src.getAddr32(x0, y1);
            SkPMColor s11 = *src.getAddr32(x1, y1);
            SkPMColor s21 = *src.getAddr32(x2, y1);
            SkPMColor s31 = *src.getAddr32(x3, y1);
            SkPMColor s1 = cubicBlend(fCoefficients, fractx, s01, s11, s21, s31);
            SkPMColor s02 = *src.getAddr32(x0, y2);
            SkPMColor s12 = *src.getAddr32(x1, y2);
            SkPMColor s22 = *src.getAddr32(x2, y2);
            SkPMColor s32 = *src.getAddr32(x3, y2);
            SkPMColor s2 = cubicBlend(fCoefficients, fractx, s02, s12, s22, s32);
            SkPMColor s03 = *src.getAddr32(x0, y3);
            SkPMColor s13 = *src.getAddr32(x1, y3);
            SkPMColor s23 = *src.getAddr32(x2, y3);
            SkPMColor s33 = *src.getAddr32(x3, y3);
            SkPMColor s3 = cubicBlend(fCoefficients, fractx, s03, s13, s23, s33);
            *dptr++ = cubicBlend(fCoefficients, fracty, s0, s1, s2, s3);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

bool SkBicubicImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                          SkBitmap* result, SkIPoint* offset) {
    SkBitmap srcBM;
    if (!SkImageFilterUtils::GetInputResultGPU(getInput(0), proxy, src, ctm, &srcBM, offset)) {
        return false;
    }
    GrTexture* srcTexture = srcBM.getTexture();
    GrContext* context = srcTexture->getContext();

    SkRect dstRect = SkRect::MakeWH(srcBM.width() * fScale.fWidth,
                                    srcBM.height() * fScale.fHeight);

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = SkScalarCeilToInt(dstRect.width());
    desc.fHeight = SkScalarCeilToInt(dstRect.height());
    desc.fConfig = kSkia8888_GrPixelConfig;

    GrAutoScratchTexture ast(context, desc);
    SkAutoTUnref<GrTexture> dst(ast.detach());
    if (!dst) {
        return false;
    }
    GrContext::AutoRenderTarget art(context, dst->asRenderTarget());
    GrPaint paint;
    paint.addColorEffect(GrBicubicEffect::Create(srcTexture, fCoefficients))->unref();
    SkRect srcRect;
    srcBM.getBounds(&srcRect);
    context->drawRectToRect(paint, dstRect, srcRect);
    return SkImageFilterUtils::WrapTexture(dst, desc.fWidth, desc.fHeight, result);
}
#endif

///////////////////////////////////////////////////////////////////////////////
