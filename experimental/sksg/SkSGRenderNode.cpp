/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRenderNode.h"

namespace sksg {

RenderNode::RenderNode()
    : fBounds(SkRect::MakeLTRB(SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax)) {}

void RenderNode::render(SkCanvas* canvas) const {
    this->onRender(canvas);
}

const SkRect& RenderNode::getBounds() {
    if (this->getInvalState() & kGeometry_Inval) {
        fBounds = this->onComputeBounds();
        this->clearInvalState(kGeometry_Inval);
    }

    return fBounds;
}

} // namespace sksg
