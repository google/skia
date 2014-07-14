/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureImageFilter.h"
#include "SkDevice.h"
#include "SkCanvas.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkValidationUtils.h"

SkPictureImageFilter::SkPictureImageFilter(const SkPicture* picture)
  : INHERITED(0, 0),
    fPicture(picture),
    fCropRect(SkRect::MakeWH(picture ? SkIntToScalar(picture->width()) : 0,
                             picture ? SkIntToScalar(picture->height()) : 0)) {
    SkSafeRef(fPicture);
}

SkPictureImageFilter::SkPictureImageFilter(const SkPicture* picture, const SkRect& cropRect)
  : INHERITED(0, 0),
    fPicture(picture),
    fCropRect(cropRect) {
    SkSafeRef(fPicture);
}

SkPictureImageFilter::~SkPictureImageFilter() {
    SkSafeUnref(fPicture);
}

SkPictureImageFilter::SkPictureImageFilter(SkReadBuffer& buffer)
  : INHERITED(0, buffer),
    fPicture(NULL) {
    if (!buffer.isCrossProcess()) {
        if (buffer.readBool()) {
            fPicture = SkPicture::CreateFromBuffer(buffer);
        }
    } else {
        buffer.validate(!buffer.readBool());
    }
    buffer.readRect(&fCropRect);
}

void SkPictureImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    if (!buffer.isCrossProcess()) {
        bool hasPicture = (fPicture != NULL);
        buffer.writeBool(hasPicture);
        if (hasPicture) {
            fPicture->flatten(buffer);
        }
    } else {
        buffer.writeBool(false);
    }
    buffer.writeRect(fCropRect);
}

bool SkPictureImageFilter::onFilterImage(Proxy* proxy, const SkBitmap&, const Context& ctx,
                                         SkBitmap* result, SkIPoint* offset) const {
    if (!fPicture) {
        offset->fX = offset->fY = 0;
        return true;
    }

    SkRect floatBounds;
    SkIRect bounds;
    ctx.ctm().mapRect(&floatBounds, fCropRect);
    floatBounds.roundOut(&bounds);

    if (bounds.isEmpty()) {
        offset->fX = offset->fY = 0;
        return true;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (NULL == device.get()) {
        return false;
    }

    SkCanvas canvas(device.get());
    SkPaint paint;

    canvas.translate(-SkIntToScalar(bounds.fLeft), -SkIntToScalar(bounds.fTop));
    canvas.concat(ctx.ctm());
    canvas.drawPicture(fPicture);

    *result = device.get()->accessBitmap(false);
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return true;
}

bool SkPictureImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                          SkIRect* dst) const {
    *dst = src;
    return true;
}

