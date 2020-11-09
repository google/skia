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

sk_sp<SkImageFilter> SkSVGFilterContext::findResultById(const SkSVGStringType& id) const {
    const sk_sp<SkImageFilter>* res = fResults.find(id);
    return res ? *res : nullptr;
}

void SkSVGFilterContext::registerResult(const SkSVGStringType& id,
                                        const sk_sp<SkImageFilter>& result) {
    SkASSERT(!id.isEmpty());
    fResults[id] = result;
}

sk_sp<SkImageFilter> SkSVGFilterContext::resolveInput(const SkSVGRenderContext& ctx,
                                                      const SkSVGFeInputType& inputType) const {
    switch (inputType.type()) {
        case SkSVGFeInputType::Type::kSourceGraphic:
            return nullptr;
        case SkSVGFeInputType::Type::kFillPaint:
            return SkImageFilters::Paint(*ctx.fillPaint());
        case SkSVGFeInputType::Type::kStrokePaint: {
            // The paint filter doesn't handle stroke paints properly, so convert to fill for
            // simplicity.
            // TODO: Paint filter is deprecated, but the replacement (SkShaders::*())
            //       requires some extra work to handle all paint features (gradients, etc).
            SkPaint p = *ctx.strokePaint();
            p.setStyle(SkPaint::kFill_Style);
            return SkImageFilters::Paint(p);
        }
        case SkSVGFeInputType::Type::kFilterPrimitiveReference:
            return findResultById(inputType.id());
        default:
            SkDebugf("unhandled filter input type %d\n", inputType.type());
            return nullptr;
    }
}
