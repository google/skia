/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGTransformableNode.h"
#include "modules/svg/include/SkSVGValue.h"

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
    path->transform(fTransform);
}

void SkSVGTransformableNode::mapToParent(SkRect* rect) const {
    *rect = fTransform.mapRect(*rect);
}
