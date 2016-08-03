/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGSVG.h"
#include "SkSVGValue.h"

SkSVGSVG::SkSVGSVG() : INHERITED(SkSVGTag::kSvg) { }

void SkSVGSVG::setX(const SkSVGLength& x) {
    fX = x;
}

void SkSVGSVG::setY(const SkSVGLength& y) {
    fY = y;
}

void SkSVGSVG::setWidth(const SkSVGLength& w) {
    fWidth = w;
}

void SkSVGSVG::setHeight(const SkSVGLength& h) {
    fHeight = h;
}

void SkSVGSVG::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
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
