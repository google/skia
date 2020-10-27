/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkGradientShader.h"
#include "modules/svg/include/SkSVGLinearGradient.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGLinearGradient::SkSVGLinearGradient() : INHERITED(SkSVGTag::kLinearGradient) {}

void SkSVGLinearGradient::setX1(const SkSVGLength& x1) {
    fX1 = x1;
}

void SkSVGLinearGradient::setY1(const SkSVGLength& y1) {
    fY1 = y1;
}

void SkSVGLinearGradient::setX2(const SkSVGLength& x2) {
    fX2 = x2;
}

void SkSVGLinearGradient::setY2(const SkSVGLength& y2) {
    fY2 = y2;
}

void SkSVGLinearGradient::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kX1:
        if (const auto* x1 = v.as<SkSVGLengthValue>()) {
            this->setX1(*x1);
        }
        break;
    case SkSVGAttribute::kY1:
        if (const auto* y1 = v.as<SkSVGLengthValue>()) {
            this->setY1(*y1);
        }
        break;
    case SkSVGAttribute::kX2:
        if (const auto* x2 = v.as<SkSVGLengthValue>()) {
            this->setX2(*x2);
        }
        break;
    case SkSVGAttribute::kY2:
        if (const auto* y2 = v.as<SkSVGLengthValue>()) {
            this->setY2(*y2);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

sk_sp<SkShader> SkSVGLinearGradient::onMakeShader(const SkSVGRenderContext& ctx,
                                                  const SkColor* colors, const SkScalar* pos,
                                                  int count, SkTileMode tm,
                                                  const SkMatrix& localMatrix) const {
    const SkSVGLengthContext lctx =
            fGradientUnits.type() == SkSVGGradientUnits::Type::kObjectBoundingBox
                    ? SkSVGLengthContext({1, 1})
                    : ctx.lengthContext();

    SkScalar x1 = lctx.resolve(convertLengthForGradientUnits(fX1),
                               SkSVGLengthContext::LengthType::kHorizontal);
    SkScalar y1 = lctx.resolve(convertLengthForGradientUnits(fY1),
                               SkSVGLengthContext::LengthType::kVertical);
    SkScalar x2 = lctx.resolve(convertLengthForGradientUnits(fX2),
                               SkSVGLengthContext::LengthType::kHorizontal);
    SkScalar y2 = lctx.resolve(convertLengthForGradientUnits(fY2),
                               SkSVGLengthContext::LengthType::kVertical);

    if (fGradientUnits.type() == SkSVGGradientUnits::Type::kObjectBoundingBox) {
        SkASSERT(ctx.node());
        const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
        x1 = objBounds.fLeft + objBounds.width() * x1;
        y1 = objBounds.fTop + objBounds.height() * y1;
        x2 = objBounds.fLeft + objBounds.width() * x2;
        y2 = objBounds.fTop + objBounds.height() * y2;
    }

    const SkPoint pts[2] = { {x1, y1}, {x2, y2}};

    return SkGradientShader::MakeLinear(pts, colors, pos, count, tm, 0, &localMatrix);
}
