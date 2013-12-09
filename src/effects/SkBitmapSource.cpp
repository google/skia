/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapSource.h"
#include "SkDevice.h"
#include "SkCanvas.h"
#include "SkFlattenableBuffers.h"
#include "SkValidationUtils.h"

SkBitmapSource::SkBitmapSource(const SkBitmap& bitmap)
  : INHERITED(0, 0),
    fBitmap(bitmap),
    fSrcRect(SkRect::MakeWH(SkIntToScalar(bitmap.width()),
                            SkIntToScalar(bitmap.height()))),
    fDstRect(fSrcRect) {
}

SkBitmapSource::SkBitmapSource(const SkBitmap& bitmap, const SkRect& srcRect, const SkRect& dstRect)
  : INHERITED(0, 0),
    fBitmap(bitmap),
    fSrcRect(srcRect),
    fDstRect(dstRect) {
}

SkBitmapSource::SkBitmapSource(SkFlattenableReadBuffer& buffer)
  : INHERITED(0, buffer) {
    fBitmap.unflatten(buffer);
    buffer.readRect(&fSrcRect);
    buffer.readRect(&fDstRect);
    buffer.validate(SkIsValidRect(fSrcRect) && SkIsValidRect(fDstRect));
}

void SkBitmapSource::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    fBitmap.flatten(buffer);
    buffer.writeRect(fSrcRect);
    buffer.writeRect(fDstRect);
}

bool SkBitmapSource::onFilterImage(Proxy* proxy, const SkBitmap&, const SkMatrix& matrix,
                                   SkBitmap* result, SkIPoint* offset) {
    SkRect bounds, dstRect;
    fBitmap.getBounds(&bounds);
    matrix.mapRect(&dstRect, fDstRect);
    if (fSrcRect == bounds && dstRect == bounds) {
        // No regions cropped out or resized; return entire bitmap.
        *result = fBitmap;
        return true;
    }
    SkIRect dstIRect;
    dstRect.roundOut(&dstIRect);

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(dstIRect.width(), dstIRect.height()));
    if (NULL == device.get()) {
        return false;
    }

    SkCanvas canvas(device.get());
    SkPaint paint;

    // Subtract off the integer component of the translation (will be applied in loc, below).
    dstRect.offset(-SkIntToScalar(dstIRect.fLeft), -SkIntToScalar(dstIRect.fTop));
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    // FIXME: this probably shouldn't be necessary, but drawBitmapRectToRect asserts
    // None filtering when it's translate-only
    paint.setFilterLevel(
        fSrcRect.width() == dstRect.width() && fSrcRect.height() == dstRect.height() ?
        SkPaint::kNone_FilterLevel : SkPaint::kMedium_FilterLevel);
    canvas.drawBitmapRectToRect(fBitmap, &fSrcRect, dstRect, &paint);

    *result = device.get()->accessBitmap(false);
    offset->fX += dstIRect.fLeft;
    offset->fY += dstIRect.fTop;
    return true;
}
