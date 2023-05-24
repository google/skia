/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkSpecialImage.h"

#include <optional>
#include <utility>

class SkMatrix;
class SkReadBuffer;

namespace {

class SkComposeImageFilter final : public SkImageFilter_Base {
public:
    explicit SkComposeImageFilter(sk_sp<SkImageFilter> inputs[2])
            : INHERITED(inputs, 2, nullptr,
                        // Compose only uses the source if the inner filter uses the source image.
                        // Any outer reference to source is rebound to the result of the inner.
                        inputs[1] ? as_IFB(inputs[1])->usesSource() : false) {
        SkASSERT(inputs[0].get());
        SkASSERT(inputs[1].get());
    }

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;
    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

private:
    friend void ::SkRegisterComposeImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkComposeImageFilter)

    using INHERITED = SkImageFilter_Base;
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
    return SkImageFilters::Compose(common.getInput(0), common.getInput(1));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRect SkComposeImageFilter::computeFastBounds(const SkRect& src) const {
    const SkImageFilter* outer = this->getInput(0);
    const SkImageFilter* inner = this->getInput(1);

    return outer->computeFastBounds(inner->computeFastBounds(src));
}

sk_sp<SkSpecialImage> SkComposeImageFilter::onFilterImage(const Context& ctx,
                                                          SkIPoint* offset) const {
    // The bounds passed to the inner filter must be filtered by the outer
    // filter, so that the inner filter produces the pixels that the outer
    // filter requires as input. This matters if the outer filter moves pixels. The content
    // bounds of the outer filter is the expected output bounds of the inner filter.
    SkIRect innerOutputBounds = this->getInput(1)->filterBounds(SkIRect(ctx.source().layerBounds()),
                                                                ctx.ctm(), kForward_MapDirection);
    SkIRect innerClipBounds;
    innerClipBounds = this->getInput(0)->filterBounds(ctx.clipBounds(), ctx.ctm(),
                                                      kReverse_MapDirection, &innerOutputBounds);
    Context innerContext = ctx.withNewDesiredOutput(skif::LayerSpace<SkIRect>(innerClipBounds));
    SkIPoint innerOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> inner(this->filterInput(1, innerContext, &innerOffset));
    if (!inner) {
        return nullptr;
    }

    // NOTE: This is the only spot in image filtering where the source image of the context
    // is not constant for the entire DAG evaluation. Given that the inner and outer DAG branches
    // were already created, there's no alternative way for the leaf nodes of the outer DAG to
    // get the results of the inner DAG. Overriding the source image of the context has the correct
    // effect, but means that the source image is not fixed for the entire filter process.
    Context outerContext = ctx.withNewSource(inner, skif::LayerSpace<SkIPoint>(innerOffset));

    SkIPoint outerOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> outer(this->filterInput(0, outerContext, &outerOffset));
    if (!outer) {
        return nullptr;
    }

    // TODO: Remove including innerOffset in this calculation once withNewSource() does not change
    // the param-to-layer matrix. Once all filter implementations support non (0,0) source origins,
    // Compose() will not change the param-to-layer mapping. Any impact from innerOffset will be
    // automatically taken into account by the inner FilterResult's internal origin.
    *offset = innerOffset + outerOffset;
    return outer;
}

SkIRect SkComposeImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                             MapDirection dir, const SkIRect* inputRect) const {
    const SkImageFilter* outer = this->getInput(0);
    const SkImageFilter* inner = this->getInput(1);

    if (dir == kReverse_MapDirection) {
        // The output 'src' is processed by the outer filter, producing its required input bounds,
        // which is then the output bounds required of the inner filter. We pass the inputRect to
        // outer and not inner to match the default recursion logic of onGetInputLayerBounds
        const SkIRect outerRect = outer->filterBounds(src, ctm, dir, inputRect);
        return inner->filterBounds(outerRect, ctm, dir);
    } else {
        // The input 'src' is processed by the inner filter, producing the input bounds for the
        // outer filter of the composition, which then produces the final forward output bounds
        const SkIRect innerRect = inner->filterBounds(src, ctm, dir);
        return outer->filterBounds(innerRect, ctm, dir);
    }
}
