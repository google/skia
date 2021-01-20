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
    SkScalar dx = this->getDx(), dy = this->getDy();
    if (fctx.primitiveUnits().type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        SkASSERT(ctx.node());
        const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
        dx *= objBounds.width();
        dy *= objBounds.height();
    }

    sk_sp<SkImageFilter> in =
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx));
    return SkImageFilters::Offset(dx, dy, std::move(in), this->resolveFilterSubregion(ctx, fctx));
}
