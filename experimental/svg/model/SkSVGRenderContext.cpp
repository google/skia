/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGRenderContext.h"

SkSVGRenderContext::SkSVGRenderContext() { }

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
