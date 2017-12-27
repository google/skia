/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGStroke.h"

namespace sksg {

Stroke::Stroke(sk_sp<PaintNode> paint)
    : fPaint(std::move(paint)) {
    fPaint->addInvalReceiver(this);
}

Stroke::~Stroke() {
    fPaint->removeInvalReceiver(this);
}

void Stroke::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    fPaint->revalidate(ic, ctm);
    INHERITED::onRevalidate(ic, ctm);
}

SkPaint Stroke::onMakePaint() const {
    SkPaint paint = fPaint->makePaint();

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(fStrokeWidth);
    paint.setStrokeMiter(fStrokeMiter);
    paint.setStrokeJoin(fStrokeJoin);
    paint.setStrokeCap(fStrokeCap);

    return paint;
}

} // namespace sksg
