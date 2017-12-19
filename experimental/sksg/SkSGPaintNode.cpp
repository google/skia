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

    fPaint = this->onMakePaint();
}

} // namespace sksg
