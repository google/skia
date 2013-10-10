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
#include "SkFlattenableBuffers.h"
#include "SkMatrix.h"
#include "SkPaint.h"

bool SkOffsetImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source,
                                        const SkMatrix& matrix,
                                        SkBitmap* result,
                                        SkIPoint* loc) {
    SkImageFilter* input = getInput(0);
    SkBitmap src = source;
    if (!cropRectIsSet()) {
        if (input && !input->filterImage(proxy, source, matrix, &src, loc)) {
            return false;
        }

        SkVector vec;
        matrix.mapVectors(&vec, &fOffset, 1);

        loc->fX += SkScalarRoundToInt(vec.fX);
        loc->fY += SkScalarRoundToInt(vec.fY);
        *result = src;
    } else {
        SkIPoint srcOffset = SkIPoint::Make(0, 0);
        if (input && !input->filterImage(proxy, source, matrix, &src, &srcOffset)) {
            return false;
        }

        SkIRect bounds;
        src.getBounds(&bounds);

        if (!applyCropRect(&bounds, matrix)) {
            return false;
        }

        SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
        SkCanvas canvas(device);
        SkPaint paint;
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas.drawBitmap(src, fOffset.fX - bounds.left(), fOffset.fY - bounds.top(), &paint);
        *result = device->accessBitmap(false);
        loc->fX += bounds.left();
        loc->fY += bounds.top();
    }
    return true;
}

bool SkOffsetImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                         SkIRect* dst) {
    SkVector vec;
    ctm.mapVectors(&vec, &fOffset, 1);

    *dst = src;
    dst->offset(SkScalarRoundToInt(vec.fX), SkScalarRoundToInt(vec.fY));
    return true;
}

void SkOffsetImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fOffset);
}

SkOffsetImageFilter::SkOffsetImageFilter(SkScalar dx, SkScalar dy, SkImageFilter* input,
                                         const CropRect* cropRect) : INHERITED(input, cropRect) {
    fOffset.set(dx, dy);
}

SkOffsetImageFilter::SkOffsetImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    buffer.readPoint(&fOffset);
}
