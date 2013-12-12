/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureImageFilter.h"
#include "SkDevice.h"
#include "SkCanvas.h"
#include "SkFlattenableBuffers.h"
#include "SkValidationUtils.h"

SkPictureImageFilter::SkPictureImageFilter(SkPicture* picture)
  : INHERITED(0, 0),
    fPicture(picture),
    fRect(SkRect::MakeWH(picture ? SkIntToScalar(picture->width()) : 0,
                         picture ? SkIntToScalar(picture->height()) : 0)) {
    SkSafeRef(fPicture);
}

SkPictureImageFilter::SkPictureImageFilter(SkPicture* picture, const SkRect& rect)
  : INHERITED(0, 0),
    fPicture(picture),
    fRect(rect) {
    SkSafeRef(fPicture);
}

SkPictureImageFilter::~SkPictureImageFilter() {
    SkSafeUnref(fPicture);
}

SkPictureImageFilter::SkPictureImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(0, buffer),
    fPicture(NULL) {
    // FIXME: unflatten picture here.
    buffer.readRect(&fRect);
}

void SkPictureImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    // FIXME: flatten picture here.
    buffer.writeRect(fRect);
}

bool SkPictureImageFilter::onFilterImage(Proxy* proxy, const SkBitmap&, const SkMatrix& matrix,
                                   SkBitmap* result, SkIPoint* offset) {
    if (!fPicture) {
        return true;
    }

    SkRect floatBounds;
    SkIRect bounds;
    matrix.mapRect(&floatBounds, fRect);
    floatBounds.roundOut(&bounds);

    if (bounds.isEmpty()) {
        return true;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (NULL == device.get()) {
        return false;
    }

    SkCanvas canvas(device.get());
    SkPaint paint;

    canvas.translate(-SkIntToScalar(bounds.fLeft), -SkIntToScalar(bounds.fTop));
    canvas.concat(matrix);
    canvas.drawPicture(*fPicture);

    *result = device.get()->accessBitmap(false);
    offset->fX += bounds.fLeft;
    offset->fY += bounds.fTop;
    return true;
}
