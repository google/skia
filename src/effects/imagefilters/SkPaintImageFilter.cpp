/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/effects/SkPaintImageFilter.h"
#include "src/core/SkImageFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

sk_sp<SkImageFilter> SkPaintImageFilter::Make(const SkPaint& paint,
                                              const CropRect* cropRect) {
    return sk_sp<SkImageFilter>(new SkPaintImageFilter(paint, cropRect));
}

SkPaintImageFilter::SkPaintImageFilter(const SkPaint& paint, const CropRect* cropRect)
    : INHERITED(nullptr, 0, cropRect)
    , fPaint(paint) {
}

sk_sp<SkFlattenable> SkPaintImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    SkPaint paint;
    buffer.readPaint(&paint, nullptr);
    return SkPaintImageFilter::Make(paint, &common.cropRect());
}

void SkPaintImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePaint(fPaint);
}

sk_sp<SkSpecialImage> SkPaintImageFilter::onFilterImage(SkSpecialImage* source,
                                                        const Context& ctx,
                                                        SkIPoint* offset) const {
    SkIRect bounds;
    const SkIRect srcBounds = SkIRect::MakeWH(source->width(), source->height());
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    SkRect rect = SkRect::MakeIWH(bounds.width(), bounds.height());
    SkMatrix inverse;
    if (matrix.invert(&inverse)) {
        inverse.mapRect(&rect);
    }
    canvas->setMatrix(matrix);
    if (rect.isFinite()) {
        canvas->drawRect(rect, fPaint);
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

bool SkPaintImageFilter::affectsTransparentBlack() const {
    return true;
}
