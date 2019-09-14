/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGRect.h"
#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"

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

void SkSVGRect::setRx(const SkSVGLength& rx) {
    fRx = rx;
}

void SkSVGRect::setRy(const SkSVGLength& ry) {
    fRy = ry;
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
    case SkSVGAttribute::kRx:
        if (const auto* rx = v.as<SkSVGLengthValue>()) {
            this->setRx(*rx);
        }
        break;
    case SkSVGAttribute::kRy:
        if (const auto* ry = v.as<SkSVGLengthValue>()) {
            this->setRy(*ry);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

SkRRect SkSVGRect::resolve(const SkSVGLengthContext& lctx) const {
    const SkRect rect = lctx.resolveRect(fX, fY, fWidth, fHeight);
    const SkScalar rx = lctx.resolve(fRx, SkSVGLengthContext::LengthType::kHorizontal);
    const SkScalar ry = lctx.resolve(fRy, SkSVGLengthContext::LengthType::kVertical);

    return SkRRect::MakeRectXY(rect, rx ,ry);
}

void SkSVGRect::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                       const SkPaint& paint, SkPath::FillType) const {
    canvas->drawRRect(this->resolve(lctx), paint);
}

SkPath SkSVGRect::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPath path;
    path.addRRect(this->resolve(ctx.lengthContext()));

    this->mapToParent(&path);

    return path;
}
