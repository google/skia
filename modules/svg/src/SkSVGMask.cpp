/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGMask.h"

#include "include/core/SkCanvas.h"
#include "include/effects/SkLumaColorFilter.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "src/core/SkTLazy.h"

bool SkSVGMask::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", n, v)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", n, v)) ||
           this->setWidth(SkSVGAttributeParser::parse<SkSVGLength>("width", n, v)) ||
           this->setHeight(SkSVGAttributeParser::parse<SkSVGLength>("height", n, v)) ||
           this->setMaskUnits(
                SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>("maskUnits", n, v)) ||
           this->setMaskContentUnits(
                SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>("maskContentUnits", n, v));
}

void SkSVGMask::renderMask(SkSVGRenderContext* ctx) const {
    SkAutoCanvasRestore acr(ctx->canvas(), false);

    SkPaint mask_filter;
    mask_filter.setColorFilter(SkLumaColorFilter::Make());

    ctx->canvas()->saveLayer(nullptr, &mask_filter);

    if (fMaskContentUnits.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        const auto obb = ctx->node()->objectBoundingBox(*ctx);
        ctx->canvas()->translate(obb.x(), obb.y());
        ctx->canvas()->scale(obb.width(), obb.height());
    }

    for (const auto& child : fChildren) {
        child->render(*ctx);
    }
}

void SkSVGMask::apply(SkSVGRenderContext* ctx) const {
    const auto bounds = ctx->resolveOOBRect(fX, fY, fWidth, fHeight, fMaskUnits);

    ctx->canvas()->saveLayer(bounds, nullptr);
    ctx->canvas()->clipRect(bounds, true);
    this->renderMask(ctx);

    SkPaint masking_paint;
    masking_paint.setBlendMode(SkBlendMode::kSrcIn);
    ctx->canvas()->saveLayer(bounds, &masking_paint);
}