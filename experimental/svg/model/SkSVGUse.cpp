/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGUse.h"

#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"

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
    if (fHref.value().isEmpty() || !INHERITED::onPrepareToRender(ctx)) {
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
    const auto* ref = ctx.findNodeById(fHref);
    if (!ref) {
        return;
    }

    ref->render(ctx);
}

SkPath SkSVGUse::onAsPath(const SkSVGRenderContext& ctx) const {
    const auto* ref = ctx.findNodeById(fHref);
    if (!ref) {
        return SkPath();
    }

    return ref->asPath(ctx);
}
