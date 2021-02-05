/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "modules/svg/include/SkSVGLine.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGLine::SkSVGLine() : INHERITED(SkSVGTag::kLine) {}

bool SkSVGLine::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setX1(SkSVGAttributeParser::parse<SkSVGLength>("x1", n, v)) ||
           this->setY1(SkSVGAttributeParser::parse<SkSVGLength>("y1", n, v)) ||
           this->setX2(SkSVGAttributeParser::parse<SkSVGLength>("x2", n, v)) ||
           this->setY2(SkSVGAttributeParser::parse<SkSVGLength>("y2", n, v));
}

std::tuple<SkPoint, SkPoint> SkSVGLine::resolve(const SkSVGLengthContext& lctx) const {
    return std::make_tuple(
        SkPoint::Make(lctx.resolve(fX1, SkSVGLengthContext::LengthType::kHorizontal),
                      lctx.resolve(fY1, SkSVGLengthContext::LengthType::kVertical)),
        SkPoint::Make(lctx.resolve(fX2, SkSVGLengthContext::LengthType::kHorizontal),
                      lctx.resolve(fY2, SkSVGLengthContext::LengthType::kVertical)));
}

void SkSVGLine::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                       const SkPaint& paint, SkPathFillType) const {
    SkPoint p0, p1;
    std::tie(p0, p1) = this->resolve(lctx);

    canvas->drawLine(p0, p1, paint);
}

SkPath SkSVGLine::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPoint p0, p1;
    std::tie(p0, p1) = this->resolve(ctx.lengthContext());

    SkPath path = SkPath::Line(p0, p1);
    this->mapToParent(&path);

    return path;
}
