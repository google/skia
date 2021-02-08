/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGSVG.h"
#include "modules/svg/include/SkSVGValue.h"

bool SkSVGSVG::onPrepareToRender(SkSVGRenderContext* ctx) const {
    // x/y are ignored for outermost svg elements
    const auto x = fType == Type::kInner ? fX : SkSVGLength(0);
    const auto y = fType == Type::kInner ? fY : SkSVGLength(0);

    auto viewPortRect  = ctx->lengthContext().resolveRect(x, y, fWidth, fHeight);
    auto contentMatrix = SkMatrix::Translate(viewPortRect.x(), viewPortRect.y());
    auto viewPort      = SkSize::Make(viewPortRect.width(), viewPortRect.height());

    if (fViewBox.isValid()) {
        const SkRect& viewBox = *fViewBox;

        // An empty viewbox disables rendering.
        if (viewBox.isEmpty()) {
            return false;
        }

        // A viewBox overrides the intrinsic viewport.
        viewPort = SkSize::Make(viewBox.width(), viewBox.height());

        contentMatrix.preConcat(ComputeViewboxMatrix(viewBox, viewPortRect, fPreserveAspectRatio));
    }

    if (!contentMatrix.isIdentity()) {
        ctx->saveOnce();
        ctx->canvas()->concat(contentMatrix);
    }

    if (viewPort != ctx->lengthContext().viewPort()) {
        ctx->writableLengthContext()->setViewPort(viewPort);
    }

    return this->INHERITED::onPrepareToRender(ctx);
}

void SkSVGSVG::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kX:
        if (const auto* x = v.as<SkSVGLengthValue>()) {
            this->setX(*x);
        }
        break;
    case SkSVGAttribute::kY:
        if (const auto* y = v.as<SkSVGLengthValue>()) {
            this->setY(*y);
        }
        break;
    case SkSVGAttribute::kWidth:
        if (const auto* w = v.as<SkSVGLengthValue>()) {
            this->setWidth(*w);
        }
        break;
    case SkSVGAttribute::kHeight:
        if (const auto* h = v.as<SkSVGLengthValue>()) {
            this->setHeight(*h);
        }
        break;
    case SkSVGAttribute::kViewBox:
        if (const auto* vb = v.as<SkSVGViewBoxValue>()) {
            this->setViewBox(*vb);
        }
        break;
    case SkSVGAttribute::kPreserveAspectRatio:
        if (const auto* par = v.as<SkSVGPreserveAspectRatioValue>()) {
            this->setPreserveAspectRatio(*par);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

// https://www.w3.org/TR/SVG11/coords.html#IntrinsicSizing
SkSize SkSVGSVG::intrinsicSize(const SkSVGLengthContext& lctx) const {
    // Percentage values do not provide an intrinsic size.
    if (fWidth.unit() == SkSVGLength::Unit::kPercentage ||
        fHeight.unit() == SkSVGLength::Unit::kPercentage) {
        return SkSize::Make(0, 0);
    }

    return SkSize::Make(lctx.resolve(fWidth, SkSVGLengthContext::LengthType::kHorizontal),
                        lctx.resolve(fHeight, SkSVGLengthContext::LengthType::kVertical));
}
