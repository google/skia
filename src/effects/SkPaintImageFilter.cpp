/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaintImageFilter.h"
#include "SkCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"

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
    buffer.readPaint(&paint);
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
    canvas->drawRect(rect, fPaint);

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

sk_sp<SkImageFilter> SkPaintImageFilter::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    SkPaint paint = xformer->apply(fPaint);
    if (paint != fPaint) {
        return SkPaintImageFilter::Make(paint, this->getCropRectIfSet());
    }
    return this->refMe();
}

bool SkPaintImageFilter::affectsTransparentBlack() const {
    return true;
}

#ifndef SK_IGNORE_TO_STRING
void SkPaintImageFilter::toString(SkString* str) const {
    str->appendf("SkPaintImageFilter: (");
    fPaint.toString(str);
    str->append(")");
}
#endif
