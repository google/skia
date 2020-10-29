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
        case SkSVGAttribute::kFeTurbulenceType:
            if (const auto* t = v.as<SkSVGFeTurbulenceTypeValue>()) {
                this->setType(*t);
            }
            break;
        case SkSVGAttribute::kFeTurbulenceNumOctaves:
            if (const auto* n = v.as<SkSVGNumberValue>()) {
                this->setNumOctaves(*n);
            }
            break;
        default:
            this->INHERITED::onSetAttribute(attr, v);
    }
}

sk_sp<SkImageFilter> SkSVGFeTurbulence::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                          const SkSVGFilterContext& fctx) const {
    const SkScalar baseFrequencyX = fBaseFrequency.size() < 1 ? 0 : fBaseFrequency[0];
    const SkScalar baseFrequencyY = fBaseFrequency.size() < 2 ? baseFrequencyX : fBaseFrequency[1];
    const int numOctaves = static_cast<int>(fNumOctaves);
    const SkScalar seed = 0;
    return SkImageFilters::Shader(
            SkPerlinNoiseShader::MakeTurbulence(
                    baseFrequencyX, baseFrequencyY, numOctaves, seed, nullptr),
            fctx.filterEffectsRegion());
}
