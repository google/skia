/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"

#include <optional>
#include <utility>

class SkReadBuffer;

namespace {

class SkComposeImageFilter final : public SkImageFilter_Base {
    static constexpr int kOuter = 0;
    static constexpr int kInner = 1;

public:
    explicit SkComposeImageFilter(sk_sp<SkImageFilter> inputs[2])
            : SkImageFilter_Base(inputs, 2,
                                 // Compose only uses the source if the inner filter uses the source
                                 // image. Any outer reference to source is rebound to the result of
                                 // the inner.
                                 inputs[kInner] ? as_IFB(inputs[kInner])->usesSource() : false) {
        SkASSERT(inputs[kOuter].get());
        SkASSERT(inputs[kInner].get());
    }

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    // No flatten() needed since this does not add state beyond the input image filters handled
    // by the parent implementation.

private:
    friend void ::SkRegisterComposeImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkComposeImageFilter)

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Compose(sk_sp<SkImageFilter> outer,
                                             sk_sp<SkImageFilter> inner) {
    if (!outer) {
        return inner;
    }
    if (!inner) {
        return outer;
    }
    sk_sp<SkImageFilter> inputs[2] = { std::move(outer), std::move(inner) };
    return sk_sp<SkImageFilter>(new SkComposeImageFilter(inputs));
}

void SkRegisterComposeImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkComposeImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkComposeImageFilterImpl", SkComposeImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkComposeImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    return SkImageFilters::Compose(common.getInput(kOuter), common.getInput(kInner));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkComposeImageFilter::onFilterImage(const skif::Context& ctx) const {
    // Get the expected output of the inner filter, given the source image's layer bounds as content
    auto innerOutputBounds =
            this->getChildOutputLayerBounds(kInner, ctx.mapping(), ctx.source().layerBounds());
    // Get the required input for the outer filter, that it needs to cover the desired output.
    skif::LayerSpace<SkIRect> outerRequiredInput =
            this->getChildInputLayerBounds(kOuter,
                                           ctx.mapping(),
                                           ctx.desiredOutput(),
                                           innerOutputBounds);

    // Evalute the inner filter and pass that to the outer filter.
    skif::FilterResult innerResult =
            this->getChildOutput(kInner, ctx.withNewDesiredOutput(outerRequiredInput));

    // NOTE: This is the only spot in image filtering where the source image of the context
    // is not constant for the entire DAG evaluation. Given that the inner and outer DAG branches
    // were already created, there's no alternative way for the leaf nodes of the outer DAG to
    // get the results of the inner DAG. Overriding the source image of the context has the correct
    // effect, but means that the source image is not fixed for the entire filter process.
    return this->getChildOutput(kOuter, ctx.withNewSource(innerResult));
}

skif::LayerSpace<SkIRect> SkComposeImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // The outer filter must produce 'desiredOutput'. Its required input bounds becomes the desired
    // output of the inner filter. However, 'contentBounds' is the bounds visible to the input
    // filter. The output bounds of the inner filter represents the content bounds of the outer.
    std::optional<skif::LayerSpace<SkIRect>> outerContentBounds;
    if (contentBounds) {
        outerContentBounds = this->getChildOutputLayerBounds(kInner, mapping, *contentBounds);
    } // else leave outer's content bounds "unbounded"

    skif::LayerSpace<SkIRect> innerDesiredOutput =
            this->getChildInputLayerBounds(kOuter, mapping, desiredOutput, outerContentBounds);
    return this->getChildInputLayerBounds(kInner, mapping, innerDesiredOutput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkComposeImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // The 'contentBounds' is processed by the inner filter, producing the content bounds for the
    // outer filter of the composition, which then produces the final output bounds.
    auto innerBounds = this->getChildOutputLayerBounds(kInner, mapping, contentBounds);
    // NOTE: Even if innerBounds is unbounded, the outer image filter may be capable of restricting
    // it if it contains a crop image filter.
    return this->getChildOutputLayerBounds(kOuter, mapping, innerBounds);
}

SkRect SkComposeImageFilter::computeFastBounds(const SkRect& src) const {
    return this->getInput(kOuter)->computeFastBounds(
            this->getInput(kInner)->computeFastBounds(src));
}
