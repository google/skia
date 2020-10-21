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

#include <vector>

SkSVGLinearGradient::SkSVGLinearGradient() : INHERITED(SkSVGTag::kLinearGradient) {}

#define MAKEPCT(len)                                                          \
    if (len.unit() != SkSVGLength::Unit::kPercentage) {                       \
        SkASSERT(len.unit() == SkSVGLength::Unit::kNumber);                   \
        len = SkSVGLength(len.value() * 100, SkSVGLength::Unit::kPercentage); \
    }

void SkSVGLinearGradient::setX1(const SkSVGLength& x1) {
    fX1 = x1;
    MAKEPCT(fX1);
}

void SkSVGLinearGradient::setY1(const SkSVGLength& y1) {
    fY1 = y1;
    MAKEPCT(fY1);
}

void SkSVGLinearGradient::setX2(const SkSVGLength& x2) {
    fX2 = x2;
    MAKEPCT(fX2);
}

void SkSVGLinearGradient::setY2(const SkSVGLength& y2) {
    fY2 = y2;
    MAKEPCT(fY2);
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
    const auto& lctx = ctx.lengthContext();
    const auto x1 = lctx.resolve(fX1, SkSVGLengthContext::LengthType::kHorizontal);
    const auto y1 = lctx.resolve(fY1, SkSVGLengthContext::LengthType::kVertical);
    const auto x2 = lctx.resolve(fX2, SkSVGLengthContext::LengthType::kHorizontal);
    const auto y2 = lctx.resolve(fY2, SkSVGLengthContext::LengthType::kVertical);

    SkDebugf("lin gradient x1=%f, y1=%f, x2=%f, y2=%f\n", x1, y1, x2, y2);

    const SkPoint pts[2] = { {x1, y1}, {x2, y2}};

    // std::vector<SkScalar> resolvedPos(count, 0);
    // for (int i = 0; i < count; i++) {
    //     const SkSVGLength len = SkSVGLength(pos[i], SkSVGLength::Unit::kNumber);
    //     resolvedPos[i] = lctx.resolve(len, SkSVGLengthContext::LengthType::kHorizontal);
    //     SkDebugf("lin gradient pos[%d] = %f, resolvedPos[%d] = %f\n", i, pos[i], i, resolvedPos[i]);
    // }

    return SkGradientShader::MakeLinear(pts, colors, pos, count, tm, 0, &localMatrix);
}
