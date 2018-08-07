/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRenderContext.h"

#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkTemplates.h"

namespace sksg {

RenderContext::RenderContext(SkCanvas* canvas)
    : fCanvas(canvas)
    , fRestoreCount(fCanvas->getSaveCount()) {}

RenderContext::~RenderContext() {
    fCanvas->restoreToCount(fRestoreCount);
}

RenderContext RenderContext::makeLocal() const {
    RenderContext local(fCanvas);
    local.applyOpacity(fOpacity);
    local.applyColorFilter(fColorFilter);
    return local;
}

void RenderContext::applyOpacity(SkScalar o) {
    fOpacity = SkTPin(fOpacity * o, 0.0f, 1.0f);
}

void RenderContext::applyColorFilter(sk_sp<SkColorFilter> cf) {
    fColorFilter = SkColorFilter::MakeComposeFilter(std::move(fColorFilter), std::move(cf));
}

bool RenderContext::modulateLayer(const SkRect& bounds) {
    if (fOpacity >= 1 && !fColorFilter) {
        return false;
    }

    SkPaint paint;
    paint.setAlpha(roundf(fOpacity * 255));
    paint.setColorFilter(fColorFilter);
    fCanvas->saveLayer(&bounds, &paint);

    fOpacity = 1;
    fColorFilter = nullptr;

    return true;
}

SkTCopyOnFirstWrite<SkPaint> RenderContext::modulatePaint(const SkPaint& paint) const {
    SkTCopyOnFirstWrite<SkPaint> modulated(paint);

    if (fOpacity < 1 || fColorFilter) {
        modulated.writable()->setAlpha(roundf(modulated->getAlpha() * fOpacity));
        modulated.writable()->setColorFilter(fColorFilter);
    }

    return modulated;
}

} // namespace sksg
