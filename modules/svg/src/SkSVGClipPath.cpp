/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGClipPath.h"

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGRenderContext.h"

SkSVGClipPath::SkSVGClipPath() : INHERITED(SkSVGTag::kClipPath) {}

bool SkSVGClipPath::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setClipPathUnits(
                SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>("clipPathUnits", n, v));
}

SkPath SkSVGClipPath::resolveClip(const SkSVGRenderContext& ctx) const {
    auto clip = this->asPath(ctx);

    const auto obbt = ctx.transformForCurrentOBB(fClipPathUnits);
    const auto m = SkMatrix::Translate(obbt.offset.x, obbt.offset.y)
                 * SkMatrix::Scale(obbt.scale.x, obbt.scale.y);
    clip.transform(m);

    return clip;
}
