/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeGaussianBlur.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

bool SkSVGFeGaussianBlur::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setStdDeviation(SkSVGAttributeParser::parse<SkSVGFeGaussianBlur::StdDeviation>(
                   "stdDeviation", name, value));
}

sk_sp<SkImageFilter> SkSVGFeGaussianBlur::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                            const SkSVGFilterContext& fctx) const {
    SkScalar sigmaX = fStdDeviation.fX;
    SkScalar sigmaY = fStdDeviation.fY;
    if (fctx.primitiveUnits().type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        SkASSERT(ctx.node());
        const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
        sigmaX *= objBounds.width();
        sigmaY *= objBounds.height();
    }

    return SkImageFilters::Blur(
            sigmaX, sigmaY,
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx)),
            this->resolveFilterSubregion(ctx, fctx));
}

template <>
bool SkSVGAttributeParser::parse<SkSVGFeGaussianBlur::StdDeviation>(
        SkSVGFeGaussianBlur::StdDeviation* stdDeviation) {
    std::vector<SkSVGNumberType> values;
    if (!this->parse(&values)) {
        return false;
    }

    stdDeviation->fX = values[0];
    stdDeviation->fY = values.size() > 1 ? values[1] : values[0];
    return true;
}
