/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRenderEffect.h"

#include "SkDropShadowImageFilter.h"
#include "SkSGColor.h"

namespace sksg {

sk_sp<DropShadowEffect> DropShadowEffect::Make(sk_sp<RenderNode> child, sk_sp<Color> color) {
    return (child && color)
        ? sk_sp<DropShadowEffect>(new DropShadowEffect(std::move(child), std::move(color)))
        : nullptr;
}

DropShadowEffect::DropShadowEffect(sk_sp<RenderNode> child, sk_sp<Color> color)
    : INHERITED(std::move(child))
    , fColor(std::move(color)) {
    this->observeInval(fColor);
}

DropShadowEffect::~DropShadowEffect() {
    this->unobserveInval(fColor);
}

SkRect DropShadowEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    fColor->revalidate(ic, ctm);
    auto bounds = this->INHERITED::onRevalidate(ic, ctm);

    // TODO: still computing shadow bounds with hard-coded heuristics...
    const auto shadow_padding = fSigma * 3;
    const auto shadow_bounds = bounds.makeOffset(fOffset.x(), fOffset.y())
                                     .makeOutset(shadow_padding.x(), shadow_padding.y());
    bounds.join(shadow_bounds);

    // TODO: avoid rebuilding the filter when/if we know all invals are descendent-triggered?
    const SkImageFilter::CropRect crop(bounds);
    const auto shadow_mode = (fMode == Mode::kShadowOnly)
        ? SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode
        : SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode;
    fFilter = SkDropShadowImageFilter::Make(fOffset.x(), fOffset.y(),
                                            fSigma.x(), fSigma.y(),
                                            fColor->getColor(),
                                            shadow_mode,
                                            nullptr, &crop);

    return bounds;
}

void DropShadowEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    // TODO: hoist these checks to RenderNode?
    if (this->bounds().isEmpty())
        return;

    const auto shadow_ctx = ScopedRenderContext(canvas, ctx).setFilterIsolation(this->bounds(),
                                                                                fFilter);
    this->INHERITED::onRender(canvas, shadow_ctx);
}

} // namespace sksg
