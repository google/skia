/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGFilterContext.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGTypes.h"
#include "src/base/SkTLazy.h"

#include <utility>

namespace {

sk_sp<SkImageFilter> ConvertFilterColorspace(sk_sp<SkImageFilter>&& input,
                                             SkSVGColorspace src,
                                             SkSVGColorspace dst) {
    if (src == dst) {
        return std::move(input);
    } else if (src == SkSVGColorspace::kSRGB && dst == SkSVGColorspace::kLinearRGB) {
        return SkImageFilters::ColorFilter(SkColorFilters::SRGBToLinearGamma(), input);
    } else {
        SkASSERT(src == SkSVGColorspace::kLinearRGB && dst == SkSVGColorspace::kSRGB);
        return SkImageFilters::ColorFilter(SkColorFilters::LinearToSRGBGamma(), input);
    }
}

sk_sp<SkShader> paint_as_shader(const SkPaint& paint) {
    sk_sp<SkShader> shader = paint.refShader();
    auto color = paint.getColor4f();
    if (shader && color.fA < 1.f) {
        // Multiply by paint alpha
        shader = shader->makeWithColorFilter(
                SkColorFilters::Blend(color, /*colorSpace=*/nullptr, SkBlendMode::kDstIn));
    } else if (!shader) {
        shader = SkShaders::Color(color, /*colorSpace=*/nullptr);
    }
    if (paint.getColorFilter()) {
        shader = shader->makeWithColorFilter(paint.refColorFilter());
    }
    return shader;
}

}  // namespace

const SkSVGFilterContext::Result* SkSVGFilterContext::findResultById(
        const SkSVGStringType& id) const {
    return fResults.find(id);
}

const SkRect& SkSVGFilterContext::filterPrimitiveSubregion(const SkSVGFeInputType& input) const {
    const Result* res = nullptr;
    if (input.type() == SkSVGFeInputType::Type::kFilterPrimitiveReference) {
        res = fResults.find(input.id());
    } else if (input.type() == SkSVGFeInputType::Type::kUnspecified) {
        res = &fPreviousResult;
    }
    return res ? res->fFilterSubregion : fFilterEffectsRegion;
}

void SkSVGFilterContext::registerResult(const SkSVGStringType& id,
                                        const sk_sp<SkImageFilter>& result,
                                        const SkRect& subregion,
                                        SkSVGColorspace resultColorspace) {
    SkASSERT(!id.isEmpty());
    fResults[id] = {result, subregion, resultColorspace};
}

void SkSVGFilterContext::setPreviousResult(const sk_sp<SkImageFilter>& result,
                                           const SkRect& subregion,
                                           SkSVGColorspace resultColorspace) {
    fPreviousResult = {result, subregion, resultColorspace};
}

bool SkSVGFilterContext::previousResultIsSourceGraphic() const {
    return fPreviousResult.fImageFilter == nullptr;
}

// https://www.w3.org/TR/SVG11/filters.html#FilterPrimitiveInAttribute
std::tuple<sk_sp<SkImageFilter>, SkSVGColorspace> SkSVGFilterContext::getInput(
        const SkSVGRenderContext& ctx, const SkSVGFeInputType& inputType) const {
    SkSVGColorspace inputCS = SkSVGColorspace::kSRGB;
    sk_sp<SkImageFilter> result;
    switch (inputType.type()) {
        case SkSVGFeInputType::Type::kSourceAlpha: {
            SkColorMatrix m;
            m.setScale(0, 0, 0, 1.0f);
            result = SkImageFilters::ColorFilter(SkColorFilters::Matrix(m), nullptr);
            break;
        }
        case SkSVGFeInputType::Type::kSourceGraphic:
            // Do nothing.
            break;
        case SkSVGFeInputType::Type::kFillPaint: {
            const auto& fillPaint = ctx.fillPaint();
            if (fillPaint.isValid()) {
                auto dither = fillPaint->isDither() ? SkImageFilters::Dither::kYes
                                                    : SkImageFilters::Dither::kNo;
                result = SkImageFilters::Shader(paint_as_shader(*fillPaint), dither);
            }
            break;
        }
        case SkSVGFeInputType::Type::kStrokePaint: {
            // The paint filter doesn't apply fill/stroke styling, but use the paint settings
            // defined for strokes.
            const auto& strokePaint = ctx.strokePaint();
            if (strokePaint.isValid()) {
                auto dither = strokePaint->isDither() ? SkImageFilters::Dither::kYes
                                                      : SkImageFilters::Dither::kNo;
                result = SkImageFilters::Shader(paint_as_shader(*strokePaint), dither);
            }
            break;
        }
        case SkSVGFeInputType::Type::kFilterPrimitiveReference: {
            const Result* res = findResultById(inputType.id());
            if (res) {
                result = res->fImageFilter;
                inputCS = res->fColorspace;
            }
            break;
        }
        case SkSVGFeInputType::Type::kUnspecified: {
            result = fPreviousResult.fImageFilter;
            inputCS = fPreviousResult.fColorspace;
            break;
        }
        default:
            SkDEBUGF("unhandled filter input type %d\n", (int)inputType.type());
            break;
    }

    return {result, inputCS};
}

SkSVGColorspace SkSVGFilterContext::resolveInputColorspace(
        const SkSVGRenderContext& ctx, const SkSVGFeInputType& inputType) const {
    return std::get<1>(this->getInput(ctx, inputType));
}

sk_sp<SkImageFilter> SkSVGFilterContext::resolveInput(const SkSVGRenderContext& ctx,
                                                      const SkSVGFeInputType& inputType) const {
    return std::get<0>(this->getInput(ctx, inputType));
}

sk_sp<SkImageFilter> SkSVGFilterContext::resolveInput(const SkSVGRenderContext& ctx,
                                                      const SkSVGFeInputType& inputType,
                                                      SkSVGColorspace colorspace) const {
    auto [result, inputCS] = this->getInput(ctx, inputType);
    return ConvertFilterColorspace(std::move(result), inputCS, colorspace);
}
