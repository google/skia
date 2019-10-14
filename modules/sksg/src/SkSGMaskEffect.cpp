/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGMaskEffect.h"

#include "include/core/SkCanvas.h"
#include "include/effects/SkLumaColorFilter.h"

namespace sksg {

static bool is_inverted(sksg::MaskEffect::Mode mode) {
    return static_cast<uint32_t>(mode) & 1;
};

static bool is_luma(sksg::MaskEffect::Mode mode) {
    return static_cast<uint32_t>(mode) & 2;
}

MaskEffect::MaskEffect(sk_sp<RenderNode> child, sk_sp<RenderNode> mask, Mode mode)
    : INHERITED(std::move(child))
    , fMaskNode(std::move(mask))
    , fMaskMode(mode) {
    this->observeInval(fMaskNode);
}

MaskEffect::~MaskEffect() {
    this->unobserveInval(fMaskNode);
}

void MaskEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    SkAutoCanvasRestore acr(canvas, false);

    SkPaint mask_paint;
    if (ctx) {
        // Apply all context overrides on the top mask layer.
        ctx->modulatePaint(canvas->getTotalMatrix(), &mask_paint);
    }

    RenderContext mask_render_context;
    if (is_luma(fMaskMode)) {
        mask_render_context.fColorFilter = SkLumaColorFilter::Make();
    }

    canvas->saveLayer(this->bounds(), &mask_paint);
    fMaskNode->render(canvas, &mask_render_context);

    SkPaint p;
    p.setBlendMode(is_inverted(fMaskMode) ? SkBlendMode::kSrcOut : SkBlendMode::kSrcIn);
    canvas->saveLayer(this->bounds(), &p);

    this->INHERITED::onRender(canvas, nullptr);
}

const RenderNode* MaskEffect::onNodeAt(const SkPoint& p) const {
    const auto mask_hit = (!!fMaskNode->nodeAt(p) == !is_inverted(fMaskMode));

    return mask_hit ? this->INHERITED::onNodeAt(p) : nullptr;
}

SkRect MaskEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    const auto maskBounds = fMaskNode->revalidate(ic, ctm);
    auto childBounds = this->INHERITED::onRevalidate(ic, ctm);

    return (is_inverted(fMaskMode) || childBounds.intersect(maskBounds))
        ? childBounds
        : SkRect::MakeEmpty();
}

} // namespace sksg
