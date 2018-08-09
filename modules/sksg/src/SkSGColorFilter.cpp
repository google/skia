/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGColorFilter.h"

#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkSGColor.h"

namespace sksg {

ColorFilter::ColorFilter(sk_sp<RenderNode> child)
    : INHERITED(std::move(child)) {}

void ColorFilter::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    if (this->bounds().isEmpty())
        return;

    SkAutoCanvasRestore acr(canvas, false);

    if (fColorFilter) {
        SkPaint p;
        p.setColorFilter(fColorFilter);
        canvas->saveLayer(this->bounds(), &p);
    }

    this->INHERITED::onRender(canvas, ctx);
}

ColorModeFilter::ColorModeFilter(sk_sp<RenderNode> child, sk_sp<Color> color, SkBlendMode mode)
    : INHERITED(std::move(child))
    , fColor(std::move(color))
    , fMode(mode) {
    this->observeInval(fColor);
}

ColorModeFilter::~ColorModeFilter() {
    this->unobserveInval(fColor);
}

SkRect ColorModeFilter::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    fColor->revalidate(ic, ctm);
    fColorFilter = SkColorFilter::MakeModeFilter(fColor->getColor(), fMode);

    return this->INHERITED::onRevalidate(ic, ctm);
}

} // namespace sksg
