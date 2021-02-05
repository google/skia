/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "modules/svg/include/SkSVGRect.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGRect::SkSVGRect() : INHERITED(SkSVGTag::kRect) {}

bool SkSVGRect::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", n, v)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", n, v)) ||
           this->setWidth(SkSVGAttributeParser::parse<SkSVGLength>("width", n, v)) ||
           this->setHeight(SkSVGAttributeParser::parse<SkSVGLength>("height", n, v)) ||
           this->setRx(SkSVGAttributeParser::parse<SkSVGLength>("rx", n, v)) ||
           this->setRy(SkSVGAttributeParser::parse<SkSVGLength>("ry", n, v));
}

SkRRect SkSVGRect::resolve(const SkSVGLengthContext& lctx) const {
    const SkRect rect = lctx.resolveRect(fX, fY, fWidth, fHeight);
    const SkScalar rx = lctx.resolve(fRx, SkSVGLengthContext::LengthType::kHorizontal);
    const SkScalar ry = lctx.resolve(fRy, SkSVGLengthContext::LengthType::kVertical);

    return SkRRect::MakeRectXY(rect, rx ,ry);
}

void SkSVGRect::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                       const SkPaint& paint, SkPathFillType) const {
    canvas->drawRRect(this->resolve(lctx), paint);
}

SkPath SkSVGRect::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPath path = SkPath::RRect(this->resolve(ctx.lengthContext()));

    this->mapToParent(&path);

    return path;
}

SkRect SkSVGRect::onObjectBoundingBox(const SkSVGRenderContext& ctx) const {
    return ctx.lengthContext().resolveRect(fX, fY, fWidth, fHeight);
}
