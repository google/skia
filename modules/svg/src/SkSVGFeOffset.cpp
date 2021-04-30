/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeOffset.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

bool SkSVGFeOffset::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setDx(SkSVGAttributeParser::parse<SkSVGNumberType>("dx", name, value)) ||
           this->setDy(SkSVGAttributeParser::parse<SkSVGNumberType>("dy", name, value));
}

sk_sp<SkImageFilter> SkSVGFeOffset::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                      const SkSVGFilterContext& fctx) const {
    const auto d = SkV2{this->getDx(), this->getDy()}
                 * ctx.transformForCurrentOBB(fctx.primitiveUnits()).scale;

    sk_sp<SkImageFilter> in =
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx));
    return SkImageFilters::Offset(d.x, d.y, std::move(in), this->resolveFilterSubregion(ctx, fctx));
}
