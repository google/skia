/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkComposeImageFilter final : public SkImageFilter_Base {
public:
    explicit SkComposeImageFilter(sk_sp<SkImageFilter> inputs[2])
            : INHERITED(inputs, 2, nullptr) {
        SkASSERT(inputs[0].get());
        SkASSERT(inputs[1].get());
    }

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;
    bool onCanHandleComplexCTM() const override { return true; }

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
    // filter requires as input. This matters if the outer filter moves pixels.
    SkIRect innerClipBounds;
    innerClipBounds = this->getInput(0)->filterBounds(ctx.clipBounds(), ctx.ctm(),
                                                      kReverse_MapDirection, &ctx.clipBounds());
    Context innerContext = ctx.withNewDesiredOutput(skif::LayerSpace<SkIRect>(innerClipBounds));
    SkIPoint innerOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> inner(this->filterInput(1, innerContext, &innerOffset));
    if (!inner) {
        return nullptr;
    }

    // TODO (michaelludwig) - Once all filters are updated to process coordinate spaces more
    // robustly, we can allow source images to have non-(0,0) origins, which will mean that the
    // CTM/clipBounds modifications for the outerContext can go away.
    SkMatrix outerMatrix(ctx.ctm());
    outerMatrix.postTranslate(SkIntToScalar(-innerOffset.x()), SkIntToScalar(-innerOffset.y()));
    SkIRect clipBounds = ctx.clipBounds();
    clipBounds.offset(-innerOffset.x(), -innerOffset.y());
    // NOTE: This is the only spot in image filtering where the source image of the context
    // is not constant for the entire DAG evaluation. Given that the inner and outer DAG branches
    // were already created, there's no alternative way for the leaf nodes of the outer DAG to
    // get the results of the inner DAG. Overriding the source image of the context has the correct
    // effect, but means that the source image is not fixed for the entire filter process.
    Context outerContext(outerMatrix, clipBounds, ctx.cache(), ctx.colorType(), ctx.colorSpace(),
                         inner.get());

    SkIPoint outerOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> outer(this->filterInput(0, outerContext, &outerOffset));
    if (!outer) {
        return nullptr;
    }

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
