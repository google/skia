/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkGradientShader.h"
#include "modules/svg/include/SkSVGRadialGradient.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGRadialGradient::SkSVGRadialGradient() : INHERITED(SkSVGTag::kRadialGradient) {}

bool SkSVGRadialGradient::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setCx(SkSVGAttributeParser::parse<SkSVGLength>("cx", name, value)) ||
           this->setCy(SkSVGAttributeParser::parse<SkSVGLength>("cy", name, value)) ||
           this->setR(SkSVGAttributeParser::parse<SkSVGLength>("r", name, value)) ||
           this->setFx(SkSVGAttributeParser::parse<SkSVGLength>("fx", name, value)) ||
           this->setFy(SkSVGAttributeParser::parse<SkSVGLength>("fy", name, value));
}

sk_sp<SkShader> SkSVGRadialGradient::onMakeShader(const SkSVGRenderContext& ctx,
                                                  const SkColor* colors, const SkScalar* pos,
                                                  int count, SkTileMode tm,
                                                  const SkMatrix& m) const {
    const SkSVGLengthContext lctx =
            this->getGradientUnits().type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox
                    ? SkSVGLengthContext({1, 1})
                    : ctx.lengthContext();

    const auto      r = lctx.resolve(fR , SkSVGLengthContext::LengthType::kOther);
    const auto center = SkPoint::Make(
            lctx.resolve(fCx, SkSVGLengthContext::LengthType::kHorizontal),
            lctx.resolve(fCy, SkSVGLengthContext::LengthType::kVertical));
    const auto  focal = SkPoint::Make(
        fFx.isValid() ? lctx.resolve(*fFx, SkSVGLengthContext::LengthType::kHorizontal)
                      : center.x(),
        fFy.isValid() ? lctx.resolve(*fFy, SkSVGLengthContext::LengthType::kVertical)
                      : center.y());

    if (r == 0) {
        const auto last_color = count > 0 ? colors[count - 1] : SK_ColorBLACK;
        return SkShaders::Color(last_color);
    }

    return center == focal
        ? SkGradientShader::MakeRadial(center, r, colors, pos, count, tm, 0, &m)
        : SkGradientShader::MakeTwoPointConical(focal, 0, center, r, colors, pos, count, tm, 0, &m);
}
