/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkSVGNode.h"
#include "SkSVGRenderContext.h"
#include "SkSVGValue.h"
#include "SkTLazy.h"

SkSVGNode::SkSVGNode(SkSVGTag t) : fTag(t) { }

SkSVGNode::~SkSVGNode() { }

void SkSVGNode::render(const SkSVGRenderContext& ctx) const {
    SkSVGRenderContext localContext(ctx);

    if (this->onPrepareToRender(&localContext)) {
        this->onRender(localContext);
    }
}

bool SkSVGNode::onPrepareToRender(SkSVGRenderContext* ctx) const {
    ctx->applyPresentationAttributes(fPresentationAttributes);
    return true;
}

void SkSVGNode::setAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    this->onSetAttribute(attr, v);
}

void SkSVGNode::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kFill:
        if (const SkSVGPaintValue* paint = v.as<SkSVGPaintValue>()) {
            fPresentationAttributes.fFill.set(*paint);
        }
        break;
    case SkSVGAttribute::kFillOpacity:
        if (const SkSVGNumberValue* opacity = v.as<SkSVGNumberValue>()) {
            fPresentationAttributes.fFillOpacity.set(
                SkSVGNumberType(SkTPin<SkScalar>((*opacity)->value(), 0, 1)));
        }
        break;
    case SkSVGAttribute::kStroke:
        if (const SkSVGPaintValue* paint = v.as<SkSVGPaintValue>()) {
            fPresentationAttributes.fStroke.set(*paint);
        }
        break;
    case SkSVGAttribute::kStrokeOpacity:
        if (const SkSVGNumberValue* opacity = v.as<SkSVGNumberValue>()) {
            fPresentationAttributes.fStrokeOpacity.set(
                SkSVGNumberType(SkTPin<SkScalar>((*opacity)->value(), 0, 1)));
        }
        break;
    case SkSVGAttribute::kStrokeLineCap:
        if (const SkSVGLineCapValue* lineCap = v.as<SkSVGLineCapValue>()) {
            fPresentationAttributes.fStrokeLineCap.set(*lineCap);
        }
        break;
    case SkSVGAttribute::kStrokeLineJoin:
        if (const SkSVGLineJoinValue* lineJoin = v.as<SkSVGLineJoinValue>()) {
            fPresentationAttributes.fStrokeLineJoin.set(*lineJoin);
        }
        break;
    case SkSVGAttribute::kStrokeWidth:
        if (const SkSVGLengthValue* strokeWidth = v.as<SkSVGLengthValue>()) {
            fPresentationAttributes.fStrokeWidth.set(*strokeWidth);
        }
        break;
    default:
        SkDebugf("attribute ID <%d> ignored for node <%d>\n", attr, fTag);
        break;
    }
}
