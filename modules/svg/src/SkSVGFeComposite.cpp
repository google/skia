/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGFeComposite.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkAssert.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFilterContext.h"

#include <tuple>

class SkSVGRenderContext;

bool SkSVGFeComposite::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           // SkSVGFeInputType parsing defined in SkSVGFe.cpp:
           this->setIn2(SkSVGAttributeParser::parse<SkSVGFeInputType>("in2", name, value)) ||
           this->setK1(SkSVGAttributeParser::parse<SkSVGNumberType>("k1", name, value)) ||
           this->setK2(SkSVGAttributeParser::parse<SkSVGNumberType>("k2", name, value)) ||
           this->setK3(SkSVGAttributeParser::parse<SkSVGNumberType>("k3", name, value)) ||
           this->setK4(SkSVGAttributeParser::parse<SkSVGNumberType>("k4", name, value)) ||
           this->setOperator(
                   SkSVGAttributeParser::parse<SkSVGFeCompositeOperator>("operator", name, value));
}

SkBlendMode SkSVGFeComposite::BlendModeForOperator(SkSVGFeCompositeOperator op) {
    switch (op) {
        case SkSVGFeCompositeOperator::kOver:
            return SkBlendMode::kSrcOver;
        case SkSVGFeCompositeOperator::kIn:
            return SkBlendMode::kSrcIn;
        case SkSVGFeCompositeOperator::kOut:
            return SkBlendMode::kSrcOut;
        case SkSVGFeCompositeOperator::kAtop:
            return SkBlendMode::kSrcATop;
        case SkSVGFeCompositeOperator::kXor:
            return SkBlendMode::kXor;
        case SkSVGFeCompositeOperator::kArithmetic:
            // Arithmetic is not handled with a blend
            SkASSERT(false);
            return SkBlendMode::kSrcOver;
    }

    SkUNREACHABLE;
}

sk_sp<SkImageFilter> SkSVGFeComposite::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                         const SkSVGFilterContext& fctx) const {
    const SkRect cropRect = this->resolveFilterSubregion(ctx, fctx);
    const SkSVGColorspace colorspace = this->resolveColorspace(ctx, fctx);
    const sk_sp<SkImageFilter> background = fctx.resolveInput(ctx, fIn2, colorspace);
    const sk_sp<SkImageFilter> foreground = fctx.resolveInput(ctx, this->getIn(), colorspace);
    if (fOperator == SkSVGFeCompositeOperator::kArithmetic) {
        constexpr bool enforcePMColor = true;
        return SkImageFilters::Arithmetic(
                fK1, fK2, fK3, fK4, enforcePMColor, background, foreground, cropRect);
    } else {
        return SkImageFilters::Blend(
                BlendModeForOperator(fOperator), background, foreground, cropRect);
    }
}

template <> bool SkSVGAttributeParser::parse(SkSVGFeCompositeOperator* op) {
    static constexpr std::tuple<const char*, SkSVGFeCompositeOperator> gOpMap[] = {
            {"over", SkSVGFeCompositeOperator::kOver},
            {"in", SkSVGFeCompositeOperator::kIn},
            {"out", SkSVGFeCompositeOperator::kOut},
            {"atop", SkSVGFeCompositeOperator::kAtop},
            {"xor", SkSVGFeCompositeOperator::kXor},
            {"arithmetic", SkSVGFeCompositeOperator::kArithmetic},
    };

    return this->parseEnumMap(gOpMap, op) && this->parseEOSToken();
}
