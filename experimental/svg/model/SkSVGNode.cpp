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

void SkSVGNode::render(SkCanvas* canvas) const {
    this->render(canvas, SkSVGRenderContext());
}

void SkSVGNode::render(SkCanvas* canvas, const SkSVGRenderContext& ctx) const {
    SkTCopyOnFirstWrite<SkSVGRenderContext> localContext(ctx);
    fPresentationAttributes.applyTo(localContext);

    SkAutoCanvasRestore acr(canvas, false);
    const SkMatrix& m = this->onLocalMatrix();
    if (!m.isIdentity()) {
        canvas->save();
        canvas->concat(m);
    }

    this->onRender(canvas, *localContext);
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
        break;
    }
}

const SkMatrix& SkSVGNode::onLocalMatrix() const {
    return SkMatrix::I();
}
