/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGTypes.h"

sk_sp<SkImageFilter> SkSVGFilterContext::findResultById(const SkString& id) const {
    for (auto it = fResults.rbegin(), ite = fResults.rend(); it != ite; ++it) {
        if (it->fId == id) {
            return it->fResult;
        }
    }
    return nullptr;
}

void SkSVGFilterContext::registerResult(const SkString& id, const sk_sp<SkImageFilter>& result) {
    fResults.push_back({id, result});
}

sk_sp<SkImageFilter> SkSVGFilterContext::resolveInput(const SkSVGRenderContext& ctx,
                                                      const SkSVGFeInputType& inputType) const {
    switch (inputType.type()) {
        case SkSVGFeInputType::Type::kSourceGraphic:
            return nullptr;
        case SkSVGFeInputType::Type::kFillPaint:
            return SkImageFilters::Paint(*ctx.fillPaint());
        case SkSVGFeInputType::Type::kStrokePaint:
            return SkImageFilters::Paint(*ctx.strokePaint());
        case SkSVGFeInputType::Type::kFilterPrimitiveReference:
            return findResultById(inputType.id());
        default:
            SkDebugf("unhandled filter input type %d\n", inputType.type());
            return nullptr;
    }
}
