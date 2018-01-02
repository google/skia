/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRect.h"

#include "SkCanvas.h"
#include "SkPaint.h"

namespace sksg {

Rect::Rect(const SkRect& rect) : fRect(rect) {}

void Rect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawRect(fRect, paint);
}

SkRect Rect::onComputeBounds() const {
    return fRect;
}

RRect::RRect(const SkRRect& rr) : fRRect(rr) {}

void RRect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawRRect(fRRect, paint);
}

SkRect RRect::onComputeBounds() const {
    return fRRect.getBounds();
}

} // namespace sksg
