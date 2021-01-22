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

void SkSVGUse::setHref(const SkSVGStringType& href) {
    fHref = href;
}

void SkSVGUse::setX(const SkSVGLength& x) {
    fX = x;
}

void SkSVGUse::setY(const SkSVGLength& y) {
    fY = y;
}

void SkSVGUse::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kHref:
        if (const auto* href = v.as<SkSVGStringValue>()) {
            this->setHref(*href);
        }
        break;
    case SkSVGAttribute::kX:
        if (const auto* x = v.as<SkSVGLengthValue>()) {
            this->setX(*x);
        }
        break;
    case SkSVGAttribute::kY:
        if (const auto* y = v.as<SkSVGLengthValue>()) {
            this->setY(*y);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

bool SkSVGUse::onPrepareToRender(SkSVGRenderContext* ctx) const {
    if (fHref.isEmpty() || !INHERITED::onPrepareToRender(ctx)) {
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

    const SkRect bounds = ref->objectBoundingBox(ctx);
    const SkScalar x = ctx.lengthContext().resolve(fX, SkSVGLengthContext::LengthType::kHorizontal);
    const SkScalar y = ctx.lengthContext().resolve(fY, SkSVGLengthContext::LengthType::kVertical);
    return SkRect::MakeXYWH(x, y, bounds.width(), bounds.height());
}
