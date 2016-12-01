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
    case SkSVGAttribute::kFillRule:
        // The fill rule is logically part of the presentation attributes (for
        // inheritance semantics) => we need to support overriding the path fill type at
        // render time.  BUT, in order to avoid unnecessary/temp path copies, we can detect
        // the case where the fill rule is applied directly to the path element (most common
        // case), as opposed to inherited, and update the local path upfront.
        this->INHERITED::onSetAttribute(attr, v);
        if (const auto* fillRule = v.as<SkSVGFillRuleValue>()) {
            fPath.setFillType(FillRuleToFillType(*fillRule));
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

void SkSVGPath::onDraw(SkCanvas* canvas, const SkSVGLengthContext&, const SkPaint& paint,
                       SkPath::FillType fillType) const {
    SkTCopyOnFirstWrite<SkPath> path(fPath);
    if (fillType != path->getFillType()) {
        path.writable()->setFillType(fillType);
    }

    canvas->drawPath(*path, paint);
}
