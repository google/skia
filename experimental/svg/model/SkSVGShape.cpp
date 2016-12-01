/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGRenderContext.h"
#include "SkSVGShape.h"

SkSVGShape::SkSVGShape(SkSVGTag t) : INHERITED(t) {}

void SkSVGShape::onRender(const SkSVGRenderContext& ctx) const {
    const SkPath::FillType fillType =
        FillRuleToFillType(*ctx.presentationContext().fInherited.fFillRule.get());

    // TODO: this approach forces duplicate geometry resolution in onDraw(); refactor to avoid.
    if (const SkPaint* fillPaint = ctx.fillPaint()) {
        this->onDraw(ctx.canvas(), ctx.lengthContext(), *fillPaint, fillType);
    }

    if (const SkPaint* strokePaint = ctx.strokePaint()) {
        this->onDraw(ctx.canvas(), ctx.lengthContext(), *strokePaint, fillType);
    }
}

void SkSVGShape::appendChild(sk_sp<SkSVGNode>) {
    SkDebugf("cannot append child nodes to an SVG shape.\n");
}

SkPath::FillType SkSVGShape::FillRuleToFillType(const SkSVGFillRule& fillRule) {
    switch (fillRule.type()) {
    case SkSVGFillRule::Type::kNonZero:
        return SkPath::kWinding_FillType;
    case SkSVGFillRule::Type::kEvenOdd:
        return SkPath::kEvenOdd_FillType;
    default:
        SkASSERT(false);
        return SkPath::kWinding_FillType;
    }
}
