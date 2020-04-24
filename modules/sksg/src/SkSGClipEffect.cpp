/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGClipEffect.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "modules/sksg/include/SkSGGeometryNode.h"

namespace sksg {

ClipEffect::ClipEffect(sk_sp<RenderNode> child, sk_sp<GeometryNode> clip, bool aa)
    : INHERITED(std::move(child))
    , fClipNode(std::move(clip))
    , fAntiAlias(aa) {
    this->observeInval(fClipNode);
}

ClipEffect::~ClipEffect() {
    this->unobserveInval(fClipNode);
}

void ClipEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    SkAutoCanvasRestore acr(canvas, !fNoop);
    if (!fNoop) {
        fClipNode->clip(canvas, fAntiAlias);
    }

    this->INHERITED::onRender(canvas, ctx);
}

const RenderNode* ClipEffect::onNodeAt(const SkPoint& p) const {
    return fClipNode->contains(p) ? this->INHERITED::onNodeAt(p) : nullptr;
}

SkRect ClipEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    const auto clipBounds = fClipNode->revalidate(ic, ctm);
    auto childBounds = this->INHERITED::onRevalidate(ic, ctm);

    fNoop = fClipNode->asPath().conservativelyContainsRect(childBounds);

    return childBounds.intersect(clipBounds) ? childBounds : SkRect::MakeEmpty();
}

} // namespace sksg
