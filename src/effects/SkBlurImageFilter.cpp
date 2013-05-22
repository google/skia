/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBlurImageFilter.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "SkImageFilterUtils.h"
#endif

SkBlurImageFilter::SkBlurImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    fSigma.fWidth = buffer.readScalar();
    fSigma.fHeight = buffer.readScalar();
}

SkBlurImageFilter::SkBlurImageFilter(SkScalar sigmaX, SkScalar sigmaY, SkImageFilter* input)
    : INHERITED(input), fSigma(SkSize::Make(sigmaX, sigmaY)) {
    SkASSERT(sigmaX >= 0 && sigmaY >= 0);
}

void SkBlurImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSigma.fWidth);
    buffer.writeScalar(fSigma.fHeight);
}

static void boxBlurX(const SkBitmap& src, SkBitmap* dst, int kernelSize,
                     int leftOffset, int rightOffset)
{
    int width = src.width(), height = src.height();
    int rightBorder = SkMin32(rightOffset + 1, width);
    for (int y = 0; y < height; ++y) {
        int sumA = 0, sumR = 0, sumG = 0, sumB = 0;
        SkPMColor* p = src.getAddr32(0, y);
        for (int i = 0; i < rightBorder; ++i) {
            sumA += SkGetPackedA32(*p);
            sumR += SkGetPackedR32(*p);
            sumG += SkGetPackedG32(*p);
            sumB += SkGetPackedB32(*p);
            p++;
        }

        const SkColor* sptr = src.getAddr32(0, y);
        SkColor* dptr = dst->getAddr32(0, y);
        for (int x = 0; x < width; ++x) {
            *dptr = SkPackARGB32(sumA / kernelSize,
                                 sumR / kernelSize,
                                 sumG / kernelSize,
                                 sumB / kernelSize);
            if (x >= leftOffset) {
                SkColor l = *(sptr - leftOffset);
                sumA -= SkGetPackedA32(l);
                sumR -= SkGetPackedR32(l);
                sumG -= SkGetPackedG32(l);
                sumB -= SkGetPackedB32(l);
            }
            if (x + rightOffset + 1 < width) {
                SkColor r = *(sptr + rightOffset + 1);
                sumA += SkGetPackedA32(r);
                sumR += SkGetPackedR32(r);
                sumG += SkGetPackedG32(r);
                sumB += SkGetPackedB32(r);
            }
            sptr++;
            dptr++;
        }
    }
}

static void boxBlurY(const SkBitmap& src, SkBitmap* dst, int kernelSize,
                     int topOffset, int bottomOffset)
{
    int width = src.width(), height = src.height();
    int bottomBorder = SkMin32(bottomOffset + 1, height);
    int srcStride = src.rowBytesAsPixels();
    int dstStride = dst->rowBytesAsPixels();
    for (int x = 0; x < width; ++x) {
        int sumA = 0, sumR = 0, sumG = 0, sumB = 0;
        SkColor* p = src.getAddr32(x, 0);
        for (int i = 0; i < bottomBorder; ++i) {
            sumA += SkGetPackedA32(*p);
            sumR += SkGetPackedR32(*p);
            sumG += SkGetPackedG32(*p);
            sumB += SkGetPackedB32(*p);
            p += srcStride;
        }

        const SkColor* sptr = src.getAddr32(x, 0);
        SkColor* dptr = dst->getAddr32(x, 0);
        for (int y = 0; y < height; ++y) {
            *dptr = SkPackARGB32(sumA / kernelSize,
                                 sumR / kernelSize,
                                 sumG / kernelSize,
                                 sumB / kernelSize);
            if (y >= topOffset) {
                SkColor l = *(sptr - topOffset * srcStride);
                sumA -= SkGetPackedA32(l);
                sumR -= SkGetPackedR32(l);
                sumG -= SkGetPackedG32(l);
                sumB -= SkGetPackedB32(l);
            }
            if (y + bottomOffset + 1 < height) {
                SkColor r = *(sptr + (bottomOffset + 1) * srcStride);
                sumA += SkGetPackedA32(r);
                sumR += SkGetPackedR32(r);
                sumG += SkGetPackedG32(r);
                sumB += SkGetPackedB32(r);
            }
            sptr += srcStride;
            dptr += dstStride;
        }
    }
}

