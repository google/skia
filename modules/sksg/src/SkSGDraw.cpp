/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGDraw.h"

#include "SkCanvas.h"
#include "SkPath.h"
#include "SkSGGeometryNode.h"
#include "SkSGInvalidationController.h"
#include "SkSGPaint.h"
#include "SkTLazy.h"

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

void Draw::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    auto paint = fPaint->makePaint();
    if (ctx) {
        ctx->modulatePaint(canvas->getTotalMatrix(), &paint);
    }

    const auto skipDraw = paint.nothingToDraw() ||
            (paint.getStyle() == SkPaint::kStroke_Style && paint.getStrokeWidth() <= 0);

    if (!skipDraw) {
        fGeometry->draw(canvas, paint);
    }
}

const RenderNode* Draw::onNodeAt(const SkPoint& p) const {
    const auto paint = fPaint->makePaint();

    if (!paint.getAlpha()) {
        return nullptr;
    }

    if (paint.getStyle() == SkPaint::Style::kFill_Style && fGeometry->contains(p)) {
        return this;
    }

    SkPath stroke_path;
    if (!paint.getFillPath(fGeometry->asPath(), &stroke_path)) {
        return nullptr;
    }

    return stroke_path.contains(p.x(), p.y()) ? this : nullptr;
}

SkRect Draw::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    auto bounds = fGeometry->revalidate(ic, ctm);
    fPaint->revalidate(ic, ctm);

    const auto paint = fPaint->makePaint();
    SkASSERT(paint.canComputeFastBounds());

    return paint.computeFastBounds(bounds, &bounds);
}

} // namespace sksg
