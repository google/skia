/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGCircle.h"
#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"

SkSVGCircle::SkSVGCircle() : INHERITED(SkSVGTag::kCircle) {}

void SkSVGCircle::setCx(const SkSVGLength& cx) {
    fCx = cx;
}

void SkSVGCircle::setCy(const SkSVGLength& cy) {
    fCy = cy;
}

void SkSVGCircle::setR(const SkSVGLength& r) {
    fR = r;
}

void SkSVGCircle::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kCx:
        if (const auto* cx = v.as<SkSVGLengthValue>()) {
            this->setCx(*cx);
        }
        break;
    case SkSVGAttribute::kCy:
        if (const auto* cy = v.as<SkSVGLengthValue>()) {
            this->setCy(*cy);
        }
        break;
    case SkSVGAttribute::kR:
        if (const auto* r = v.as<SkSVGLengthValue>()) {
            this->setR(*r);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

std::tuple<SkPoint, SkScalar> SkSVGCircle::resolve(const SkSVGLengthContext& lctx) const {
    const auto cx = lctx.resolve(fCx, SkSVGLengthContext::LengthType::kHorizontal);
    const auto cy = lctx.resolve(fCy, SkSVGLengthContext::LengthType::kVertical);
    const auto  r = lctx.resolve(fR , SkSVGLengthContext::LengthType::kOther);

    return std::make_tuple(SkPoint::Make(cx, cy), r);
}
void SkSVGCircle::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                         const SkPaint& paint, SkPathFillType) const {
    SkPoint pos;
    SkScalar r;
    std::tie(pos, r) = this->resolve(lctx);

    if (r > 0) {
        canvas->drawCircle(pos.x(), pos.y(), r, paint);
    }
}

SkPath SkSVGCircle::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPoint pos;
    SkScalar r;
    std::tie(pos, r) = this->resolve(ctx.lengthContext());

    SkPath path;
    path.addCircle(pos.x(), pos.y(), r);
    this->mapToParent(&path);

    return path;
}
