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
