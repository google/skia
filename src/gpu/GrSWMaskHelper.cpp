/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSWMaskHelper.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/geometry/GrStyledShape.h"

/*
 * Convert a boolean operation into a transfer mode code
 */
static SkBlendMode op_to_mode(SkRegion::Op op) {

    static const SkBlendMode modeMap[] = {
        SkBlendMode::kDstOut,   // kDifference_Op
        SkBlendMode::kModulate, // kIntersect_Op
        SkBlendMode::kSrcOver,  // kUnion_Op
        SkBlendMode::kXor,      // kXOR_Op
        SkBlendMode::kClear,    // kReverseDifference_Op
        SkBlendMode::kSrc,      // kReplace_Op
    };

    return modeMap[op];
}

static SkPaint get_paint(SkRegion::Op op, GrAA aa, uint8_t alpha) {
    SkPaint paint;
    paint.setBlendMode(op_to_mode(op));
    paint.setAntiAlias(GrAA::kYes == aa);
    // SkPaint's color is unpremul so this will produce alpha in every channel.
    paint.setColor(SkColorSetARGB(alpha, 255, 255, 255));
    return paint;
}

GrSurfaceProxyView GrSWMaskHelper::MakeTexture(SkIRect bounds,
                                               GrRecordingContext* context,
                                               SkBackingFit fit,
                                               DrawFunc&& draw) {
    SkTaskGroup* taskGroup = nullptr;
    if (auto dContext = context->asDirectContext()) {
        taskGroup = dContext->priv().getTaskGroup();
    }
    if (taskGroup) {
        GrSWMaskHelper* helper = new GrSWMaskHelper(bounds);
        return helper->threadedExecute(taskGroup, context, fit, std::move(draw));
    } else {
        GrSWMaskHelper helper(bounds);
        return helper.nonThreadedExecute(context, fit, draw);
    }
}

GrSurfaceProxyView GrSWMaskHelper::threadedExecute(SkTaskGroup* taskGroup,
                                                   GrRecordingContext* context,
                                                   SkBackingFit fit,
                                                   DrawFunc&& draw) {
    sk_sp<GrSWMaskHelper> spThis(this);
    taskGroup->add([spThis, draw{std::move(draw)}]() {
        if (spThis->allocate()) {
            draw(spThis.get());
            spThis->fBitmap.setImmutable();
        }
        spThis->fSemaphore.signal();
    });
    auto lazy_cb = [spThis{std::move(spThis)}](GrResourceProvider* provider,
                                               const GrProxyProvider::LazySurfaceDesc& desc) {
        spThis->fSemaphore.wait();
        const SkBitmap& mask = spThis->fBitmap;
        if (!mask.getPixels()) {
            return GrProxyProvider::LazyCallbackResult();
        }
        GrMipLevel mip{mask.getPixels(), mask.pixmap().rowBytes()};
        GrColorType ct{SkColorTypeToGrColorType(mask.colorType())};
        sk_sp<GrTexture> tex = provider->createTexture(desc.fDimensions,
                                                       desc.fFormat,
                                                       ct,
                                                       desc.fRenderable,
                                                       desc.fSampleCnt,
                                                       desc.fBudgeted,
                                                       desc.fFit,
                                                       desc.fProtected,
                                                       mip);
        GrProxyProvider::LazyCallbackResult result(std::move(tex));
        // Callback refs us, we own bitmap, don't release callback.
        result.fReleaseCallback = false;
        return result;
    };
    const GrCaps* caps = context->priv().caps();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

    GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                           GrRenderable::kNo);
    GrSwizzle swizzle = caps->getReadSwizzle(format, GrColorType::kAlpha_8);
    sk_sp<GrTextureProxy> p = proxyProvider->createLazyProxy(std::move(lazy_cb),
                                                             format,
                                                             fBitmap.dimensions(),
                                                             GrMipMapped::kNo,
                                                             GrMipmapStatus::kNotAllocated,
                                                             GrInternalSurfaceFlags::kNone,
                                                             fit,
                                                             SkBudgeted::kYes,
                                                             GrProtected::kNo,
                                                             GrProxyProvider::UseAllocator::kYes);
    return GrSurfaceProxyView(std::move(p), kTopLeft_GrSurfaceOrigin, swizzle);
}

