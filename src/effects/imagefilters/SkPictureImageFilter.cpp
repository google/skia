/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <optional>
#include <utility>

namespace {

class SkPictureImageFilter final : public SkImageFilter_Base {
public:
    SkPictureImageFilter(sk_sp<SkPicture> picture, const SkRect& cullRect)
            : SkImageFilter_Base(nullptr, 0)
            , fPicture(std::move(picture))
            , fCullRect(cullRect) {
        // The external cullrect should already have been intersected with the internal cull rect
        SkASSERT(fPicture && fPicture->cullRect().contains(cullRect));
    }

    SkRect computeFastBounds(const SkRect&) const override { return SkRect(fCullRect); }

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterPictureImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkPictureImageFilter)

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    skif::FilterResult onFilterImage(const skif::Context& ctx) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    sk_sp<SkPicture> fPicture;
    skif::ParameterSpace<SkRect> fCullRect;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Picture(sk_sp<SkPicture> pic, const SkRect& targetRect) {
    if (pic) {
        SkRect cullRect = pic->cullRect();
        if (cullRect.intersect(targetRect)) {
            return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(pic), cullRect));
        }
    }
    return SkImageFilters::Empty();
}

void SkRegisterPictureImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkPictureImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkPictureImageFilterImpl", SkPictureImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkPictureImageFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPicture> picture;
    if (buffer.readBool()) {
        picture = SkPicturePriv::MakeFromBuffer(buffer);
    }

    SkRect cullRect;
    buffer.readRect(&cullRect);
    return SkImageFilters::Picture(std::move(picture), cullRect);
}

void SkPictureImageFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeBool(SkToBool(fPicture));
    if (fPicture) {
        SkPicturePriv::Flatten(fPicture, buffer);
    }
    buffer.writeRect(SkRect(fCullRect));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkPictureImageFilter::onFilterImage(const skif::Context& ctx) const {
    return skif::FilterResult::MakeFromPicture(ctx, fPicture, fCullRect);
}

skif::LayerSpace<SkIRect> SkPictureImageFilter::onGetInputLayerBounds(
        const skif::Mapping&,
        const skif::LayerSpace<SkIRect>&,
        std::optional<skif::LayerSpace<SkIRect>>) const {
    // This is a leaf filter, it requires no input and no further recursion
    return skif::LayerSpace<SkIRect>::Empty();
}

std::optional<skif::LayerSpace<SkIRect>> SkPictureImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>>) const {
    // The output is the transformed bounds of the picture.
    return mapping.paramToLayer(fCullRect).roundOut();
}
