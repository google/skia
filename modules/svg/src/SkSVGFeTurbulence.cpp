/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeTurbulence.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"
#include "src/xml/SkDOM.h"

sk_sp<SkSVGFeTurbulence> SkSVGFeTurbulence::Make(const SkDOM& xmlDom, const SkDOM::Node* xmlNode) {
    auto res = sk_sp<SkSVGFeTurbulence>(new SkSVGFeTurbulence);
    const char *name, *value;
    SkDOM::AttrIter attrIter(xmlDom, xmlNode);
    while ((name = attrIter.next(&value))) {
        SVG_ATTR_PARSE_AND_SET(name,
                               value,
                               "baseFrequency",
                               SkSVGFeTurbulenceBaseFrequency,
                               res->setBaseFrequency);
        SVG_ATTR_PARSE_AND_SET(name, value, "numOctaves", SkSVGIntegerType, res->setNumOctaves);
        SVG_ATTR_PARSE_AND_SET(name, value, "seed", SkSVGNumberType, res->setSeed);
        SVG_ATTR_PARSE_AND_SET(name, value, "type", SkSVGFeTurbulenceType, res->setTurbulenceType);
    }
    return res;
}

sk_sp<SkImageFilter> SkSVGFeTurbulence::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                          const SkSVGFilterContext& fctx) const {
    const SkScalar baseFrequencyX = fBaseFrequency.size() < 1 ? 0 : fBaseFrequency[0];
    const SkScalar baseFrequencyY = fBaseFrequency.size() < 2 ? baseFrequencyX : fBaseFrequency[1];
    const SkISize* tileSize = nullptr;  // TODO: needs filter element subregion properties

    sk_sp<SkShader> shader;
    switch (fTurbulenceType.fType) {
        case SkSVGFeTurbulenceType::Type::kTurbulence:
            shader = SkPerlinNoiseShader::MakeTurbulence(
                    baseFrequencyX, baseFrequencyY, fNumOctaves, fSeed, tileSize);
            break;
        case SkSVGFeTurbulenceType::Type::kFractalNoise:
            shader = SkPerlinNoiseShader::MakeFractalNoise(
                    baseFrequencyX, baseFrequencyY, fNumOctaves, fSeed, tileSize);
            break;
    }

    return SkImageFilters::Shader(shader, fctx.filterEffectsRegion());
}
