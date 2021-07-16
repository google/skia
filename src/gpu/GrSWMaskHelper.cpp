/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSWMaskHelper.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkMatrixProvider.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/SurfaceContext.h"
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

bool GrSWMaskHelper::init(const SkIRect& resultBounds) {
    // We will need to translate draws so the bound's UL corner is at the origin
    fTranslate = {-SkIntToScalar(resultBounds.fLeft), -SkIntToScalar(resultBounds.fTop)};
    SkIRect bounds = SkIRect::MakeWH(resultBounds.width(), resultBounds.height());

    const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(bounds.width(), bounds.height());
    if (!fPixels->tryAlloc(bmImageInfo)) {
        return false;
    }
    fPixels->erase(0);

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

    return std::get<0>(GrMakeUncachedBitmapProxyView(rContext, bitmap, GrMipmapped::kNo, fit));
}
