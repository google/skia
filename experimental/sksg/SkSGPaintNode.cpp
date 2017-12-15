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
    if (this->getInvalState() & kPaint_Inval) {
        fPaint = this->onMakePaint();
        this->clearInvalState(kPaint_Inval);
    }

    return fPaint;
}

} // namespace sksg
