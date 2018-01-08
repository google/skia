/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGOpacityEffect.h"

#include "SkCanvas.h"

#include <math.h>

namespace sksg {

OpacityEffect::OpacityEffect(sk_sp<RenderNode> child, float opacity)
    : INHERITED(std::move(child))
    , fOpacity(opacity) {}

void OpacityEffect::onRender(SkCanvas* canvas) const {
    // opacity <= 0 disables rendering
    if (fOpacity <= 0)
        return;

    // TODO: we could avoid savelayer if there is no more than one drawing primitive
    //       in the sub-DAG.
    SkAutoCanvasRestore acr(canvas, false);
    if (fOpacity < 1) {
        canvas->saveLayerAlpha(&this->bounds(), roundf(fOpacity * 255));
    }

    this->INHERITED::onRender(canvas);
}

SkRect OpacityEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // opacity <= 0 disables rendering AND revalidation for the sub-DAG
    return fOpacity > 0 ? this->INHERITED::onRevalidate(ic, ctm) : SkRect::MakeEmpty();
}

} // namespace sksg
