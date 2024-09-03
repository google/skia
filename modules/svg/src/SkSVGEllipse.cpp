/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGEllipse.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGTypes.h"
#include "modules/svg/src/SkSVGRectPriv.h"

class SkPaint;
enum class SkPathFillType;

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

    // https://www.w3.org/TR/SVG2/shapes.html#EllipseElement
    //
    // An auto value for either rx or ry is converted to a used value, following the rules given
    // above for rectangles (but without any clamping based on width or height).
    const auto [ rx, ry ] = ResolveOptionalRadii(fRx, fRy, lctx);

    // A computed value of zero for either dimension, or a computed value of auto for both
    // dimensions, disables rendering of the element.
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
