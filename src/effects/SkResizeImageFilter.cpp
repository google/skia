/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkResizeImageFilter.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMatrix.h"
#include "SkRect.h"

SkResizeImageFilter::SkResizeImageFilter(SkScalar sx, SkScalar sy, SkPaint::FilterLevel filterLevel,
                                         SkImageFilter* input)
  : INHERITED(input),
    fSx(sx),
    fSy(sy),
    fFilterLevel(filterLevel) {
}

SkResizeImageFilter::SkResizeImageFilter(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    fSx = buffer.readScalar();
    fSy = buffer.readScalar();
    fFilterLevel = static_cast<SkPaint::FilterLevel>(buffer.readInt());
}

void SkResizeImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSx);
    buffer.writeScalar(fSy);
    buffer.writeInt(fFilterLevel);
}

SkResizeImageFilter::~SkResizeImageFilter() {
}

bool SkResizeImageFilter::onFilterImage(Proxy* proxy,
                                         const SkBitmap& source,
                                         const SkMatrix& matrix,
                                         SkBitmap* result,
                                         SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, matrix, &src, &srcOffset)) {
        return false;
    }

    SkRect dstRect;
    SkIRect srcBounds, dstBounds;
    src.getBounds(&srcBounds);
    srcBounds.offset(srcOffset);
    SkRect srcRect = SkRect::Make(srcBounds);
    SkMatrix dstMatrix;
    dstMatrix.setScale(fSx, fSy);
    dstMatrix.mapRect(&dstRect, srcRect);
    dstRect.roundOut(&dstBounds);

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(dstBounds.width(), dstBounds.height()));
    if (NULL == device.get()) {
        return false;
    }

    SkCanvas canvas(device.get());
    canvas.translate(-SkIntToScalar(dstBounds.fLeft), -SkIntToScalar(dstBounds.fTop));
    SkPaint paint;

    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setFilterLevel(fFilterLevel);
    canvas.concat(dstMatrix);
    canvas.drawBitmap(src, srcRect.left(), srcRect.top(), &paint);

    *result = device.get()->accessBitmap(false);
    offset->fX = dstBounds.fLeft;
    offset->fY = dstBounds.fTop;
    return true;
}
