/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermodeImageFilter.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkXfermode.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContext.h"
#include "effects/GrTextureDomain.h"
#include "effects/GrSimpleTextureEffect.h"
#include "SkGr.h"
#endif

///////////////////////////////////////////////////////////////////////////////

SkXfermodeImageFilter::SkXfermodeImageFilter(SkXfermode* mode,
                                             SkImageFilter* inputs[2],
                                             const CropRect* cropRect)
  : INHERITED(2, inputs, cropRect), fMode(mode) {
    SkSafeRef(fMode);
}

SkXfermodeImageFilter::~SkXfermodeImageFilter() {
    SkSafeUnref(fMode);
}

SkFlattenable* SkXfermodeImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    SkAutoTUnref<SkXfermode> mode(buffer.readXfermode());
    return Create(mode, common.getInput(0), common.getInput(1), &common.cropRect());
}

void SkXfermodeImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fMode);
}

bool SkXfermodeImageFilter::onFilterImage(Proxy* proxy,
                                            const SkBitmap& src,
                                            const Context& ctx,
                                            SkBitmap* dst,
                                            SkIPoint* offset) const {
    SkBitmap background = src, foreground = src;
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    if (!this->filterInput(0, proxy, src, ctx, &background, &backgroundOffset)) {
        background.reset();
    }
    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    if (!this->filterInput(1, proxy, src, ctx, &foreground, &foregroundOffset)) {
        foreground.reset();
    }

    SkIRect bounds, foregroundBounds;
    if (!applyCropRect(ctx, foreground, foregroundOffset, &foregroundBounds)) {
        foregroundBounds.setEmpty();
        foreground.reset();
    }
    if (!applyCropRect(ctx, background, backgroundOffset, &bounds)) {
        bounds.setEmpty();
        background.reset();
    }
    bounds.join(foregroundBounds);
    if (bounds.isEmpty()) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (nullptr == device.get()) {
        return false;
    }
    SkCanvas canvas(device);
    canvas.translate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas.drawBitmap(background, SkIntToScalar(backgroundOffset.fX),
                      SkIntToScalar(backgroundOffset.fY), &paint);
    paint.setXfermode(fMode);
    canvas.drawBitmap(foreground, SkIntToScalar(foregroundOffset.fX),
                      SkIntToScalar(foregroundOffset.fY), &paint);
    canvas.clipRect(SkRect::Make(foregroundBounds), SkRegion::kDifference_Op);
    paint.setColor(SK_ColorTRANSPARENT);
    canvas.drawPaint(paint);
    *dst = device->accessBitmap(false);
    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return true;
}

#ifndef SK_IGNORE_TO_STRING
void SkXfermodeImageFilter::toString(SkString* str) const {
    str->appendf("SkXfermodeImageFilter: (");
    str->appendf("xfermode: (");
    if (fMode) {
        fMode->toString(str);
    }
    str->append(")");
    if (this->getInput(0)) {
        str->appendf("foreground: (");
        this->getInput(0)->toString(str);
        str->appendf(")");
    }
    if (this->getInput(1)) {
        str->appendf("background: (");
        this->getInput(1)->toString(str);
        str->appendf(")");
    }
    str->append(")");
}
#endif

#if SK_SUPPORT_GPU

bool SkXfermodeImageFilter::canFilterImageGPU() const {
    return fMode && fMode->asFragmentProcessor(nullptr, nullptr) && !cropRectIsSet();
}

bool SkXfermodeImageFilter::filterImageGPU(Proxy* proxy,
                                           const SkBitmap& src,
                                           const Context& ctx,
                                           SkBitmap* result,
                                           SkIPoint* offset) const {
    SkBitmap background = src;
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputGPU(0, proxy, src, ctx, &background, &backgroundOffset)) {
        return false;
    }

    GrTexture* backgroundTex = background.getTexture();
    if (nullptr == backgroundTex) {
        SkASSERT(false);
        return false;
    }

    SkBitmap foreground = src;
    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputGPU(1, proxy, src, ctx, &foreground, &foregroundOffset)) {
        return false;
    }
    GrTexture* foregroundTex = foreground.getTexture();
    GrContext* context = foregroundTex->getContext();
    SkIRect bounds = background.bounds().makeOffset(backgroundOffset.x(), backgroundOffset.y());
    bounds.join(foreground.bounds().makeOffset(foregroundOffset.x(), foregroundOffset.y()));
    if (bounds.isEmpty()) {
        return false;
    }

    const GrFragmentProcessor* xferFP = nullptr;

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = bounds.width();
    desc.fHeight = bounds.height();
    desc.fConfig = kSkia8888_GrPixelConfig;
    SkAutoTUnref<GrTexture> dst(context->textureProvider()->createApproxTexture(desc));
    if (!dst) {
        return false;
    }

    GrPaint paint;
    SkMatrix backgroundMatrix;
    backgroundMatrix.setIDiv(backgroundTex->width(), backgroundTex->height());
    backgroundMatrix.preTranslate(SkIntToScalar(-backgroundOffset.fX),
                                  SkIntToScalar(-backgroundOffset.fY));
    SkAutoTUnref<const GrFragmentProcessor> bgFP(GrTextureDomainEffect::Create(
        backgroundTex, backgroundMatrix,
        GrTextureDomain::MakeTexelDomain(backgroundTex, background.bounds()),
        GrTextureDomain::kDecal_Mode,
        GrTextureParams::kNone_FilterMode)
    );
    if (!fMode || !fMode->asFragmentProcessor(&xferFP, bgFP)) {
        // canFilterImageGPU() should've taken care of this
        SkASSERT(false);
        return false;
    }

    SkMatrix foregroundMatrix;
    foregroundMatrix.setIDiv(foregroundTex->width(), foregroundTex->height());
    foregroundMatrix.preTranslate(SkIntToScalar(-foregroundOffset.fX),
                                  SkIntToScalar(-foregroundOffset.fY));


    SkAutoTUnref<const GrFragmentProcessor> foregroundFP(GrTextureDomainEffect::Create(
        foregroundTex, foregroundMatrix,
        GrTextureDomain::MakeTexelDomain(foregroundTex, foreground.bounds()),
        GrTextureDomain::kDecal_Mode,
        GrTextureParams::kNone_FilterMode)
    );

    paint.addColorFragmentProcessor(foregroundFP.get());
    if (xferFP) {
        paint.addColorFragmentProcessor(xferFP)->unref();
    }
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(dst->asRenderTarget()));
    if (!drawContext) {
        return false;
    }

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    drawContext->drawRect(GrClip::WideOpen(), paint, matrix, SkRect::Make(bounds));

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    GrWrapTextureInBitmap(dst, bounds.width(), bounds.height(), false, result);
    return true;
}

#endif

