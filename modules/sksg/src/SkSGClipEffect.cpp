/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGClipEffect.h"

#include "SkCanvas.h"
#include "SkPath.h"
#include "SkSGGeometryNode.h"
#include "SkSGRenderContext.h"

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

void ClipEffect::onRender(const RenderContext& ctx) const {
    if (this->bounds().isEmpty())
        return;

    SkAutoCanvasRestore acr(ctx.canvas(), !fNoop);
    if (!fNoop) {
        fClipNode->clip(ctx.canvas(), fAntiAlias);
    }

    this->INHERITED::onRender(ctx);
}

SkRect ClipEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    const auto clipBounds = fClipNode->revalidate(ic, ctm);
    auto childBounds = this->INHERITED::onRevalidate(ic, ctm);

    fNoop = fClipNode->asPath().conservativelyContainsRect(childBounds);

    return childBounds.intersect(clipBounds) ? childBounds : SkRect::MakeEmpty();
}

} // namespace sksg
