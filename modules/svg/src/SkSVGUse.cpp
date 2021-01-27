/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGUse.h"

#include "include/core/SkCanvas.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGUse::SkSVGUse() : INHERITED(SkSVGTag::kUse) {}

void SkSVGUse::appendChild(sk_sp<SkSVGNode>) {
    SkDebugf("cannot append child nodes to this element.\n");
}

bool SkSVGUse::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", n, v)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", n, v)) ||
           this->setHref(SkSVGAttributeParser::parse<SkSVGIRI>("xlink:href", n, v));
}

bool SkSVGUse::onPrepareToRender(SkSVGRenderContext* ctx) const {
    if (fHref.iri().isEmpty() || !INHERITED::onPrepareToRender(ctx)) {
        return false;
    }

    if (fX.value() || fY.value()) {
        // Restored when the local SkSVGRenderContext leaves scope.
        ctx->saveOnce();
        ctx->canvas()->translate(fX.value(), fY.value());
    }

    // TODO: width/height override for <svg> targets.

    return true;
}

void SkSVGUse::onRender(const SkSVGRenderContext& ctx) const {
    const auto ref = ctx.findNodeById(fHref);
    if (!ref) {
        return;
    }

    ref->render(ctx);
}

SkPath SkSVGUse::onAsPath(const SkSVGRenderContext& ctx) const {
    const auto ref = ctx.findNodeById(fHref);
    if (!ref) {
        return SkPath();
    }

    return ref->asPath(ctx);
}

SkRect SkSVGUse::onObjectBoundingBox(const SkSVGRenderContext& ctx) const {
    const auto ref = ctx.findNodeById(fHref);
    if (!ref) {
        return SkRect::MakeEmpty();
    }

    const SkSVGLengthContext& lctx = ctx.lengthContext();
    const SkScalar x = lctx.resolve(fX, SkSVGLengthContext::LengthType::kHorizontal);
    const SkScalar y = lctx.resolve(fY, SkSVGLengthContext::LengthType::kVertical);

    SkRect bounds = ref->objectBoundingBox(ctx);
    bounds.offset(x, y);

    return bounds;
}
