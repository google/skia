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
    fPresentationAttributes.applyTo(ctx);
    return true;
}

void SkSVGNode::setAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    this->onSetAttribute(attr, v);
}

void SkSVGNode::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kFill:
        if (const SkSVGColorValue* color = v.as<SkSVGColorValue>()) {
            fPresentationAttributes.setFill(*color);
        }
        break;
    case SkSVGAttribute::kStroke:
        if (const SkSVGColorValue* color = v.as<SkSVGColorValue>()) {
            fPresentationAttributes.setStroke(*color);
        }
        break;
    default:
        SkDebugf("attribute ID <%d> ignored for node <%d>\n", attr, fTag);
        break;
    }
}
