/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGTransformableNode.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"

SkSVGTransformableNode::SkSVGTransformableNode(SkSVGTag tag)
    : INHERITED(tag)
    , fTransform(SkMatrix::I()) { }


bool SkSVGTransformableNode::onPrepareToRender(SkSVGRenderContext* ctx) const {
    if (!fTransform.value().isIdentity()) {
        ctx->saveOnce();
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
    // transforms the path to parent node coordinates.
    path->transform(fTransform.value());
}
