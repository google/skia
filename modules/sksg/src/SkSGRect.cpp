/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGRect.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

namespace sksg {

Rect::Rect(const SkRect& rect) : fRect(rect) {}

void Rect::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipRect(fRect, SkClipOp::kIntersect, antiAlias);
}

void Rect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawRect(fRect, paint);
}

bool Rect::onContains(const SkPoint& p) const {
    return fRect.contains(p.x(), p.y());
}

SkRect Rect::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasInval());

    return fRect;
}

SkPath Rect::onAsPath() const {
    SkPath path;
    path.addRect(fRect, this->getDirection(), this->getInitialPointIndex());
    return path;
}

RRect::RRect(const SkRRect& rr) : fRRect(rr) {}

void RRect::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipRRect(fRRect, SkClipOp::kIntersect, antiAlias);
}

void RRect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawRRect(fRRect, paint);
}

bool RRect::onContains(const SkPoint& p) const {
    if (!fRRect.rect().contains(p.x(), p.y())) {
        return false;
    }

    if (fRRect.isRect()) {
        return true;
    }

    // TODO: no SkRRect::contains(x, y)
    return fRRect.contains(SkRect::MakeLTRB(p.x() - SK_ScalarNearlyZero,
                                            p.y() - SK_ScalarNearlyZero,
                                            p.x() + SK_ScalarNearlyZero,
                                            p.y() + SK_ScalarNearlyZero));
}

SkRect RRect::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasInval());

    return fRRect.getBounds();
}

SkPath RRect::onAsPath() const {
    SkPath path;
    path.addRRect(fRRect, this->getDirection(), this->getInitialPointIndex());
    return path;
}

} // namespace sksg
