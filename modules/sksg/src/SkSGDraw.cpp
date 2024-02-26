/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPoint.h"
#include "include/private/base/SkAssert.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGeometryNode.h"
#include "modules/sksg/include/SkSGNode.h"
#include "modules/sksg/include/SkSGPaint.h"

class SkMatrix;

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
    if (!skpathutils::FillPathWithPaint(fGeometry->asPath(), paint, &stroke_path)) {
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
