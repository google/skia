/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGRenderContext.h"
#include "SkSVGTypes.h"

namespace {

SkScalar length_size_for_type(const SkSize& viewport, SkSVGLengthContext::LengthType t) {
    switch (t) {
    case SkSVGLengthContext::LengthType::kHorizontal:
        return viewport.width();
    case SkSVGLengthContext::LengthType::kVertical:
        return viewport.height();
    case SkSVGLengthContext::LengthType::kOther:
        return SkScalarSqrt(viewport.width() * viewport.height());
    }

    SkASSERT(false);  // Not reached.
    return 0;
}

} // anonymous ns

SkScalar SkSVGLengthContext::resolve(const SkSVGLength& l, LengthType t) const {
    switch (l.unit()) {
    case SkSVGLength::Unit::kNumber:
        return l.value();
        break;
    case SkSVGLength::Unit::kPercentage:
        return l.value() * length_size_for_type(fViewport, t) / 100;
        break;
    default:
        SkDebugf("unsupported unit type: <%d>\n", l.unit());
        break;
    }

    return 0;
}

SkSVGRenderContext::SkSVGRenderContext(const SkSize& initialViewport)
    : fLengthContext(initialViewport) {}

SkSVGRenderContext& SkSVGRenderContext::operator=(const SkSVGRenderContext& other) {
    if (other.fFill.isValid()) {
        fFill.set(*other.fFill.get());
    } else {
        fFill.reset();
    }

    if (other.fStroke.isValid()) {
        fStroke.set(*other.fStroke.get());
    } else {
        fStroke.reset();
    }

    return *this;
}

SkPaint& SkSVGRenderContext::ensureFill() {
    if (!fFill.isValid()) {
        fFill.init();
        fFill.get()->setStyle(SkPaint::kFill_Style);
        fFill.get()->setAntiAlias(true);
    }
    return *fFill.get();
}

SkPaint& SkSVGRenderContext::ensureStroke() {
    if (!fStroke.isValid()) {
        fStroke.init();
        fStroke.get()->setStyle(SkPaint::kStroke_Style);
        fStroke.get()->setAntiAlias(true);
    }
    return *fStroke.get();
}

void SkSVGRenderContext::setFillColor(SkColor color) {
    this->ensureFill().setColor(color);
}

void SkSVGRenderContext::setStrokeColor(SkColor color) {
    this->ensureStroke().setColor(color);
}
