/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBlurImageFilter.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkGpuBlurUtils.h"
#include "SkBlurImage_opts.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

// This rather arbitrary-looking value results in a maximum box blur kernel size
// of 1000 pixels on the raster path, which matches the WebKit and Firefox
// implementations. Since the GPU path does not compute a box blur, putting
// the limit on sigma ensures consistent behaviour between the GPU and
// raster paths.
#define MAX_SIGMA SkIntToScalar(532)

SkBlurImageFilter::SkBlurImageFilter(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    fSigma.fWidth = buffer.readScalar();
    fSigma.fHeight = buffer.readScalar();
    buffer.validate(SkScalarIsFinite(fSigma.fWidth) &&
                    SkScalarIsFinite(fSigma.fHeight) &&
                    (fSigma.fWidth >= 0) &&
                    (fSigma.fHeight >= 0));
}

SkBlurImageFilter::SkBlurImageFilter(SkScalar sigmaX,
                                     SkScalar sigmaY,
                                     SkImageFilter* input,
                                     const CropRect* cropRect)
    : INHERITED(1, &input, cropRect), fSigma(SkSize::Make(sigmaX, sigmaY)) {
    SkASSERT(sigmaX >= 0 && sigmaY >= 0);
}

void SkBlurImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSigma.fWidth);
    buffer.writeScalar(fSigma.fHeight);
}

enum BlurDirection {
    kX, kY
};

/**
 *
 * In order to make memory accesses cache-friendly, we reorder the passes to
 * use contiguous memory reads wherever possible.
 *
 * For example, the 6 passes of the X-and-Y blur case are rewritten as
 * follows. Instead of 3 passes in X and 3 passes in Y, we perform
 * 2 passes in X, 1 pass in X transposed to Y on write, 2 passes in X,
 * then 1 pass in X transposed to Y on write.
 *
 * +----+       +----+       +----+        +---+       +---+       +---+        +----+
 * + AB + ----> | AB | ----> | AB | -----> | A | ----> | A | ----> | A | -----> | AB |
 * +----+ blurX +----+ blurX +----+ blurXY | B | blurX | B | blurX | B | blurXY +----+
 *                                         +---+       +---+       +---+
 *
 * In this way, two of the y-blurs become x-blurs applied to transposed
 * images, and all memory reads are contiguous.
 */

template<BlurDirection srcDirection, BlurDirection dstDirection>
static void boxBlur(const SkPMColor* src, int srcStride, SkPMColor* dst, int kernelSize,
                    int leftOffset, int rightOffset, int width, int height)
{
    int rightBorder = SkMin32(rightOffset + 1, width);
    int srcStrideX = srcDirection == kX ? 1 : srcStride;
    int dstStrideX = dstDirection == kX ? 1 : height;
    int srcStrideY = srcDirection == kX ? srcStride : 1;
    int dstStrideY = dstDirection == kX ? width : 1;
    uint32_t scale = (1 << 24) / kernelSize;
    uint32_t half = 1 << 23;
    for (int y = 0; y < height; ++y) {
        int sumA = 0, sumR = 0, sumG = 0, sumB = 0;
        const SkPMColor* p = src;
        for (int i = 0; i < rightBorder; ++i) {
            sumA += SkGetPackedA32(*p);
            sumR += SkGetPackedR32(*p);
            sumG += SkGetPackedG32(*p);
            sumB += SkGetPackedB32(*p);
            p += srcStrideX;
        }

        const SkPMColor* sptr = src;
        SkColor* dptr = dst;
        for (int x = 0; x < width; ++x) {
            *dptr = SkPackARGB32((sumA * scale + half) >> 24,
                                 (sumR * scale + half) >> 24,
                                 (sumG * scale + half) >> 24,
                                 (sumB * scale + half) >> 24);
            if (x >= leftOffset) {
                SkColor l = *(sptr - leftOffset * srcStrideX);
                sumA -= SkGetPackedA32(l);
                sumR -= SkGetPackedR32(l);
                sumG -= SkGetPackedG32(l);
                sumB -= SkGetPackedB32(l);
            }
            if (x + rightOffset + 1 < width) {
                SkColor r = *(sptr + (rightOffset + 1) * srcStrideX);
                sumA += SkGetPackedA32(r);
                sumR += SkGetPackedR32(r);
                sumG += SkGetPackedG32(r);
                sumB += SkGetPackedB32(r);
            }
            sptr += srcStrideX;
            if (srcDirection == kY) {
                SK_PREFETCH(sptr + (rightOffset + 1) * srcStrideX);
            }
            dptr += dstStrideX;
        }
        src += srcStrideY;
        dst += dstStrideY;
    }
}

