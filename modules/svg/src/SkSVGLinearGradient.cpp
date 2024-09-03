/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGLinearGradient.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPoint.h"
#include "include/effects/SkGradientShader.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGRenderContext.h"

class SkMatrix;
class SkShader;
enum class SkTileMode;

SkSVGLinearGradient::SkSVGLinearGradient() : INHERITED(SkSVGTag::kLinearGradient) {}

bool SkSVGLinearGradient::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setX1(SkSVGAttributeParser::parse<SkSVGLength>("x1", name, value)) ||
           this->setY1(SkSVGAttributeParser::parse<SkSVGLength>("y1", name, value)) ||
           this->setX2(SkSVGAttributeParser::parse<SkSVGLength>("x2", name, value)) ||
           this->setY2(SkSVGAttributeParser::parse<SkSVGLength>("y2", name, value));
}

sk_sp<SkShader> SkSVGLinearGradient::onMakeShader(const SkSVGRenderContext& ctx,
                                                  const SkColor4f* colors, const SkScalar* pos,
                                                  int count, SkTileMode tm,
                                                  const SkMatrix& localMatrix) const {
    const SkSVGLengthContext lctx =
            this->getGradientUnits().type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox
                    ? SkSVGLengthContext({1, 1})
                    : ctx.lengthContext();

    const auto x1 = lctx.resolve(fX1, SkSVGLengthContext::LengthType::kHorizontal);
    const auto y1 = lctx.resolve(fY1, SkSVGLengthContext::LengthType::kVertical);
    const auto x2 = lctx.resolve(fX2, SkSVGLengthContext::LengthType::kHorizontal);
    const auto y2 = lctx.resolve(fY2, SkSVGLengthContext::LengthType::kVertical);

    const SkPoint pts[2] = { {x1, y1}, {x2, y2}};

    return SkGradientShader::MakeLinear(pts, colors, nullptr, pos, count, tm, 0, &localMatrix);
}
