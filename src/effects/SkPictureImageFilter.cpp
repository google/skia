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
#include "SkSurfaceProps.h"
#include "SkWriteBuffer.h"
#include "SkValidationUtils.h"

SkPictureImageFilter::SkPictureImageFilter(sk_sp<SkPicture> picture)
    : INHERITED(nullptr, 0, nullptr)
    , fPicture(std::move(picture))
    , fCropRect(fPicture ? fPicture->cullRect() : SkRect::MakeEmpty())
    , fPictureResolution(kDeviceSpace_PictureResolution)
    , fFilterQuality(kLow_SkFilterQuality) {
}

SkPictureImageFilter::SkPictureImageFilter(sk_sp<SkPicture> picture, const SkRect& cropRect,
                                           PictureResolution pictureResolution,
                                           SkFilterQuality filterQuality)
    : INHERITED(nullptr, 0, nullptr)
    , fPicture(std::move(picture))
    , fCropRect(cropRect)
    , fPictureResolution(pictureResolution)
    , fFilterQuality(filterQuality) {
}

sk_sp<SkFlattenable> SkPictureImageFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPicture> picture;
    SkRect cropRect;

    if (buffer.isCrossProcess() && SkPicture::PictureIOSecurityPrecautionsEnabled()) {
        buffer.validate(!buffer.readBool());
    } else {
        if (buffer.readBool()) {
            picture = SkPicture::MakeFromBuffer(buffer);
        }
    }
    buffer.readRect(&cropRect);
    PictureResolution pictureResolution;
    if (buffer.isVersionLT(SkReadBuffer::kPictureImageFilterResolution_Version)) {
        pictureResolution = kDeviceSpace_PictureResolution;
    } else {
        pictureResolution = (PictureResolution)buffer.readInt();
    }

    if (kLocalSpace_PictureResolution == pictureResolution) {
        //filterLevel is only serialized if pictureResolution is LocalSpace
        SkFilterQuality filterQuality;
        if (buffer.isVersionLT(SkReadBuffer::kPictureImageFilterLevel_Version)) {
            filterQuality = kLow_SkFilterQuality;
        } else {
            filterQuality = (SkFilterQuality)buffer.readInt();
        }
        return MakeForLocalSpace(picture, cropRect, filterQuality);
    }
    return Make(picture, cropRect);
}

void SkPictureImageFilter::flatten(SkWriteBuffer& buffer) const {
    if (buffer.isCrossProcess() && SkPicture::PictureIOSecurityPrecautionsEnabled()) {
        buffer.writeBool(false);
    } else {
        bool hasPicture = (fPicture != nullptr);
        buffer.writeBool(hasPicture);
        if (hasPicture) {
            fPicture->flatten(buffer);
        }
    }
    buffer.writeRect(fCropRect);
    buffer.writeInt(fPictureResolution);
    if (kLocalSpace_PictureResolution == fPictureResolution) {
        buffer.writeInt(fFilterQuality);
    }
}

bool SkPictureImageFilter::onFilterImageDeprecated(Proxy* proxy, const SkBitmap&,
                                                   const Context& ctx,
                                                   SkBitmap* result, SkIPoint* offset) const {
    if (!fPicture) {
        offset->fX = offset->fY = 0;
        return true;
    }

    SkRect floatBounds;
    ctx.ctm().mapRect(&floatBounds, fCropRect);
    SkIRect bounds = floatBounds.roundOut();
    if (!bounds.intersect(ctx.clipBounds())) {
        return false;
    }

    if (bounds.isEmpty()) {
        offset->fX = offset->fY = 0;
        return true;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (nullptr == device.get()) {
        return false;
    }

    if (kDeviceSpace_PictureResolution == fPictureResolution ||
        0 == (ctx.ctm().getType() & ~SkMatrix::kTranslate_Mask)) {
        this->drawPictureAtDeviceResolution(device.get(), bounds, ctx);
    } else {
        this->drawPictureAtLocalResolution(proxy, device.get(), bounds, ctx);
    }

    *result = device.get()->accessBitmap(false);
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return true;
}

void SkPictureImageFilter::drawPictureAtDeviceResolution(SkBaseDevice* device,
                                                         const SkIRect& deviceBounds,
                                                         const Context& ctx) const {
    SkCanvas canvas(device);

    canvas.translate(-SkIntToScalar(deviceBounds.fLeft), -SkIntToScalar(deviceBounds.fTop));
    canvas.concat(ctx.ctm());
    canvas.drawPicture(fPicture);
}

void SkPictureImageFilter::drawPictureAtLocalResolution(Proxy* proxy, SkBaseDevice* device,
                                                        const SkIRect& deviceBounds,
                                                        const Context& ctx) const {
    SkMatrix inverseCtm;
    if (!ctx.ctm().invert(&inverseCtm)) {
        return;
    }

    SkRect localBounds = SkRect::Make(ctx.clipBounds());
    inverseCtm.mapRect(&localBounds);
    if (!localBounds.intersect(fCropRect)) {
        return;
    }
    SkIRect localIBounds = localBounds.roundOut();
    SkAutoTUnref<SkBaseDevice> localDevice(proxy->createDevice(localIBounds.width(), localIBounds.height()));

    SkCanvas localCanvas(localDevice);
    localCanvas.translate(-SkIntToScalar(localIBounds.fLeft), -SkIntToScalar(localIBounds.fTop));
    localCanvas.drawPicture(fPicture);

    SkCanvas canvas(device);

    canvas.translate(-SkIntToScalar(deviceBounds.fLeft), -SkIntToScalar(deviceBounds.fTop));
    canvas.concat(ctx.ctm());
    SkPaint paint;
    paint.setFilterQuality(fFilterQuality);
    canvas.drawBitmap(localDevice.get()->accessBitmap(false), SkIntToScalar(localIBounds.fLeft),
                      SkIntToScalar(localIBounds.fTop), &paint);
}

#ifndef SK_IGNORE_TO_STRING
void SkPictureImageFilter::toString(SkString* str) const {
    str->appendf("SkPictureImageFilter: (");
    str->appendf("crop: (%f,%f,%f,%f) ",
                 fCropRect.fLeft, fCropRect.fTop, fCropRect.fRight, fCropRect.fBottom);
    if (fPicture) {
        str->appendf("picture: (%f,%f,%f,%f)",
                     fPicture->cullRect().fLeft, fPicture->cullRect().fTop,
                     fPicture->cullRect().fRight, fPicture->cullRect().fBottom);
    }
    str->append(")");
}
#endif
