/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGPath.h"
#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

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
                       SkPathFillType fillType) const {
    // the passed fillType follows inheritance rules and needs to be applied at draw time.
    fPath.setFillType(fillType);
    canvas->drawPath(fPath, paint);
}

SkPath SkSVGPath::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPath path = fPath;
    // clip-rule can be inherited and needs to be applied at clip time.
    path.setFillType(ctx.presentationContext().fInherited.fClipRule.get()->asFillType());
    this->mapToParent(&path);
    return path;
}
