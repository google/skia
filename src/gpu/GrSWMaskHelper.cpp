/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSWMaskHelper.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrShape.h"
#include "GrSurfaceContext.h"
#include "GrTextureProxy.h"

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

/**
 * Draw a single rect element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawRect(const SkRect& rect, const SkMatrix& matrix, SkRegion::Op op, GrAA aa,
                              uint8_t alpha) {
    SkPaint paint;
    paint.setBlendMode(op_to_mode(op));
    paint.setAntiAlias(GrAA::kYes == aa);
    paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));

    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    fDraw.fMatrix = &translatedMatrix;

    fDraw.drawRect(rect, paint);
}

/**
 * Draw a single path element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawShape(const GrShape& shape, const SkMatrix& matrix, SkRegion::Op op,
                               GrAA aa, uint8_t alpha) {
    SkPaint paint;
    paint.setPathEffect(shape.style().refPathEffect());
    shape.style().strokeRec().applyToPaint(&paint);
    paint.setAntiAlias(GrAA::kYes == aa);

    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    fDraw.fMatrix = &translatedMatrix;

    SkPath path;
    shape.asPath(&path);
    if (SkRegion::kReplace_Op == op && 0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint);
    } else {
        paint.setBlendMode(op_to_mode(op));
        paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));
        fDraw.drawPath(path, paint);
    }
};

bool GrSWMaskHelper::init(const SkIRect& resultBounds) {
    // We will need to translate draws so the bound's UL corner is at the origin
    fTranslate = {-SkIntToScalar(resultBounds.fLeft), -SkIntToScalar(resultBounds.fTop)};
    SkIRect bounds = SkIRect::MakeWH(resultBounds.width(), resultBounds.height());

    const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(bounds.width(), bounds.height());
    if (!fPixels->tryAlloc(bmImageInfo)) {
        return false;
    }
    fPixels->erase(0);

    sk_bzero(&fDraw, sizeof(fDraw));
    fDraw.fDst      = *fPixels;
    fRasterClip.setRect(bounds);
    fDraw.fRC       = &fRasterClip;
    return true;
}

sk_sp<GrTextureProxy> GrSWMaskHelper::toTextureProxy(GrContext* context, SkBackingFit fit) {
    GrSurfaceDesc desc;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = fPixels->width();
    desc.fHeight = fPixels->height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeDeferredSurfaceContext(
                                                                                desc,
                                                                                GrMipMapped::kNo,
                                                                                fit,
                                                                                SkBudgeted::kYes);
    if (!sContext || !sContext->asTextureProxy()) {
        return nullptr;
    }

    SkImageInfo ii = SkImageInfo::MakeA8(desc.fWidth, desc.fHeight);
    if (!sContext->writePixels(ii, fPixels->addr(), fPixels->rowBytes(), 0, 0)) {
        return nullptr;
    }

    return sContext->asTextureProxyRef();
}