static void getBox3Params(SkScalar s, int *kernelSize, int* kernelSize3, int *lowOffset,
                          int *highOffset)
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
                                      const SkBitmap& source, const Context& ctx,
                                      SkBitmap* dst, SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    if (src.colorType() != kN32_SkColorType) {
        return false;
    }

    SkIRect srcBounds, dstBounds;
    if (!this->applyCropRect(ctx, proxy, src, &srcOffset, &srcBounds, &src)) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    if (!dst->allocPixels(src.info().makeWH(srcBounds.width(), srcBounds.height()))) {
        return false;
    }
    dst->getBounds(&dstBounds);

    SkVector sigma = SkVector::Make(fSigma.width(), fSigma.height());
    ctx.ctm().mapVectors(&sigma, 1);
    sigma.fX = SkMinScalar(sigma.fX, MAX_SIGMA);
    sigma.fY = SkMinScalar(sigma.fY, MAX_SIGMA);

    int kernelSizeX, kernelSizeX3, lowOffsetX, highOffsetX;
    int kernelSizeY, kernelSizeY3, lowOffsetY, highOffsetY;
    getBox3Params(sigma.x(), &kernelSizeX, &kernelSizeX3, &lowOffsetX, &highOffsetX);
    getBox3Params(sigma.y(), &kernelSizeY, &kernelSizeY3, &lowOffsetY, &highOffsetY);

    if (kernelSizeX < 0 || kernelSizeY < 0) {
        return false;
    }

    if (kernelSizeX == 0 && kernelSizeY == 0) {
        src.copyTo(dst, dst->colorType());
        offset->fX = srcBounds.fLeft;
        offset->fY = srcBounds.fTop;
        return true;
    }

    SkBitmap temp;
    if (!temp.allocPixels(dst->info())) {
        return false;
    }

    offset->fX = srcBounds.fLeft;
    offset->fY = srcBounds.fTop;
    srcBounds.offset(-srcOffset);
    const SkPMColor* s = src.getAddr32(srcBounds.left(), srcBounds.top());
    SkPMColor* t = temp.getAddr32(0, 0);
    SkPMColor* d = dst->getAddr32(0, 0);
    int w = dstBounds.width(), h = dstBounds.height();
    int sw = src.rowBytesAsPixels();
    SkBoxBlurProc boxBlurX, boxBlurY, boxBlurXY, boxBlurYX;
    if (!SkBoxBlurGetPlatformProcs(&boxBlurX, &boxBlurY, &boxBlurXY, &boxBlurYX)) {
        boxBlurX = boxBlur<kX, kX>;
        boxBlurY = boxBlur<kY, kY>;
        boxBlurXY = boxBlur<kX, kY>;
        boxBlurYX = boxBlur<kY, kX>;
    }

    if (kernelSizeX > 0 && kernelSizeY > 0) {
        boxBlurX(s,  sw, t, kernelSizeX,  lowOffsetX,  highOffsetX, w, h);
        boxBlurX(t,  w,  d, kernelSizeX,  highOffsetX, lowOffsetX,  w, h);
        boxBlurXY(d, w,  t, kernelSizeX3, highOffsetX, highOffsetX, w, h);
        boxBlurX(t,  h,  d, kernelSizeY,  lowOffsetY,  highOffsetY, h, w);
        boxBlurX(d,  h,  t, kernelSizeY,  highOffsetY, lowOffsetY,  h, w);
        boxBlurXY(t, h,  d, kernelSizeY3, highOffsetY, highOffsetY, h, w);
    } else if (kernelSizeX > 0) {
        boxBlurX(s,  sw, d, kernelSizeX,  lowOffsetX,  highOffsetX, w, h);
        boxBlurX(d,  w,  t, kernelSizeX,  highOffsetX, lowOffsetX,  w, h);
        boxBlurX(t,  w,  d, kernelSizeX3, highOffsetX, highOffsetX, w, h);
    } else if (kernelSizeY > 0) {
        boxBlurYX(s, sw, d, kernelSizeY,  lowOffsetY,  highOffsetY, h, w);
        boxBlurX(d,  h,  t, kernelSizeY,  highOffsetY, lowOffsetY,  h, w);
        boxBlurXY(t, h,  d, kernelSizeY3, highOffsetY, highOffsetY, h, w);
    }
    return true;
}


void SkBlurImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    if (getInput(0)) {
        getInput(0)->computeFastBounds(src, dst);
    } else {
        *dst = src;
    }

    dst->outset(SkScalarMul(fSigma.width(), SkIntToScalar(3)),
                SkScalarMul(fSigma.height(), SkIntToScalar(3)));
}

bool SkBlurImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                       SkIRect* dst) const {
    SkIRect bounds = src;
    if (getInput(0) && !getInput(0)->filterBounds(src, ctm, &bounds)) {
        return false;
    }
    SkVector sigma = SkVector::Make(fSigma.width(), fSigma.height());
    ctm.mapVectors(&sigma, 1);
    bounds.outset(SkScalarCeilToInt(SkScalarMul(sigma.x(), SkIntToScalar(3))),
                  SkScalarCeilToInt(SkScalarMul(sigma.y(), SkIntToScalar(3))));
    *dst = bounds;
    return true;
}

bool SkBlurImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                       SkBitmap* result, SkIPoint* offset) const {
#if SK_SUPPORT_GPU
    SkBitmap input = src;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->getInputResultGPU(proxy, src, ctx, &input, &srcOffset)) {
        return false;
    }
    SkIRect rect;
    if (!this->applyCropRect(ctx, proxy, input, &srcOffset, &rect, &input)) {
        return false;
    }
    GrTexture* source = input.getTexture();
    SkVector sigma = SkVector::Make(fSigma.width(), fSigma.height());
    ctx.ctm().mapVectors(&sigma, 1);
    sigma.fX = SkMinScalar(sigma.fX, MAX_SIGMA);
    sigma.fY = SkMinScalar(sigma.fY, MAX_SIGMA);
    offset->fX = rect.fLeft;
    offset->fY = rect.fTop;
    rect.offset(-srcOffset);
    SkAutoTUnref<GrTexture> tex(SkGpuBlurUtils::GaussianBlur(source->getContext(),
                                                             source,
                                                             false,
                                                             SkRect::Make(rect),
                                                             true,
                                                             sigma.x(),
                                                             sigma.y()));
    WrapTexture(tex, rect.width(), rect.height(), result);
    return true;
#else
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
#endif
}