GrSurfaceProxyView GrSWMaskHelper::nonThreadedExecute(GrRecordingContext* context,
                                                      SkBackingFit fit,
                                                      const DrawFunc& draw) {
    if (!this->allocate()) {
        return {};
    }
    draw(this);
    fBitmap.setImmutable();
    GrBitmapTextureMaker maker(context, fBitmap, fit);
    return maker.view(GrMipmapped::kNo);
}

GrSWMaskHelper::GrSWMaskHelper(const SkIRect& resultBounds) {
    // We will need to translate draws so the bound's UL corner is at the origin
    fTranslate = {-SkIntToScalar(resultBounds.fLeft), -SkIntToScalar(resultBounds.fTop)};
    SkIRect bounds = SkIRect::MakeWH(resultBounds.width(), resultBounds.height());
    fBitmap.setInfo(SkImageInfo::MakeA8(bounds.width(), bounds.height()));
}

/**
 * Draw a single rect element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawRect(const SkRect& rect, const SkMatrix& matrix, SkRegion::Op op, GrAA aa,
                              uint8_t alpha) {
    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    SkSimpleMatrixProvider matrixProvider(translatedMatrix);
    fDraw.fMatrixProvider = &matrixProvider;

    fDraw.drawRect(rect, get_paint(op, aa, alpha));
}

void GrSWMaskHelper::drawRRect(const SkRRect& rrect, const SkMatrix& matrix, SkRegion::Op op,
                               GrAA aa, uint8_t alpha) {
    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    SkSimpleMatrixProvider matrixProvider(translatedMatrix);
    fDraw.fMatrixProvider = &matrixProvider;

    fDraw.drawRRect(rrect, get_paint(op, aa, alpha));
}

/**
 * Draw a single path element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawShape(const GrStyledShape& shape, const SkMatrix& matrix, SkRegion::Op op,
                               GrAA aa, uint8_t alpha) {
    SkPaint paint = get_paint(op, aa, alpha);
    paint.setPathEffect(shape.style().refPathEffect());
    shape.style().strokeRec().applyToPaint(&paint);

    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    SkSimpleMatrixProvider matrixProvider(translatedMatrix);
    fDraw.fMatrixProvider = &matrixProvider;

    SkPath path;
    shape.asPath(&path);
    if (SkRegion::kReplace_Op == op && 0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint);
    } else {
        fDraw.drawPath(path, paint);
    }
}

void GrSWMaskHelper::drawShape(const GrShape& shape, const SkMatrix& matrix, SkRegion::Op op,
                               GrAA aa, uint8_t alpha) {
    SkPaint paint = get_paint(op, aa, alpha);

    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    SkSimpleMatrixProvider matrixProvider(translatedMatrix);
    fDraw.fMatrixProvider = &matrixProvider;

    if (shape.inverted()) {
        if (shape.isEmpty() || shape.isLine() || shape.isPoint()) {
            // These shapes are empty for simple fills, so when inverted, cover everything
            fDraw.drawPaint(paint);
            return;
        }
        // Else fall through to the draw method using asPath(), which will toggle fill type properly
    } else if (shape.isEmpty() || shape.isLine() || shape.isPoint()) {
        // Do nothing, these shapes do not cover any pixels for simple fills
        return;
    } else if (shape.isRect()) {
        fDraw.drawRect(shape.rect(), paint);
        return;
    } else if (shape.isRRect()) {
        fDraw.drawRRect(shape.rrect(), paint);
        return;
    }

    // A complex, or inverse-filled shape, so go through drawPath.
    SkPath path;
    shape.asPath(&path);
    if (SkRegion::kReplace_Op == op && 0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint);
    } else {
        fDraw.drawPath(path, paint);
    }
}

bool GrSWMaskHelper::allocate() {
    if (!fBitmap.tryAllocPixels()) {
        SkDEBUGFAIL("Unable to allocate SW mask.");
        return false;
    }

    fDraw.fDst      = fBitmap.pixmap();
    fRasterClip.setRect(fBitmap.info().bounds());
    fDraw.fRC       = &fRasterClip;
    return true;
}
