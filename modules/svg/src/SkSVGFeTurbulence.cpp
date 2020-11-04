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

bool SkSVGFeTurbulence::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setNumOctaves(
                   SkSVGAttributeParser::parse<SkSVGIntegerType>("numOctaves", name, value)) ||
           this->setSeed(SkSVGAttributeParser::parse<SkSVGNumberType>("seed", name, value)) ||
           this->setBaseFrequency(SkSVGAttributeParser::parse<SkSVGFeTurbulenceBaseFrequency>(
                   "baseFrequency", name, value, SkSVGFeTurbulence::parse)) ||
           this->setTurbulenceType(SkSVGAttributeParser::parse<SkSVGFeTurbulenceType>(
                   "type", name, value, SkSVGFeTurbulence::parse));
}

bool SkSVGFeTurbulence::parse(const char* v, SkSVGFeTurbulenceBaseFrequency* freq) {
    SkSVGAttributeParser parser(v);

    SkSVGNumberType freqX;
    if (!parser.parseNumber(&freqX)) {
        return false;
    }

    SkSVGNumberType freqY;
    parser.parseCommaWspToken();
    if (parser.parseNumber(&freqY)) {
        *freq = SkSVGFeTurbulenceBaseFrequency(freqX, freqY);
    } else {
        *freq = SkSVGFeTurbulenceBaseFrequency(freqX, freqX);
    }

    return parser.parseEOSToken();
}

bool SkSVGFeTurbulence::parse(const char* v, SkSVGFeTurbulenceType* type) {
    SkSVGAttributeParser parser(v);
    bool parsedValue = false;

    if (parser.parseExpectedStringToken("fractalNoise")) {
        *type = SkSVGFeTurbulenceType(SkSVGFeTurbulenceType::kFractalNoise);
        parsedValue = true;
    } else if (parser.parseExpectedStringToken("turbulence")) {
        *type = SkSVGFeTurbulenceType(SkSVGFeTurbulenceType::kTurbulence);
        parsedValue = true;
    }

    return parsedValue && parser.parseEOSToken();
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
