/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRenderNode.h"

namespace sksg {

RenderNode::RenderNode() : INHERITED(0) {}

void RenderNode::render(SkCanvas* canvas) const {
    SkASSERT(!this->hasInval());
    this->onRender(canvas);
}

} // namespace sksg
