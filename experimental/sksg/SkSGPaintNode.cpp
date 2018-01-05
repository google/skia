/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGPaintNode.h"

namespace sksg {

// Paint nodes don't generate damage on their own, but via their aggregation ancestor Draw nodes.
PaintNode::PaintNode() : INHERITED(kBubbleDamage_Trait) {}

const SkPaint& PaintNode::makePaint() {
    SkASSERT(!this->hasInval());

    return fPaint;
}

SkRect PaintNode::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasInval());

    fPaint.reset();
    fPaint.setAntiAlias(fAntiAlias);
    fPaint.setStyle(fStyle);
    fPaint.setStrokeWidth(fStrokeWidth);
    fPaint.setStrokeMiter(fStrokeMiter);
    fPaint.setStrokeJoin(fStrokeJoin);
    fPaint.setStrokeCap(fStrokeCap);

    this->onApplyToPaint(&fPaint);

    return SkRect::MakeEmpty();
}

} // namespace sksg
