/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGClipEffect.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/private/base/SkAssert.h"
#include "modules/sksg/include/SkSGGeometryNode.h"
#include "modules/sksg/include/SkSGNode.h"

class SkMatrix;
struct SkPoint;

namespace sksg {
class InvalidationController;

ClipEffect::ClipEffect(sk_sp<RenderNode> child, sk_sp<GeometryNode> clip, bool aa, bool force_clip)
    : INHERITED(std::move(child))
    , fClipNode(std::move(clip))
    , fAntiAlias(aa)
    , fForceClip(force_clip) {
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

    // When the child node is fully contained within the clip, it is usually safe to elide.
    // An exception is clip-dependent sizing for saveLayer buffers, where the clip is always
    // significant.  For those cases, we provide a mechanism to disable elision.
    fNoop = !fForceClip && fClipNode->asPath().conservativelyContainsRect(childBounds);

    return childBounds.intersect(clipBounds) ? childBounds : SkRect::MakeEmpty();
}

} // namespace sksg
