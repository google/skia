/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGPoly.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPoint.h"
#include "modules/svg/include/SkSVGAttribute.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGRenderContext.h"

class SkPaint;
enum class SkPathFillType;

SkSVGPoly::SkSVGPoly(SkSVGTag t) : INHERITED(t) {}

bool SkSVGPoly::parseAndSetAttribute(const char* n, const char* v) {
    if (INHERITED::parseAndSetAttribute(n, v)) {
        return true;
    }

    if (this->setPoints(SkSVGAttributeParser::parse<SkSVGPointsType>("points", n, v))) {
        // TODO: we can likely just keep the points array and create the SkPath when needed.
        // only polygons are auto-closed
        fPath = SkPath::Polygon(fPoints, this->tag() == SkSVGTag::kPolygon);
    }

    // No other attributes on this node
    return false;
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
    path.setFillType(ctx.presentationContext().fInherited.fClipRule->asFillType());

    this->mapToParent(&path);
    return path;
}

SkRect SkSVGPoly::onTransformableObjectBoundingBox(const SkSVGRenderContext& ctx) const {
    return fPath.getBounds();
}
