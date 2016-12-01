/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkTLazy.h"
#include "SkSVGRenderContext.h"
#include "SkSVGPoly.h"
#include "SkSVGValue.h"

SkSVGPoly::SkSVGPoly(SkSVGTag t) : INHERITED(t) {}

void SkSVGPoly::setPoints(const SkSVGPointsType& pts) {
    fPath.reset();
    fPath.addPoly(pts.value().begin(),
                  pts.value().count(),
                  this->tag() == SkSVGTag::kPolygon); // only polygons are auto-closed
}

void SkSVGPoly::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
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
    case SkSVGAttribute::kPoints:
        if (const auto* pts = v.as<SkSVGPointsValue>()) {
            this->setPoints(*pts);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

void SkSVGPoly::onDraw(SkCanvas* canvas, const SkSVGLengthContext&, const SkPaint& paint,
                       SkPath::FillType fillType) const {
    SkTCopyOnFirstWrite<SkPath> path(fPath);
    if (fillType != path->getFillType()) {
        path.writable()->setFillType(fillType);
    }

    canvas->drawPath(*path, paint);
}
