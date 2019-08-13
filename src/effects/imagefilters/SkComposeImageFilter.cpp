/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkComposeImageFilter.h"

#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

SK_USE_FLUENT_IMAGE_FILTER_TYPES

namespace {

class SkComposeImageFilterImpl final : public SkImageFilter_Base {
public:
    explicit SkComposeImageFilterImpl(sk_sp<SkImageFilter> inputs[2])
            : INHERITED(inputs, 2, nullptr) {
        SkASSERT(inputs[0].get());
        SkASSERT(inputs[1].get());
    }

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

private:
    // Composition needs to link one filter to the next, not the default union aggregation logic.
    skif::IRect<In::kLayer, For::kOutput> onFilterOutputBounds(
            const skif::IRect<In::kLayer, For::kInput>& contentBounds,
            const SkMatrix& layerMatrix) const override;

    skif::IRect<In::kLayer, For::kInput> onFilterLayerBounds(
            const skif::IRect<In::kLayer, For::kOutput>& targetOutputBounds,
            const SkMatrix& layerMatrix,
            const skif::IRect<In::kLayer, For::kInput>& originalInput) const override;

    bool onCanHandleComplexCTM() const override { return true; }

private:
    friend void SkComposeImageFilter::RegisterFlattenables();
    SK_FLATTENABLE_HOOKS(SkComposeImageFilterImpl)

    typedef SkImageFilter_Base INHERITED;
};

} // end namespace

sk_sp<SkImageFilter> SkComposeImageFilter::Make(sk_sp<SkImageFilter> outer,
                                                sk_sp<SkImageFilter> inner) {
    if (!outer) {
        return inner;
    }
    if (!inner) {
        return outer;
    }
    sk_sp<SkImageFilter> inputs[2] = { std::move(outer), std::move(inner) };
    return sk_sp<SkImageFilter>(new SkComposeImageFilterImpl(inputs));
}

void SkComposeImageFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkComposeImageFilterImpl);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkComposeImageFilter", SkComposeImageFilterImpl::CreateProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkComposeImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    return SkComposeImageFilter::Make(common.getInput(0), common.getInput(1));
}

SkRect SkComposeImageFilterImpl::computeFastBounds(const SkRect& src) const {
    const SkImageFilter* outer = this->getInput(0);
    const SkImageFilter* inner = this->getInput(1);

    return outer->computeFastBounds(inner->computeFastBounds(src));
}

sk_sp<SkSpecialImage> SkComposeImageFilterImpl::onFilterImage(const Context& ctx,
                                                              SkIPoint* offset) const {
    // The bounds passed to the inner filter must be filtered by the outer
    // filter, so that the inner filter produces the pixels that the outer
    // filter requires as input. This matters if the outer filter moves pixels.
    SkIRect innerClipBounds;
    innerClipBounds = this->getInput(0)->filterBounds(ctx.clipBounds(), ctx.ctm(),
                                                      kReverse_MapDirection, &ctx.clipBounds());
    Context innerContext = ctx.withClipBounds(innerClipBounds);
    SkIPoint innerOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> inner(this->filterInput(1, innerContext, &innerOffset));
    if (!inner) {
        return nullptr;
    }

    SkMatrix outerMatrix(ctx.ctm());
    outerMatrix.postTranslate(SkIntToScalar(-innerOffset.x()), SkIntToScalar(-innerOffset.y()));
    SkIRect clipBounds = ctx.clipBounds();
    clipBounds.offset(-innerOffset.x(), -innerOffset.y());
    Context outerContext(outerMatrix, clipBounds, ctx.cache(), ctx.colorType(),
                         ctx.colorSpace(), std::move(inner));

    SkIPoint outerOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> outer(this->filterInput(0, outerContext, &outerOffset));
    if (!outer) {
        return nullptr;
    }

    *offset = innerOffset + outerOffset;
    return outer;
}

skif::IRect<In::kLayer, For::kOutput> SkComposeImageFilterImpl::onFilterOutputBounds(
        const skif::IRect<In::kLayer, For::kInput>& contentBounds,
        const SkMatrix& layerMatrix) const {
    const SkImageFilter* outer = this->getInput(0);
    const SkImageFilter* inner = this->getInput(1);

    // First determine what the inner filter would produce when processing 'contentBounds'
    skif::IRect<In::kLayer, For::kOutput> innerOut = as_IFB(inner)->filterOutputBounds(
            contentBounds, layerMatrix);
    // Use the output of the inner filter as the content to the outer filter
    return as_IFB(outer)->filterOutputBounds(skif::LayerCast<For::kInput>(innerOut), layerMatrix);
}

skif::IRect<In::kLayer, For::kInput> SkComposeImageFilterImpl::onFilterLayerBounds(
        const skif::IRect<In::kLayer, For::kOutput>& targetOutputBounds,
        const SkMatrix& layerMatrix,
        const skif::IRect<In::kLayer, For::kInput>& originalInput) const {
    const SkImageFilter* outer = this->getInput(0);
    const SkImageFilter* inner = this->getInput(1);

    // First determine what the outer filter requires in order to cover targetOutputBounds
    skif::IRect<In::kLayer, For::kInput> outerInput = as_IFB(outer)->filterLayerBounds(
            targetOutputBounds, layerMatrix, &originalInput);
    // Use the required input as the target output for the inner filter
    return as_IFB(inner)->filterLayerBounds(skif::LayerCast<For::kOutput>(outerInput), layerMatrix,
                                            &outerInput);
}
