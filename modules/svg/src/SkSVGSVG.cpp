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

SkSVGSVG::SkSVGSVG() : INHERITED(SkSVGTag::kSvg) { }

static SkMatrix ViewboxMatrix(const SkRect& view_box, const SkRect& view_port,
                              const SkSVGPreserveAspectRatio& par) {
    SkASSERT(!view_box.isEmpty());
    SkASSERT(!view_port.isEmpty());

    auto compute_scale = [&]() -> SkV2 {
        const auto sx = view_port.width()  / view_box.width(),
                   sy = view_port.height() / view_box.height();

        if (par.fAlign == SkSVGPreserveAspectRatio::kNone) {
            // none -> anisotropic scaling, regardless of fScale
            return {sx,sy};
        }

        // isotropic scaling
        const auto s = par.fScale == SkSVGPreserveAspectRatio::kMeet
                            ? std::min(sx, sy)
                            : std::max(sx, sy);
        return {s,s};
    };

    auto compute_trans = [&](const SkV2& scale) -> SkV2 {
        static constexpr float gAlignCoeffs[] = {
            0.0f, // Min
            0.5f, // Mid
            1.0f  // Max
        };

        const size_t x_coeff = par.fAlign>>0 & 0x03,
                     y_coeff = par.fAlign>>2 & 0x03;

        SkASSERT(x_coeff < SK_ARRAY_COUNT(gAlignCoeffs) &&
                 y_coeff < SK_ARRAY_COUNT(gAlignCoeffs));

        const auto tx = -view_box.x() * scale.x,
                   ty = -view_box.y() * scale.y,
                   dx = view_port.width()  - view_box.width() * scale.x,
                   dy = view_port.height() - view_box.height()* scale.y;

        return {
            tx + dx * gAlignCoeffs[x_coeff],
            ty + dy * gAlignCoeffs[y_coeff]
        };
    };

    const auto s = compute_scale(),
               t = compute_trans(s);

    return SkMatrix::Translate(t.x, t.y) *
           SkMatrix::Scale(s.x, s.y);
}

bool SkSVGSVG::onPrepareToRender(SkSVGRenderContext* ctx) const {
    auto viewPortRect  = ctx->lengthContext().resolveRect(fX, fY, fWidth, fHeight);
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

        contentMatrix.preConcat(ViewboxMatrix(viewBox, viewPortRect, fPreserveAspectRatio));
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
