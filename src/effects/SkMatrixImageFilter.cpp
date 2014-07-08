/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrixImageFilter.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMatrix.h"
#include "SkRect.h"

SkMatrixImageFilter::SkMatrixImageFilter(const SkMatrix& transform,
                                         SkPaint::FilterLevel filterLevel,
                                         SkImageFilter* input)
  : INHERITED(1, &input),
    fTransform(transform),
    fFilterLevel(filterLevel) {
}

SkMatrixImageFilter* SkMatrixImageFilter::Create(const SkMatrix& transform,
                                                 SkPaint::FilterLevel filterLevel,
                                                 SkImageFilter* input) {
    return SkNEW_ARGS(SkMatrixImageFilter, (transform, filterLevel, input));
}

SkMatrixImageFilter::SkMatrixImageFilter(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    buffer.readMatrix(&fTransform);
    fFilterLevel = static_cast<SkPaint::FilterLevel>(buffer.readInt());
}

void SkMatrixImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeMatrix(fTransform);
    buffer.writeInt(fFilterLevel);
}

SkMatrixImageFilter::~SkMatrixImageFilter() {
}

bool SkMatrixImageFilter::onFilterImage(Proxy* proxy,
                                        const SkBitmap& source,
                                        const Context& ctx,
                                        SkBitmap* result,
                                        SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    SkRect dstRect;
    SkIRect srcBounds, dstBounds;
    src.getBounds(&srcBounds);
    srcBounds.offset(srcOffset);
    SkRect srcRect = SkRect::Make(srcBounds);
    SkMatrix matrix;
    if (!ctx.ctm().invert(&matrix)) {
        return false;
    }
    matrix.postConcat(fTransform);
    matrix.postConcat(ctx.ctm());
    matrix.mapRect(&dstRect, srcRect);
    dstRect.roundOut(&dstBounds);

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(dstBounds.width(), dstBounds.height()));
    if (NULL == device.get()) {
        return false;
    }

    SkCanvas canvas(device.get());
    canvas.translate(-SkIntToScalar(dstBounds.x()), -SkIntToScalar(dstBounds.y()));
    canvas.concat(matrix);
    SkPaint paint;

    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setFilterLevel(fFilterLevel);
    canvas.drawBitmap(src, srcRect.x(), srcRect.y(), &paint);

    *result = device.get()->accessBitmap(false);
    offset->fX = dstBounds.fLeft;
    offset->fY = dstBounds.fTop;
    return true;
}

void SkMatrixImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    SkRect bounds = src;
    if (getInput(0)) {
        getInput(0)->computeFastBounds(src, &bounds);
    }
    SkMatrix matrix;
    matrix.setTranslate(-bounds.x(), -bounds.y());
    matrix.postConcat(fTransform);
    matrix.postTranslate(bounds.x(), bounds.y());
    matrix.mapRect(dst, bounds);
}

bool SkMatrixImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                         SkIRect* dst) const {
    SkMatrix transformInverse;
    if (!fTransform.invert(&transformInverse)) {
        return false;
    }
    SkMatrix matrix;
    if (!ctm.invert(&matrix)) {
        return false;
    }
    matrix.postConcat(transformInverse);
    matrix.postConcat(ctm);
    SkRect floatBounds;
    matrix.mapRect(&floatBounds, SkRect::Make(src));
    SkIRect bounds;
    floatBounds.roundOut(&bounds);
    if (getInput(0) && !getInput(0)->filterBounds(bounds, ctm, &bounds)) {
        return false;
    }

    *dst = bounds;
    return true;
}
