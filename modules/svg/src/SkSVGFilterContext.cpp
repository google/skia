/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGTypes.h"

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
                result = SkImageFilters::Paint(*fillPaint);
            }
            break;
        }
        case SkSVGFeInputType::Type::kStrokePaint: {
            // The paint filter doesn't handle stroke paints properly, so convert to fill for
            // simplicity.
            // TODO: Paint filter is deprecated, but the replacement (SkShaders::*())
            //       requires some extra work to handle all paint features (gradients, etc).
            const auto& strokePaint = ctx.strokePaint();
            if (strokePaint.isValid()) {
                SkPaint p = *strokePaint;
                p.setStyle(SkPaint::kFill_Style);
                result = SkImageFilters::Paint(p);
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
            SkDebugf("unhandled filter input type %d\n", (int)inputType.type());
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
