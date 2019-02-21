/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRenderNode.h"

#include "SkCanvas.h"
#include "SkImageFilter.h"
#include "SkPaint.h"

namespace sksg {

RenderNode::RenderNode(uint32_t inval_traits) : INHERITED(inval_traits) {}

void RenderNode::render(SkCanvas* canvas, const RenderContext* ctx) const {
    SkASSERT(!this->hasInval());
    if (!this->bounds().isEmpty()) {
        this->onRender(canvas, ctx);
    }
}

const RenderNode* RenderNode::nodeAt(const SkPoint& p) const {
    return this->bounds().contains(p.x(), p.y()) ? this->onNodeAt(p) : nullptr;
}

bool RenderNode::RenderContext::modulatePaint(SkPaint* paint) const {
    const auto initial_alpha = paint->getAlpha(),
                       alpha = SkToU8(sk_float_round2int(initial_alpha * fOpacity));

    if (alpha != initial_alpha || fColorFilter) {
        paint->setAlpha(alpha);
        paint->setColorFilter(SkColorFilter::MakeComposeFilter(fColorFilter,
                                                               paint->refColorFilter()));
        return true;
    }

    return false;
}

RenderNode::ScopedRenderContext::ScopedRenderContext(SkCanvas* canvas, const RenderContext* ctx)
    : fCanvas(canvas)
    , fCtx(ctx ? *ctx : RenderContext())
    , fRestoreCount(canvas->getSaveCount()) {}

RenderNode::ScopedRenderContext::~ScopedRenderContext() {
    if (fRestoreCount >= 0) {
        fCanvas->restoreToCount(fRestoreCount);
    }
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::modulateOpacity(float opacity) {
    SkASSERT(opacity >= 0 && opacity <= 1);
    fCtx.fOpacity *= opacity;
    return std::move(*this);
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::modulateColorFilter(sk_sp<SkColorFilter> cf) {
    fCtx.fColorFilter = SkColorFilter::MakeComposeFilter(std::move(fCtx.fColorFilter),
                                                         std::move(cf));
    return std::move(*this);
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::setIsolation(const SkRect& bounds, bool isolation) {
    if (isolation) {
        SkPaint layer_paint;
        if (fCtx.modulatePaint(&layer_paint)) {
            fCanvas->saveLayer(bounds, &layer_paint);
            fCtx = RenderContext();
        }
    }
    return std::move(*this);
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::setFilterIsolation(const SkRect& bounds,
                                                    sk_sp<SkImageFilter> filter) {
    SkPaint layer_paint;
    fCtx.modulatePaint(&layer_paint);

    SkASSERT(!layer_paint.getImageFilter());
    layer_paint.setImageFilter(std::move(filter));
    fCanvas->saveLayer(bounds, &layer_paint);
    fCtx = RenderContext();

    return std::move(*this);
}

} // namespace sksg
