/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkSVGRenderContext.h"
#include "SkSVGTransformableNode.h"
#include "SkSVGValue.h"

SkSVGTransformableNode::SkSVGTransformableNode(SkSVGTag tag)
    : INHERITED(tag)
    , fMatrix(SkMatrix::I()) { }


bool SkSVGTransformableNode::onPrepareToRender(SkSVGRenderContext* ctx) const {
    if (!fMatrix.isIdentity()) {
        ctx->canvas()->save();
        ctx->canvas()->concat(fMatrix);
    }

    return this->INHERITED::onPrepareToRender(ctx);
}

void SkSVGTransformableNode::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kTransform:
        if (const auto* transform = v.as<SkSVGTransformValue>()) {
            this->setTransform(*transform);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
        break;
    }
}
