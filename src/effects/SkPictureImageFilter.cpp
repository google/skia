/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureImageFilter.h"

#include "SkCanvas.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkImageSource.h"
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
    return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(picture), cropRect, nullptr));
}

SkPictureImageFilter::SkPictureImageFilter(sk_sp<SkPicture> picture)
    : INHERITED(nullptr, 0, nullptr)
    , fPicture(std::move(picture))
    , fCropRect(fPicture ? fPicture->cullRect() : SkRect::MakeEmpty()) {
}

SkPictureImageFilter::SkPictureImageFilter(sk_sp<SkPicture> picture, const SkRect& cropRect,
                                           sk_sp<SkColorSpace> colorSpace)
    : INHERITED(nullptr, 0, nullptr)
    , fPicture(std::move(picture))
    , fCropRect(cropRect)
    , fColorSpace(std::move(colorSpace)) {
}

enum PictureResolution {
    kDeviceSpace_PictureResolution,
    kLocalSpace_PictureResolution
};
static sk_sp<SkImageFilter> make_localspace_filter(sk_sp<SkPicture> pic, const SkRect& cropRect,
                                                   SkFilterQuality fq) {
    SkISize dim = { SkScalarRoundToInt(cropRect.width()), SkScalarRoundToInt(cropRect.height()) };
    auto img = SkImage::MakeFromPicture(std::move(pic), dim, nullptr, nullptr,
                                        SkImage::BitDepth::kU8, SkColorSpace::MakeSRGB());
    return SkImageSource::Make(img, cropRect, cropRect, fq);
}

sk_sp<SkFlattenable> SkPictureImageFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPicture> picture;
    SkRect cropRect;

    if (buffer.readBool()) {
        picture = SkPicture::MakeFromBuffer(buffer);
    }
    buffer.readRect(&cropRect);

    if (buffer.isVersionLT(SkReadBuffer::kRemovePictureImageFilterLocalSpace)) {
        PictureResolution pictureResolution = buffer.checkRange<PictureResolution>(
            kDeviceSpace_PictureResolution, kLocalSpace_PictureResolution);
        if (kLocalSpace_PictureResolution == pictureResolution) {
            return make_localspace_filter(std::move(picture), cropRect,
                                          buffer.checkFilterQuality());
        }
    }
    return sk_sp<SkImageFilter>(new SkPictureImageFilter(picture, cropRect, nullptr));
}

void SkPictureImageFilter::flatten(SkWriteBuffer& buffer) const {
    bool hasPicture = (fPicture != nullptr);
    buffer.writeBool(hasPicture);
    if (hasPicture) {
        fPicture->flatten(buffer);
    }
    buffer.writeRect(fCropRect);
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

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);
    canvas->clear(0x0);

    std::unique_ptr<SkCanvas> xformCanvas;
    if (fColorSpace) {
        // Only non-null in the case where onMakeColorSpace() was called.  This instructs
        // us to do the color space xform on playback.
        xformCanvas = SkCreateColorSpaceXformCanvas(canvas, fColorSpace);
        canvas = xformCanvas.get();
    }
    canvas->translate(-SkIntToScalar(bounds.fLeft), -SkIntToScalar(bounds.fTop));
    canvas->concat(ctx.ctm());
    canvas->drawPicture(fPicture);

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

sk_sp<SkImageFilter> SkPictureImageFilter::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    sk_sp<SkColorSpace> dstCS = xformer->dst();
    if (SkColorSpace::Equals(dstCS.get(), fColorSpace.get())) {
        return this->refMe();
    }

    return sk_sp<SkImageFilter>(new SkPictureImageFilter(fPicture, fCropRect, std::move(dstCS)));
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
