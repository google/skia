/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGShape.h"

#include "include/core/SkPaint.h"  // IWYU pragma: keep
#include "include/private/base/SkDebug.h"
#include "modules/svg/include/SkSVGAttribute.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGTypes.h"
#include "src/base/SkTLazy.h"

class SkSVGNode;
enum class SkSVGTag;

SkSVGShape::SkSVGShape(SkSVGTag t) : INHERITED(t) {}

void SkSVGShape::onRender(const SkSVGRenderContext& ctx) const {
    const auto fillType = ctx.presentationContext().fInherited.fFillRule->asFillType();

    const auto fillPaint = ctx.fillPaint(),
             strokePaint = ctx.strokePaint();

    // TODO: this approach forces duplicate geometry resolution in onDraw(); refactor to avoid.
    if (fillPaint.isValid()) {
        this->onDraw(ctx.canvas(), ctx.lengthContext(), *fillPaint, fillType);
    }

    if (strokePaint.isValid()) {
        this->onDraw(ctx.canvas(), ctx.lengthContext(), *strokePaint, fillType);
    }
}

void SkSVGShape::appendChild(sk_sp<SkSVGNode>) {
    SkDEBUGF("cannot append child nodes to an SVG shape.\n");
}
