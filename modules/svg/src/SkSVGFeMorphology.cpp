/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeMorphology.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

bool SkSVGFeMorphology::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setOperator(SkSVGAttributeParser::parse<SkSVGFeMorphology::Operator>(
                   "operator", name, value)) ||
           this->setRadius(SkSVGAttributeParser::parse<SkSVGFeMorphology::Radius>(
                   "radius", name, value));
}

sk_sp<SkImageFilter> SkSVGFeMorphology::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                          const SkSVGFilterContext& fctx) const {
    const SkRect cropRect = this->resolveFilterSubregion(ctx, fctx);
    const SkSVGColorspace colorspace = this->resolveColorspace(ctx, fctx);
    sk_sp<SkImageFilter> input = fctx.resolveInput(ctx, this->getIn(), colorspace);

    SkScalar rx = fRadius.fX;
    SkScalar ry = fRadius.fY;
    if (fctx.primitiveUnits().type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        SkASSERT(ctx.node());
        const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
        rx *= objBounds.width();
        ry *= objBounds.height();
    }

    switch (fOperator) {
        case Operator::kErode:
            return SkImageFilters::Erode(rx, ry, input, cropRect);
        case Operator::kDilate:
            return SkImageFilters::Dilate(rx, ry, input, cropRect);
    }

    SkUNREACHABLE;
}

template <>
bool SkSVGAttributeParser::parse<SkSVGFeMorphology::Operator>(SkSVGFeMorphology::Operator* op) {
    static constexpr std::tuple<const char*, SkSVGFeMorphology::Operator> gMap[] = {
            { "dilate", SkSVGFeMorphology::Operator::kDilate },
            { "erode" , SkSVGFeMorphology::Operator::kErode  },
    };

    return this->parseEnumMap(gMap, op) && this->parseEOSToken();
}

template <>
bool SkSVGAttributeParser::parse<SkSVGFeMorphology::Radius>(SkSVGFeMorphology::Radius* radius) {
    std::vector<SkSVGNumberType> values;
    if (!this->parse(&values)) {
        return false;
    }

    radius->fX = values[0];
    radius->fY = values.size() > 1 ? values[1] : values[0];
    return true;
}
