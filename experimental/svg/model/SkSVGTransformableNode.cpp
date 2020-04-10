/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGTransformableNode.h"
#include "include/core/SkCanvas.h"

SkSVGTransformableNode::SkSVGTransformableNode(SkSVGTag tag)
    : INHERITED(tag)
    , fTransform(SkMatrix::I()) { }


bool SkSVGTransformableNode::onPrepareToRender(SkSVGRenderContext* ctx) const {
    if (!fTransform.isIdentity()) {
        ctx->saveOnce();
        ctx->canvas()->concat(fTransform);
    }

    return this->INHERITED::onPrepareToRender(ctx);
}

void SkSVGTransformableNode::onSetAttribute(SkSVGAttribute attr, const SkSVGAttributeValue& v) {
    switch (attr) {
    case SkSVGAttribute::kTransform:
        if (const auto* transform = std::get_if<SkSVGTransformType>(&v)) {
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
    path->transform(fTransform);
}
