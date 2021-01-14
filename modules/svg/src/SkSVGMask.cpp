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

void SkSVGMask::apply(SkSVGRenderContext* ctx) const {
    const auto bounds = ctx->resolveOBBRect(fX, fY, fWidth, fHeight, fMaskUnits);

    // Isolation/mask layer.
    ctx->canvas()->saveLayer(bounds, nullptr);

    // Mask bounds act as a clip.
    ctx->canvas()->clipRect(bounds, true);

    // Render and filter mask content.
    // (https://www.w3.org/TR/SVG11/masking.html#Masking)
    {
        SkAutoCanvasRestore acr(ctx->canvas(), false);

        // Generate mask alpha based on mask content luminance (LUMA function).
        // TODO: color-interpolation support
        SkPaint mask_filter;
        mask_filter.setColorFilter(SkLumaColorFilter::Make());

        // Mask color filter layer.
        // Note: We could avoid this extra layer if we invert the stacking order
        // (mask/content -> content/mask, kSrcIn -> kDstIn) and apply the filter
        // via the top (mask) layer paint.  That requires deferring mask rendering
        // until after node content, which introduces extra state/complexity.
        // Something to consider if masking performance ever becomes an issue.
        ctx->canvas()->saveLayer(bounds, &mask_filter);

        if (fMaskContentUnits.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
            // Fot maskContentUnits == OBB the mask content is rendered in a normalized coordinate
            // system, which maps to the node OBB.
            const auto obb = ctx->node()->objectBoundingBox(*ctx);
            ctx->canvas()->translate(obb.x(), obb.y());
            ctx->canvas()->scale(obb.width(), obb.height());
        }

        for (const auto& child : fChildren) {
            child->render(*ctx);
        }
    }

    // Content layer
    SkPaint masking_paint;
    masking_paint.setBlendMode(SkBlendMode::kSrcIn);
    ctx->canvas()->saveLayer(bounds, &masking_paint);

    // At this point we're set up for content rendering.
    // The pending layers are restored in the destructor (render context scope exit).
    // Restoring triggers srcIn-compositing the content against the mask.
}
