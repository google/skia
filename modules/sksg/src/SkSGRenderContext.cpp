/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRenderContext.h"

#include "SkCanvas.h"
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
    return local;
}

void RenderContext::applyOpacity(SkScalar o) {
    fOpacity = SkTPin(fOpacity * o, 0.0f, 1.0f);
}

bool RenderContext::modulateLayer(const SkRect& bounds) {
    if (fOpacity >= 1) {
        return false;
    }

    fCanvas->saveLayerAlpha(&bounds, roundf(fOpacity * 255));

    fOpacity = 1;

    return true;
}

SkTCopyOnFirstWrite<SkPaint> RenderContext::modulatePaint(const SkPaint& paint) const {
    SkTCopyOnFirstWrite<SkPaint> modulated(paint);

    if (fOpacity < 1) {
        modulated.writable()->setAlpha(roundf(modulated->getAlpha() * fOpacity));
    }

    return modulated;
}

} // namespace sksg
