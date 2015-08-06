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
  , fFilterQuality(kHigh_SkFilterQuality) {
}

SkBitmapSource::SkBitmapSource(const SkBitmap& bitmap, 
                               const SkRect& srcRect, const SkRect& dstRect,
                               SkFilterQuality filterQuality)
  : INHERITED(0, 0)
  , fBitmap(bitmap)
  , fSrcRect(srcRect)
  , fDstRect(dstRect)
  , fFilterQuality(filterQuality) {
}

SkFlattenable* SkBitmapSource::CreateProc(SkReadBuffer& buffer) {
    SkFilterQuality filterQuality;
    if (buffer.isVersionLT(SkReadBuffer::kBitmapSourceFilterQuality_Version)) {
        filterQuality = kHigh_SkFilterQuality;
    } else {
        filterQuality = (SkFilterQuality)buffer.readInt();
    }
    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);
    SkBitmap bitmap;
    if (!buffer.readBitmap(&bitmap)) {
        return NULL;
    }
    return SkBitmapSource::Create(bitmap, src, dst, filterQuality);
}

void SkBitmapSource::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(fFilterQuality);
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

    const SkIRect dstIRect = dstRect.roundOut();
    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(dstIRect.width(), dstIRect.height()));
    if (NULL == device.get()) {
        return false;
    }

    SkCanvas canvas(device.get());
    SkPaint paint;

    // Subtract off the integer component of the translation (will be applied in loc, below).
    dstRect.offset(-SkIntToScalar(dstIRect.fLeft), -SkIntToScalar(dstIRect.fTop));
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    // FIXME: this probably shouldn't be necessary, but drawBitmapRect asserts
    // None filtering when it's translate-only
    paint.setFilterQuality(
        fSrcRect.width() == dstRect.width() && fSrcRect.height() == dstRect.height() ?
               kNone_SkFilterQuality : fFilterQuality);
    canvas.drawBitmapRect(fBitmap, fSrcRect, dstRect, &paint, SkCanvas::kStrict_SrcRectConstraint);

    *result = device.get()->accessBitmap(false);
    offset->fX = dstIRect.fLeft;
    offset->fY = dstIRect.fTop;

    return true;
}

void SkBitmapSource::computeFastBounds(const SkRect&, SkRect* dst) const {
    *dst = fDstRect;
}

#ifndef SK_IGNORE_TO_STRING
void SkBitmapSource::toString(SkString* str) const {
    str->appendf("SkBitmapSource: (");
    str->appendf("src: (%f,%f,%f,%f) dst: (%f,%f,%f,%f) ",
                 fSrcRect.fLeft, fSrcRect.fTop, fSrcRect.fRight, fSrcRect.fBottom,
                 fDstRect.fLeft, fDstRect.fTop, fDstRect.fRight, fDstRect.fBottom);
    str->appendf("bitmap: (%d,%d)",
                 fBitmap.width(), fBitmap.height());
    str->append(")");
}
#endif
