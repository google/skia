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
    , fTransform(SkMatrix::I()) { }


bool SkSVGTransformableNode::onPrepareToRender(SkSVGRenderContext* ctx) const {
    if (!fTransform.value().isIdentity()) {
        ctx->canvas()->save();
        ctx->canvas()->concat(fTransform);
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

void SkSVGTransformableNode::mapToParent(SkPath* path) const {
    if (fTransform.value().isIdentity()) {
        return;
    }

    SkMatrix inv;
    if (!fTransform.value().invert(&inv)) {
        return;
    }

    path->transform(inv);
}
