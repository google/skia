/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkSVGPath.h"
#include "SkSVGRenderContext.h"
#include "SkSVGValue.h"

SkSVGPath::SkSVGPath() : INHERITED(SkSVGTag::path) { }

void SkSVGPath::doRender(SkCanvas* canvas, const SkPaint* paint) const {
    if (paint) {
        canvas->drawPath(fPath, *paint);
    }
}

void SkSVGPath::onRender(SkCanvas* canvas, const SkSVGRenderContext& ctx) const {
    this->doRender(canvas, ctx.fillPaint());
    this->doRender(canvas, ctx.strokePaint());
}

void SkSVGPath::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::d:
        if (const auto* path = v.as<SkSVGPathValue>()) {
            this->setPath(*path);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}
