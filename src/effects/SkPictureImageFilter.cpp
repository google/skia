/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureImageFilter.h"

#include "SkCanvas.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#include "SkValidationUtils.h"

sk_sp<SkImageFilter> SkPictureImageFilter::Make(sk_sp<SkPicture> picture) {
    return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(picture)));
}

sk_sp<SkImageFilter> SkPictureImageFilter::Make(sk_sp<SkPicture> picture,
                                                const SkRect& cropRect) {
    return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(picture), 
                                                         cropRect,
                                                         kDeviceSpace_PictureResolution,
                                                         kLow_SkFilterQuality));
}

sk_sp<SkImageFilter> SkPictureImageFilter::MakeForLocalSpace(sk_sp<SkPicture> picture,
                                                             const SkRect& cropRect,
                                                             SkFilterQuality filterQuality) {
    return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(picture),
                                                         cropRect,
                                                         kLocalSpace_PictureResolution,
                                                         filterQuality));
}

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

sk_sp<SkSpecialImage> SkPictureImageFilter::onFilterImage(SkSpecialImage* source,
                                                          const Context& ctx,
                                                          SkIPoint* offset) const {
    if (!fPicture) {
        return nullptr;
    }

    SkRect floatBounds;
    ctx.ctm().mapRect(&floatBounds, fCropRect);
    SkIRect bounds = floatBounds.roundOut();
    if (!bounds.intersect(ctx.clipBounds())) {
        return nullptr;
    }

    SkASSERT(!bounds.isEmpty());

    SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(), kPremul_SkAlphaType);
    sk_sp<SkSpecialSurface> surf(source->makeSurface(info));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    if (kDeviceSpace_PictureResolution == fPictureResolution ||
        0 == (ctx.ctm().getType() & ~SkMatrix::kTranslate_Mask)) {
        this->drawPictureAtDeviceResolution(canvas, bounds, ctx);
    } else {
        this->drawPictureAtLocalResolution(source, canvas, bounds, ctx);
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

void SkPictureImageFilter::drawPictureAtDeviceResolution(SkCanvas* canvas,
                                                         const SkIRect& deviceBounds,
                                                         const Context& ctx) const {
    canvas->translate(-SkIntToScalar(deviceBounds.fLeft), -SkIntToScalar(deviceBounds.fTop));
    canvas->concat(ctx.ctm());
    canvas->drawPicture(fPicture);
}

void SkPictureImageFilter::drawPictureAtLocalResolution(SkSpecialImage* source,
                                                        SkCanvas* canvas,
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

    sk_sp<SkSpecialImage> localImg;
    {                                                            
        const SkImageInfo info = SkImageInfo::MakeN32(localIBounds.width(), localIBounds.height(),
                                                      kPremul_SkAlphaType);

        sk_sp<SkSpecialSurface> localSurface(source->makeSurface(info));
        if (!localSurface) {
            return;
        }

        SkCanvas* localCanvas = localSurface->getCanvas();
        SkASSERT(localCanvas);
        
        localCanvas->clear(0x0);

        localCanvas->translate(-SkIntToScalar(localIBounds.fLeft),
                               -SkIntToScalar(localIBounds.fTop));
        localCanvas->drawPicture(fPicture);

        localImg = localSurface->makeImageSnapshot();
        SkASSERT(localImg);
    }

    {
        canvas->translate(-SkIntToScalar(deviceBounds.fLeft), -SkIntToScalar(deviceBounds.fTop));
        canvas->concat(ctx.ctm());
        SkPaint paint;
        paint.setFilterQuality(fFilterQuality);

        localImg->draw(canvas,
                       SkIntToScalar(localIBounds.fLeft),
                       SkIntToScalar(localIBounds.fTop),
                       &paint);
    }
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
