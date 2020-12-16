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

template <>
bool SkSVGAttributeParser::parse<SkSVGFeGaussianBlur::StdDeviation>(
        SkSVGFeGaussianBlur::StdDeviation* stdDeviation) {
    SkSVGNumberType freqX;
    if (!this->parse(&freqX)) {
        return false;
    }

    SkSVGNumberType freqY;
    this->parseCommaWspToken();
    if (this->parse(&freqY)) {
        *stdDeviation = SkSVGFeGaussianBlur::StdDeviation({freqX, freqY});
    } else {
        *stdDeviation = SkSVGFeGaussianBlur::StdDeviation({freqX, freqX});
    }

    return this->parseEOSToken();
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
    return SkImageFilters::Blur(sigmaX, sigmaY,
                                fctx.resolveInput(ctx, this->getIn()),
                                this->resolveFilterSubregion(ctx, fctx));
}
