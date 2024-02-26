/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/sksg/include/SkSGEffectNode.h"

#include "include/private/base/SkAssert.h"
#include "modules/sksg/include/SkSGNode.h"

#include <utility>

class SkCanvas;
class SkMatrix;
struct SkPoint;

namespace sksg {
class InvalidationController;

EffectNode::EffectNode(sk_sp<RenderNode> child, uint32_t inval_traits)
    : INHERITED(inval_traits)
    , fChild(std::move(child)) {
    this->observeInval(fChild);
}

EffectNode::~EffectNode() {
    this->unobserveInval(fChild);
}

void EffectNode::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    fChild->render(canvas, ctx);
}

const RenderNode* EffectNode::onNodeAt(const SkPoint& p) const {
    return fChild->nodeAt(p);
}

SkRect EffectNode::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    return fChild->revalidate(ic, ctm);
}

} // namespace sksg
