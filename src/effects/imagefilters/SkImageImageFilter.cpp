/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkImageImageFilter final : public SkImageFilter_Base {
public:
    SkImageImageFilter(sk_sp<SkImage> image, const SkRect& srcRect, const SkRect& dstRect,
                       const SkSamplingOptions& sampling)
            : INHERITED(nullptr, 0, nullptr)
            , fImage(std::move(image))
            , fSrcRect(srcRect)
            , fDstRect(dstRect)
            , fSampling(sampling) {}

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    friend void ::SkRegisterImageImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkImageImageFilter)

    sk_sp<SkImage>    fImage;
    SkRect            fSrcRect, fDstRect;
    SkSamplingOptions fSampling;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Image(sk_sp<SkImage> image,
                                           const SkRect& srcRect,
                                           const SkRect& dstRect,
                                           const SkSamplingOptions& sampling) {
    if (!image || srcRect.width() <= 0.0f || srcRect.height() <= 0.0f) {
        return nullptr;
    }

    return sk_sp<SkImageFilter>(new SkImageImageFilter(
            std::move(image), srcRect, dstRect, sampling));
}

void SkRegisterImageImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkImageImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkImageSourceImpl", SkImageImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkImageImageFilter::CreateProc(SkReadBuffer& buffer) {
    SkSamplingOptions sampling;
    if (buffer.isVersionLT(SkPicturePriv::kImageFilterImageSampling_Version)) {
        sampling = SkSamplingOptions(buffer.checkFilterQuality(),
                                     SkSamplingOptions::kMedium_asMipmapLinear);
    } else {
        sampling = buffer.readSampling();
    }

    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);

    sk_sp<SkImage> image(buffer.readImage());
    if (!image) {
        return nullptr;
    }

    return SkImageFilters::Image(std::move(image), src, dst, sampling);
}

void SkImageImageFilter::flatten(SkWriteBuffer& buffer) const {
    SkSamplingPriv::Write(buffer, fSampling);
    buffer.writeRect(fSrcRect);
    buffer.writeRect(fDstRect);
    buffer.writeImage(fImage.get());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkImageImageFilter::onFilterImage(const Context& ctx,
                                                        SkIPoint* offset) const {
    SkRect dstRect;
    ctx.ctm().mapRect(&dstRect, fDstRect);

    SkRect bounds = SkRect::MakeIWH(fImage->width(), fImage->height());
    if (fSrcRect == bounds) {
        int iLeft = dstRect.fLeft;
        int iTop = dstRect.fTop;
        // TODO: this seems to be a very noise-prone way to determine this (esp. the floating-point
        // widths & heights).
        if (dstRect.width() == bounds.width() && dstRect.height() == bounds.height() &&
            iLeft == dstRect.fLeft && iTop == dstRect.fTop) {
            // The dest is just an un-scaled integer translation of the entire image; return it
            offset->fX = iLeft;
            offset->fY = iTop;

            return SkSpecialImage::MakeFromImage(ctx.getContext(),
                                                 SkIRect::MakeWH(fImage->width(), fImage->height()),
                                                 fImage, ctx.surfaceProps());
        }
    }

    const SkIRect dstIRect = dstRect.roundOut();

    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(dstIRect.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    // TODO: it seems like this clear shouldn't be necessary (see skbug.com/5075)
    canvas->clear(0x0);

    SkPaint paint;

    // Subtract off the integer component of the translation (will be applied in offset, below).
    dstRect.offset(-SkIntToScalar(dstIRect.fLeft), -SkIntToScalar(dstIRect.fTop));
    paint.setBlendMode(SkBlendMode::kSrc);

    // FIXME: this probably shouldn't be necessary, but drawImageRect asserts
    SkSamplingOptions sampling = fSampling;
    // None filtering when it's translate-only (even for cubicresampling? <reed>)
    if (fSrcRect.width() == dstRect.width() && fSrcRect.height() == dstRect.height()) {
        sampling = SkSamplingOptions();
    }
    canvas->drawImageRect(fImage.get(), fSrcRect, dstRect, sampling, &paint,
                          SkCanvas::kStrict_SrcRectConstraint);

    offset->fX = dstIRect.fLeft;
    offset->fY = dstIRect.fTop;
    return surf->makeImageSnapshot();
}

SkRect SkImageImageFilter::computeFastBounds(const SkRect& src) const {
    return fDstRect;
}

SkIRect SkImageImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                               MapDirection direction,
                                               const SkIRect* inputRect) const {
    if (kReverse_MapDirection == direction) {
        return INHERITED::onFilterNodeBounds(src, ctm, direction, inputRect);
    }

    SkRect dstRect = fDstRect;
    ctm.mapRect(&dstRect);
    return dstRect.roundOut();
}
