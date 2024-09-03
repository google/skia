/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGFeBlend.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkAssert.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFilterContext.h"

#include <tuple>

class SkSVGRenderContext;

bool SkSVGFeBlend::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setIn2(SkSVGAttributeParser::parse<SkSVGFeInputType>("in2", name, value)) ||
           this->setMode(SkSVGAttributeParser::parse<SkSVGFeBlend::Mode>("mode", name, value));
}

static SkBlendMode GetBlendMode(SkSVGFeBlend::Mode mode) {
    switch (mode) {
        case SkSVGFeBlend::Mode::kNormal:
            return SkBlendMode::kSrcOver;
        case SkSVGFeBlend::Mode::kMultiply:
            return SkBlendMode::kMultiply;
        case SkSVGFeBlend::Mode::kScreen:
            return SkBlendMode::kScreen;
        case SkSVGFeBlend::Mode::kDarken:
            return SkBlendMode::kDarken;
        case SkSVGFeBlend::Mode::kLighten:
            return SkBlendMode::kLighten;
    }

    SkUNREACHABLE;
}

sk_sp<SkImageFilter> SkSVGFeBlend::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                     const SkSVGFilterContext& fctx) const {
    const SkRect cropRect = this->resolveFilterSubregion(ctx, fctx);
    const SkBlendMode blendMode = GetBlendMode(this->getMode());
    const SkSVGColorspace colorspace = this->resolveColorspace(ctx, fctx);
    const sk_sp<SkImageFilter> background = fctx.resolveInput(ctx, fIn2, colorspace);
    const sk_sp<SkImageFilter> foreground = fctx.resolveInput(ctx, this->getIn(), colorspace);
    return SkImageFilters::Blend(blendMode, background, foreground, cropRect);
}

template <>
bool SkSVGAttributeParser::parse<SkSVGFeBlend::Mode>(
        SkSVGFeBlend::Mode* mode) {
    static constexpr std::tuple<const char*, SkSVGFeBlend::Mode> gMap[] = {
        { "normal"  , SkSVGFeBlend::Mode::kNormal   },
        { "multiply", SkSVGFeBlend::Mode::kMultiply },
        { "screen"  , SkSVGFeBlend::Mode::kScreen   },
        { "darken"  , SkSVGFeBlend::Mode::kDarken   },
        { "lighten" , SkSVGFeBlend::Mode::kLighten  },
    };

    return this->parseEnumMap(gMap, mode) && this->parseEOSToken();
}
