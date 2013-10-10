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

SkImageFilter::SkImageFilter(int inputCount, SkImageFilter** inputs, const CropRect* cropRect)
  : fInputCount(inputCount),
    fInputs(new SkImageFilter*[inputCount]),
#ifdef SK_CROP_RECT_IS_INT
    fCropRect(cropRect ? *cropRect : SkIRect::MakeLargest()) {
#else
    fCropRect(cropRect ? *cropRect : CropRect(SkRect(), 0x0)) {
#endif
    for (int i = 0; i < inputCount; ++i) {
        fInputs[i] = inputs[i];
        SkSafeRef(fInputs[i]);
    }
}

SkImageFilter::SkImageFilter(SkImageFilter* input, const CropRect* cropRect)
  : fInputCount(1),
    fInputs(new SkImageFilter*[1]),
#ifdef SK_CROP_RECT_IS_INT
    fCropRect(cropRect ? *cropRect : SkIRect::MakeLargest()) {
#else
    fCropRect(cropRect ? *cropRect : CropRect(SkRect(), 0x0)) {
#endif
    fInputs[0] = input;
    SkSafeRef(fInputs[0]);
}

SkImageFilter::SkImageFilter(SkImageFilter* input1, SkImageFilter* input2, const CropRect* cropRect)
  : fInputCount(2), fInputs(new SkImageFilter*[2]),
#ifdef SK_CROP_RECT_IS_INT
    fCropRect(cropRect ? *cropRect : SkIRect::MakeLargest()) {
#else
    fCropRect(cropRect ? *cropRect : CropRect(SkRect(), 0x0)) {
#endif
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
#ifdef SK_CROP_RECT_IS_INT
    buffer.readIRect(&fCropRect);
#else
    buffer.readRect(&fCropRect.fRect);
    fCropRect.fFlags = buffer.readUInt();
#endif
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
#ifdef SK_CROP_RECT_IS_INT
    buffer.writeIRect(fCropRect);
#else
    buffer.writeRect(fCropRect.fRect);
    buffer.writeUInt(fCropRect.fFlags);
#endif
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
    return this->asNewEffect(NULL, NULL, SkMatrix::I());
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
    if (!this->applyCropRect(&bounds, ctm)) {
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
    SkMatrix matrix(ctm);
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    this->asNewEffect(&effect, srcTexture, matrix);
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

bool SkImageFilter::applyCropRect(SkIRect* rect, const SkMatrix& matrix) const {
    SkRect cropRect;
#ifdef SK_CROP_RECT_IS_INT
    matrix.mapRect(&cropRect, SkRect::Make(fCropRect));
    SkIRect cropRectI;
    cropRect.roundOut(&cropRectI);
    // If the original crop rect edges were unset, max out the new crop edges
    if (fCropRect.fLeft == SK_MinS32) cropRectI.fLeft = SK_MinS32;
    if (fCropRect.fTop == SK_MinS32) cropRectI.fTop = SK_MinS32;
    if (fCropRect.fRight == SK_MaxS32) cropRectI.fRight = SK_MaxS32;
    if (fCropRect.fBottom == SK_MaxS32) cropRectI.fBottom = SK_MaxS32;
#else
    matrix.mapRect(&cropRect, fCropRect.fRect);
    SkIRect cropRectI;
    cropRect.roundOut(&cropRectI);
    // If the original crop rect edges were unset, max out the new crop edges
    if (!(fCropRect.fFlags & CropRect::kHasLeft_CropEdge)) cropRectI.fLeft = SK_MinS32;
    if (!(fCropRect.fFlags & CropRect::kHasTop_CropEdge)) cropRectI.fTop = SK_MinS32;
    if (!(fCropRect.fFlags & CropRect::kHasRight_CropEdge)) cropRectI.fRight = SK_MaxS32;
    if (!(fCropRect.fFlags & CropRect::kHasBottom_CropEdge)) cropRectI.fBottom = SK_MaxS32;
#endif
    return rect->intersect(cropRectI);
}

bool SkImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                   SkIRect* dst) {
    *dst = src;
    return true;
}

bool SkImageFilter::asNewEffect(GrEffectRef**, GrTexture*, const SkMatrix&) const {
    return false;
}

bool SkImageFilter::asColorFilter(SkColorFilter**) const {
    return false;
}
