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

SkPaint PaintNode::makePaint() const {
    SkASSERT(!this->hasInval());

    SkPaint paint;

    paint.setAntiAlias(fAntiAlias);
    paint.setBlendMode(fBlendMode);
    paint.setStyle(fStyle);
    paint.setStrokeWidth(fStrokeWidth);
    paint.setStrokeMiter(fStrokeMiter);
    paint.setStrokeJoin(fStrokeJoin);
    paint.setStrokeCap(fStrokeCap);

    this->onApplyToPaint(&paint);

    // Compose opacity on top of the subclass value.
    paint.setAlpha(SkScalarRoundToInt(paint.getAlpha() * SkTPin<SkScalar>(fOpacity, 0, 1)));

    return paint;
}

SkRect PaintNode::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasInval());

    return SkRect::MakeEmpty();
}

} // namespace sksg
