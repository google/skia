/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkPictureImageFilter final : public SkImageFilter_Base {
public:
    SkPictureImageFilter(sk_sp<SkPicture> picture, const SkRect& cropRect)
            : INHERITED(nullptr, 0, nullptr)
            , fPicture(std::move(picture))
            , fCropRect(cropRect) {}

protected:
    /*  Constructs an SkPictureImageFilter object from an SkReadBuffer.
     *  Note: If the SkPictureImageFilter object construction requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling this constructor.
     *  @param SkReadBuffer Serialized picture data.
     */
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

    SkRect computeFastBounds(const SkRect& src) const override;
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    friend void ::SkRegisterPictureImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkPictureImageFilter)

    sk_sp<SkPicture>    fPicture;
    SkRect              fCropRect;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Picture(sk_sp<SkPicture> pic, const SkRect& targetRect) {
    return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(pic), targetRect));
}

void SkRegisterPictureImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkPictureImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkPictureImageFilterImpl", SkPictureImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkPictureImageFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPicture> picture;
    SkRect cropRect;

    if (buffer.readBool()) {
        picture = SkPicturePriv::MakeFromBuffer(buffer);
    }
    buffer.readRect(&cropRect);

    return SkImageFilters::Picture(std::move(picture), cropRect);
}

void SkPictureImageFilter::flatten(SkWriteBuffer& buffer) const {
    bool hasPicture = (fPicture != nullptr);
    buffer.writeBool(hasPicture);
    if (hasPicture) {
        SkPicturePriv::Flatten(fPicture, buffer);
    }
    buffer.writeRect(fCropRect);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkPictureImageFilter::onFilterImage(const Context& ctx,
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
    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(bounds.size(), &props));
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

SkRect SkPictureImageFilter::computeFastBounds(const SkRect& src) const {
    return fCropRect;
}

SkIRect SkPictureImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                 MapDirection direction,
                                                 const SkIRect* inputRect) const {
    if (kReverse_MapDirection == direction) {
        return INHERITED::onFilterNodeBounds(src, ctm, direction, inputRect);
    }

    SkRect dstRect = fCropRect;
    ctm.mapRect(&dstRect);
    return dstRect.roundOut();
}
