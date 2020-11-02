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

sk_sp<SkImageFilter> SkSVGFilter::onAsImageFilter(const SkSVGRenderContext& ctx) const {
    SkRect filterRegion;
    if (fFilterUnits.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        SkASSERT(ctx.node());
        const SkSVGLengthContext lctx = SkSVGLengthContext({1, 1});
        const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
        const SkScalar x = lctx.resolve(fX, SkSVGLengthContext::LengthType::kHorizontal);
        const SkScalar y = lctx.resolve(fY, SkSVGLengthContext::LengthType::kVertical);
        const SkScalar w = lctx.resolve(fWidth, SkSVGLengthContext::LengthType::kHorizontal);
        const SkScalar h = lctx.resolve(fHeight, SkSVGLengthContext::LengthType::kVertical);
        filterRegion = SkRect::MakeXYWH(objBounds.fLeft + objBounds.fLeft * x,
                                        objBounds.fTop + objBounds.fTop * y,
                                        objBounds.width() * w,
                                        objBounds.height() * h);
    } else {
        filterRegion = ctx.lengthContext().resolveRect(fX, fY, fWidth, fHeight);
    }

    sk_sp<SkImageFilter> filter;
    SkSVGFilterContext fctx(filterRegion);
    for (const auto& child : fChildren) {
        if (!SkSVGFe::IsFilterElement(child)) {
            continue;
        }

        const auto& feNode = static_cast<const SkSVGFe&>(*child);
        sk_sp<SkImageFilter> imageFilter = feNode.makeImageFilter(ctx, &fctx);
        if (imageFilter) {
            // TODO: there are specific composition rules that need to be followed
            filter = SkImageFilters::Compose(imageFilter, filter);
        }
    }

    return filter;
}
