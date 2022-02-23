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
    const auto rect = lctx.resolveRect(fX, fY, fWidth, fHeight);

    // https://www.w3.org/TR/SVG11/shapes.html#RectElementRXAttribute:
    //
    //   - Let rx and ry be length values.
    //   - If neither ‘rx’ nor ‘ry’ are properly specified, then set both rx and ry to 0.
    //   - Otherwise, if a properly specified value is provided for ‘rx’, but not for ‘ry’,
    //     then set both rx and ry to the value of ‘rx’.
    //   - Otherwise, if a properly specified value is provided for ‘ry’, but not for ‘rx’,
    //     then set both rx and ry to the value of ‘ry’.
    //   - Otherwise, both ‘rx’ and ‘ry’ were specified properly. Set rx to the value of ‘rx’
    //     and ry to the value of ‘ry’.
    //   - If rx is greater than half of ‘width’, then set rx to half of ‘width’.
    //   - If ry is greater than half of ‘height’, then set ry to half of ‘height’.
    //   - The effective values of ‘rx’ and ‘ry’ are rx and ry, respectively.
    //
    auto radii = [this]() {
        return fRx.isValid()
                ? fRy.isValid()
                    ? std::make_tuple(*fRx, *fRy)
                    : std::make_tuple(*fRx, *fRx)
                : fRy.isValid()
                    ? std::make_tuple(*fRy, *fRy)
                    : std::make_tuple(SkSVGLength(0), SkSVGLength(0));
    };

    const auto [ rxlen, rylen ] = radii();
    const auto rx = std::min(lctx.resolve(rxlen, SkSVGLengthContext::LengthType::kHorizontal),
                             rect.width() / 2),
               ry = std::min(lctx.resolve(rylen, SkSVGLengthContext::LengthType::kVertical),
                             rect.height() / 2);

    return SkRRect::MakeRectXY(rect, rx, ry);
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
