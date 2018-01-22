/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGDraw.h"

#include "SkSGGeometryNode.h"
#include "SkSGInvalidationController.h"
#include "SkSGPaintNode.h"

namespace sksg {

Draw::Draw(sk_sp<GeometryNode> geometry, sk_sp<PaintNode> paint)
    : fGeometry(std::move(geometry))
    , fPaint(std::move(paint)) {
    this->observeInval(fGeometry);
    this->observeInval(fPaint);
}

Draw::~Draw() {
    this->unobserveInval(fGeometry);
    this->unobserveInval(fPaint);
}

void Draw::onRender(SkCanvas* canvas) const {
    const auto& paint   = fPaint->makePaint();
    const auto skipDraw = paint.nothingToDraw() ||
            (paint.getStyle() == SkPaint::kStroke_Style && paint.getStrokeWidth() <= 0);

    if (!skipDraw) {
        fGeometry->draw(canvas, paint);
    }
}

SkRect Draw::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    auto bounds = fGeometry->revalidate(ic, ctm);
    fPaint->revalidate(ic, ctm);

    const auto& paint = fPaint->makePaint();
    SkASSERT(paint.canComputeFastBounds());

    return paint.computeFastBounds(bounds, &bounds);
}

} // namespace sksg
