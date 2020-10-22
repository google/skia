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

void SkSVGRadialGradient::setCx(const SkSVGLength& cx) {
    fCx = cx;
}

void SkSVGRadialGradient::setCy(const SkSVGLength& cy) {
    fCy = cy;
}

void SkSVGRadialGradient::setR(const SkSVGLength& r) {
    fR = r;
}

void SkSVGRadialGradient::setFx(const SkSVGLength& fx) {
    fFx.set(fx);
}

void SkSVGRadialGradient::setFy(const SkSVGLength& fy) {
    fFy.set(fy);
}

void SkSVGRadialGradient::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kCx:
        if (const auto* cx = v.as<SkSVGLengthValue>()) {
            this->setCx(*cx);
        }
        break;
    case SkSVGAttribute::kCy:
        if (const auto* cy = v.as<SkSVGLengthValue>()) {
            this->setCy(*cy);
        }
        break;
    case SkSVGAttribute::kR:
        if (const auto* r = v.as<SkSVGLengthValue>()) {
            this->setR(*r);
        }
        break;
    case SkSVGAttribute::kFx:
        if (const auto* fx = v.as<SkSVGLengthValue>()) {
            this->setFx(*fx);
        }
        break;
    case SkSVGAttribute::kFy:
        if (const auto* fy = v.as<SkSVGLengthValue>()) {
            this->setFy(*fy);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

sk_sp<SkShader> SkSVGRadialGradient::onMakeShader(const SkSVGRenderContext& ctx,
                                                  const SkColor* colors, const SkScalar* pos,
                                                  int count, SkTileMode tm,
                                                  const SkMatrix& m) const {
    const SkSVGLengthContext lctx = lengthContextForGradientUnits(ctx);
    const auto r =
            lctx.resolve(convertLengthForGradientUnits(fR), SkSVGLengthContext::LengthType::kOther);
    const auto center = SkPoint::Make(lctx.resolve(convertLengthForGradientUnits(fCx),
                                                   SkSVGLengthContext::LengthType::kHorizontal),
                                      lctx.resolve(convertLengthForGradientUnits(fCy),
                                                   SkSVGLengthContext::LengthType::kVertical));
    const auto focal =
            SkPoint::Make(fFx.isValid() ? lctx.resolve(convertLengthForGradientUnits(*fFx),
                                                       SkSVGLengthContext::LengthType::kHorizontal)
                                        : center.x(),
                          fFy.isValid() ? lctx.resolve(convertLengthForGradientUnits(*fFy),
                                                       SkSVGLengthContext::LengthType::kVertical)
                                        : center.y());

    SkMatrix localMatrix;
    localMatrix.preTranslate(center.fX, center.fY);

    if (fGradientUnits.type() == SkSVGGradientUnits::Type::kObjectBoundingBox) {
        // TODO: Handle r == 0 which has a specific meaning according to the spec
        SkASSERT(r != 0);
        const SkRect viewPort = lctx.viewPort();
        localMatrix.preScale(viewPort.width() / r, viewPort.height() / r);
    }

    localMatrix.preConcat(m);

    return center == focal ? SkGradientShader::MakeRadial({0, 0}, r, colors, pos, count, tm, 0,
                                                          &localMatrix)
                           : SkGradientShader::MakeTwoPointConical(focal, 0, {0, 0}, r, colors, pos,
                                                                   count, tm, 0, &localMatrix);
}
