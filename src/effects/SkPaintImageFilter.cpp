/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaintImageFilter.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkImageFilter* SkPaintImageFilter::Create(const SkPaint& paint, const CropRect* cropRect) {
    return new SkPaintImageFilter(paint, cropRect);
}

SkPaintImageFilter::SkPaintImageFilter(const SkPaint& paint, const CropRect* cropRect)
  : INHERITED(0, nullptr, cropRect)
  , fPaint(paint) {
}

SkFlattenable* SkPaintImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    SkPaint paint;
    buffer.readPaint(&paint);
    return Create(paint, &common.cropRect());
}

void SkPaintImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePaint(fPaint);
}

bool SkPaintImageFilter::onFilterImage(Proxy* proxy,
                                            const SkBitmap& source,
                                            const Context& ctx,
                                            SkBitmap* result,
                                            SkIPoint* offset) const {
    SkIRect bounds;
    if (!this->applyCropRect(ctx, source, SkIPoint::Make(0, 0), &bounds)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(),
                                                          bounds.height()));
    if (nullptr == device.get()) {
        return false;
    }
    SkCanvas canvas(device.get());

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    SkRect rect = SkRect::MakeWH(SkIntToScalar(bounds.width()), SkIntToScalar(bounds.height()));
    SkMatrix inverse;
    if (matrix.invert(&inverse)) {
        inverse.mapRect(&rect);
    }
    canvas.setMatrix(matrix);
    canvas.drawRect(rect, fPaint);

    *result = device.get()->accessBitmap(false);
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return true;
}

bool SkPaintImageFilter::canComputeFastBounds() const {
    // http:skbug.com/4627: "make computeFastBounds and onFilterBounds() CropRect-aware"
    // computeFastBounds() doesn't currently take the crop rect into account,
    // so we can't compute it. If a full crop rect is set, we should return true here.
    return false;
}

#ifndef SK_IGNORE_TO_STRING
void SkPaintImageFilter::toString(SkString* str) const {
    str->appendf("SkPaintImageFilter: (");
    str->append(")");
}
#endif
