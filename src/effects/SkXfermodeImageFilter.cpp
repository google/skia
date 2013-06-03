/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermodeImageFilter.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkXfermode.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "effects/GrSimpleTextureEffect.h"
#include "SkGr.h"
#include "SkImageFilterUtils.h"
#endif

///////////////////////////////////////////////////////////////////////////////

SkXfermodeImageFilter::SkXfermodeImageFilter(SkXfermode* mode, SkImageFilter* background, SkImageFilter* foreground)
  : INHERITED(background, foreground), fMode(mode) {
    SkSafeRef(fMode);
}

SkXfermodeImageFilter::~SkXfermodeImageFilter() {
    SkSafeUnref(fMode);
}

SkXfermodeImageFilter::SkXfermodeImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    fMode = buffer.readFlattenableT<SkXfermode>();
}

void SkXfermodeImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fMode);
}

bool SkXfermodeImageFilter::onFilterImage(Proxy* proxy,
                                            const SkBitmap& src,
                                            const SkMatrix& ctm,
                                            SkBitmap* dst,
                                            SkIPoint* offset) {
    SkBitmap background = src, foreground = src;
    SkImageFilter* backgroundInput = getInput(0);
    SkImageFilter* foregroundInput = getInput(1);
    if (backgroundInput && !backgroundInput->filterImage(proxy, src, ctm, &background, offset)) {
        return false;
    }
    if (foregroundInput && !foregroundInput->filterImage(proxy, src, ctm, &foreground, offset)) {
        return false;
    }
    dst->setConfig(background.config(), background.width(), background.height());
    dst->allocPixels();
    SkCanvas canvas(*dst);
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas.drawBitmap(background, 0, 0, &paint);
    paint.setXfermode(fMode);
    canvas.drawBitmap(foreground, 0, 0, &paint);
    return true;
}

#if SK_SUPPORT_GPU

bool SkXfermodeImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, SkBitmap* result) {
    SkBitmap background;
    if (!SkImageFilterUtils::GetInputResultGPU(getInput(0), proxy, src, &background)) {
        return false;
    }
    GrTexture* backgroundTex = (GrTexture*) background.getTexture();
    SkBitmap foreground;
    if (!SkImageFilterUtils::GetInputResultGPU(getInput(1), proxy, src, &foreground)) {
        return false;
    }
    GrTexture* foregroundTex = (GrTexture*) foreground.getTexture();
    GrContext* context = foregroundTex->getContext();

    GrEffectRef* xferEffect = NULL;

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = src.width();
    desc.fHeight = src.height();
    desc.fConfig = kSkia8888_GrPixelConfig;

    GrAutoScratchTexture ast(context, desc);
    SkAutoTUnref<GrTexture> dst(ast.detach());

    GrContext::AutoRenderTarget art(context, dst->asRenderTarget());

    SkXfermode::Coeff sm, dm;
    if (!SkXfermode::AsNewEffectOrCoeff(fMode, context, &xferEffect, &sm, &dm, backgroundTex)) {
        return false;
    }

    GrPaint paint;
    SkRect srcRect;
    src.getBounds(&srcRect);
    if (NULL != xferEffect) {
        paint.colorStage(0)->setEffect(
            GrSimpleTextureEffect::Create(foregroundTex, GrEffect::MakeDivByTextureWHMatrix(foregroundTex)))->unref();
        paint.colorStage(1)->setEffect(xferEffect)->unref();
        context->drawRect(paint, srcRect);
    } else {
        paint.colorStage(0)->setEffect(
            GrSimpleTextureEffect::Create(backgroundTex, GrEffect::MakeDivByTextureWHMatrix(backgroundTex)))->unref();
        context->drawRect(paint, srcRect);
        paint.setBlendFunc(sk_blend_to_grblend(sm), sk_blend_to_grblend(dm));
        paint.colorStage(0)->setEffect(
            GrSimpleTextureEffect::Create(foregroundTex, GrEffect::MakeDivByTextureWHMatrix(foregroundTex)))->unref();
        context->drawRect(paint, srcRect);
    }
    return SkImageFilterUtils::WrapTexture(dst, src.width(), src.height(), result);
}

#endif
