/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGShape.h"

SkSVGShape::SkSVGShape(SkSVGTag t) : INHERITED(t) {}

void SkSVGShape::onRender(const SkSVGRenderContext& ctx) const {
    const auto fillType = ctx.presentationContext().fInherited.fFillRule.get()->asFillType();

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
