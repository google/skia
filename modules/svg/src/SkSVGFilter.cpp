/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGFe.h"
#include "modules/svg/include/SkSVGFilter.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

bool SkSVGFilter::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", name, value)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", name, value)) ||
           this->setWidth(SkSVGAttributeParser::parse<SkSVGLength>("width", name, value)) ||
           this->setHeight(SkSVGAttributeParser::parse<SkSVGLength>("height", name, value)) ||
           this->setFilterUnits(SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>(
                   "filterUnits", name, value)) ||
           this->setPrimitiveUnits(SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>(
                   "primitiveUnits", name, value));
}

SkRect SkSVGFilter::resolveFilterRegion(const SkSVGRenderContext& ctx) const {
    const SkSVGLengthContext lctx =
            fFilterUnits.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox
                    ? SkSVGLengthContext({1, 1})
                    : ctx.lengthContext();

    SkRect filterRegion = lctx.resolveRect(fX, fY, fWidth, fHeight);
    if (fFilterUnits.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        SkASSERT(ctx.node());
        const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
        filterRegion = SkRect::MakeXYWH(objBounds.fLeft + filterRegion.fLeft * objBounds.width(),
                                        objBounds.fTop + filterRegion.fTop * objBounds.height(),
                                        filterRegion.width() * objBounds.width(),
                                        filterRegion.height() * objBounds.height());
    }

    return filterRegion;
}

sk_sp<SkImageFilter> SkSVGFilter::buildFilterDAG(const SkSVGRenderContext& ctx) const {
    sk_sp<SkImageFilter> filter;
    SkSVGFilterContext fctx(resolveFilterRegion(ctx), fPrimitiveUnits);
    for (const auto& child : fChildren) {
        if (!SkSVGFe::IsFilterEffect(child)) {
            continue;
        }

        const auto& feNode = static_cast<const SkSVGFe&>(*child);
        const auto& feResultType = feNode.getResult();

        // Propagate any inherited properties that may impact filter effect behavior (e.g.
        // color-interpolation-filters). We call this explicitly here because the SkSVGFe
        // nodes do not participate in the normal onRender path, which is when property
        // propagation currently occurs.
        SkSVGRenderContext localCtx(ctx);
        feNode.applyProperties(&localCtx);

        // TODO: there are specific composition rules that need to be followed
        // TODO: perform colorspace conversions depending on 'color-interpolation-filters' setting
        // of the current node and its inputs.
        filter = feNode.makeImageFilter(localCtx, fctx);

        if (!feResultType.isEmpty()) {
            fctx.registerResult(feResultType, filter, feNode.resolveFilterSubregion(localCtx, fctx));
        }
    }

    return filter;
}
