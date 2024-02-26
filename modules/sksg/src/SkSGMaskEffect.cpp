/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGMaskEffect.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "modules/sksg/include/SkSGNode.h"

class SkMatrix;
struct SkPoint;

namespace sksg {

static bool is_inverted(sksg::MaskEffect::Mode mode) {
    return static_cast<uint32_t>(mode) & 1;
}

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

    // The mask mode covers two independent bits.
    //
    //   - mask source controls how the mask coverage is generated:
    //     * alpha => coverage = mask_alpha
    //     * luma  => coverage = luma(mask_rgb)
    //
    //   - mask type controls how the mask coverage is interpreted:
    //     * normal   => coverage' = coverage
    //     * inverted => coverage' = 1 - coverage

    {
        // Outer layer: mask coverage stored in the alpha channel.
        SkPaint mask_layer_paint;
        if (ctx) {
            // Apply all optional context overrides upfront.
            ctx->modulatePaint(canvas->getTotalMatrix(), &mask_layer_paint);
        }

        RenderContext mask_render_context;
        if (is_luma(fMaskMode)) {
            mask_render_context.fColorFilter = SkLumaColorFilter::Make();
        }

        // TODO: could be an A8 layer?
        canvas->saveLayer(this->bounds(), &mask_layer_paint);
        fMaskNode->render(canvas, &mask_render_context);

        {
            // Inner layer: masked content.
            SkPaint content_layer_paint;
            content_layer_paint.setBlendMode(is_inverted(fMaskMode) ? SkBlendMode::kSrcOut
                                                                    : SkBlendMode::kSrcIn);
            canvas->saveLayer(this->bounds(), &content_layer_paint);

            this->INHERITED::onRender(canvas, nullptr);
        }
    }
}

const RenderNode* MaskEffect::onNodeAt(const SkPoint& p) const {
    const auto mask_hit = (SkToBool(fMaskNode->nodeAt(p)) == !is_inverted(fMaskMode));

    if (!mask_hit) {
        return nullptr;
    }

    return this->INHERITED::onNodeAt(p);
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
