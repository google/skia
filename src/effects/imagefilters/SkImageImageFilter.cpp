/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkWriteBuffer.h"

#include <optional>
#include <utility>

namespace {

class SkImageImageFilter final : public SkImageFilter_Base {
public:
    SkImageImageFilter(sk_sp<SkImage> image,
                       const SkRect& srcRect,
                       const SkRect& dstRect,
                       const SkSamplingOptions& sampling)
            : SkImageFilter_Base(nullptr, 0)
            , fImage(std::move(image))
            , fSrcRect(srcRect)
            , fDstRect(dstRect)
            , fSampling(sampling) {
        // The dst rect should be non-empty
        SkASSERT(fImage && !dstRect.isEmpty());
    }

    SkRect computeFastBounds(const SkRect&) const override { return SkRect(fDstRect); }

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterImageImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkImageImageFilter)

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    sk_sp<SkImage> fImage;
    // The src rect is relative to the image's contents, so is not technically in the parameter
    // coordinate space that responds to the layer matrix (unlike fDstRect).
    SkRect fSrcRect;
    skif::ParameterSpace<SkRect> fDstRect;
    SkSamplingOptions fSampling;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Image(sk_sp<SkImage> image,
                                           const SkRect& srcRect,
                                           const SkRect& dstRect,
                                           const SkSamplingOptions& sampling) {
    if (srcRect.isEmpty() || dstRect.isEmpty() || !image) {
        // There is no content to draw, so the filter should produce transparent black
        return SkImageFilters::Empty();
    } else {
        SkRect imageBounds = SkRect::Make(image->dimensions());
        if (imageBounds.contains(srcRect)) {
            // No change to srcRect and dstRect needed
            return sk_sp<SkImageFilter>(new SkImageImageFilter(
                    std::move(image), srcRect, dstRect, sampling));
        } else {
            SkMatrix srcToDst = SkMatrix::RectToRect(srcRect, dstRect);
            if (!imageBounds.intersect(srcRect)) {
                // No overlap, so draw empty
                return SkImageFilters::Empty();
            }

            // Adjust dstRect to match the updated src (which is stored in imageBounds)
            return sk_sp<SkImageFilter>(new SkImageImageFilter(
                    std::move(image), imageBounds, srcToDst.mapRect(imageBounds), sampling));
        }
    }
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
    buffer.writeSampling(fSampling);
    buffer.writeRect(fSrcRect);
    buffer.writeRect(SkRect(fDstRect));
    buffer.writeImage(fImage.get());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkImageImageFilter::onFilterImage(const skif::Context& ctx) const {
    return skif::FilterResult::MakeFromImage(ctx, fImage, fSrcRect, fDstRect, fSampling);
}

skif::LayerSpace<SkIRect> SkImageImageFilter::onGetInputLayerBounds(
        const skif::Mapping&,
        const skif::LayerSpace<SkIRect>&,
        std::optional<skif::LayerSpace<SkIRect>>) const {
    // This is a leaf filter, it requires no input and no further recursion
    return skif::LayerSpace<SkIRect>::Empty();
}

std::optional<skif::LayerSpace<SkIRect>> SkImageImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>>) const {
    // The output is the transformed bounds of the image.
    return mapping.paramToLayer(fDstRect).roundOut();
}