static void getBox3Params(SkScalar s, int *kernelSize, int* kernelSize3, int *lowOffset, int *highOffset)
{
    float pi = SkScalarToFloat(SK_ScalarPI);
    int d = static_cast<int>(floorf(SkScalarToFloat(s) * 3.0f * sqrtf(2.0f * pi) / 4.0f + 0.5f));
    *kernelSize = d;
    if (d % 2 == 1) {
        *lowOffset = *highOffset = (d - 1) / 2;
        *kernelSize3 = d;
    } else {
        *highOffset = d / 2;
        *lowOffset = *highOffset - 1;
        *kernelSize3 = d + 1;
    }
}

bool SkBlurImageFilter::onFilterImage(Proxy* proxy,
                                      const SkBitmap& source, const SkMatrix& ctm,
                                      SkBitmap* dst, SkIPoint* offset) {
    SkBitmap src = this->getInputResult(0, proxy, source, ctm, offset);
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    dst->setConfig(src.config(), src.width(), src.height());
    dst->allocPixels();
    int kernelSizeX, kernelSizeX3, lowOffsetX, highOffsetX;
    int kernelSizeY, kernelSizeY3, lowOffsetY, highOffsetY;
    getBox3Params(fSigma.width(), &kernelSizeX, &kernelSizeX3, &lowOffsetX, &highOffsetX);
    getBox3Params(fSigma.height(), &kernelSizeY, &kernelSizeY3, &lowOffsetY, &highOffsetY);

    if (kernelSizeX < 0 || kernelSizeY < 0) {
        return false;
    }

    if (kernelSizeX == 0 && kernelSizeY == 0) {
        src.copyTo(dst, dst->config());
        return true;
    }

    SkBitmap temp;
    temp.setConfig(dst->config(), dst->width(), dst->height());
    if (!temp.allocPixels()) {
        return false;
    }

    if (kernelSizeX > 0 && kernelSizeY > 0) {
        boxBlurX(src,  &temp, kernelSizeX,  lowOffsetX, highOffsetX);
        boxBlurY(temp, dst,   kernelSizeY,  lowOffsetY, highOffsetY);
        boxBlurX(*dst, &temp, kernelSizeX,  highOffsetX,  lowOffsetX);
        boxBlurY(temp, dst,   kernelSizeY,  highOffsetY,  lowOffsetY);
        boxBlurX(*dst, &temp, kernelSizeX3, highOffsetX, highOffsetX);
        boxBlurY(temp, dst,   kernelSizeY3, highOffsetY, highOffsetY);
    } else if (kernelSizeX > 0) {
        boxBlurX(src,  dst,   kernelSizeX,  lowOffsetX, highOffsetX);
        boxBlurX(*dst, &temp, kernelSizeX,  highOffsetX,  lowOffsetX);
        boxBlurX(temp, dst,   kernelSizeX3, highOffsetX, highOffsetX);
    } else if (kernelSizeY > 0) {
        boxBlurY(src,  dst,   kernelSizeY,  lowOffsetY, highOffsetY);
        boxBlurY(*dst, &temp, kernelSizeY,  highOffsetY, lowOffsetY);
        boxBlurY(temp, dst,   kernelSizeY3, highOffsetY, highOffsetY);
    }
    return true;
}

bool SkBlurImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, SkBitmap* result) {
#if SK_SUPPORT_GPU
    SkBitmap input;
    if (!SkImageFilterUtils::GetInputResultGPU(getInput(0), proxy, src, &input)) {
        return false;
    }
    GrTexture* source = (GrTexture*) input.getTexture();
    SkRect rect;
    src.getBounds(&rect);
    SkAutoTUnref<GrTexture> tex(source->getContext()->gaussianBlur(source, false, rect,
        fSigma.width(), fSigma.height()));
    return SkImageFilterUtils::WrapTexture(tex, src.width(), src.height(), result);
#else
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
#endif
}
