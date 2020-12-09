/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTPin.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGStop.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGStop::SkSVGStop() : INHERITED(SkSVGTag::kStop) {}

void SkSVGStop::setOffset(const SkSVGLength& offset) {
    fOffset = offset;
}

void SkSVGStop::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kOffset:
        if (const auto* offset = v.as<SkSVGLengthValue>()) {
            this->setOffset(*offset);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}
