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

    // Resolve the x/y/w/h boundary rect depending on primitiveUnits setting
    SkRect boundaries;
    switch (fctx.primitiveUnits().type()) {
        case SkSVGObjectBoundingBoxUnits::Type::kUserSpaceOnUse:
            boundaries = ctx.lengthContext().resolveRect(x, y, w, h);
            break;
        case SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox: {
            SkASSERT(ctx.node());
            const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
            boundaries = SkSVGLengthContext({1, 1}).resolveRect(x, y, w, h);
            boundaries = SkRect::MakeXYWH(objBounds.fLeft + boundaries.fLeft * objBounds.width(),
                                          objBounds.fTop + boundaries.fTop * objBounds.height(),
                                          boundaries.width() * objBounds.width(),
                                          boundaries.height() * objBounds.height());

            break;
        }
    }

    return boundaries;
}

static bool AnyIsStandardInput(const SkSVGFilterContext& fctx,
                               const std::vector<SkSVGFeInputType>& inputs) {
    for (const auto& in : inputs) {
        switch (in.type()) {
            case SkSVGFeInputType::Type::kFilterPrimitiveReference:
                break;
            case SkSVGFeInputType::Type::kSourceGraphic:
            case SkSVGFeInputType::Type::kSourceAlpha:
            case SkSVGFeInputType::Type::kBackgroundImage:
            case SkSVGFeInputType::Type::kBackgroundAlpha:
            case SkSVGFeInputType::Type::kFillPaint:
            case SkSVGFeInputType::Type::kStrokePaint:
                return true;
            case SkSVGFeInputType::Type::kUnspecified:
                // Unspecified means previous result (which may be SourceGraphic).
                if (fctx.previousResultIsSourceGraphic()) {
                    return true;
                }
                break;
        }
    }

    return false;
}

SkRect SkSVGFe::resolveFilterSubregion(const SkSVGRenderContext& ctx,
                                       const SkSVGFilterContext& fctx) const {
    // From https://www.w3.org/TR/SVG11/filters.html#FilterPrimitiveSubRegion,
    // the default filter effect subregion is equal to the union of the subregions defined
    // for all "referenced nodes" (filter effect inputs). If there are no inputs, the
    // default subregion is equal to the filter effects region
    // (https://www.w3.org/TR/SVG11/filters.html#FilterEffectsRegion).
    const std::vector<SkSVGFeInputType> inputs = this->getInputs();
    SkRect subregion;
    if (inputs.empty() || AnyIsStandardInput(fctx, inputs)) {
        subregion = fctx.filterEffectsRegion();
    } else {
        subregion = fctx.filterPrimitiveSubregion(inputs[0]);
        for (size_t i = 1; i < inputs.size(); i++) {
            subregion.join(fctx.filterPrimitiveSubregion(inputs[i]));
        }
    }

    // Next resolve the rect specified by the x, y, width, height attributes on this filter effect.
    // If those attributes were given, they override the corresponding attribute of the default
    // filter effect subregion calculated above.
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

SkSVGColorspace SkSVGFe::resolveColorspace(const SkSVGRenderContext& ctx,
                                           const SkSVGFilterContext&) const {
    constexpr SkSVGColorspace kDefaultCS = SkSVGColorspace::kSRGB;
    const SkSVGColorspace cs = *ctx.presentationContext().fInherited.fColorInterpolationFilters;
    return cs == SkSVGColorspace::kAuto ? kDefaultCS : cs;
}

void SkSVGFe::applyProperties(SkSVGRenderContext* ctx) const { this->onPrepareToRender(ctx); }

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
