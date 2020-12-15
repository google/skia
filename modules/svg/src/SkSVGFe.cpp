/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFe.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"

sk_sp<SkImageFilter> SkSVGFe::makeImageFilter(const SkSVGRenderContext& ctx,
                                              const SkSVGFilterContext& fctx) const {
    return this->onMakeImageFilter(ctx, fctx);
}

SkRect SkSVGFe::resolveBoundaries(const SkSVGRenderContext& ctx,
                                  const SkSVGFilterContext& fctx) const {
    const auto x = fX.isValid() ? *fX : SkSVGLength(0, SkSVGLength::Unit::kPercentage);
    const auto y = fY.isValid() ? *fY : SkSVGLength(0, SkSVGLength::Unit::kPercentage);
    const auto w = fWidth.isValid() ? *fWidth : SkSVGLength(100, SkSVGLength::Unit::kPercentage);
    const auto h = fHeight.isValid() ? *fHeight : SkSVGLength(100, SkSVGLength::Unit::kPercentage);

    const SkSVGObjectBoundingBoxUnits::Type primitiveUnits = fctx.primitiveUnits().type();
    const SkSVGLengthContext lctx =
            primitiveUnits == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox
                    ? SkSVGLengthContext({1, 1})
                    : ctx.lengthContext();
    SkRect boundaries = lctx.resolveRect(x, y, w, h);
    if (primitiveUnits == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        SkASSERT(ctx.node());
        const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
        boundaries = SkRect::MakeXYWH(objBounds.fLeft + boundaries.fLeft * objBounds.width(),
                                      objBounds.fTop + boundaries.fTop * objBounds.height(),
                                      boundaries.width() * objBounds.width(),
                                      boundaries.height() * objBounds.height());
    }

    return boundaries;
}

SkRect SkSVGFe::resolveFilterSubregion(const SkSVGRenderContext& ctx,
                                       const SkSVGFilterContext& fctx) const {
    // Get default subregion, which depends on the inputs
    const std::vector<SkSVGFeInputType> inputs = this->getInputs();
    SkRect subregion;
    if (inputs.empty()) {
        subregion = fctx.filterEffectsRegion();
    } else {
        subregion = fctx.filterPrimitiveSubregion(inputs[0]);
        for (size_t i = 1; i < inputs.size(); i++) {
            subregion.join(fctx.filterPrimitiveSubregion(inputs[i]));
        }
    }

    // Resolve rect specified by x, y, w, h attributes, using those values only if given.
    const SkRect boundaries = this->resolveBoundaries(ctx, fctx);
    if (fX.isValid()) {
        subregion.fLeft = boundaries.fLeft;
    }
    if (fY.isValid()) {
        subregion.fTop = boundaries.fTop;
    }
    if (fWidth.isValid()) {
        subregion.fRight = subregion.fLeft + boundaries.width();
    }
    if (fHeight.isValid()) {
        subregion.fBottom = subregion.fTop + boundaries.height();
    }

    return subregion;
}

bool SkSVGFe::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setIn(SkSVGAttributeParser::parse<SkSVGFeInputType>("in", name, value)) ||
           this->setResult(SkSVGAttributeParser::parse<SkSVGStringType>("result", name, value)) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", name, value)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", name, value)) ||
           this->setWidth(SkSVGAttributeParser::parse<SkSVGLength>("width", name, value)) ||
           this->setHeight(SkSVGAttributeParser::parse<SkSVGLength>("height", name, value));
}

template <> bool SkSVGAttributeParser::parse(SkSVGFeInputType* type) {
    static constexpr std::tuple<const char*, SkSVGFeInputType::Type> gTypeMap[] = {
            {"SourceGraphic", SkSVGFeInputType::Type::kSourceGraphic},
            {"SourceAlpha", SkSVGFeInputType::Type::kSourceAlpha},
            {"BackgroundImage", SkSVGFeInputType::Type::kBackgroundImage},
            {"BackgroundAlpha", SkSVGFeInputType::Type::kBackgroundAlpha},
            {"FillPaint", SkSVGFeInputType::Type::kFillPaint},
            {"StrokePaint", SkSVGFeInputType::Type::kStrokePaint},
    };

    SkSVGStringType resultId;
    SkSVGFeInputType::Type t;
    bool parsedValue = false;
    if (this->parseEnumMap(gTypeMap, &t)) {
        *type = SkSVGFeInputType(t);
        parsedValue = true;
    } else if (parse(&resultId)) {
        *type = SkSVGFeInputType(resultId);
        parsedValue = true;
    }

    return parsedValue && this->parseEOSToken();
}
