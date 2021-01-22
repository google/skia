/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGFe.h"
#include "modules/svg/include/SkSVGFilter.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

void SkSVGFilter::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
        case SkSVGAttribute::kX:
            if (const auto* x = v.as<SkSVGLengthValue>()) {
                this->setX(*x);
            }
            break;
        case SkSVGAttribute::kY:
            if (const auto* y = v.as<SkSVGLengthValue>()) {
                this->setY(*y);
            }
            break;
        case SkSVGAttribute::kWidth:
            if (const auto* w = v.as<SkSVGLengthValue>()) {
                this->setWidth(*w);
            }
            break;
        case SkSVGAttribute::kHeight:
            if (const auto* h = v.as<SkSVGLengthValue>()) {
                this->setHeight(*h);
            }
            break;
        case SkSVGAttribute::kFilterUnits:
            if (const auto* filterUnits = v.as<SkSVGObjectBoundingBoxUnitsValue>()) {
                this->setFilterUnits(*filterUnits);
            }
            break;
        default:
            this->INHERITED::onSetAttribute(attr, v);
    }
}

SkRect SkSVGFilter::resolveFilterRegion(const SkSVGRenderContext& ctx) const {
    const SkSVGLengthContext lctx =
            fFilterUnits.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox
                    ? SkSVGLengthContext({1, 1})
                    : ctx.lengthContext();

    SkRect filterRegion = lctx.resolveRect(fX, fY, fWidth, fHeight);
    if (fFilterUnits.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        SkASSERT(ctx.node());
        const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
        filterRegion = SkRect::MakeXYWH(objBounds.fLeft + filterRegion.fLeft * objBounds.fLeft,
                                        objBounds.fTop + filterRegion.fTop * objBounds.fTop,
                                        filterRegion.width() * objBounds.width(),
                                        filterRegion.height() * objBounds.height());
    }

    return filterRegion;
}

sk_sp<SkImageFilter> SkSVGFilter::buildFilterDAG(const SkSVGRenderContext& ctx) const {
    sk_sp<SkImageFilter> filter;
    SkSVGFilterContext fctx(resolveFilterRegion(ctx));
    for (const auto& child : fChildren) {
        if (!SkSVGFe::IsFilterEffect(child)) {
            continue;
        }

        const auto& feNode = static_cast<const SkSVGFe&>(*child);
        const auto& feResultType = feNode.getResult();

        // TODO: there are specific composition rules that need to be followed
        filter = feNode.makeImageFilter(ctx, fctx);

        if (!feResultType.isEmpty()) {
            fctx.registerResult(feResultType, filter);
        }
    }

    return filter;
}
