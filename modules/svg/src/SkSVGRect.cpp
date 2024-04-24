/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <tuple>

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "modules/svg/include/SkSVGRect.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/src/SkSVGRectPriv.h"

std::tuple<float, float> ResolveOptionalRadii(const SkTLazy<SkSVGLength>& opt_rx,
                                              const SkTLazy<SkSVGLength>& opt_ry,
                                              const SkSVGLengthContext& lctx) {
    // https://www.w3.org/TR/SVG2/shapes.html#RectElement
    //
    // The used values for rx and ry are determined from the computed values by following these
    // steps in order:
    //
    // 1. If both rx and ry have a computed value of auto (since auto is the initial value for both
    //    properties, this will also occur if neither are specified by the author or if all
    //    author-supplied values are invalid), then the used value of both rx and ry is 0.
    //    (This will result in square corners.)
    // 2. Otherwise, convert specified values to absolute values as follows:
    //     1. If rx is set to a length value or a percentage, but ry is auto, calculate an absolute
    //        length equivalent for rx, resolving percentages against the used width of the
    //        rectangle; the absolute value for ry is the same.
    //     2. If ry is set to a length value or a percentage, but rx is auto, calculate the absolute
    //        length equivalent for ry, resolving percentages against the used height of the
    //        rectangle; the absolute value for rx is the same.
    //     3. If both rx and ry were set to lengths or percentages, absolute values are generated
    //        individually, resolving rx percentages against the used width, and resolving ry
    //        percentages against the used height.
    const float rx = opt_rx.isValid()
        ? lctx.resolve(*opt_rx, SkSVGLengthContext::LengthType::kHorizontal)
        : 0;
    const float ry = opt_ry.isValid()
        ? lctx.resolve(*opt_ry, SkSVGLengthContext::LengthType::kVertical)
        : 0;

    return std::make_tuple(opt_rx.isValid() ? rx : ry,
                           opt_ry.isValid() ? ry : rx);
}

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
    const auto rect = lctx.resolveRect(fX, fY, fWidth, fHeight);
    const auto [ rx, ry ] = ResolveOptionalRadii(fRx, fRy, lctx);

    // https://www.w3.org/TR/SVG2/shapes.html#RectElement
    // ...
    // 3. Finally, apply clamping to generate the used values:
    //     1. If the absolute rx (after the above steps) is greater than half of the used width,
    //        then the used value of rx is half of the used width.
    //     2. If the absolute ry (after the above steps) is greater than half of the used height,
    //        then the used value of ry is half of the used height.
    //     3. Otherwise, the used values of rx and ry are the absolute values computed previously.

    return SkRRect::MakeRectXY(rect,
                               std::min(rx, rect.width() / 2),
                               std::min(ry, rect.height() / 2));
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
