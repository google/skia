/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRenderEffect.h"

#include "SkDropShadowImageFilter.h"
#include "SkMakeUnique.h"
#include "SkSGColor.h"

namespace sksg {

sk_sp<RenderNode> ImageFilterEffect::Make(sk_sp<RenderNode> child, sk_sp<ImageFilter> filter) {
    return filter ? sk_sp<RenderNode>(new ImageFilterEffect(std::move(child), std::move(filter)))
                  : child;
}

ImageFilterEffect::ImageFilterEffect(sk_sp<RenderNode> child, sk_sp<ImageFilter> filter)
    // filters always override descendent damage
    : INHERITED(std::move(child), kOverrideDamage_Trait)
    , fImageFilter(std::move(filter)) {
    this->observeInval(fImageFilter);
}

ImageFilterEffect::~ImageFilterEffect() {
    this->unobserveInval(fImageFilter);
}

SkRect ImageFilterEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    // FIXME: image filter effects should replace the descendents' damage!
    fImageFilter->revalidate(ic, ctm);

    const auto& filter = fImageFilter->getFilter();
    SkASSERT(filter->canComputeFastBounds());

    return filter->computeFastBounds(this->INHERITED::onRevalidate(ic, ctm));
}

const RenderNode* ImageFilterEffect::onNodeAt(const SkPoint& p) const {
    // TODO: map p through the filter DAG and dispatch to descendants?
    // For now, image filters occlude hit-testing.
    SkASSERT(this->bounds().contains(p.x(), p.y()));
    return this;
}

void ImageFilterEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    // Note: we're using the source content bounds for saveLayer, not our local/filtered bounds.
    const auto filter_ctx =
        ScopedRenderContext(canvas, ctx).setFilterIsolation(this->getChild()->bounds(),
                                                            fImageFilter->getFilter());
    this->INHERITED::onRender(canvas, filter_ctx);
}

ImageFilter::ImageFilter(sk_sp<ImageFilter> input)
    : ImageFilter(input ? skstd::make_unique<InputsT>(1, std::move(input)) : nullptr) {}

ImageFilter::ImageFilter(std::unique_ptr<InputsT> inputs)
    : INHERITED(kBubbleDamage_Trait)
    , fInputs(std::move(inputs)) {
    if (fInputs) {
        for (const auto& input : *fInputs) {
            this->observeInval(input);
        }
    }
}

ImageFilter::~ImageFilter() {
    if (fInputs) {
        for (const auto& input : *fInputs) {
            this->unobserveInval(input);
        }
    }
}

sk_sp<SkImageFilter> ImageFilter::refInput(size_t i) const {
    return (fInputs && i < fInputs->size()) ? (*fInputs)[i]->getFilter() : nullptr;
}

SkRect ImageFilter::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasInval());

    fFilter = this->onRevalidateFilter();
    return SkRect::MakeEmpty();
}

sk_sp<DropShadowImageFilter> DropShadowImageFilter::Make(sk_sp<ImageFilter> input) {
    return sk_sp<DropShadowImageFilter>(new DropShadowImageFilter(std::move(input)));
}

DropShadowImageFilter::DropShadowImageFilter(sk_sp<ImageFilter> input)
    : INHERITED(std::move(input)) {}

DropShadowImageFilter::~DropShadowImageFilter() = default;

sk_sp<SkImageFilter> DropShadowImageFilter::onRevalidateFilter() {
    const auto mode = (fMode == Mode::kShadowOnly)
            ? SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode
            : SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode;

    return SkDropShadowImageFilter::Make(fOffset.x(), fOffset.y(),
                                         fSigma.x(), fSigma.y(),
                                         fColor, mode, this->refInput(0));
}

} // namespace sksg
