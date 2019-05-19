/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGOpacityEffect.h"

namespace sksg {

OpacityEffect::OpacityEffect(sk_sp<RenderNode> child, float opacity)
    : INHERITED(std::move(child))
    , fOpacity(opacity) {}

void OpacityEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    // opacity <= 0 disables rendering
    if (fOpacity <= 0)
        return;

    const auto local_context = ScopedRenderContext(canvas, ctx).modulateOpacity(fOpacity);

    this->INHERITED::onRender(canvas, local_context);
}

const RenderNode* OpacityEffect::onNodeAt(const SkPoint& p) const {
    return (fOpacity > 0) ? this->INHERITED::onNodeAt(p) : nullptr;
}

SkRect OpacityEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // opacity <= 0 disables rendering AND revalidation for the sub-DAG
    return fOpacity > 0 ? this->INHERITED::onRevalidate(ic, ctm) : SkRect::MakeEmpty();
}

} // namespace sksg
