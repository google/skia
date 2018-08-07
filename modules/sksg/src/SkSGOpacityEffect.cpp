/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGOpacityEffect.h"

#include "SkCanvas.h"
#include "SkSGRenderContext.h"

#include <math.h>

namespace sksg {

OpacityEffect::OpacityEffect(sk_sp<RenderNode> child, float opacity)
    : INHERITED(std::move(child))
    , fOpacity(opacity) {}

void OpacityEffect::onRender(const RenderContext& ctx) const {
    // opacity <= 0 disables rendering
    if (fOpacity <= 0)
        return;

    auto local_ctx = ctx.makeLocal();
    local_ctx.applyOpacity(fOpacity);

//    SkAutoCanvasRestore acr(ctx.canvas(), false);
//    if (fOpacity < 1) {
//        ctx.canvas()->saveLayerAlpha(&this->bounds(), roundf(fOpacity * 255));
//    }

    this->INHERITED::onRender(local_ctx);
}

SkRect OpacityEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // opacity <= 0 disables rendering AND revalidation for the sub-DAG
    return fOpacity > 0 ? this->INHERITED::onRevalidate(ic, ctm) : SkRect::MakeEmpty();
}

} // namespace sksg
