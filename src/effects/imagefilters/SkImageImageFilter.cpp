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

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

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
        sampling = SkSamplingPriv::FromFQ(buffer.checkFilterQuality(), kLinear_SkMediumAs);
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
    const SkRect dstBounds = ctx.ctm().mapRect(fDstRect);
    const SkIRect dstIBounds = dstBounds.roundOut();

    // Quick check to see if we can return the image directly, which can be done if the transform
    // ends up being an integer translate and sampling would have no effect on the output.
    // TODO: This currently means cubic sampling can be skipped, even though it would change results
    // for integer translation draws.
    // TODO: This is prone to false negatives due to the floating point math; we could probably
    // get away with dimensions and translates being epsilon close to integers.
    const bool passthroughTransform = ctx.ctm().isScaleTranslate() &&
                                      ctx.ctm().getScaleX() > 0.f &&
                                      ctx.ctm().getScaleY() > 0.f;
    const bool passthroughSrcOffsets = SkScalarIsInt(fSrcRect.fLeft) &&
                                       SkScalarIsInt(fSrcRect.fTop);
    const bool passthroughDstOffsets = SkScalarIsInt(dstBounds.fLeft) &&
                                       SkScalarIsInt(dstBounds.fTop);
    const bool passthroughDims =
            SkScalarIsInt(fSrcRect.width()) && fSrcRect.width() == dstBounds.width() &&
            SkScalarIsInt(fSrcRect.height()) && fSrcRect.height() == dstBounds.height();

    if (passthroughTransform && passthroughSrcOffsets && passthroughDstOffsets && passthroughDims) {
        // Can pass through fImage directly, applying the dst's location to 'offset'. If fSrcRect
        // extends outside of the image, we adjust dst to match since those areas would have been
        // transparent black anyways.
        SkIRect srcIBounds = fSrcRect.roundOut();
        SkIPoint srcOffset = srcIBounds.topLeft();
        if (!srcIBounds.intersect(SkIRect::MakeWH(fImage->width(), fImage->height()))) {
            return nullptr;
        }

        *offset = dstIBounds.topLeft() + srcIBounds.topLeft() - srcOffset;
        return SkSpecialImage::MakeFromImage(ctx.getContext(), srcIBounds, fImage,
                                             ctx.surfaceProps());
    }

    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(dstIBounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    // Subtract off the integer component of the translation (will be applied in offset, below).
    canvas->translate(-dstIBounds.fLeft, -dstIBounds.fTop);
    canvas->concat(ctx.ctm());
    // TODO(skbug.com/5075): Canvases from GPU special surfaces come with unitialized content
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->drawImageRect(fImage.get(), fSrcRect, fDstRect, fSampling, nullptr,
                          SkCanvas::kStrict_SrcRectConstraint);

    *offset = dstIBounds.topLeft();
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
