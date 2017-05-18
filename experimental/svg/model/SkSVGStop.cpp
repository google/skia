/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGRenderContext.h"
#include "SkSVGStop.h"
#include "SkSVGValue.h"

SkSVGStop::SkSVGStop() : INHERITED(SkSVGTag::kStop) {}

void SkSVGStop::setOffset(const SkSVGLength& offset) {
    fOffset = offset;
}

void SkSVGStop::setStopColor(const SkSVGColorType& color) {
    fStopColor = color;
}

void SkSVGStop::setStopOpacity(const SkSVGNumberType& opacity) {
    fStopOpacity = SkTPin<SkScalar>(opacity.value(), 0, 1);
}

void SkSVGStop::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kOffset:
        if (const auto* offset = v.as<SkSVGLengthValue>()) {
            this->setOffset(*offset);
        }
        break;
    case SkSVGAttribute::kStopColor:
        if (const auto* color = v.as<SkSVGColorValue>()) {
            this->setStopColor(*color);
        }
        break;
    case SkSVGAttribute::kStopOpacity:
        if (const auto* opacity = v.as<SkSVGNumberValue>()) {
            this->setStopOpacity(*opacity);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}
