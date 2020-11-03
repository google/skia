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
    return res;
}

bool SkSVGFeTurbulence::parseAndSetAttribute(const char* name, const char* value) {
    bool consumedAttribute = INHERITED::parseAndSetAttribute(name, value);
    SVG_ATTR_PARSE_AND_SET(
            name, value, "baseFrequency", SkSVGFeTurbulenceBaseFrequency, this->setBaseFrequency);
    SVG_ATTR_PARSE_AND_SET(name, value, "numOctaves", SkSVGIntegerType, this->setNumOctaves);
    SVG_ATTR_PARSE_AND_SET(name, value, "seed", SkSVGNumberType, this->setSeed);
    SVG_ATTR_PARSE_AND_SET(name, value, "type", SkSVGFeTurbulenceType, this->setTurbulenceType);
    return consumedAttribute;
}

sk_sp<SkImageFilter> SkSVGFeTurbulence::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                          const SkSVGFilterContext& fctx) const {
    const SkISize* tileSize = nullptr;  // TODO: needs filter element subregion properties

    sk_sp<SkShader> shader;
    switch (fTurbulenceType.fType) {
        case SkSVGFeTurbulenceType::Type::kTurbulence:
            shader = SkPerlinNoiseShader::MakeTurbulence(
                    fBaseFrequency.freqX(), fBaseFrequency.freqY(), fNumOctaves, fSeed, tileSize);
            break;
        case SkSVGFeTurbulenceType::Type::kFractalNoise:
            shader = SkPerlinNoiseShader::MakeFractalNoise(
                    fBaseFrequency.freqX(), fBaseFrequency.freqY(), fNumOctaves, fSeed, tileSize);
            break;
    }

    return SkImageFilters::Shader(shader, fctx.filterEffectsRegion());
}
