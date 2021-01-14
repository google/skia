/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGClipPath.h"

#include "modules/svg/include/SkSVGRenderContext.h"

SkSVGClipPath::SkSVGClipPath() : INHERITED(SkSVGTag::kClipPath) {}

bool SkSVGClipPath::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setClipPathUnits(
                SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>("clipPathUnits", n, v));
}

SkPath SkSVGClipPath::resolveClip(const SkSVGRenderContext& ctx) const {
    auto clip = this->asPath(ctx);

    if (fClipPathUnits.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        const auto obb = ctx.node()->objectBoundingBox(ctx);
        const auto obb_transform = SkMatrix::Translate(obb.x(), obb.y()) *
                                   SkMatrix::Scale(obb.width(), obb.height());
        clip.transform(obb_transform);
    }

    return clip;
}
