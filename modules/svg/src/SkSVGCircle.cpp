/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGCircle.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPoint.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGRenderContext.h"

class SkPaint;
enum class SkPathFillType;

SkSVGCircle::SkSVGCircle() : INHERITED(SkSVGTag::kCircle) {}

bool SkSVGCircle::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setCx(SkSVGAttributeParser::parse<SkSVGLength>("cx", n, v)) ||
           this->setCy(SkSVGAttributeParser::parse<SkSVGLength>("cy", n, v)) ||
           this->setR(SkSVGAttributeParser::parse<SkSVGLength>("r", n, v));
}

std::tuple<SkPoint, SkScalar> SkSVGCircle::resolve(const SkSVGLengthContext& lctx) const {
    const auto cx = lctx.resolve(fCx, SkSVGLengthContext::LengthType::kHorizontal);
    const auto cy = lctx.resolve(fCy, SkSVGLengthContext::LengthType::kVertical);
    const auto  r = lctx.resolve(fR , SkSVGLengthContext::LengthType::kOther);

    return std::make_tuple(SkPoint::Make(cx, cy), r);
}
void SkSVGCircle::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                         const SkPaint& paint, SkPathFillType) const {
    SkPoint pos;
    SkScalar r;
    std::tie(pos, r) = this->resolve(lctx);

    if (r > 0) {
        canvas->drawCircle(pos.x(), pos.y(), r, paint);
    }
}

SkPath SkSVGCircle::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPoint pos;
    SkScalar r;
    std::tie(pos, r) = this->resolve(ctx.lengthContext());

    SkPath path = SkPath::Circle(pos.x(), pos.y(), r);
    this->mapToParent(&path);

    return path;
}

SkRect SkSVGCircle::onTransformableObjectBoundingBox(const SkSVGRenderContext& ctx) const {
    const auto [pos, r] = this->resolve(ctx.lengthContext());
    return SkRect::MakeXYWH(pos.fX - r, pos.fY - r, 2 * r, 2 * r);
}
