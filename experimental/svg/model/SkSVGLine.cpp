/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkSVGLine.h"
#include "SkSVGRenderContext.h"
#include "SkSVGValue.h"

SkSVGLine::SkSVGLine() : INHERITED(SkSVGTag::kLine) {}

void SkSVGLine::setX1(const SkSVGLength& x1) {
    fX1 = x1;
}

void SkSVGLine::setY1(const SkSVGLength& y1) {
    fY1 = y1;
}

void SkSVGLine::setX2(const SkSVGLength& x2) {
    fX2 = x2;
}

void SkSVGLine::setY2(const SkSVGLength& y2) {
    fY2 = y2;
}

void SkSVGLine::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kX1:
        if (const auto* x1 = v.as<SkSVGLengthValue>()) {
            this->setX1(*x1);
        }
        break;
    case SkSVGAttribute::kY1:
        if (const auto* y1 = v.as<SkSVGLengthValue>()) {
            this->setY1(*y1);
        }
        break;
    case SkSVGAttribute::kX2:
        if (const auto* x2 = v.as<SkSVGLengthValue>()) {
            this->setX2(*x2);
        }
        break;
    case SkSVGAttribute::kY2:
        if (const auto* y2 = v.as<SkSVGLengthValue>()) {
            this->setY2(*y2);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

void SkSVGLine::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                       const SkPaint& paint, SkPath::FillType) const {
    const auto x1 = lctx.resolve(fX1, SkSVGLengthContext::LengthType::kHorizontal);
    const auto y1 = lctx.resolve(fY1, SkSVGLengthContext::LengthType::kVertical);
    const auto x2 = lctx.resolve(fX2, SkSVGLengthContext::LengthType::kHorizontal);
    const auto y2 = lctx.resolve(fY2, SkSVGLengthContext::LengthType::kVertical);

    canvas->drawLine(x1, y1, x2, y2, paint);
}
