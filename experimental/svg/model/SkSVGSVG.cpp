/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGSVG.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"

SkSVGSVG::SkSVGSVG() : INHERITED(SkSVGTag::kSvg) { }

bool SkSVGSVG::onPrepareToRender(SkSVGRenderContext* ctx) const {
    auto viewPortRect  = ctx->lengthContext().resolveRect(fX, fY, fWidth, fHeight);
    auto contentMatrix = SkMatrix::MakeTrans(viewPortRect.x(), viewPortRect.y());
    auto viewPort      = SkSize::Make(viewPortRect.width(), viewPortRect.height());

    if (fViewBox.isValid()) {
        const SkRect& viewBox = *fViewBox.get();

        // An empty viewbox disables rendering.
        if (viewBox.isEmpty()) {
            return false;
        }

        // A viewBox overrides the intrinsic viewport.
        viewPort = SkSize::Make(viewBox.width(), viewBox.height());

        contentMatrix.preConcat(
            SkMatrix::MakeRectToRect(viewBox, viewPortRect, SkMatrix::kFill_ScaleToFit));
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

void SkSVGSVG::setX(const SkSVGLength& x) {
    fX = x;
}

void SkSVGSVG::setY(const SkSVGLength& y) {
    fY = y;
}

void SkSVGSVG::setWidth(const SkSVGLength& w) {
    fWidth = w;
}

void SkSVGSVG::setHeight(const SkSVGLength& h) {
    fHeight = h;
}

void SkSVGSVG::setViewBox(const SkSVGViewBoxType& vb) {
    fViewBox.set(vb);
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
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

// https://www.w3.org/TR/SVG/coords.html#IntrinsicSizing
SkSize SkSVGSVG::intrinsicSize(const SkSVGLengthContext& lctx) const {
    // Percentage values do not provide an intrinsic size.
    if (fWidth.unit() == SkSVGLength::Unit::kPercentage ||
        fHeight.unit() == SkSVGLength::Unit::kPercentage) {
        return SkSize::Make(0, 0);
    }

    return SkSize::Make(lctx.resolve(fWidth, SkSVGLengthContext::LengthType::kHorizontal),
                        lctx.resolve(fHeight, SkSVGLengthContext::LengthType::kVertical));
}
