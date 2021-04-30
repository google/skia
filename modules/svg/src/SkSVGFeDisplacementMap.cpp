/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeDisplacementMap.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

bool SkSVGFeDisplacementMap::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setIn2(SkSVGAttributeParser::parse<SkSVGFeInputType>("in2", name, value)) ||
           this->setXChannelSelector(
                   SkSVGAttributeParser::parse<SkSVGFeDisplacementMap::ChannelSelector>(
                           "xChannelSelector", name, value)) ||
           this->setYChannelSelector(
                   SkSVGAttributeParser::parse<SkSVGFeDisplacementMap::ChannelSelector>(
                           "yChannelSelector", name, value)) ||
           this->setScale(SkSVGAttributeParser::parse<SkSVGNumberType>("scale", name, value));
}

sk_sp<SkImageFilter> SkSVGFeDisplacementMap::onMakeImageFilter(
        const SkSVGRenderContext& ctx, const SkSVGFilterContext& fctx) const {
    const SkRect cropRect = this->resolveFilterSubregion(ctx, fctx);
    const SkSVGColorspace colorspace = this->resolveColorspace(ctx, fctx);

    // According to spec https://www.w3.org/TR/SVG11/filters.html#feDisplacementMapElement,
    // the 'in' source image must remain in its current colorspace.
    sk_sp<SkImageFilter> in = fctx.resolveInput(ctx, this->getIn());
    sk_sp<SkImageFilter> in2 = fctx.resolveInput(ctx, this->getIn2(), colorspace);

    SkScalar scale = fScale;
    if (fctx.primitiveUnits().type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        const auto obbt = ctx.transformForCurrentOBB(fctx.primitiveUnits());
        scale = SkSVGLengthContext({obbt.scale.x, obbt.scale.y})
                    .resolve(SkSVGLength(scale, SkSVGLength::Unit::kPercentage),
                             SkSVGLengthContext::LengthType::kOther);
    }

    return SkImageFilters::DisplacementMap(
            fXChannelSelector, fYChannelSelector, scale, in2, in, cropRect);
}

SkSVGColorspace SkSVGFeDisplacementMap::resolveColorspace(const SkSVGRenderContext& ctx,
                                                          const SkSVGFilterContext& fctx) const {
    // According to spec https://www.w3.org/TR/SVG11/filters.html#feDisplacementMapElement,
    // the 'in' source image must remain in its current colorspace, which means the colorspace of
    // this FE node is the same as the input.
    return fctx.resolveInputColorspace(ctx, this->getIn());
}

template <>
bool SkSVGAttributeParser::parse<SkSVGFeDisplacementMap::ChannelSelector>(
        SkSVGFeDisplacementMap::ChannelSelector* channel) {
    static constexpr std::tuple<const char*, SkSVGFeDisplacementMap::ChannelSelector> gMap[] = {
            { "R", SkSVGFeDisplacementMap::ChannelSelector::kR },
            { "G", SkSVGFeDisplacementMap::ChannelSelector::kG },
            { "B", SkSVGFeDisplacementMap::ChannelSelector::kB },
            { "A", SkSVGFeDisplacementMap::ChannelSelector::kA },
    };

    return this->parseEnumMap(gMap, channel) && this->parseEOSToken();
}
