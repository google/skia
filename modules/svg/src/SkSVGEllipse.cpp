/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "modules/svg/include/SkSVGEllipse.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGEllipse::SkSVGEllipse() : INHERITED(SkSVGTag::kEllipse) {}

bool SkSVGEllipse::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setCx(SkSVGAttributeParser::parse<SkSVGLength>("cx", n, v)) ||
           this->setCy(SkSVGAttributeParser::parse<SkSVGLength>("cy", n, v)) ||
           this->setRx(SkSVGAttributeParser::parse<SkSVGLength>("rx", n, v)) ||
           this->setRy(SkSVGAttributeParser::parse<SkSVGLength>("ry", n, v));
}

SkRect SkSVGEllipse::resolve(const SkSVGLengthContext& lctx) const {
    const auto cx = lctx.resolve(fCx, SkSVGLengthContext::LengthType::kHorizontal);
    const auto cy = lctx.resolve(fCy, SkSVGLengthContext::LengthType::kVertical);
    const auto rx = lctx.resolve(fRx, SkSVGLengthContext::LengthType::kHorizontal);
    const auto ry = lctx.resolve(fRy, SkSVGLengthContext::LengthType::kVertical);

    return (rx > 0 && ry > 0)
        ? SkRect::MakeXYWH(cx - rx, cy - ry, rx * 2, ry * 2)
        : SkRect::MakeEmpty();
}

void SkSVGEllipse::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                          const SkPaint& paint, SkPathFillType) const {
    canvas->drawOval(this->resolve(lctx), paint);
}

SkPath SkSVGEllipse::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPath path = SkPath::Oval(this->resolve(ctx.lengthContext()));
    this->mapToParent(&path);

    return path;
}
