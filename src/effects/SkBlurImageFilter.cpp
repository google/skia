/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBlurImageFilter.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkGpuBlurUtils.h"
#include "SkOpts.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

// This rather arbitrary-looking value results in a maximum box blur kernel size
// of 1000 pixels on the raster path, which matches the WebKit and Firefox
// implementations. Since the GPU path does not compute a box blur, putting
// the limit on sigma ensures consistent behaviour between the GPU and
// raster paths.
#define MAX_SIGMA SkIntToScalar(532)

static SkVector mapSigma(const SkSize& localSigma, const SkMatrix& ctm) {
    SkVector sigma = SkVector::Make(localSigma.width(), localSigma.height());
    ctm.mapVectors(&sigma, 1);
    sigma.fX = SkMinScalar(SkScalarAbs(sigma.fX), MAX_SIGMA);
    sigma.fY = SkMinScalar(SkScalarAbs(sigma.fY), MAX_SIGMA);
    return sigma;
}

SkBlurImageFilter::SkBlurImageFilter(SkScalar sigmaX,
                                     SkScalar sigmaY,
                                     SkImageFilter* input,
                                     const CropRect* cropRect)
    : INHERITED(1, &input, cropRect), fSigma(SkSize::Make(sigmaX, sigmaY)) {
}

SkFlattenable* SkBlurImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    return Create(sigmaX, sigmaY, common.getInput(0), &common.cropRect());
}

void SkBlurImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSigma.fWidth);
    buffer.writeScalar(fSigma.fHeight);
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
    if (!this->filterInput(0, proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    if (src.colorType() != kN32_SkColorType) {
        return false;
    }

    SkIRect srcBounds, dstBounds;
    if (!this->applyCropRect(ctx, src, srcOffset, &dstBounds, &srcBounds)) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(dstBounds.width(), dstBounds.height()));
    if (!device) {
        return false;
    }
    *dst = device->accessBitmap(false);
    SkAutoLockPixels alp_dst(*dst);

    SkVector sigma = mapSigma(fSigma, ctx.ctm());

    int kernelSizeX, kernelSizeX3, lowOffsetX, highOffsetX;
    int kernelSizeY, kernelSizeY3, lowOffsetY, highOffsetY;
    getBox3Params(sigma.x(), &kernelSizeX, &kernelSizeX3, &lowOffsetX, &highOffsetX);
    getBox3Params(sigma.y(), &kernelSizeY, &kernelSizeY3, &lowOffsetY, &highOffsetY);

    if (kernelSizeX < 0 || kernelSizeY < 0) {
        return false;
    }

    if (kernelSizeX == 0 && kernelSizeY == 0) {
        src.copyTo(dst, dst->colorType());
        offset->fX = dstBounds.x() + srcOffset.x();
        offset->fY = dstBounds.y() + srcOffset.y();
        return true;
    }

    SkAutoTUnref<SkBaseDevice> tempDevice(proxy->createDevice(dst->width(), dst->height()));
    if (!tempDevice) {
        return false;
    }
    SkBitmap temp = tempDevice->accessBitmap(false);
    SkAutoLockPixels alpTemp(temp);

    offset->fX = dstBounds.fLeft;
    offset->fY = dstBounds.fTop;
    SkPMColor* t = temp.getAddr32(0, 0);
    SkPMColor* d = dst->getAddr32(0, 0);
    int w = dstBounds.width(), h = dstBounds.height();
    const SkPMColor* s = src.getAddr32(srcBounds.x() - srcOffset.x(), srcBounds.y() - srcOffset.y());
    srcBounds.offset(-dstBounds.x(), -dstBounds.y());
    dstBounds.offset(-dstBounds.x(), -dstBounds.y());
    SkIRect srcBoundsT = SkIRect::MakeLTRB(srcBounds.top(), srcBounds.left(), srcBounds.bottom(), srcBounds.right());
    SkIRect dstBoundsT = SkIRect::MakeWH(dstBounds.height(), dstBounds.width());
    int sw = src.rowBytesAsPixels();

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
    if (kernelSizeX > 0 && kernelSizeY > 0) {
        SkOpts::box_blur_xx(s, sw,  srcBounds,  t, kernelSizeX,  lowOffsetX,  highOffsetX, w, h);
        SkOpts::box_blur_xx(t,  w,  dstBounds,  d, kernelSizeX,  highOffsetX, lowOffsetX,  w, h);
        SkOpts::box_blur_xy(d,  w,  dstBounds,  t, kernelSizeX3, highOffsetX, highOffsetX, w, h);
        SkOpts::box_blur_xx(t,  h,  dstBoundsT, d, kernelSizeY,  lowOffsetY,  highOffsetY, h, w);
        SkOpts::box_blur_xx(d,  h,  dstBoundsT, t, kernelSizeY,  highOffsetY, lowOffsetY,  h, w);
        SkOpts::box_blur_xy(t,  h,  dstBoundsT, d, kernelSizeY3, highOffsetY, highOffsetY, h, w);
    } else if (kernelSizeX > 0) {
        SkOpts::box_blur_xx(s, sw,  srcBounds,  d, kernelSizeX,  lowOffsetX,  highOffsetX, w, h);
        SkOpts::box_blur_xx(d,  w,  dstBounds,  t, kernelSizeX,  highOffsetX, lowOffsetX,  w, h);
        SkOpts::box_blur_xx(t,  w,  dstBounds,  d, kernelSizeX3, highOffsetX, highOffsetX, w, h);
    } else if (kernelSizeY > 0) {
        SkOpts::box_blur_yx(s, sw,  srcBoundsT, d, kernelSizeY,  lowOffsetY,  highOffsetY, h, w);
        SkOpts::box_blur_xx(d,  h,  dstBoundsT, t, kernelSizeY,  highOffsetY, lowOffsetY,  h, w);
        SkOpts::box_blur_xy(t,  h,  dstBoundsT, d, kernelSizeY3, highOffsetY, highOffsetY, h, w);
    }
    return true;
}


void SkBlurImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    if (this->getInput(0)) {
        this->getInput(0)->computeFastBounds(src, dst);
    } else {
        *dst = src;
    }

    dst->outset(SkScalarMul(fSigma.width(), SkIntToScalar(3)),
                SkScalarMul(fSigma.height(), SkIntToScalar(3)));
}

bool SkBlurImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                       SkIRect* dst) const {
    SkIRect bounds = src;
    SkVector sigma = mapSigma(fSigma, ctm);
    bounds.outset(SkScalarCeilToInt(SkScalarMul(sigma.x(), SkIntToScalar(3))),
                  SkScalarCeilToInt(SkScalarMul(sigma.y(), SkIntToScalar(3))));
    if (this->getInput(0) && !this->getInput(0)->filterBounds(bounds, ctm, &bounds)) {
        return false;
    }
    *dst = bounds;
    return true;
}

bool SkBlurImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                       SkBitmap* result, SkIPoint* offset) const {
#if SK_SUPPORT_GPU
    SkBitmap input = src;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputGPU(0, proxy, src, ctx, &input, &srcOffset)) {
        return false;
    }
    SkIRect srcBounds, dstBounds;
    if (!this->applyCropRect(ctx, input, srcOffset, &dstBounds, &srcBounds)) {
        return false;
    }
    GrTexture* source = input.getTexture();
    SkVector sigma = mapSigma(fSigma, ctx.ctm());
    offset->fX = dstBounds.fLeft;
    offset->fY = dstBounds.fTop;
    srcBounds.offset(-srcOffset);
    dstBounds.offset(-srcOffset);
    SkRect srcBoundsF(SkRect::Make(srcBounds));
    auto constraint = GrTextureProvider::FromImageFilter(ctx.sizeConstraint());
    SkAutoTUnref<GrTexture> tex(SkGpuBlurUtils::GaussianBlur(source->getContext(),
                                                             source,
                                                             false,
                                                             SkRect::Make(dstBounds),
                                                             &srcBoundsF,
                                                             sigma.x(),
                                                             sigma.y(),
                                                             constraint));
    if (!tex) {
        return false;
    }
    WrapTexture(tex, dstBounds.width(), dstBounds.height(), result);
    return true;
#else
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
#endif
}

#ifndef SK_IGNORE_TO_STRING
void SkBlurImageFilter::toString(SkString* str) const {
    str->appendf("SkBlurImageFilter: (");
    str->appendf("sigma: (%f, %f) input (", fSigma.fWidth, fSigma.fHeight);

    if (this->getInput(0)) {
        this->getInput(0)->toString(str);
    }

    str->append("))");
}
#endif
