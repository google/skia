/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapSource.h"
#include "SkDevice.h"
#include "SkCanvas.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkValidationUtils.h"

SkBitmapSource::SkBitmapSource(const SkBitmap& bitmap)
  : INHERITED(0, 0)
  , fBitmap(bitmap)
  , fSrcRect(SkRect::MakeWH(SkIntToScalar(bitmap.width()),
                            SkIntToScalar(bitmap.height())))
  , fDstRect(fSrcRect)
{}

SkBitmapSource::SkBitmapSource(const SkBitmap& bitmap, const SkRect& srcRect, const SkRect& dstRect)
  : INHERITED(0, 0)
  , fBitmap(bitmap)
  , fSrcRect(srcRect)
  , fDstRect(dstRect) {}

#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
SkBitmapSource::SkBitmapSource(SkReadBuffer& buffer) : INHERITED(0, buffer) {
    if (buffer.isVersionLT(SkReadBuffer::kNoMoreBitmapFlatten_Version)) {
        fBitmap.legacyUnflatten(buffer);
    } else {
        buffer.readBitmap(&fBitmap);
    }
    buffer.readRect(&fSrcRect);
    buffer.readRect(&fDstRect);
    buffer.validate(buffer.isValid() && SkIsValidRect(fSrcRect) && SkIsValidRect(fDstRect));
}
#endif

SkFlattenable* SkBitmapSource::CreateProc(SkReadBuffer& buffer) {
    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);
    SkBitmap bitmap;
    if (!buffer.readBitmap(&bitmap)) {
        return NULL;
    }
    return SkBitmapSource::Create(bitmap, src, dst);
}

void SkBitmapSource::flatten(SkWriteBuffer& buffer) const {
    buffer.writeRect(fSrcRect);
    buffer.writeRect(fDstRect);
    buffer.writeBitmap(fBitmap);
}

bool SkBitmapSource::onFilterImage(Proxy* proxy, const SkBitmap&, const Context& ctx,
                                   SkBitmap* result, SkIPoint* offset) const {
    SkRect bounds, dstRect;
    fBitmap.getBounds(&bounds);
    ctx.ctm().mapRect(&dstRect, fDstRect);
    if (fSrcRect == bounds && dstRect == bounds) {
        // No regions cropped out or resized; return entire bitmap.
        *result = fBitmap;
        offset->fX = offset->fY = 0;
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
        SkPaint::kNone_FilterLevel : SkPaint::kHigh_FilterLevel);
    canvas.drawBitmapRectToRect(fBitmap, &fSrcRect, dstRect, &paint);

    *result = device.get()->accessBitmap(false);
    offset->fX = dstIRect.fLeft;
    offset->fY = dstIRect.fTop;
    return true;
}

void SkBitmapSource::computeFastBounds(const SkRect&, SkRect* dst) const {
    *dst = fDstRect;
}

bool SkBitmapSource::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                    SkIRect* dst) const {
    *dst = src;
    return true;
}
