/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGTransformableNode.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "modules/svg/include/SkSVGAttribute.h"
#include "modules/svg/include/SkSVGRenderContext.h"
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

SkRect SkSVGTransformableNode::onTransformableObjectBoundingBox(const SkSVGRenderContext&) const {
    return SkRect::MakeEmpty();
}

SkRect SkSVGTransformableNode::onObjectBoundingBox(const SkSVGRenderContext& ctx) const {
    SkRect obb = this->onTransformableObjectBoundingBox(ctx);

    if (ctx.currentOBBScope().fNode != this && !fTransform.isIdentity()) {
        this->mapToParent(&obb);
    }
    return obb;
}

