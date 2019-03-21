/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureImageFilter.h"

#include "SkCanvas.h"
#include "SkImageSource.h"
#include "SkPicturePriv.h"
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
    return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(picture), cropRect));
}

SkPictureImageFilter::SkPictureImageFilter(sk_sp<SkPicture> picture)
    : INHERITED(nullptr, 0, nullptr)
    , fPicture(std::move(picture))
    , fCropRect(fPicture ? fPicture->cullRect() : SkRect::MakeEmpty()) {
}

SkPictureImageFilter::SkPictureImageFilter(sk_sp<SkPicture> picture, const SkRect& cropRect)
    : INHERITED(nullptr, 0, nullptr)
    , fPicture(std::move(picture))
    , fCropRect(cropRect) {
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
        picture = SkPicturePriv::MakeFromBuffer(buffer);
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
    return sk_sp<SkImageFilter>(new SkPictureImageFilter(picture, cropRect));
}

void SkPictureImageFilter::flatten(SkWriteBuffer& buffer) const {
    bool hasPicture = (fPicture != nullptr);
    buffer.writeBool(hasPicture);
    if (hasPicture) {
        SkPicturePriv::Flatten(fPicture, buffer);
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

    // Given the standard usage of the picture image filter (i.e., to render content at a fixed
    // resolution that, most likely, differs from the screen's) disable LCD text.
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size(),
                                                     kPremul_SkAlphaType, &props));
    if (!surf) {
        return nullptr;
    }

    SkASSERT(kUnknown_SkPixelGeometry == surf->props().pixelGeometry());

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);
    canvas->clear(0x0);

    canvas->translate(-SkIntToScalar(bounds.fLeft), -SkIntToScalar(bounds.fTop));
    canvas->concat(ctx.ctm());
    canvas->drawPicture(fPicture);

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}
