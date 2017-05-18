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

SkSVGPath::SkSVGPath() : INHERITED(SkSVGTag::kPath) { }

void SkSVGPath::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kD:
        if (const auto* path = v.as<SkSVGPathValue>()) {
            this->setPath(*path);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

void SkSVGPath::onDraw(SkCanvas* canvas, const SkSVGLengthContext&, const SkPaint& paint,
                       SkPath::FillType fillType) const {
    // the passed fillType follows inheritance rules and needs to be applied at draw time.
    fPath.setFillType(fillType);
    canvas->drawPath(fPath, paint);
}

SkPath SkSVGPath::onAsPath(const SkSVGRenderContext& ctx) const {
    // the computed fillType follows inheritance rules and needs to be applied at draw time.
    fPath.setFillType(FillRuleToFillType(*ctx.presentationContext().fInherited.fFillRule.get()));

    SkPath path = fPath;
    this->mapToParent(&path);
    return path;
}
