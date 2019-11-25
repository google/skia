/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGPoly.h"
#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"
#include "src/core/SkTLazy.h"

SkSVGPoly::SkSVGPoly(SkSVGTag t) : INHERITED(t) {}

void SkSVGPoly::setPoints(const SkSVGPointsType& pts) {
    fPath.reset();
    fPath.addPoly(pts.value().begin(),
                  pts.value().count(),
                  this->tag() == SkSVGTag::kPolygon); // only polygons are auto-closed
}

void SkSVGPoly::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
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
                       SkPathFillType fillType) const {
    // the passed fillType follows inheritance rules and needs to be applied at draw time.
    fPath.setFillType(fillType);
    canvas->drawPath(fPath, paint);
}

SkPath SkSVGPoly::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPath path = fPath;

    // clip-rule can be inherited and needs to be applied at clip time.
    path.setFillType(ctx.presentationContext().fInherited.fClipRule.get()->asFillType());

    this->mapToParent(&path);
    return path;
}
