/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOffsetImageFilter.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMatrix.h"
#include "SkPaint.h"

bool SkOffsetImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source,
                                        const Context& ctx,
                                        SkBitmap* result,
                                        SkIPoint* offset) const {
    SkImageFilter* input = getInput(0);
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
#ifdef SK_DISABLE_OFFSETIMAGEFILTER_OPTIMIZATION
    if (false) {
#else
    if (!cropRectIsSet()) {
#endif
        if (input && !input->filterImage(proxy, source, ctx, &src, &srcOffset)) {
            return false;
        }

        SkVector vec;
        ctx.ctm().mapVectors(&vec, &fOffset, 1);

        offset->fX = srcOffset.fX + SkScalarRoundToInt(vec.fX);
        offset->fY = srcOffset.fY + SkScalarRoundToInt(vec.fY);
        *result = src;
    } else {
        if (input && !input->filterImage(proxy, source, ctx, &src, &srcOffset)) {
            return false;
        }

        SkIRect bounds;
        if (!this->applyCropRect(ctx, src, srcOffset, &bounds)) {
            return false;
        }

        SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
        if (NULL == device.get()) {
            return false;
        }
        SkCanvas canvas(device);
        SkPaint paint;
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas.translate(SkIntToScalar(srcOffset.fX - bounds.fLeft),
                         SkIntToScalar(srcOffset.fY - bounds.fTop));
        SkVector vec;
        ctx.ctm().mapVectors(&vec, &fOffset, 1);
        canvas.drawBitmap(src, vec.x(), vec.y(), &paint);
        *result = device->accessBitmap(false);
        offset->fX = bounds.fLeft;
        offset->fY = bounds.fTop;
    }
    return true;
}

void SkOffsetImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    if (getInput(0)) {
        getInput(0)->computeFastBounds(src, dst);
    } else {
        *dst = src;
    }
    SkRect copy = *dst;
    dst->offset(fOffset.fX, fOffset.fY);
    dst->join(copy);
}

bool SkOffsetImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                         SkIRect* dst) const {
    SkVector vec;
    ctm.mapVectors(&vec, &fOffset, 1);

    SkIRect bounds = src;
    bounds.offset(-SkScalarCeilToInt(vec.fX), -SkScalarCeilToInt(vec.fY));
    bounds.join(src);
    *dst = bounds;
    return true;
}

void SkOffsetImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fOffset);
}

SkOffsetImageFilter::SkOffsetImageFilter(SkScalar dx, SkScalar dy, SkImageFilter* input,
                                         const CropRect* cropRect) : INHERITED(input, cropRect) {
    fOffset.set(dx, dy);
}

SkOffsetImageFilter::SkOffsetImageFilter(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    buffer.readPoint(&fOffset);
    buffer.validate(SkScalarIsFinite(fOffset.fX) &&
                    SkScalarIsFinite(fOffset.fY));
}
