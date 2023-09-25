/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <optional>
#include <utility>

namespace {

class SkColorFilterImageFilter final : public SkImageFilter_Base {
public:
    SkColorFilterImageFilter(sk_sp<SkColorFilter> cf, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fColorFilter(std::move(cf)) {}

    SkRect computeFastBounds(const SkRect& bounds) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterColorFilterImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkColorFilterImageFilter)

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    bool onAffectsTransparentBlack() const override {
        return as_CFB(fColorFilter)->affectsTransparentBlack();
    }

    bool onIsColorFilterNode(SkColorFilter** filter) const override {
        SkASSERT(1 == this->countInputs());
        if (filter) {
            *filter = SkRef(fColorFilter.get());
        }
        return true;
    }

    sk_sp<SkColorFilter> fColorFilter;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::ColorFilter(sk_sp<SkColorFilter> cf,
                                                 sk_sp<SkImageFilter> input,
                                                 const CropRect& cropRect) {
    if (cf) {
        SkColorFilter* inputCF;
        // This is an optimization, as it collapses the hierarchy by just combining the two
        // colorfilters into a single one, which the new imagefilter will wrap.
        // NOTE: FilterResults are capable of composing non-adjacent CF nodes together. We could
        // remove this optimization at construction time, but may as well do the work just once.
        if (input && input->isColorFilterNode(&inputCF)) {
            cf = cf->makeComposed(sk_sp<SkColorFilter>(inputCF));
            input = sk_ref_sp(input->getInput(0));
        }
    }

    sk_sp<SkImageFilter> filter = std::move(input);
    if (cf) {
        filter = sk_sp<SkImageFilter>(
                new SkColorFilterImageFilter(std::move(cf), std::move(filter)));
    }
    if (cropRect) {
        filter = SkImageFilters::Crop(*cropRect, std::move(filter));
    }
    return filter;
}

void SkRegisterColorFilterImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkColorFilterImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkColorFilterImageFilterImpl", SkColorFilterImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkColorFilterImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    sk_sp<SkColorFilter> cf(buffer.readColorFilter());
    return SkImageFilters::ColorFilter(std::move(cf), common.getInput(0), common.cropRect());
}

void SkColorFilterImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeFlattenable(fColorFilter.get());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkColorFilterImageFilter::onFilterImage(const skif::Context& ctx) const {
    return this->getChildOutput(0, ctx).applyColorFilter(ctx, fColorFilter);
}

skif::LayerSpace<SkIRect> SkColorFilterImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    return this->getChildInputLayerBounds(0, mapping, desiredOutput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkColorFilterImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // For bounds calculations, we only need to consider the current node's transparency
    // effect, since any child's transparency-affecting behavior should be accounted for in
    // the child's bounds call.
    if (as_CFB(fColorFilter)->affectsTransparentBlack()) {
        return skif::LayerSpace<SkIRect>::Unbounded();
    } else {
        return this->getChildOutputLayerBounds(0, mapping, contentBounds);
    }
}

SkRect SkColorFilterImageFilter::computeFastBounds(const SkRect& bounds) const {
    // See comment in onGetOutputLayerBounds().
    if (as_CFB(fColorFilter)->affectsTransparentBlack()) {
        return SkRectPriv::MakeLargeS32();
    } else if (this->getInput(0)) {
        return this->getInput(0)->computeFastBounds(bounds);
    } else {
        return bounds;
    }
}
