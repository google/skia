/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGStop.h"

SkSVGStop::SkSVGStop() : INHERITED(SkSVGTag::kStop) {}

void SkSVGStop::setOffset(const SkSVGLength& offset) {
    fOffset = offset;
}

void SkSVGStop::setStopColor(const SkSVGColorType& color) {
    fStopColor = color;
}

void SkSVGStop::setStopOpacity(const SkSVGNumberType& opacity) {
    fStopOpacity = SkTPin<SkScalar>(opacity, 0, 1);
}

void SkSVGStop::onSetAttribute(SkSVGAttribute attr, const SkSVGAttributeValue& v) {
    switch (attr) {
    case SkSVGAttribute::kOffset:
        if (const auto* offset = std::get_if<SkSVGLength>(&v)) {
            this->setOffset(*offset);
        }
        break;
    case SkSVGAttribute::kStopColor:
        if (const auto* color = std::get_if<SkSVGColorType>(&v)) {
            this->setStopColor(*color);
        }
        break;
    case SkSVGAttribute::kStopOpacity:
        if (const auto* opacity = std::get_if<SkSVGNumberType>(&v)) {
            this->setStopOpacity(*opacity);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}
