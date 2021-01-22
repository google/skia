/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
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

sk_sp<SkImageFilter> SkSVGFilter::buildFilterDAG(const SkSVGRenderContext& ctx) const {
    sk_sp<SkImageFilter> filter;
    SkSVGFilterContext fctx(ctx.resolveOBBRect(fX, fY, fWidth, fHeight, fFilterUnits),
                            fPrimitiveUnits);
    SkSVGColorspace cs = SkSVGColorspace::kSRGB;
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

        const SkRect filterSubregion = feNode.resolveFilterSubregion(localCtx, fctx);
        cs = feNode.resolveColorspace(ctx, fctx);
        filter = feNode.makeImageFilter(localCtx, fctx);

        if (!feResultType.isEmpty()) {
            fctx.registerResult(feResultType, filter, filterSubregion, cs);
        }

        // Unspecified 'in' and 'in2' inputs implicitly resolve to the previous filter's result.
        fctx.setPreviousResult(filter, filterSubregion, cs);
    }

    // Convert to final destination colorspace
    if (cs != SkSVGColorspace::kSRGB) {
        filter = SkImageFilters::ColorFilter(SkColorFilters::LinearToSRGBGamma(), filter);
    }

    return filter;
}
