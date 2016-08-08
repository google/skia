/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
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

SkRect SkSVGLengthContext::resolveRect(const SkSVGLength& x, const SkSVGLength& y,
                                       const SkSVGLength& w, const SkSVGLength& h) const {
    return SkRect::MakeXYWH(
        this->resolve(x, SkSVGLengthContext::LengthType::kHorizontal),
        this->resolve(y, SkSVGLengthContext::LengthType::kVertical),
        this->resolve(w, SkSVGLengthContext::LengthType::kHorizontal),
        this->resolve(h, SkSVGLengthContext::LengthType::kVertical));
}

SkSVGPresentationContext::SkSVGPresentationContext() {}

SkSVGPresentationContext::SkSVGPresentationContext(const SkSVGPresentationContext& o) {
    this->initFrom(o);
}

SkSVGPresentationContext& SkSVGPresentationContext::operator=(const SkSVGPresentationContext& o) {
    this->initFrom(o);
    return *this;
}

void SkSVGPresentationContext::initFrom(const SkSVGPresentationContext& other) {
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
}

SkPaint& SkSVGPresentationContext::ensureFill() {
    if (!fFill.isValid()) {
        fFill.init();
        fFill.get()->setStyle(SkPaint::kFill_Style);
        fFill.get()->setAntiAlias(true);
    }
    return *fFill.get();
}

SkPaint& SkSVGPresentationContext::ensureStroke() {
    if (!fStroke.isValid()) {
        fStroke.init();
        fStroke.get()->setStyle(SkPaint::kStroke_Style);
        fStroke.get()->setAntiAlias(true);
    }
    return *fStroke.get();
}

void SkSVGPresentationContext::setFillColor(SkColor color) {
    this->ensureFill().setColor(color);
}

void SkSVGPresentationContext::setStrokeColor(SkColor color) {
    this->ensureStroke().setColor(color);
}

SkSVGRenderContext::SkSVGRenderContext(SkCanvas* canvas,
                                       const SkSVGLengthContext& lctx,
                                       const SkSVGPresentationContext& pctx)
    : fLengthContext(lctx)
    , fPresentationContext(pctx)
    , fCanvas(canvas)
    , fCanvasSaveCount(canvas->getSaveCount()) {}

SkSVGRenderContext::SkSVGRenderContext(const SkSVGRenderContext& other)
    : SkSVGRenderContext(other.canvas(),
                         other.lengthContext(),
                         other.presentationContext()) {}

SkSVGRenderContext::~SkSVGRenderContext() {
    fCanvas->restoreToCount(fCanvasSaveCount);
}
