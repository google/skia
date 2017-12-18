/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGDraw.h"

#include "SkSGGeometryNode.h"
#include "SkSGPaintNode.h"

namespace sksg {

Draw::Draw(sk_sp<GeometryNode> geometry, sk_sp<PaintNode> paint)
    : fGeometry(std::move(geometry))
    , fPaint(std::move(paint)) {
    fGeometry->addParent(this);
    fPaint->addParent(this);
}

Draw::~Draw() {
    fGeometry->removeParent(this);
    fPaint->removeParent(this);
}

void Draw::onRender(SkCanvas* canvas) const {
    fGeometry->draw(canvas, fPaint->makePaint());
}

void Draw::onRevalidate(InvalidationController* ic) {
    SkASSERT(this->isInvalidated());

    fGeometry->revalidate(ic);
    fPaint->revalidate(ic);
}

} // namespace sksg
