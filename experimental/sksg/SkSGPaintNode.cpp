/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGPaintNode.h"

namespace sksg {

PaintNode::PaintNode() {}

const SkPaint& PaintNode::makePaint() {
    SkASSERT(!this->isInvalidated());

    return fPaint;
}

void PaintNode::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->isInvalidated());

    fPaint.reset();
    fPaint.setAntiAlias(fAntiAlias);
    fPaint.setStyle(fStyle);
    fPaint.setStrokeWidth(fStrokeWidth);
    fPaint.setStrokeMiter(fStrokeMiter);
    fPaint.setStrokeJoin(fStrokeJoin);
    fPaint.setStrokeCap(fStrokeCap);

    this->onApplyToPaint(&fPaint);
}

} // namespace sksg
