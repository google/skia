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
#include "effects/GrConstColorProcessor.h"
#include "effects/GrTextureDomain.h"
#include "effects/GrSimpleTextureEffect.h"
#include "SkGr.h"
#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkXfermodeImageFilter::Make(sk_sp<SkXfermode> mode,
                                                 sk_sp<SkImageFilter> background,
                                                 sk_sp<SkImageFilter> foreground,
                                                 const CropRect* cropRect) {
    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    return sk_sp<SkImageFilter>(new SkXfermodeImageFilter(mode, inputs, cropRect));
}

SkXfermodeImageFilter::SkXfermodeImageFilter(sk_sp<SkXfermode> mode,
                                             sk_sp<SkImageFilter> inputs[2],
                                             const CropRect* cropRect)
    : INHERITED(inputs, 2, cropRect)
    , fMode(std::move(mode))
{}

sk_sp<SkFlattenable> SkXfermodeImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    sk_sp<SkXfermode> mode(buffer.readXfermode());
    return Make(std::move(mode), common.getInput(0), common.getInput(1),
                &common.cropRect());
}

void SkXfermodeImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fMode.get());
}

bool SkXfermodeImageFilter::onFilterImageDeprecated(Proxy* proxy,
                                                    const SkBitmap& src,
                                                    const Context& ctx,
                                                    SkBitmap* dst,
                                                    SkIPoint* offset) const {
    SkBitmap background = src, foreground = src;
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputDeprecated(0, proxy, src, ctx, &background, &backgroundOffset)) {
        background.reset();
    }
    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputDeprecated(1, proxy, src, ctx, &foreground, &foregroundOffset)) {
        foreground.reset();
    }

    SkIRect foregroundBounds = foreground.bounds();
    foregroundBounds.offset(foregroundOffset);
    SkIRect srcBounds = background.bounds();
    srcBounds.offset(backgroundOffset);
    srcBounds.join(foregroundBounds);
    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
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
    return !this->cropRectIsSet();
}

#include "SkXfermode_proccoeff.h"

bool SkXfermodeImageFilter::filterImageGPUDeprecated(Proxy* proxy,
                                                     const SkBitmap& src,
                                                     const Context& ctx,
                                                     SkBitmap* result,
                                                     SkIPoint* offset) const {
    GrContext* context = nullptr;
    SkBitmap background = src;
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputGPUDeprecated(0, proxy, src, ctx, &background, &backgroundOffset)) {
        background.reset();
    }
    GrTexture* backgroundTex = background.getTexture();
    if (backgroundTex) {
        context = backgroundTex->getContext();
    }

    SkBitmap foreground = src;
    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputGPUDeprecated(1, proxy, src, ctx, &foreground, &foregroundOffset)) {
        foreground.reset();
    }
    GrTexture* foregroundTex = foreground.getTexture();
    if (foregroundTex) {
        context = foregroundTex->getContext();
    }

    if (!context) {
        return false;
    }

    SkIRect bounds = background.bounds().makeOffset(backgroundOffset.x(), backgroundOffset.y());
    bounds.join(foreground.bounds().makeOffset(foregroundOffset.x(), foregroundOffset.y()));
    if (bounds.isEmpty()) {
        return false;
    }

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
    // SRGBTODO: AllowSRGBInputs?
    SkAutoTUnref<const GrFragmentProcessor> bgFP;

    if (backgroundTex) {
        SkMatrix backgroundMatrix;
        backgroundMatrix.setIDiv(backgroundTex->width(), backgroundTex->height());
        backgroundMatrix.preTranslate(SkIntToScalar(-backgroundOffset.fX),
                                      SkIntToScalar(-backgroundOffset.fY));
        bgFP.reset(GrTextureDomainEffect::Create(
                            backgroundTex, backgroundMatrix,
                            GrTextureDomain::MakeTexelDomain(backgroundTex, background.bounds()),
                            GrTextureDomain::kDecal_Mode,
                            GrTextureParams::kNone_FilterMode));
    } else {
        bgFP.reset(GrConstColorProcessor::Create(GrColor_TRANSPARENT_BLACK,
                                                 GrConstColorProcessor::kIgnore_InputMode));
    }

    if (foregroundTex) {
        SkMatrix foregroundMatrix;
        foregroundMatrix.setIDiv(foregroundTex->width(), foregroundTex->height());
        foregroundMatrix.preTranslate(SkIntToScalar(-foregroundOffset.fX),
                                      SkIntToScalar(-foregroundOffset.fY));

        SkAutoTUnref<const GrFragmentProcessor> foregroundFP;

        foregroundFP.reset(GrTextureDomainEffect::Create(
                            foregroundTex, foregroundMatrix,
                            GrTextureDomain::MakeTexelDomain(foregroundTex, foreground.bounds()),
                            GrTextureDomain::kDecal_Mode,
                            GrTextureParams::kNone_FilterMode));

        paint.addColorFragmentProcessor(foregroundFP.get());

        // A null fMode is interpreted to mean kSrcOver_Mode (to match raster).
        SkAutoTUnref<SkXfermode> mode(SkSafeRef(fMode.get()));
        if (!mode) {
            // It would be awesome to use SkXfermode::Create here but it knows better
            // than us and won't return a kSrcOver_Mode SkXfermode. That means we
            // have to get one the hard way.
            struct ProcCoeff rec;
            rec.fProc = SkXfermode::GetProc(SkXfermode::kSrcOver_Mode);
            SkXfermode::ModeAsCoeff(SkXfermode::kSrcOver_Mode, &rec.fSC, &rec.fDC);

            mode.reset(new SkProcCoeffXfermode(rec, SkXfermode::kSrcOver_Mode));
        }

        SkAutoTUnref<const GrFragmentProcessor> xferFP(mode->getFragmentProcessorForImageFilter(bgFP));

        // A null 'xferFP' here means kSrc_Mode was used in which case we can just proceed
        if (xferFP) {
            paint.addColorFragmentProcessor(xferFP);
        }
    } else {
        paint.addColorFragmentProcessor(bgFP);
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
