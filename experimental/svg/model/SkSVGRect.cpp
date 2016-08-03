/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkRect.h"
#include "SkSVGRect.h"
#include "SkSVGRenderContext.h"
#include "SkSVGValue.h"

SkSVGRect::SkSVGRect() : INHERITED(SkSVGTag::kRect) {}

void SkSVGRect::setX(const SkSVGLength& x) {
    fX = x;
}

void SkSVGRect::setY(const SkSVGLength& y) {
    fY = y;
}

void SkSVGRect::setWidth(const SkSVGLength& w) {
    fWidth = w;
}

void SkSVGRect::setHeight(const SkSVGLength& h) {
    fHeight = h;
}

void SkSVGRect::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
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
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

void SkSVGRect::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                       const SkPaint& paint) const {
    const SkRect r = SkRect::MakeXYWH(
        lctx.resolve(fX, SkSVGLengthContext::LengthType::kHorizontal),
        lctx.resolve(fY, SkSVGLengthContext::LengthType::kVertical),
        lctx.resolve(fWidth, SkSVGLengthContext::LengthType::kHorizontal),
        lctx.resolve(fHeight, SkSVGLengthContext::LengthType::kVertical));

    canvas->drawRect(r, paint);
}
