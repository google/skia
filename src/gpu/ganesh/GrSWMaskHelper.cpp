/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrSWMaskHelper.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkBlitter_A8.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"

static SkPaint get_paint(GrAA aa, uint8_t alpha) {
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);  // "Replace" mode
    paint.setAntiAlias(GrAA::kYes == aa);
    // SkPaint's color is unpremul so this will produce alpha in every channel.
    paint.setColor(SkColorSetARGB(alpha, 255, 255, 255));
    return paint;
}

/**
 * Draw a single rect element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawRect(const SkRect& rect, const SkMatrix& matrix, GrAA aa, uint8_t alpha) {
    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    fDraw.fCTM = &translatedMatrix;

    fDraw.drawRect(rect, get_paint(aa, alpha));
}

void GrSWMaskHelper::drawRRect(const SkRRect& rrect, const SkMatrix& matrix,
                               GrAA aa, uint8_t alpha) {
    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    fDraw.fCTM = &translatedMatrix;

    fDraw.drawRRect(rrect, get_paint(aa, alpha));
}

/**
 * Draw a single path element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawShape(const GrStyledShape& shape, const SkMatrix& matrix,
                               GrAA aa, uint8_t alpha) {
    SkPaint paint = get_paint(aa, alpha);
    paint.setPathEffect(shape.style().refPathEffect());
    shape.style().strokeRec().applyToPaint(&paint);

    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    fDraw.fCTM = &translatedMatrix;

    SkPath path;
    shape.asPath(&path);
    if (0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint);
    } else {
        fDraw.drawPath(path, paint);
    }
}

void GrSWMaskHelper::drawShape(const GrShape& shape, const SkMatrix& matrix,
                               GrAA aa, uint8_t alpha) {
    SkPaint paint = get_paint(aa, alpha);

    SkMatrix translatedMatrix = matrix;
    translatedMatrix.postTranslate(fTranslate.fX, fTranslate.fY);
    fDraw.fCTM = &translatedMatrix;

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
    if (0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint);
    } else {
        fDraw.drawPath(path, paint);
    }
}

bool GrSWMaskHelper::init(const SkIRect& resultBounds) {
    // We will need to translate draws so the bound's UL corner is at the origin
    fTranslate = {-SkIntToScalar(resultBounds.fLeft), -SkIntToScalar(resultBounds.fTop)};
    SkIRect bounds = SkIRect::MakeWH(resultBounds.width(), resultBounds.height());

    const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(bounds.width(), bounds.height());
    if (!fPixels->tryAlloc(bmImageInfo)) {
        return false;
    }
    fPixels->erase(0);

    fDraw.fBlitterChooser = SkA8Blitter_Choose;
    fDraw.fDst      = *fPixels;
    fRasterClip.setRect(bounds);
    fDraw.fRC       = &fRasterClip;
    return true;
}

GrSurfaceProxyView GrSWMaskHelper::toTextureView(GrRecordingContext* rContext, SkBackingFit fit) {
    SkImageInfo ii = SkImageInfo::MakeA8(fPixels->width(), fPixels->height());
    size_t rowBytes = fPixels->rowBytes();

    SkBitmap bitmap;
    SkAssertResult(bitmap.installPixels(ii, fPixels->detachPixels(), rowBytes,
                                        [](void* addr, void* context) { sk_free(addr); },
                                        nullptr));
    bitmap.setImmutable();

    return std::get<0>(GrMakeUncachedBitmapProxyView(rContext, bitmap, skgpu::Mipmapped::kNo, fit));
}
