/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGRenderContext.h"
#include "SkSVGShape.h"

SkSVGShape::SkSVGShape(SkSVGTag t) : INHERITED(t) {}

void SkSVGShape::onRender(SkCanvas* canvas, const SkSVGRenderContext& ctx) const {
    if (const SkPaint* fillPaint = ctx.fillPaint()) {
        this->onDraw(canvas, ctx.lengthContext(), *fillPaint);
    }

    if (const SkPaint* strokePaint = ctx.strokePaint()) {
        this->onDraw(canvas, ctx.lengthContext(), *strokePaint);
    }
}

void SkSVGShape::appendChild(sk_sp<SkSVGNode>) {
    SkDebugf("cannot append child nodes to an SVG shape.\n");
}
