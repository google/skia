/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageFilter.h"

#include "SkBitmap.h"
#include "SkFlattenableBuffers.h"
#include "SkRect.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "SkImageFilterUtils.h"
#endif

SK_DEFINE_INST_COUNT(SkImageFilter)

SkImageFilter::SkImageFilter(int inputCount, SkImageFilter** inputs, const SkIRect* cropRect)
  : fInputCount(inputCount),
    fInputs(new SkImageFilter*[inputCount]),
    fCropRect(cropRect ? *cropRect : SkIRect::MakeLargest()) {
    for (int i = 0; i < inputCount; ++i) {
        fInputs[i] = inputs[i];
        SkSafeRef(fInputs[i]);
    }
}

SkImageFilter::SkImageFilter(SkImageFilter* input, const SkIRect* cropRect)
  : fInputCount(1),
    fInputs(new SkImageFilter*[1]),
    fCropRect(cropRect ? *cropRect : SkIRect::MakeLargest()) {
    fInputs[0] = input;
    SkSafeRef(fInputs[0]);
}

SkImageFilter::SkImageFilter(SkImageFilter* input1, SkImageFilter* input2, const SkIRect* cropRect)
  : fInputCount(2), fInputs(new SkImageFilter*[2]),
  fCropRect(cropRect ? *cropRect : SkIRect::MakeLargest()) {
    fInputs[0] = input1;
    fInputs[1] = input2;
    SkSafeRef(fInputs[0]);
    SkSafeRef(fInputs[1]);
}

SkImageFilter::~SkImageFilter() {
    for (int i = 0; i < fInputCount; i++) {
        SkSafeUnref(fInputs[i]);
    }
    delete[] fInputs;
}

SkImageFilter::SkImageFilter(SkFlattenableReadBuffer& buffer)
    : fInputCount(buffer.readInt()), fInputs(new SkImageFilter*[fInputCount]) {
    for (int i = 0; i < fInputCount; i++) {
        if (buffer.readBool()) {
            fInputs[i] = static_cast<SkImageFilter*>(buffer.readFlattenable());
        } else {
            fInputs[i] = NULL;
        }
    }
    buffer.readIRect(&fCropRect);
}

void SkImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.writeInt(fInputCount);
    for (int i = 0; i < fInputCount; i++) {
        SkImageFilter* input = getInput(i);
        buffer.writeBool(input != NULL);
        if (input != NULL) {
            buffer.writeFlattenable(input);
        }
    }
    buffer.writeIRect(fCropRect);
}

bool SkImageFilter::filterImage(Proxy* proxy, const SkBitmap& src,
                                const SkMatrix& ctm,
                                SkBitmap* result, SkIPoint* loc) {
    SkASSERT(result);
    SkASSERT(loc);
    /*
     *  Give the proxy first shot at the filter. If it returns false, ask
     *  the filter to do it.
     */
    return (proxy && proxy->filterImage(this, src, ctm, result, loc)) ||
           this->onFilterImage(proxy, src, ctm, result, loc);
}

bool SkImageFilter::filterBounds(const SkIRect& src, const SkMatrix& ctm,
                                 SkIRect* dst) {
    SkASSERT(&src);
    SkASSERT(dst);
    return this->onFilterBounds(src, ctm, dst);
}

bool SkImageFilter::onFilterImage(Proxy*, const SkBitmap&, const SkMatrix&,
                                  SkBitmap*, SkIPoint*) {
    return false;
}

bool SkImageFilter::canFilterImageGPU() const {
    return this->asNewEffect(NULL, NULL, SkIPoint::Make(0, 0));
}

bool SkImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                   SkBitmap* result, SkIPoint* offset) {
#if SK_SUPPORT_GPU
    SkBitmap input;
    SkASSERT(fInputCount == 1);
    if (!SkImageFilterUtils::GetInputResultGPU(this->getInput(0), proxy, src, ctm, &input, offset)) {
        return false;
    }
    GrTexture* srcTexture = input.getTexture();
    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds)) {
        return false;
    }
    SkRect srcRect = SkRect::Make(bounds);
    SkRect dstRect = SkRect::MakeWH(srcRect.width(), srcRect.height());
    GrContext* context = srcTexture->getContext();

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit,
    desc.fWidth = bounds.width();
    desc.fHeight = bounds.height();
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    GrAutoScratchTexture dst(context, desc);
    GrContext::AutoMatrix am;
    am.setIdentity(context);
    GrContext::AutoRenderTarget art(context, dst.texture()->asRenderTarget());
    GrContext::AutoClip acs(context, dstRect);
    GrEffectRef* effect;
    this->asNewEffect(&effect, srcTexture, SkIPoint::Make(bounds.left(), bounds.top()));
    SkASSERT(effect);
    SkAutoUnref effectRef(effect);
    GrPaint paint;
    paint.addColorEffect(effect);
    context->drawRectToRect(paint, dstRect, srcRect);

    SkAutoTUnref<GrTexture> resultTex(dst.detach());
    SkImageFilterUtils::WrapTexture(resultTex, bounds.width(), bounds.height(), result);
    offset->fX += bounds.left();
    offset->fY += bounds.top();
    return true;
#else
    return false;
#endif
}

bool SkImageFilter::applyCropRect(SkIRect* rect) const {
    return rect->intersect(fCropRect);
}

bool SkImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                   SkIRect* dst) {
    *dst = src;
    return true;
}

bool SkImageFilter::asNewEffect(GrEffectRef**, GrTexture*, const SkIPoint& offset) const {
    return false;
}

bool SkImageFilter::asColorFilter(SkColorFilter**) const {
    return false;
}
