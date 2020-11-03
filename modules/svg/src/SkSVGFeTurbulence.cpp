/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "modules/svg/include/SkSVGFeTurbulence.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

void SkSVGFeTurbulence::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
        case SkSVGAttribute::kFeTurbulenceBaseFrequency:
            if (const auto* f = v.as<SkSVGFeTurbulenceBaseFrequencyValue>()) {
                this->setBaseFrequency(*f);
            }
            break;
        case SkSVGAttribute::kFeTurbulenceNumOctaves:
            if (const auto* n = v.as<SkSVGIntegerValue>()) {
                this->setNumOctaves(*n);
            }
            break;
        case SkSVGAttribute::kFeTurbulenceSeed:
            if (const auto* s = v.as<SkSVGNumberValue>()) {
                this->setSeed(*s);
            }
            break;
        case SkSVGAttribute::kFeTurbulenceType:
            if (const auto* t = v.as<SkSVGFeTurbulenceTypeValue>()) {
                this->setTurbulenceType(*t);
            }
            break;
        default:
            this->INHERITED::onSetAttribute(attr, v);
    }
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
