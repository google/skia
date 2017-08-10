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

std::tuple<SkPoint, SkPoint> SkSVGLine::resolve(const SkSVGLengthContext& lctx) const {
    return std::make_tuple(
        SkPoint::Make(lctx.resolve(fX1, SkSVGLengthContext::LengthType::kHorizontal),
                      lctx.resolve(fY1, SkSVGLengthContext::LengthType::kVertical)),
        SkPoint::Make(lctx.resolve(fX2, SkSVGLengthContext::LengthType::kHorizontal),
                      lctx.resolve(fY2, SkSVGLengthContext::LengthType::kVertical)));
}

void SkSVGLine::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                       const SkPaint& paint, SkPath::FillType) const {
    SkPoint p0, p1;
    std::tie(p0, p1) = this->resolve(lctx);

    canvas->drawLine(p0, p1, paint);
}

SkPath SkSVGLine::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPoint p0, p1;
    std::tie(p0, p1) = this->resolve(ctx.lengthContext());

    SkPath path;
    path.moveTo(p0);
    path.lineTo(p1);
    this->mapToParent(&path);

    return path;
}
